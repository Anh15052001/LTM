#define FD_SETSIZE 64

#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <process.h>
#include <fstream>
#include <map>
#include <string>
#include <algorithm>
#include "SessionManagement.h"
#include "StreamTransmission.h"
#include "CodeStatus.h"
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0);

#pragma comment (lib, "Ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define MAX_CLIENT 4096
#define BUFF_SIZE 2048

using namespace std;

string file = "account.txt";

//Declare condition variable
CONDITION_VARIABLE copy_completed;

//Declare critical section
CRITICAL_SECTION critical_section;

//Declare a bool data type named completed
bool completed;

//Declare a vector containing a list of threads
vector<HANDLE> thread_list;

//declare a vector containing a list of sessions
vector<Session> session_list;

//list of accounts in txt file
map< string, string > account;

//Prototype function declaration
unsigned _stdcall workerThread(void *);
void splitString(string, string&, string&);
bool checkAccess(Session &, char *, char *);
int handleMKDIR(Session &, char *);
int handleRMDIR(Session &, char *);
int handleCWDIR(Session &, char *);
int handleDELETE(Session &, char *);
int handleMOV(Session &, char *, char *);
int handleSHOW(Session &, char *);
int handleREGISTER(Session &, string, string);
int handleLOGIN(Session &, string, string);
auto findUser(string&);
int handleLOGOUT(Session &);
int handleMESSAGE(SOCKET, Session&, string);
int handlePWDIR(Session &);
typedef struct FILEOBJ {
	//file connection
	SOCKET fileSock;
	//file handle
	HANDLE file;

	int operation;
	enum OP {
		//retrieve file
		RETR,
		//store file
		STOR
	};

	LONG64 size;

	//for overlapped operation
	volatile LONG64 bytesSended;
	volatile LONG64 bytesRecved;
	volatile LONG64 bytesWritten;

} FILEOBJ, *LPFILEOBJ;
_Ret_maybenull_ LPFILEOBJ GetFileObj(_In_ HANDLE hfile, _In_ LONG64 size, _In_ FILEOBJ::OP op) {
	LPFILEOBJ newobj = NULL;

	if ((newobj = (LPFILEOBJ)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FILEOBJ))) == NULL)
		printf("HeapAlloc() failed with error %d\n", GetLastError());

	if (newobj) {
		newobj->file = hfile;
		newobj->size = size;
		newobj->operation = op;
	}

	return newobj;
}
//Run the program

SOCKET listenSock;
int main(int argc, char* argv[]) {

	// Validate the parameters
	if (argc != 2) {
		printf_s("Usage: %s <PortNumber>\n", argv[0]);
		return 1;
	}

	//Initiate Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf_s("Version is not supported\n");
		return 0;
	}

	//Construct socket
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		printf_s("Error %d: Cannot create server socket.", WSAGetLastError());
		return 0;
	}

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	int server_port = atoi(argv[1]);
	serverAddr.sin_port = htons(server_port);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf_s("Bind failed with ERROR = %d\n.", WSAGetLastError());
		return 0;
	}

	//Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		printf_s("(Error: %d)Cannot place server socket in state LISTEN.\n", WSAGetLastError());
		return 0;
	}
	printf_s("Server started!\n");

	//Initialize condition variable
	InitializeConditionVariable(&copy_completed);

	//Initialize critical section
	InitializeCriticalSection(&critical_section);

	SOCKET connSock;
	sockaddr_in clientAddr;
	int ret, nEvents, clientAddrLen, clientPort;
	char clientIP[INET_ADDRSTRLEN];
	vector<string> requestList;
	char buff[BUFF_SIZE];
	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step

							 //Initialize the fd_set with the value NULL
	FD_ZERO(&initfds);

	//Add socket to fd_set
	FD_SET(listenSock, &initfds);

	//Communicate with client
	while (1) {
		readfds = initfds;		/* structure assignment */
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf_s("\nError %d! Cannot poll sockets.\n", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(listenSock, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf_s("\nError %d! Cannot accept new connection.\n", WSAGetLastError());
				break;
			}
			else {
				inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
				clientPort = ntohs(clientAddr.sin_port);
				printf_s("You got a connection from %s:%d\n", clientIP, clientPort);

				//Add session to session list 
				Session session(connSock, clientIP, clientPort);
				session_list.push_back(session);

				//Add socket to initfds
				FD_SET(session_list.back().getSocket(), &initfds);

				//If number of clients == FD_SETSIZE, move session_list for workerThread,
				if (session_list.size() == FD_SETSIZE) {
					thread_list.push_back((HANDLE)_beginthreadex(0, 0, workerThread, (void*)&session_list, 0, 0));

					EnterCriticalSection(&critical_section);

					while (completed == false) {
						//Wait for the workerThread to copy the session_list with a timeout of INFINITE
						SleepConditionVariableCS(&copy_completed, &critical_section, INFINITE);
					}

					//Empty the sessionlist
					session_list.clear();
					completed = false;

					LeaveCriticalSection(&critical_section);

					FD_ZERO(&initfds); //Clear initfds
					FD_SET(listenSock, &initfds); //Add socket to fd_set and continue listen for new connection
					continue;
				}
				if (--nEvents == 0)
					continue; //no more event
			}
		}

		//receive data from clients
		vector<Session>::iterator itr;   //declare an iterator
										 //Assign itr to the start of the vector until it has not reached the end
		for (itr = session_list.begin(); itr != session_list.end();) {
			if (nEvents > 0) {
				SOCKET socket = itr->getSocket();
				char *clientIP = itr->getClientIp();
				int clientPort = itr->getClientPort();
				if (FD_ISSET(socket, &readfds)) {
					--nEvents;
					//Recieve request
					ret = recv_stream(socket, requestList);
					if (ret <= 0) {
						if (ret == 0)
							printf_s("Client disconnect.\n");
						else
							printf_s("Error %d: Cannot recieve data.\n", WSAGetLastError());

						FD_CLR(socket, &initfds); //Remove socket from fd_set
						closesocket(socket);
						itr = session_list.erase(itr);//Remove session from session_list
						continue;
					}
					else if (ret > 0) {
						for (string request : requestList) {
							printf_s("Receive from client[%s:%d] %s\n", clientIP, clientPort, request.c_str());
							//Handle
							int echo = handleMESSAGE(socket ,*itr, request);
							sprintf_s(buff, BUFF_SIZE, "%d", echo);

							//Send result
							ret = send_stream(socket, buff);
							if (ret == SOCKET_ERROR) {
								printf_s("Error %d: Cannot send data.\n", WSAGetLastError());
								break;
							}
							
						}
					}
				}//End if
			}
			++itr; //And iterate to the next element
		} //End for
	} //End While

	closesocket(listenSock);

	//Wait for all workerTheard done and returned
	WaitForMultipleObjects(thread_list.size(), thread_list.data(), TRUE, INFINITE);

	//Terminate Winsock
	WSACleanup();
	return 0;
}

/**
*@workerThread : Called every time the number of clients in mainThread exceeds FD_SETSIZE
**/
unsigned _stdcall workerThread(void *param) {
	EnterCriticalSection(&critical_section);
	vector<Session> session_list = *(vector<Session> *)param; //copy session_list from mainThread to workerThread
	completed = true;
	LeaveCriticalSection(&critical_section);
	WakeConditionVariable(&copy_completed);	//Wake mainTheard to continue the mission

	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step
	int nEvents, ret;
	vector<string> requestList;
	char buff[BUFF_SIZE];

	FD_ZERO(&initfds); //Initialize the fd_set with the value NULL
					   //Add socket to fd_set
	for (Session &session : session_list) {
		FD_SET(session.getSocket(), &initfds);
	}

	while (1) {
		readfds = initfds;
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf_s("Thread %d: Error! Cannot poll sockets: %d\n", GetCurrentThreadId(), WSAGetLastError());
			break;
		}

		//receive data from clients
		vector<Session>::iterator itr;  //declare an iterator
										//Assign itr to the start of the vector until it has not reached the end
		for (itr = session_list.begin(); itr != session_list.end();) {
			if (nEvents > 0) {
				SOCKET sock = itr->getSocket();
				char * clientIP = itr->getClientIp();
				int clientPort = itr->getClientPort();
				if (FD_ISSET(sock, &readfds)) {
					nEvents--;
					//Recieve request
					ret = recv_stream(sock, requestList);
					if (ret <= 0) {
						if (ret == 0)
							printf_s("Client disconnect\n");
						else
							printf_s("Error %d: Cannot recieve data.\n", WSAGetLastError());

						FD_CLR(sock, &initfds); //Remove socket from fd_set
						closesocket(sock);
						itr = session_list.erase(itr);//Remove session from session_list
						continue;
					}
					else {
						for (string request : requestList) {
							printf_s("Receive from client[%s:%d] %s\n", clientIP, clientPort, request.c_str());
							//Handle
							int echo = handleMESSAGE(sock, *itr, request);
							sprintf_s(buff, BUFF_SIZE, "%d", echo);

							//Send result
							ret = send_stream(sock, buff);
							if (ret == SOCKET_ERROR) {
								printf_s("Error %d: Cannot send data.\n", WSAGetLastError());
								break;
							}
						}
					}
				}//End if
			}
			itr++;  //And iterate to the next element
		}//End for

		 //If all socket disconnect, return
		if (session_list.empty())
			break;
	}//End while
	return 0;
}

/**
*@Function findUser : Find location by username in session list
*@Param [in] username : user account
**/
auto findUser(string& username) {
	EnterCriticalSection(&critical_section);
	auto pos = find_if(session_list.begin(), session_list.end(), [username](Session session) {
		return username == session.getUserName();
	});
	LeaveCriticalSection(&critical_section);
	return pos;
}

/**
*@Function splitString : If the input string is disconnexion by delimeter " "
,then split the input string into 2 strings s1 and s2.
*@Param1 [in] input : input string to split
*@Param2 [out] s1 : string s1
*@Param3 [out] s2 : string s2
*@Example : input (tungbt 0) --> s1 (tungbt) , s2(0)
**/
void splitString(string input, string& s1, string& s2) {
	size_t pos = input.find(" ");
	if (pos != string::npos) {
		s1 = input.substr(0, pos);
		s2 = input.substr(pos + 1);
	}
	else s1 = input;
}
bool CheckFileExisting(char* file_name)
{
	//return true: file exist
	//return false: otherwise
	bool is_exist = true;
	fstream data_file;
	data_file.open(file_name, ios::in);
	bool ret = data_file.fail();
	if (ret == true)
	{
		is_exist = false;
	}
	data_file.close();

	return is_exist;
}

bool checkAccess(Session &session, char *path, char *fullPath) {
	char rootPath[260];
	char temp[260];

	if (strstr(path, "~") == path)
		sprintf_s(temp, 260, "%s%s", session.getUserName(), strlen(path) > 1 ? path + 1 : "");
	else
		sprintf_s(temp, 260, "%s%s%s", session.getWorkingDir(), "\\", path);

	DWORD rootLength = GetFullPathNameA(session.getUserName(), 260, rootPath, NULL);
	DWORD pathLength = GetFullPathNameA(temp, 260, fullPath, NULL);

	if (rootLength != 0 && pathLength != 0 && strstr(fullPath, rootPath) != rootPath) {
		return TRUE;
	}
	//path invalid or dont start root path
	strcpy_s(fullPath, 260, "");
	return FALSE;
}
int Upload(Session &session, char*filename)
{
	char fullpath[260];
	//check if logged in 
	if (session.loginStatus() == 0)
	{
		return NOT_LOGIN;
	}
	//check validity of directory entered
	if (!checkAccess(session, filename, fullpath))
	{
		return NO_ACCESS;
	}
	if (CheckFileExisting(filename))
	{
		return FILE_ALREADY_EXISTS;
	}
	
	return CREATE_REMOTE_FILE;

}
int DownLoad(Session &session, char*filename)
{
	char fullpath[260];
	//check if logged in 
	if (session.loginStatus() == 0)
	{
		return NOT_LOGIN;
	}
	//check validity of directory entered
	if (!checkAccess(session, filename, fullpath))
	{
		return NO_ACCESS;
	}
	if (!CheckFileExisting(filename))
	{
		return FILE_DOES_NOT_EXISTS;
	}
	return ALREADY_DOWNLOAD;
}

int handleMKDIR(Session &session, char *path) {
	char fullPath[260];
	//check if logged in
	if (session.loginStatus() == 0) {
		return NOT_LOGIN;
	}
	//Check validity of directory entered
	if (!checkAccess(session, path, fullPath)) {
		return NO_ACCESS;
	}

	if (CreateDirectoryA(fullPath, NULL)) {
		return SUCCESS_MAKE_DIRECTORY;
	}
	else {
		DWORD error = GetLastError();
		printf_s("Create directory failed with error %d\n", error);
		if (error == ERROR_PATH_NOT_FOUND)
			return PATH_NOT_FOUND;
		else if (error == ERROR_ALREADY_EXISTS)
			return ALREADY_EXISTS;
		else if (error == ERROR_INVALID_NAME)
			return INVALID_NAME;
		else
			return SERVER_FAIL;
	}
}

int handleRMDIR(Session &session, char *path) {
	char fullPath[260];
	char rootPath[260];
	//Check if logged in
	if (session.loginStatus() == 0) {
		return NOT_LOGIN;
	}
	//Check validity of directory entered
	if (!checkAccess(session, path, fullPath) || !checkAccess(session, "~", rootPath)) {
		return NO_ACCESS;
	}
	if (strcmp(rootPath, fullPath) == 0) {
		return CANT_REMOVE;
	}
	//Remove directory
	EnterCriticalSection(&critical_section);
	if (RemoveDirectoryA(fullPath)) {
		return SUCCESS_REMOVE_DIRECTORY;
	}
	else {
		DWORD error = GetLastError();
		printf_s("Remove directory failed with error %d\n", error);
		if (error == ERROR_DIR_NOT_EMPTY) {
			return DIRECTORY_NOT_EMPTY;
		}
		else if (error == ERROR_PATH_NOT_FOUND || error == ERROR_FILE_NOT_FOUND) {
			return PATH_NOT_FOUND;
		}
		else if (error == ERROR_DIRECTORY) {
			return NOT_DIRECTORY;
		}
		else
			return SERVER_FAIL;
	}

	LeaveCriticalSection(&critical_section);
}

int	handleCWDIR(Session &session, char *path) {
	char fullPath[260];
	char fileData[BUFF_SIZE];
	HANDLE find;
	//Check if logged in
	if (session.loginStatus() == 0) {
		return NOT_LOGIN;
	}
	//Check length of path
	if (strlen(path) == 0 || strlen(path) > 260) {
		return INVALID_PATH;
	}
	//Check validity of directory entered
	if (!checkAccess(session, path, fullPath)) {
		return NO_ACCESS;
	}
	//Change working directory
	find = FindFirstFileExA(fullPath, FindExInfoBasic, fileData, FindExSearchLimitToDirectories, NULL,
		FIND_FIRST_EX_CASE_SENSITIVE);
	if (find == INVALID_HANDLE_VALUE)
		return PATH_NOT_FOUND;
	else {
		session.changeWorkingDir(fullPath);
		return SUCCESS_CHANGE_WORKING_DIRECTORY;
	}
	FindClose(find);
}

int handleDELETE(Session &session, char *path) {
	char fullPath[260];
	char rootPath[260];
	//Check if logged in
	if (session.loginStatus() == 0) {
		return NOT_LOGIN;
	}
	//Check validity of directory entered
	if (!checkAccess(session, path, fullPath) || !checkAccess(session, "~", rootPath)) {
		return NO_ACCESS;
	}

	EnterCriticalSection(&critical_section);
	//Delete file
	if (DeleteFileA(fullPath)) {
		return SUCCESS_DELETE;
	}
	else {
		DWORD error = GetLastError();
		printf_s("Delete file failed with error %d\n", error);
		if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
			return FILE_DOES_NOT_EXISTS;
		else if (error == ERROR_SHARING_VIOLATION)
			return FILE_BEING_ACCESSED;
		else if (error == ERROR_ACCESS_DENIED)
			return NOT_FILE;
		else
			return SERVER_FAIL;
	}

	LeaveCriticalSection(&critical_section);
}

int handleMOV(Session &session, char *oldPath, char *newPath) {
	char fullOldPath[260];
	char fullNewPath[260];
	//Check if logged in
	if (session.loginStatus() == 0) {
		return NOT_LOGIN;
	}
	//Check validity of directory entered
	if (!checkAccess(session, oldPath, fullOldPath) || !checkAccess(session, newPath, fullNewPath)) {
		return NO_ACCESS;
	}

	EnterCriticalSection(&critical_section);
	//Move File
	if (MoveFileA(fullOldPath, fullNewPath)) {
		return SUCCESS_MOVE;
	}
	else {
		DWORD error = GetLastError();
		printf("Move file failed with error %d\n", error);
		if (error == ERROR_ALREADY_EXISTS)
			return PATH_ALREADY_EXISTS;
		else if (error == ERROR_FILE_NOT_FOUND)
			return FILE_DOES_NOT_EXISTS;
		else if (error == ERROR_PATH_NOT_FOUND)
			return PATH_NOT_FOUND;
		else if (error == ERROR_SHARING_VIOLATION)
			return FILE_BEING_ACCESSED;
		else if (error == ERROR_INVALID_NAME)
			return INVALID_NAME;
		else
			return SERVER_FAIL;
	}

	LeaveCriticalSection(&critical_section);
}

int handleSHOW(Session &session, char *path) {
	char fullPath[260];
	//Check if logged in
	if (session.loginStatus() == 0) {
		return NOT_LOGIN;
	}
	//Check length of path
	if (strlen(path) == 0 || strlen(path) > MAX_PATH) {
		return INVALID_PATH;
	}
	//Check validity of directory entered
	if (!checkAccess(session, path, fullPath)) {
		return NO_ACCESS;
	}

	string pathName = fullPath;
	pathName += "\\*";
	string names;

	WIN32_FIND_DATAA data;
	HANDLE hFind = FindFirstFileA(pathName.c_str(), &data);

	//Get file and folder names in directory 
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			string name = data.cFileName;
			if (name != "." && name != "..") {
				name += "\n";
				names += name;
			}
		} while (FindNextFileA(hFind, &data));

		printf_s("List file : %s", names.c_str());
		return SUCCESS_SHOW;
	}
	else {
		return PATH_NOT_FOUND;
	}
	FindClose(hFind);
}

int handleREGISTER(Session &session, string username, string password) {
	string userName = username;
	string passWord = password;
	EnterCriticalSection(&critical_section);
	if (!CreateDirectoryA(username.c_str(), NULL)) {
		DWORD error = GetLastError();
		printf_s("Create directory failed with error %d\n", error);
		if (error == ERROR_ALREADY_EXISTS)
			return ACCOUNT_ALREADY_EXISTS;
		else if (error == ERROR_INVALID_NAME)
			return INVALID_NAME;
		else
			return SERVER_FAIL;
	}
	else {
		ofstream outf{ file, ios::app };
		outf << userName << ' ' << passWord << '\n';
		return SUCCESS_REGISTER;
	}

}

int handleLOGIN(Session &session, string username, string password) {
	ifstream ifs(file, ios::in);
	//Client already login
	if (session.loginStatus() == 1) {
		return ALREADY_LOGIN;
	}
	auto pos = findUser(username);
	//Account login in a another session
	if (pos != session_list.end())
		return LOGIN_IN_ANOTHER_CLIENT;
	string input, s1, s2;
	while (ifs) {
		getline(ifs, input, '\n');
		splitString(input, s1, s2);
		if (strcmp(s1.c_str(), username.c_str()) == 0) {
			if (strcmp(s2.c_str(), password.c_str()) == 0) {
				session.logIn(username);
				return SUCCESS_LOGIN;
			}
			else return PASSWORD_INCORRECT;
		}
	}
	return ACCOUNT_DOES_NOT_EXIST;
}


int handleLOGOUT(Session &session) {
	//Logout successful
	if (session.loginStatus()) {
		session.logOut();
		return SUCCESS_LOGOUT;
	}
	//Not login
	else return NOT_LOGIN;
}

int handlePWDIR(Session &session) {
	char workingDir[260] = "";
	char rootPath[260] = "", workingPath[260] = "";
	DWORD rootlength;

	//Check if logged in
	if (session.loginStatus() == 0) {
		return NOT_LOGIN;
	}

	//Get full root
	rootlength = GetFullPathNameA(session.getUserName(), 260, rootPath, NULL);
	//Get full working dir
	checkAccess(session, ".", workingPath);

	//Get short form of working dir
	sprintf_s(workingDir, MAX_PATH, "%s%s", "~", strstr(workingPath, rootPath) + strlen(rootPath));
	return SUCCESS_PRINT;
}

int handleMESSAGE(SOCKET conn, Session& session, string request) {
	string header, message, username, password, oldPath, newPath, filename, content;
	char *temp = (char *)message.c_str();

	splitString(request, header, message);
	if (header == "LOGIN") {
		splitString(message, username, password);
		return handleLOGIN(session, username, password);
	}
	else if (header == "LOGOUT") {
		return handleLOGOUT(session);
	}
	else if (header == "REGISTER") {
		splitString(message, username, password);
		return handleREGISTER(session, username, password);
	}
	else if (header == "REPASSWORD")
	{
		return REPASSWORD_INCORRECT;
	}
	else if (header == "MKDIR") {
		return handleMKDIR(session, temp);
	}
	else if (header == "RMDIR") {
		return handleRMDIR(session, temp);
	}
	else if (header == "CWDIR") {
		return handleCWDIR(session, temp);
	}
	else if (header == "DELETE") {
		return handleDELETE(session, temp);
	}
	else if (header == "MOV") {
		splitString(message, oldPath, newPath);
		char *temp1 = (char *)oldPath.c_str();
		char *temp2 = (char *)newPath.c_str();
		return handleMOV(session, temp1, temp2);
	}
	else if (header == "UPLOAD")
	{
		splitString(message, filename, content);
		
		char *temp1 = (char *)filename.c_str();
		char *temp2 = (char *)content.c_str();

		fstream data_file;
		data_file.open(temp1, ios::out | ios::app);
		data_file << temp2;
		data_file.close();
		return SUCCESS_UPLOAD;
	}
	else if (header == "CREATE")
	{
		return Upload(session, temp);
	}
	else if (header == "DOWNLOAD")
	{   
		string buff1, buff2;
		splitString(message, buff1, buff2);
		char *local_file = (char *)buff1.c_str();
		char *remote_file = (char *)buff2.c_str();
		int n = DownLoad(session, remote_file);
		if (n==82)
		{
			errno_t file_in;
			FILE *FileIn;
			file_in = fopen_s(&FileIn, remote_file, "r");
			char data[BUFF_SIZE] = { 0 };
			vector<string> responseList;
			int temp1 = 0;
			int temp2 = 0;
			char name[BUFF_SIZE];
			char mess[BUFF_SIZE] = "DOWNLOAD ";
			char zero[2] = " ";
			strcpy_s(name, BUFF_SIZE, local_file);
			strcat_s(mess, BUFF_SIZE, name);
			strcat_s(mess, BUFF_SIZE, zero);
			while ((fgets(data, BUFF_SIZE, FileIn) != NULL)) {
				
				
				
				strcat_s(mess, BUFF_SIZE, data);

				
			}
			int ret = send_stream(conn, mess);
			
			if (ret == SOCKET_ERROR) {
				printf_s("Error %d: Cannot send data.\n", WSAGetLastError());
				return SERVER_FAIL;
			}
			
			int k = recv_stream(conn, responseList);
			for (string response : responseList)
			{

				if (stoi(response) == 80)
				{
					return SUCCESS_DOWNLOAD;
				}
			}
			
			bzero(mess, BUFF_SIZE);
			bzero(name, BUFF_SIZE);
			bzero(data, BUFF_SIZE);
			
		
		}
		else
		{
			return n;
		}
	}
	
	else if (header == "PWDIR") {
		return handlePWDIR(session);
	}
	else if (header == "SHOW") {
		return handleSHOW(session, temp);
	}
	else
	{
		fstream data_file;
		printf("Header: %s", header.c_str());
		data_file.open(header.c_str(), ios::out | ios::app);
		data_file << message.c_str();
		data_file.close();
		return SUCCESS_UPLOAD;
	}
}



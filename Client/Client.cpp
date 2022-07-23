#include <stdio.h>
#include <iostream>
#include <string>
#include <ws2tcpip.h>
#include <winsock2.h>
#include "CodeStatus.h"
#include "StreamTransmission.h"

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048

using namespace std;
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
} FILEOBJ, *LPFILEOBJ;

LPFILEOBJ GetFileObj(HANDLE hfile, LONG64 size, FILEOBJ::OP op) {
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
//Prototype function declaration
void menu();
bool service(char *);
int UploadFile(char*sendMess, char*localFile, char*serverFile)
{
	HANDLE Hfile;
	//open file
	Hfile = CreateFileA(localFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (Hfile == INVALID_HANDLE_VALUE)
	{
		int codeError = GetLastError();
		if (codeError == ERROR_FILE_NOT_FOUND)
		{
			printf_s("Can not find local file");
		}
		else
		{
			printf_s("Error: %d", codeError);
		}
		return 0;

	}
	LARGE_INTEGER FileSize;
	if (!GetFileSizeEx(Hfile, &FileSize)) {
		printf("GetFileSizeEx() failed with error %d\n", GetLastError());
		return 0;
	}
	LPFILEOBJ fileobject = GetFileObj(Hfile, FileSize.QuadPart, FILEOBJ::STOR);
	if (fileobject != NULL)
	{
		sprintf_s(sendMess, BUFF_SIZE, "UPLOAD %s %ld", serverFile, fileobject->size);
		return 1;
	}
	else
	{
		return 0;
	}
}
int main(int argc, char* argv[]) {

	//Enter the ip address and port entry into command
	if (argc != 3) {
		printf_s("Usage: %s <ServerIpAddress> <PortNumber>\n", argv[0]);
		return 1;
	}
	char *server_ipaddress = argv[1];
	int server_port = atoi(argv[2]);

	//Inittiate Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf_s("Version is not supported.\n");
		return 0;
	}
	printf_s("Client started!\n");

	//Construct Socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf_s("ERROR %d: Cannot create server socket", WSAGetLastError());
		return 0;
	}

	//Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	
	serverAddr.sin_port = htons(server_port);
	inet_pton(AF_INET, server_ipaddress, &serverAddr.sin_addr);

	//request to connecct server
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf_s("Error %d: Cannot connect server!\n", WSAGetLastError());
		closesocket(client);
		return 0;
	}
	printf_s("Connected server!\n");

	//Communication
	char buff[BUFF_SIZE];
	vector<string> responseList;
	int ret;
	while (1) {
		//show menu
		menu();
		bool flag;
		//Send message
		flag = service(buff);
		if (flag == true)
		{
			ret = send_stream(client, buff);
			if (ret == SOCKET_ERROR) {
				printf_s("Error %d: Cannot send data.\n", WSAGetLastError());
				continue;
			}
		}
		else
		{
			continue;
		}
		//Receive echo message
		ret = recv_stream(client, responseList);
		if (ret == SOCKET_ERROR)
			printf_s("Error %d: Cannot receive data.\n", WSAGetLastError());
		else {
			for (string response : responseList)
				//Handle message
				cout << printNotice(stoi(response)) << endl;
		}
	}

	//Close socket
	closesocket(client);

	//Terminate Winsock
	WSACleanup();

	return 0;
}

/**
*@Function menu : print to the screen the services for the user to choose
**/
void menu() {
	printf("\n******** Menu ********\n");
	printf_s("=== 1. REGISTER === \n");
	printf_s("=== 2. LOGIN === \n");
	printf_s("=== 3. LOG OUT === \n");
	printf_s("=== 4. MAKE DIRECTORY === \n");
	printf_s("=== 5. REMOVE DIRECTORY === \n");
	printf_s("=== 6. CHANGE WORKING DIRECTORY === \n");
	printf_s("=== 7. UPLOAD FILE === \n");
	printf_s("=== 8. DOWNLOAD FILE === \n");
	printf_s("=== 9. DELETE FILE === \n");
	printf_s("===10. MOVE FOLDER/FILE TO NEW LOCATION === \n");
	printf_s("===11. SHOW LIST FILE IN DIRECTORY === \n");
	printf_s("===12. PRINT WORKING DIRECTORY === \n");
	printf_s("===13. SHUT DOWN === \n");
	printf_s("\n@@@@@@@@@@@@@@@@@@@@@@\n");
	printf_s("Your choice is: \n");
	
}

/**
*@Function service : let user choose service
*@Param [out] message : message sent to server
**/
bool service(char *message) {
	int choice;
	char temp[BUFF_SIZE];
	char temp1[BUFF_SIZE];
	char temp2[BUFF_SIZE];
	while (1) {
		scanf_s("%d", &choice);

		//Clear input buffer
		int c;
		while ((c = getchar()) != '\n' && c != EOF);

		switch (choice) {
			//Register service
		case 1: {
			printf_s("===REGISTER===\n");
			printf_s(" >>> Enter your username: ");
			gets_s(temp, BUFF_SIZE);
			printf_s(" >>> Enter your password: ");
			gets_s(temp1, BUFF_SIZE);
			printf_s(" >>> Enter your Re-password: ");
			gets_s(temp2, BUFF_SIZE);
			if (strcmp(temp1, temp2)!=0)
			{
				printf_s("---> REPASSWORD INCORRECT");
				return false;
			}
			sprintf_s(message, BUFF_SIZE, "REGISTER %s %s", temp, temp1);
			return true;
		}
			//Login service
		case 2: {
			printf_s("===LOGIN===\n");
			printf_s(" >>> Enter username: ");
			gets_s(temp, BUFF_SIZE);
			printf_s(" >>> Enter password: ");
			gets_s(temp1, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "LOGIN %s %s", temp, temp1);
			return true;
		}
			//Logout service
		case 3: {
			sprintf_s(message, BUFF_SIZE, "LOGOUT");
			return true;
		}
			//Make Directory service
		case 4: {
			printf_s("===MAKE DIRECTORY===\n");
			printf_s(" >>> Enter pathname: ");
			gets_s(temp, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "MKDIR %s", temp);
			return true;
		}
			//Remove directory service
		case 5: {
			printf_s("===REMOVE DIRECTORY===\n");
			printf_s(" >>> Enter pathname: ");
			gets_s(temp, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "RMDIR %s", temp);
			return true;
		}
			//Change working directory
		case 6: {
			printf_s("===CHANGE WORKING DIRECTORY===\n");
			printf_s(" >>> Enter pathname: ");
			gets_s(temp, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "CWDIR %s", temp);
			return true;
		}
			//Upload file
		case 7: {
			printf_s("===UPLOAD FILE===\n");
			printf_s(" >>> Enter local file name: ");
			gets_s(temp, BUFF_SIZE);
			printf(" >>> Enter remote file name: ");
			gets_s(temp1, BUFF_SIZE);
			strcpy_s(message, BUFF_SIZE, "");
			if (UploadFile(message, temp, temp1) == 0)
			{
				return false;
			}
			sprintf_s(message, BUFF_SIZE, "UPLOAD %s", temp);
			return true;
		}
			//Download file
		case 8: {
			printf_s("===DOWNLOAD FILE===\n");
			printf_s(" >>> Enter local file name: ");
			gets_s(temp, BUFF_SIZE);
			printf_s(" >>> Enter remote file name: ");
			gets_s(temp1, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "DOWNLOAD %s", temp);
			return true;
		}
			//Delete file
		case 9: {
			printf_s("===DELETE FILE===\n");
			printf_s(" >>> Enter file name: ");
			gets_s(temp, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "DELETE %s", temp);
			return true;
		}
			//Move folder/file to new location
		case 10: {
			printf_s("===MOVE FOLDER\FILE===\n");
			printf_s(" >>> Enter old pathname: ");
			gets_s(temp, BUFF_SIZE);
			printf_s(" >>> Enter new pathname: ");
			gets_s(temp1, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "MOV %s %s", temp,temp1);
			return true;
		}
			//Show list file in directory
		case 11: {
			printf_s("===SHOW LIST FILE IN DIRECTORY===\n");
			printf_s(" >>> Enter path name: ");
			gets_s(temp, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "SHOW %s", temp);
			return true;
		}
			//Print working directory
		case 12: {
			printf_s("===PRINT WORKING DIRECTORY===\n");
			sprintf_s(message, BUFF_SIZE, "PWDIR");
			return true;
		}
			//Shut down
		case 13: {
			exit(0);
		}
			//Service does not exist
		default: {
			printf_s("Service does not exist, choose again.\n");
			continue;
		}
		}
	}
}
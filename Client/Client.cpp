#include <stdio.h>
#include <iostream>
#include <string>
#include <ws2tcpip.h>
#include <winsock2.h>
#include "CodeStatus.h"
#include "StreamTransmission.h"
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048

using namespace std;


//Prototype function declaration
void splitString(string input, string& s1, string& s2) {
	size_t pos = input.find(" ");
	if (pos != string::npos) {
		s1 = input.substr(0, pos);
		s2 = input.substr(pos + 1);
	}
	else s1 = input;
}
void send_file(char*temp, char*temp3, SOCKET sockfd) {
	errno_t file_in;
	FILE *FileIn;
	file_in = fopen_s(&FileIn, temp, "r");
	char data[BUFF_SIZE] = { 0 };
	char buff_rev[BUFF_SIZE] = { 0 };
	vector<string> responseList;
	char create_file[BUFF_SIZE] = "CREATE ";
	char file_create[BUFF_SIZE];
	strcpy_s(file_create, BUFF_SIZE, temp3);
	strcat_s(create_file, BUFF_SIZE, file_create);
	int ret1 = send_stream(sockfd, create_file);
	if (ret1 == SOCKET_ERROR) {
		printf_s("Error %d: Cannot send data.\n", WSAGetLastError());
		return;
	}
	int k1 = recv_stream(sockfd, responseList);
	for (string response : responseList)
	{
		cout << printNotice(stoi(response)) << endl;
		if (stoi(response) != 200)
		{
			return;
		}
	}

	printf("\n---> Starting sending....");

	char name[BUFF_SIZE];
	char mess[BUFF_SIZE] = "UPLOAD ";
	char zero[2] = " ";
	strcpy_s(name, BUFF_SIZE, temp3);
	strcat_s(mess, BUFF_SIZE, name);
	strcat_s(mess, BUFF_SIZE, zero);
	while ((fgets(data, BUFF_SIZE, FileIn) != NULL)) {

		strcat_s(mess, BUFF_SIZE, data);

	}
	int ret = send_stream(sockfd, mess);

	if (ret == SOCKET_ERROR) {
		printf_s("Error %d: Cannot send data.\n", WSAGetLastError());
		return;
	}

	int k = recv_stream(sockfd, responseList);
	for (string response : responseList)
	{
		cout << printNotice(stoi(response)) << endl;
	}

	bzero(mess, BUFF_SIZE);
	bzero(name, BUFF_SIZE);
	bzero(data, BUFF_SIZE);

}
void solveDownLoad(string message, SOCKET conn)
{
	string filename, content;
	splitString(message, filename, content);
	char *temp1 = (char *)filename.c_str();
	char *temp2 = (char *)content.c_str();
	fstream data_file;
	data_file.open(temp1, ios::out | ios::app);
	data_file << temp2;
	data_file.close();
	char buff[BUFF_SIZE] = "80";

	int ret = send_stream(conn, buff);
	if (ret == SOCKET_ERROR) {
		printf_s("Error %d: Cannot send data.\n", WSAGetLastError());
		return;
	}
	vector<string> responseList;
	int ret1 = recv_stream(conn, responseList);
	if (ret1 == SOCKET_ERROR)
	{
		printf_s("Error %d: Cannot receive data.\n", WSAGetLastError());
	}
	else {
		for (string response : responseList)
		{
			cout << printNotice(stoi(response)) << endl;
		}
	}
}
void menu();
bool service(char *);

SOCKET client;
int main(){
//int main(int argc, char* argv[]) {

	// Validate the parameters
	/*if (argc != 3) {
		printf_s("Usage: %s <ServerIpAddress> <ServerPortNumber>\n", argv[0]);
		return 1;
	}
	*/
	//Inittiate Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf_s("Version is not supported.\n");
		return 0;
	}
	printf_s("Client started!\n");

	//Construct Socket
	
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf_s("ERROR %d: Cannot create server socket", WSAGetLastError());
		return 0;
	}

	//Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	//char *server_ipaddress = argv[1];
	//int server_port = atoi(argv[2]);
	int server_port = 5555;
	serverAddr.sin_port = htons(server_port);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

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
		if (flag == true){
			ret = send_stream(client, buff);
			if (ret == SOCKET_ERROR) {
				printf_s("Error %d: Cannot send data.\n", WSAGetLastError());
				continue;
			}
		}
		else{
			continue;
		}
		//Receive echo message
		ret = recv_stream(client, responseList);
		if (ret == SOCKET_ERROR)
			printf_s("Error %d: Cannot receive data.\n", WSAGetLastError());
		else {
			for (string response : responseList) {
				string header, message;
				splitString(response, header, message);
				char *temp = (char *)header.c_str();
				int k;
				k = strcmp(temp, "DOWNLOAD");

				if (k == 0)
				{
					printf("\n ---> Start Downloading....");
					solveDownLoad(message, client);
				}
				else
				{

					if (strcmp(temp, "110") == 0) {
						cout << printNotice(stoi(header)) << endl;
						cout << message << endl;
					}
					else if (strcmp(temp, "120") == 0) {
						cout << printNotice(stoi(header)) << endl;
						cout << message << endl;
					}
					else
					{
						cout << printNotice(stoi(header)) << endl;
					}
				}
			}
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
	printf_s("Your choice is: \n");
	printf_s("@@@@@@@@@@@@@@@@@@@@@@\n");
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
			if (strcmp(temp1, temp2) != 0)
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
			printf_s(" >>> Enter remote file name: ");
			gets_s(temp1, BUFF_SIZE);
			strcpy_s(message, BUFF_SIZE, "");
			printf_s(" ---> Upload file %s ...", temp);
			send_file(temp, temp1, client);
			return false;
		}
				//Download file
		case 8: {
			printf_s("===DOWNLOAD FILE===\n");
			printf_s(" >>> Enter local file name: ");
			gets_s(temp, BUFF_SIZE);
			printf_s(" >>> Enter remote file name: ");
			gets_s(temp1, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "DOWNLOAD %s %s", temp, temp1);
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
			printf_s("===MOVE FOLDER\\FILE===\n");
			printf_s(" >>> Enter old pathname: ");
			gets_s(temp, BUFF_SIZE);
			printf_s(" >>> Enter new pathname: ");
			gets_s(temp1, BUFF_SIZE);
			sprintf_s(message, BUFF_SIZE, "MOV %s %s", temp, temp1);
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



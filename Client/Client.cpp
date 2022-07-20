// Client_Socket.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<string.h>
#include<stdlib.h>
#include<Winsock2.h>
#include<WS2tcpip.h>
#include<iostream>
#include<queue>
#include<sstream>
#include "string"
#include<stdio.h>
using namespace std;
//#define SERVER_PORT 5500
//#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")
//Initiating a queue to handle the stream problem
std::queue<char*> msgs;
//mark: Check to close the window(true) or not(false)
bool markExist = false;
//mark: logged in(true) or not(false)
bool markLogIn = false;
SOCKET client;
// handle the stream problem
//Add to queue
void SolveStream(char s[])
{
	char *pch;
	//The strtok function splits the user into the way the substring is associated with the separator "\r\n"
	pch = strtok(s, "\r\n");
	while (pch != NULL)
	{
		//Handle additional elements in the queue
		msgs.push(pch);
		pch = strtok(NULL, "\r\n");
	}

}

//check is number or not
bool CheckNumber(char s[]) {
	int i, ok;
	for (i = 0; i<strlen(s); i++) {
		if (s[i] >= '0' && s[i] <= '9') {
			ok = 1;
		}
		else {
			ok = 0;
			break;
		}
	}
	if (ok == 1)
		return true;
	else
		return false;
}
//function login
void Login()
{
	//if account have logged
	if (markLogIn == true)
	{
		printf("\n===> You have logged in before !!");
		return;
	}
	char accname[BUFF_SIZE];
	char accpass[BUFF_SIZE];
	char buff[BUFF_SIZE + 1];
	printf("\n ---> Please Enter your username account: ");
	gets_s(accname);
	printf("\n ---> Please Enter your password account: ");
	gets_s(accpass);
	char prefix[BUFF_SIZE] = "LOGIN ";
	//Log in with prefix + accname (username)
	char zero[5] = " ";
	strcat(accname, zero);
	strcat(accname, accpass);
	strcat(prefix, accname);
	strcat(prefix, "\r\n"); //delimiter
	int ret = send(client, prefix, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR)
		printf("\n ===> Error %d: Cannot send data. !!", WSAGetLastError());

	//receive code from server

	ret = recv(client, buff, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR)
		printf("\n ===> Error: %d ! Cannot receive message.", WSAGetLastError());
	else if (strlen(buff) > 0) {
		buff[ret] = 0;
		SolveStream(buff);
		while (!msgs.empty())
		{
			char *buff1 = msgs.front();
			//if code: 10 is success
			if (strcmp(buff1, "201") == 0) {
				printf("\n ===> Login successfully!");
				markLogIn = true;

			}
			//if code: 11 is account is locked
			else if (strcmp(buff1, "410") == 0) {
				printf("\n ===> Account is locked !!\n");
			}
			//if code 12 is account does not exist
			else if (strcmp(buff1, "402") == 0)
				printf("\n ===> Username does not exist !!\n");
			//if password is incorrect
			else if (strcmp(buff1, "403") == 0)
				printf("\n ===> Password is incorrect !!\n");
			//if code 13 account have logged in before
			else if (strcmp(buff1, "404") == 0)
				printf("\n ===> You have logged in before !!\n");
			//if code 99 the request message type is not specified
			else if (strcmp(buff1, "999") == 0)
				printf("\n ===> The requested message type cannot be determined !!\n");
			msgs.pop();
		}
		memset(buff, 0, BUFF_SIZE);

	}


}
//function post
void Register() {
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	char Repassword[BUFF_SIZE];
	printf("\n ---> Please Enter your username: ");
	gets_s(username);
	printf("\n ---> Please Enter your password: ");
	gets_s(password);
	printf("\n ---> Please Enter your Re-password: ");
	gets_s(Repassword);
	if (strcmp(password, Repassword)!=0)
	{
		printf("You enter Re-password is not wrong\n");
		return;
	}
	int ret;
	char prefix[BUFF_SIZE] = "REGISTER ";
	//Register with prefix + message
	char zero[5] = " ";
	strcat(username, zero);
	strcat(username, password);
	strcat(prefix, username);
	strcat(prefix, "\r\n"); //delimiter
	ret = send(client, prefix, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR)
		printf("\n ===> Error %d: Cannot send data. !!", WSAGetLastError());
	char buff[BUFF_SIZE + 1];
	//receive code from server
	ret = recv(client, buff, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR)
		printf("\n ===> Error: %d ! Cannot receive message.", WSAGetLastError());
	else if (strlen(buff) > 0) {
		buff[ret] = 0;
		SolveStream(buff);
		while (!msgs.empty())
		{
			char *buff1 = msgs.front();
			//if code 200 register successfully
			if (strcmp(buff1, "200") == 0) {
				printf("\n ===> Register successfully \n");
			}
			//if code 401 Username have already exist
			else if (strcmp(buff1, "401") == 0) {
				printf("\n ===> Username have already exist!!\n");
			}
			else if (strcmp(buff1, "404") == 0)
				printf("\n ===> You have logged in before !!\n");
			//if code 99 the request message type is not specified
			else if (strcmp(buff1, "999") == 0) {
				printf("\n ===> The requested message type cannot be determined !!\n");
			}

			msgs.pop();
		}
		memset(buff, 0, BUFF_SIZE);


	}

}
//function logout
void Logout()
{
	////if account have not yet
	if (!markLogIn)
	{
		printf("\n ===> You haven't logged yet !!\n");
		return;
	}
	char prefix[BUFF_SIZE] = "BYE";
	int ret;
	//send message BYE
	strcat(prefix, "\r\n"); //delimiter
	ret = send(client, prefix, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR)
		printf("\n ===> Error %d: Cannot send data. !!", WSAGetLastError());
	char buff[BUFF_SIZE + 1];
	//receive code from server
	ret = recv(client, buff, BUFF_SIZE, 0);
	printf("Receive from server: %s\n", buff);
	if (ret == SOCKET_ERROR)
		printf("\n ===> Error: %d ! Cannot receive message.", WSAGetLastError());
	else if (strlen(buff) > 0) {
		buff[ret] = 0;
		SolveStream(buff);
		while (!msgs.empty())
		{
			char *buff1 = msgs.front();
			
			//if code 30 log out sucessfully
			if (strcmp(buff1, "30") == 0) {
				printf("\n ===> Log out successfully!\n");
				markLogIn = false;
			}
			//if code 21 account have not logged yet
			else if (strcmp(buff1, "21") == 0) {
				printf("\n ===> Log out failed because you have not logged yet !!\n");
			}
			//if code 99 the request message type is not specified
			else if (strcmp(buff1, "99") == 0) {
				printf("\n ===> The requested message type cannot be determined !!\n");
			}

			msgs.pop();
		}
		memset(buff, 0, BUFF_SIZE);



	}

}
//close windown
void Shutdown()
{
	char prefix[BUFF_SIZE] = "EXIT";
	int ret;
	//send EXIT to server
	strcat(prefix, "\r\n"); //delimiter
	ret = send(client, prefix, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR)
		printf("\n ===> Error %d: Cannot send data. !!", WSAGetLastError());
	//mark close the windown
	markExist = true;

}
void Menu() {
	printf("\n******** Menu ********\n");
	printf("=== 1. LOG IN === \n");
	printf("=== 2. REGISTER === \n");
	printf("=== 3. LOG OUT === \n");
	printf("=== 4. MAKE DIRECTORY === \n");
	printf("=== 5. DELETE DIRECTORY === \n");
	printf("=== 6. CHANGE WORKING DIRECTORY === \n");
	printf("=== 7. UPLOAD FILE === \n");
	printf("=== 8. DOWNLOAD FILE === \n");
	printf("=== 9. DELETE FILE === \n");
	printf("===10. MOVE FOLDER/FILE TO NEW LOCATION === \n");
	printf("===11. SHOW LIST FILE IN DIRECTORY === \n");
	printf("===12. PRINT WORKING DIRECTORY === \n");
	printf("===13. SHUT DOWN === \n");
	printf("@@@@@@@@@@@@@@@@@@@@@@\n");
}


int main(int argc, char*argv[])
{
	//Enter the ip address and port entry into command
	char*SERVER_ADDR = argv[1];
	int SERVER_PORT = atoi(argv[2]);
	//Step 1: Initiate winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
	{
		printf("Error: %d", WSAGetLastError());
		return 0;
	}
	//Step 2: Construct socket

	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (client == INVALID_SOCKET)
	{
		printf("Error: %d", WSAGetLastError());
		return 0;
	}
	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET; //IPv4 address
	serverAddr.sin_port = htons(SERVER_PORT); //Convert the ipv4 address to binary
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	//Step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr,
		sizeof(serverAddr))) {
		printf("Error! Cannot connect server.");
		return 0;
	}
	printf("Conneted Server\n");

	//Step 5: Communicate with server
	char buff[BUFF_SIZE];
	//display the interface until the user closes the window
	do {
		//show menu
		Menu();
		char enter[255];
		int choice;


		do
		{

			printf("\nPlease! you have to select function: ");

			//enter in user select function
			gets_s(enter);
			//if enter is not number
			if (!CheckNumber(enter))
			{
				printf("\nInvalid! You enter a character other than a number !!\n");



				continue;
			}
			//convert char to int
			choice = atoi(enter);
			if (choice < 1 || choice > 13)
			{
				printf("\nYou entered the wrong function, please! re-enter !!\n");
			}



		} while (choice < 1 || choice > 13);
		switch (choice)
		{
		case 1:
			Login();
			break;
		case 2:
			Register();
			break;
		case 3:
			Logout();
			break;
		case 13:
			Shutdown();
			break;
		default:
			break;
		}

	} while (!markExist);

	//Step 6: Close socket
	closesocket(client);
	//Step 7: Terminate Winsock
	WSACleanup();
}




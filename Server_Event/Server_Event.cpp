// WSAEventSelectServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#include<WS2tcpip.h>
#include <fstream> 
#include <string>
#include <iostream>
#include <string.h>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")

#define PORT 5500
#define BUFF_SIZE 2048
#define SERVER_ADDR "127.0.0.1"
//struct User include field of each session
typedef struct USER
{
	SOCKET socket;
	sockaddr_in sock_address;
	char username[BUFF_SIZE];
	int status;
}UserSession;

int Receive(SOCKET, char *, int, int);
int CheckLog(char *message, char*filename)
{
	//return 0: if account exists and active
	//return 1: otherwise

	//split username and password
	int temp1 = 0, temp2 = 0;
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	while (message[temp1]!=' ' && message[temp1] != '\0')
	{
		username[temp1] = message[temp1];
		temp1++;
	}
	username[temp1] = '\0';
	temp1++;
	while (message[temp1] != ' ' && message[temp1] != '\0')
	{
		password[temp2] = message[temp1];
		temp1++;
		temp2++;
	}
	password[temp2] = '\0';

	//open file account.txt
	std::ifstream file(filename);
	std::string str;
	//loop through line in file
	while (std::getline(file, str))
	{
		//get account name and status
		int i = 0;
		int k = 0;
		char text_user[255];
		char text_pass[255];
		while (str[i] != ' ' && str[i] != '\0')
		{
			text_user[i] = str[i];
			i++;

		}
		text_user[i] = '\0';
		i++;
		while (str[i] != ' ' && str[i] != '\0')
		{
			text_pass[k] = str[i];
			i++;
			k++;

		}
		text_pass[k] = '\0';
		i++;
		//if the account is in the account.txt file
		if (strcmp(text_user, username) == 0)
		{   
			if (strcmp(text_pass, password)==0)
			{
				//if account is active
				if (str[i] == '0')
				{

					return 0;
				}
				//if account is blocked
				else
				{

					return 1;
				}
			}
			else
			{
				return 2;
			}
		}
		
		

	}
	return -1;
	file.close();
	
}
void Send(SOCKET s, char *buff, int size, int flags) {
	int ret;

	ret = send(s, buff, size, flags);
	if (ret == SOCKET_ERROR) {
		printf("Error: %", WSAGetLastError());
	}

}
//function login
void Login(UserSession &session, char*message)
{
	char *replyCode;
	//if user have not logged yet
	if (session.status == 0) {
		//check name account have exist
		if (CheckLog(message, "account.txt") == 0) {
			replyCode = "201\r\n";
			//change status
			session.status = 1;
			//copy message into username
			strcpy(session.username, message);
		}
		//if account is locked
		else if (CheckLog(message, "account.txt") == 1) {
			replyCode = "410\r\n";
		}
		//if username does not exist
		else if (CheckLog(message, "account.txt") == -1) {
			replyCode = "402\r\n";
		}
		//if password incorrect
		else {
			replyCode = "403\r\n";
		}
	}
	else if (session.status == 1) {
		replyCode = "404\r\n";
	}
	Send(session.socket, replyCode, BUFF_SIZE, 0);

}
//function Post
void Register(UserSession &session, char*message)
{
	char *replyCode;
	//if status of session is logged
	if (session.status == 0)
	{
		if (CheckLog(message, "account.txt") == -1)
		{
			fstream ofs;
			//open account.txt
			ofs.open("account.txt", ios::out | ios::app);
			strcat(message, " 0");
			ofs << message << " \n";
			ofs.close();
			replyCode = "200\r\n";
		}
		else
		{
			replyCode = "401\r\n";
		}
	}
	//account have not logged yet
	else if (session.status == 1)
		replyCode = "404\r\n";

	Send(session.socket, replyCode, BUFF_SIZE, 0);

}
//function Log out
void LogOut(UserSession &session)
{
	char *replyCode;
	//if status of session is logged
	if (session.status == 1) {

		replyCode = "30\r\n";
		//change status
		session.status = 0;
	}
	//account have not logged yet
	else {
		replyCode = "21\r\n";
	}
	printf("OK replycode: %s\n", replyCode);
	Send(session.socket, replyCode, BUFF_SIZE, 0);

}
//function handle each session
//file account.txt: store account information
int processData(UserSession &session, char*buff) {

	//seperate prefix of message 
	//get the first prefix until " "
	char*prefixOfmess = strtok(buff, " ");
	//get len prefix
	int len = strlen(prefixOfmess);
	prefixOfmess[len] = '\0';

	char message[BUFF_SIZE];
	//get position of message
	int i = len + 1;
	int k = 0;
	//get the message with the prefix removed
	for (; buff[i] != '\0';)
	{
		message[k] = buff[i];
		i++;
		k++;
	}
	message[k] = '\0';
	char *replyCode; //code response to client
	printf("Prefix: %s\n", prefixOfmess);

					 //If request is log in
	if (strcmp(prefixOfmess, "LOGIN") == 0) {
		Login(session, message);
	}
	//If request is post message

	else if (strcmp(prefixOfmess, "REGISTER") == 0) {
		Register(session, message);
	}

	//If request is log out
	else if (strcmp(prefixOfmess, "BYE") == 0) {
		LogOut(session);
	}
	//if user exit windown
	else if (strcmp(prefixOfmess, "EXIT") == 0)
	{
		printf("Client disconnects! socket: %d.\n", session.socket);
		return -1;
	}
	//if the request message type is not specified
	else
	{
		replyCode = "999";
		Send(session.socket, replyCode, BUFF_SIZE, 0);
	}
}

int main(int argc, char* argv[])
{
	DWORD		nEvents = 0;
	DWORD		index;
	
	WSAEVENT	events[WSA_MAXIMUM_WAIT_EVENTS];
	UserSession client[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS sockEvent;

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	//Step 2: Construct LISTEN socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	client[0].socket = listenSock;
	events[0] = WSACreateEvent(); //create new events
	nEvents++;

	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and newEvent   
	WSAEventSelect(client[0].socket, events[0], FD_ACCEPT | FD_CLOSE);


	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	char sendBuff[BUFF_SIZE], recvBuff[BUFF_SIZE+1];
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	int ret, i;

	for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		client[i].socket = 0;
	}
	while (1) {
		//wait for network events on all socket
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
			break;
		}

		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(client[index].socket, events[index], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_ACCEPT) {
			if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
				printf("FD_ACCEPT failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				break;
			}

			if ((connSock = accept(client[index].socket, (sockaddr *)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
				printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
				break;
			}
			struct sockaddr_in clientAddr;
			int len;
			getpeername(connSock, (struct sockaddr*)&clientAddr, &len);
			//Add new client into client array
			int i;
			if (nEvents == WSA_MAXIMUM_WAIT_EVENTS) {
				printf("\nToo many clients.");
				closesocket(connSock);
			}
			else {
				len = sizeof(clientAddr);
				getpeername(connSock, (struct sockaddr*)&clientAddr, &len);
				client[nEvents].socket = connSock;
				client[nEvents].sock_address = clientAddr;
				client[nEvents].status = 0;
				events[nEvents] = WSACreateEvent();
				WSAEventSelect(client[nEvents].socket, events[nEvents], FD_READ | FD_CLOSE);
				nEvents++;
			}
				

			//reset event
			WSAResetEvent(events[index]);
		}

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				break;
			}

			ret = Receive(client[index].socket, recvBuff, BUFF_SIZE, 0);
			printf("Receive from client %s\n", recvBuff);

			//Release socket and event if an error occurs
			if (ret <= 0) {
				closesocket(client[index].socket);
				WSACloseEvent(events[index]);
				client[index].socket = 0;
				nEvents--;
			}
			else {									//echo to client
				char *ptr = strtok(recvBuff, "\r\n");
				
				processData(client[index], ptr);
				
				

				//reset event
				WSAResetEvent(events[index]);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
				//break;
			}
			//Release socket and event
			closesocket(client[index].socket);
			client[index].socket = 0;
			WSACloseEvent(events[index]);
			nEvents--;
		}
	}
	return 0;
}

/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = recv(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error %d: Cannot receive data.\n", WSAGetLastError());
	else if (n == 0)
		printf("Client disconnects.\n");
	return n;
}

/* The send() wrapper function*/



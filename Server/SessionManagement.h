#include <iostream>
#include <WinSock2.h>

using namespace std;

/**
*@Class Session : declare the variables and activities of a session
**/
class Session {
private:
	SOCKET connSock;
	char * clientIp;
	int clientPort;
	char * userName;
	int status;
	char *workingDir;
public:
	Session(SOCKET&, char*, int);
	void logIn(string &);
	void logOut();
	SOCKET getSocket();
	char* getClientIp();
	int getClientPort();
	char* getUserName();
	char* getWorkingDir();
	int loginStatus();
	void changeWorkingDir(char *);
};

/**
*@Constructor Session
*@Param [in] connSock : SOCKET
*@Param [in] clientIp : client's IP address
*@Param [in] clientPort : client port number
*@Function: assign the parameters passed to the variable in the class ,
any parameter not passed will be initialized by default
**/
Session::Session(SOCKET& connSock, char* clientIp, int clientPort) {
	this->connSock = connSock;
	this->clientIp = _strdup(clientIp);
	this->clientPort = clientPort;
	this->userName = _strdup("");
	this->workingDir = _strdup("");
	this->status = 0;
}

/**
*@Function logIn : When user login, change username and login status to status 1
*@Param [in] username : account name
**/
void Session::logIn(string& username) {
	this->userName = _strdup(username.c_str());
	this->workingDir = _strdup(username.c_str());
	status = 1;
}

/**
*@Function logOut : When user logout, change username and login status to status 0
**/
void Session::logOut() {
	this->userName = _strdup("");
	this->workingDir = _strdup("");
	status = 0;
}

/**
*@Function getSocket : Get the socket in use in the session
**/
SOCKET Session::getSocket() {
	return this->connSock;
}

/**
*@Function getClientIp : Get client's IP address in use in the session
**/
char* Session::getClientIp() {
	return this->clientIp;
}

/**
*@Function getClientPort : Get client port number in use in the session
**/
int Session::getClientPort() {
	return this->clientPort;
}

/**
*@Function getUserName : Get account name in use in the session
**/
char* Session::getUserName() {
	return this->userName;
}

char* Session::getWorkingDir() {
	return this->workingDir;
}

/**
*@Function loginStatus : Get the active status of the account in use in the session
**/
int Session::loginStatus() {
	return this->status;
}

void Session::changeWorkingDir(char *iWorkingDir) {
	strcpy_s(this->workingDir, 260, iWorkingDir);
}


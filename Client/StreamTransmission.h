#include <winsock2.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

#define BUFF_SIZE 2048
#define ENDING_DELIMITER "\r\n"

/**
*@Function send_stream: send buffer in stream
*@Param[in] connSock: the connected socket
*@Param[in] buff: the message
*@Return: Return the number of bytes sent. If not successful, return SOCKET_ERROR
**/
int send_stream(SOCKET &connSock, char * buff) {
	int ret, messageLen;

	//Add the ENDING_DELIMITER at the end
	strcat_s(buff, BUFF_SIZE, ENDING_DELIMITER);

	//Send all buff, if the buffer is still not delivered in its entirety 
	//go back to the for loop and continue sending until all the buffer are sent
	messageLen = strlen(buff);
	for (int i = 0; i < messageLen; ) {
		ret = send(connSock, buff + i, messageLen - i, 0);
		i += ret;
		if (ret == SOCKET_ERROR) break;
	}
	return ret;
}

/**
*@Function recv_stream : receive buffer in stream
*@Param[in] connSock: the connectied socket
*@Param[out] requesList: list of requests
*@return: Return the number of bytes received. If not successful, return SOCKET_ERROR
*/
int recv_stream(SOCKET &connSock, vector<string> &requestList) {
	int ret;
	char temp[BUFF_SIZE];
	string buff;
	size_t pos, messageLen;

	//Empty requestList vector
	requestList.clear();

	//Receive until the buffer ends with a ENDING_DELIMITER
	/**
	*@There are 3 cases in this situation
	*@Case 1: There is no ENDING_DELIMITER in the buffer. Eg: ABCD
	*@Case 2: The buffer received ENDING_DELIMITER, but ENDING_DELIMITER was not at the end. Eg: ABCD\r\nEF
	*@Case 3: The buffer receives ENDING_DELIMITER and ENDING_DELIMITER is at the end.Eg: AB\r\n or A\r\nB\r\n
	*@Cases 1 and 2 will continue to buffer receive messages until case 3 occurs
	*@Case 3 occurs, the message will be split into requestList. Eg: AB\r\nCD\r\n --> AB , CD
	**/
	while ((ret = recv(connSock, temp, BUFF_SIZE, 0)) > 0) {
		temp[ret] = 0;
		buff += temp;

		//Find ENDING_DELIMITER from the end of string
		pos = buff.rfind(ENDING_DELIMITER);

		//Eg: AB\r\nCD\r\n , pos = 6, strlen(ENDING_DELIMITER)=2 -> messageLen=8,buff.length()=8
		//Here has happened case 3 , we break from the loop and do the next step
		messageLen = pos + strlen(ENDING_DELIMITER);
		if (messageLen == buff.length())
			break;
	}

	//Split buffer into requestList
	while (!buff.empty()) {
		pos = buff.find(ENDING_DELIMITER);
		if (pos == string::npos) //Check if found ENDING_DELIMITER
			break;
		else {
			////separate the messages and add at the end of the requestList
			requestList.push_back(buff.substr(0, pos));
			//Remove the message just added to the vector requestList from the string buff
			buff.erase(0, pos + strlen(ENDING_DELIMITER));
		}
	}

	return ret;
}
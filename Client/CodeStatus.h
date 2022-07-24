#include <string>
#include <iostream>

using namespace std;

//Declare executable codes and their meanings
//Register
#define SUCCESS_REGISTER  10               
#define ACCOUNT_ALREADY_EXISTS 11	
#define INVALID_NAME 44
#define SERVER_FAIL 404
#define REPASSWORD_INCORRECT 405

//Login
#define SUCCESS_LOGIN 20     
#define ACCOUNT_DOES_NOT_EXIST 21		     
#define PASSWORD_INCORRECT 22				 
#define ALREADY_LOGIN 23
#define LOGIN_IN_ANOTHER_CLIENT 24

//Logout
#define SUCCESS_LOGOUT 30					   
#define NOT_LOGIN 31			           

//Make directory
#define SUCCESS_MAKE_DIRECTORY 40
#define NOT_LOGIN 31
#define NO_ACCESS 41
#define ALREADY_EXISTS 42
#define PATH_NOT_FOUND 43
#define INVALID_NAME 44
#define SERVER_FAIL 404

//Remove directory
#define SUCCESS_REMOVE_DIRECTORY 50
#define NOT_LOGIN 31
#define NO_ACCESS 41
#define PATH_NOT_FOUND 43
#define CANT_REMOVE 51
#define DIRECTORY_NOT_EMPTY 52
#define NOT_DIRECTORY 53
#define SERVER_FAIL 404

//Change working directory
#define SUCCESS_CHANGE_WORKING_DIRECTORY 60
#define NOT_LOGIN 31
#define NO_ACCESS 41
#define PATH_NOT_FOUND 43
#define INVALID_PATH 61

//Upload file
#define SUCCESS_UPLOAD 70
#define NOT_LOGIN 31
#define NO_ACCESS 41
#define PATH_NOT_FOUND 43
#define FILE_ALREADY_EXISTS 71
#define FILE_BEING_ACCESSED 72

//Download file
#define SUCCESS_DOWNLOAD 80
#define NOT_LOGIN 31
#define NO_ACCESS 41
#define FILE_DOES_NOT_EXISTS 81
#define FILE_BEING_ACCESSED 72

//Delete file
#define SUCCESS_DELETE 90
#define NOT_LOGIN 31
#define NO_ACCESS 41
#define FILE_DOES_NOT_EXISTS 81
#define FILE_BEING_ACCESSED 72
#define NOT_FILE 91
#define SERVER_FAIL 404

//Move file/directory
#define SUCCESS_MOVE 100
#define NOT_LOGIN 31
#define NO_ACCESS 41
#define FILE_DOES_NOT_EXISTS 81
#define PATH_NOT_FOUND 43
#define PATH_ALREADY_EXISTS 101
#define FILE_BEING_ACCESSED 72
#define INVALID_NAME 44
#define SERVER_FAIL 404

//Show file in directory
#define SUCCESS_SHOW 110
#define NOT_LOGIN 31
#define NO_ACCESS 41
#define INVALID_PATH 61
#define PATH_NOT_FOUND 43

//Print working directory
#define SUCCESS_PRINT 120

//Server fail
#define SERVER_FAIL 404

//Undefined messages
#define UNKNOWN 999


/**
*@Functiom printNotice : Print out the message screen
*@Param [in] code : executable code
*@Returns the string containing the message corresponding to the code to execute
**/
inline string printNotice(int code) {
	switch (code) {
	case 10: return "\n ---> SUCCESSFUL REGISTER.";
	case 11: return "\n ---> ACCOUNT ALREADY EXISTS.";
	case 20: return "\n ---> SUCCESSFUL LOGIN.";
	case 21: return "\n ---> ACCOUNT DOES NOT EXIST.";
	case 22: return "\n ---> PASSWORD INCORECT.";
	case 23: return "\n ---> ALREADY LOGIN.";
	case 24: return "\n ---> LOGIN IN ANOTHER CLIENT.";
	case 30: return "\n ---> SUCCESSFUL LOGOUT";
	case 31: return "\n ---> NOT LOGIN";
	case 40: return "\n ---> SUCCESSFUL MAKE DIRECTORY.";
	case 41: return "\n ---> NO ACCESS.";
	case 42: return "\n ---> PATH ALREADY EXISTS.";
	case 43: return "\n ---> PATH NOT FOUND.";
	case 44: return "\n ---> INVALID DIRECTORY NAME.";
	case 50: return "\n ---> SUCCESSFUL REMOVE DIRECTORY.";
	case 51: return "\n ---> CANT REMOVE ROOT DIRECTORY.";
	case 52: return "\n ---> DIRECTORY NOT EMPTY.";
	case 53: return "\n ---> NOT DIRECTORY";
	case 60: return "\n ---> SUCCESSFUL CHANGE WORKING DIRECTORY.";
	case 61: return "\n ---> INVALID PATH.";
	case 70: return "\n ---> SUCCESSFUL UPLOAD.";
	case 71: return "\n ---> FILE ALREADY EXISTS.";
	case 72: return "\n ---> FILE BEING ACCESSED.";
	case 80: return "\n ---> SUCCESSFUL DOWNLOAD.";
	case 81: return "\n ---> FILE DOES NOT EXISTS.";
	case 90: return "\n ---> SUCCESSFUL DELETE.";
	case 91: return "\n ---> NOT FILE.";
	case 200: return "\n ---> CREATE REMOTE FILE SUCCESS";
	case 100: return "\n ---> SUCCESSFUL MOVE.";
	case 101: return "\n ---> PATH ALREADY EXISTS.";
	case 110: return "\n ---> SUCCESSFUL SHOW.";
	case 120: return "\n ---> SUCCESSFUL PRINT WORKING DIRECTORY.";
	case 404: return "\n ---> SERVER FAIL.";
	case 405: return "\n ---> RE PASSWORD INCORRECT.";
	case 999: return "\n ---> UNDEFINED MESSAGES";

	default: return string();
	}
}
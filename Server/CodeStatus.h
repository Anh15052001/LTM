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
#define ALREADY_DOWNLOAD 82

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
#define CREATE_REMOTE_FILE 200

/**
*@Functiom printNotice : Print out the message screen
*@Param [in] code : executable code
*@Returns the string containing the message corresponding to the code to execute
**/
inline string printNotice(int code) {
	switch (code) {
	case 10: return "SUCCESSFUL REGISTER.";
	case 11: return "ACCOUNT ALREADY EXISTS.";
	case 20: return "SUCCESSFUL LOGIN.";
	case 21: return "ACCOUNT DOES NOT EXIST.";
	case 22: return "PASSWORD INCORECT.";
	case 23: return "ALREADY LOGIN.";
	case 24: return "LOGIN IN ANOTHER CLIENT.";
	case 30: return "SUCCESSFUL LOGOUT";
	case 31: return "NOT LOGIN";
	case 40: return "SUCCESSFUL MAKE DIRECTORY.";
	case 41: return "NO ACCESS.";
	case 42: return "PATH ALREADY EXISTS.";
	case 43: return "PATH NOT FOUND.";
	case 44: return "INVALID DIRECTORY NAME.";
	case 50: return "SUCCESSFUL REMOVE DIRECTORY.";
	case 51: return "CANT REMOVE ROOT DIRECTORY.";
	case 52: return "DIRECTORY NOT EMPTY.";
	case 53: return "NOT DIRECTORY";
	case 60: return "SUCCESSFUL CHANGE WORKING DIRECTORY.";
	case 61: return "INVALID PATH.";
	case 70: return "SUCCESSFUL UPLOAD.";
	case 71: return "FILE ALREADY EXISTS.";
	case 72: return "FILE BEING ACCESSED.";
	case 80: return "SUCCESSFUL DOWNLOAD.";
	case 81: return "FILE DOES NOT EXISTS.";
	case 90: return "SUCCESSFUL DELETE.";
	case 91: return "NOT FILE.";
	case 100: return "SUCCESSFUL MOVE.";
	case 101: return "PATH ALREADY EXISTS.";
	case 110: return "SUCCESSFUL SHOW.";
	case 120: return "SUCCESSFUL PRINT WORKING DIRECTORY.";
	case 404: return "SERVER FAIL.";
	case 999: return "UNDEFINED MESSAGES";
	case 200: return "CREATE REMOTE FILE SUCCESS";

	default: return string();
	}
}
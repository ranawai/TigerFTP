/******************************************************************************
 * 
 * Copyright: Nelson H. Powell III,  22 Dec 2020
 *            nhpeec@rit.edu
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 ******************************************************************************
 * 
 * Description:
 * 
 * This library provides students with the re2c lexer to parse command line
 * interface data from a TTY. The TTY is the stdin stream. This also provides
 * the functions for transitioning the parser from an unauthenticated user to 
 * an authenticated user, and back.
 * 
 * The second set of function declarations provides the library a method to 
 * call into the students code. Students are responsible for completing these 
 * functions are part of the TigerC project.
 */



#ifndef _PROCESSING_H
#define _PROCESSING_H   1

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * The following function declarations are exported from the parsing library
 * for student use.  The ProcessLoop is already placed in the main() for you,
 * but the Authenticated() and Deauthenticate() function calls are left for 
 * you (the student) to call based on the state of your client connection to
 * the server. You should not need to access the variables or statemachines
 * within the library directly.
 *****************************************************************************/

/*
 * This function is called when a successful connect to the server is created
 * and the username-password challenge is approved by the server. This funciton 
 * changes the authentication state allowing the client to user commands 
 * other than TCONNECT.
 */ 
void Authenticated( void );

/*
 * This function is used to change the parser's state machine such that the 
 * client only allows the use of the TCONNECT command. Call this function 
 * after disconnecting from the server.
 */
void Deauthenticate( void );

/*
 * This is the main CLI parsing loop. This is already strategically placed 
 * in the main.cpp file for you.
 */
void ProceessLoop( void );


//---------------------------------------------------------------------------//


/*****************************************************************************
 * The following functions must be implemented by you (the student) in 
 * the main source package. These declarations allow the proceessing.cpp 
 * functions to call them when needed.
 *****************************************************************************/

/*
 * Fill in code for the Login process here. This function is called by the
 * lexer library wheen the user insantiaties the TCONNECT command. Be sure
 * to return a success status of 0 or failure status of -1. This informs
 * the lexer code of the authentication status. Note that this only accounts
 * for the authentication of the client side. Be sure you support an
 * authentication with the server side as well.
 * 
 * @param uint32_t  IPv4 Address of the server as a 32-bit number
 * @param char *    NULL-terminated Usersname char array
 * @param char *    NULL-terminated Password char array
 * 
 * @returns int     Success status: 0 success, -1 failure
 */
int ProcessLogin( uint32_t sipaddr, char *username, char *password );

/*
 * This function is called by the lexer code when the user submits the exit 
 * command after the client is already logged in and authenticated with the 
 * server.  This function should close the connection with the server and 
 * then call the Deauthenticate() function.
 * 
 * @return int      Success status: 0 success, -1 failure
 */
int ProcessLogout( void );

/*
 * Fill in code for the TPUT command. This function accepts a NULL-terminated
 * character array containing the filename of the local filesystem file to 
 * push to the server. You must open and read the local file, and then send the 
 * file to the serveer along with the filename of the file. Return the status 
 * of the operation as an integer value (using POSIX standards).
 * 
 * @param char*     Filename of the local file to send
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessPutFile( char *filename );

/*
 * Fill in code for the TGET command. This function accepts a NULL-terminated
 * character array containing the filename of the server filesystem file to 
 * pull from the server. You must send the command and filename to the server.
 * The server should report a success/failure status to the clieent, and if 
 * success, it should then transmit the entire file.  This funciton must then
 * store the data using the same filename on the local filesystem.
 * 
 * @param char*     Filename of the remote file to receive
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessGetFile( char *filename );

/*
 * This function should send a command to the server to get a listing of the 
 * server's current working directory, for the thread or context serving the 
 * client instance. This implies that the server should be able to have different
 * working directories active for each authenticated client. The server should 
 * return success or failure, and if success, it should alos provide the 
 * listing of the server's currwnt directory.
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessListDir( void );

/*
 * This function is used to change the working directory of the server for the 
 * active client only. Changing the server's working directory from one instance
 * of the client should have no affect on another active instance of the same 
 * client.  The server should report success or failure.
 * 
 * @returns int     Status of the operation 0 for success, -1 for failure
 */
int ProcessChangeDir( char *dirname );

/*
 * This function is used to perform an emergency shutdown. If the STDIN stream
 * returns an EOF, then the client must close the active connection and clean
 * up any dyanmic memory or files opened. When you return from this function
 * the ProcessCommand loop will break out and return to the main function to 
 * complete the app processing.
 *
 */
void EmergencyShutdown( void );

#ifdef __cplusplus
}
#endif

#endif    // _PROCESSING_H

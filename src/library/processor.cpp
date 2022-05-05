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
 * This file contains all of the string parsing functions to support the TigerC
 * lexer. The lexer code is found in lexer.cpp, which is autogenerated by the 
 * re2c application. The Tokenizer.h header file provides enumerations and 
 * variables used for both the lexer.cpp and this file.
 * 
 * Do not change anything in this file for the project, unless you really know
 * what you're doing.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <arpa/inet.h>

#include "../../include/Tokenizer.h"
#include "../../include/processing.h"

extern Token_s*  GetNextCommandToken( char* string );
extern Token_s*  GetParameterToken( char* string );

FtpStates_e authState; 
UserInputBuffer userInputBuf;

TokenList list;

#define INVALID_IP  0x00000000
uint32_t  gServerIpAddress = INVALID_IP;
char gUsername[MAX_CMD_SIZE];
char gPassword[MAX_CMD_SIZE];

void Authenticated() {
  authState = PROC_AUTHENTICATED;
}

void Deauthenticate() {
  authState = PROC_LOGIN;
}
      
bool ProcessIpAddress() {
  
  bool success = false;
  
  // Dump something specific if more than one token
  if ( authState == PROC_AUTHENTICATED) 
  {
    printf("\nERROR: invalid state (%d) connecintg to server\n\n", authState );
  }
  else
  {
    bool valid = true;
    int retval = -1;
    unsigned int b1, b2, b3, b4 = 0;
    
    gServerIpAddress = INVALID_IP;
    
    // Convert the string in list[2] to a uint32_t
    if ( sscanf( list[1]->start, "%3u.%3u.%3u.%3u", &b4, &b3, &b2, &b1 ) != 4 )
      printf("\nERROR: IPv4 address invalid format\n\n");
    else
    {
      if ( b1 > 255 || b2 > 255 || b3 > 255 || b4 > 255 )
        printf("\nERROR: IPv4 octets out of range\n\n");
      else
      {
        gServerIpAddress = (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
        success = true;
      }
    }
  }
 
  return success;
}

bool ProcessUsername() {
  unsigned int Loff = 2; 
  bool success = false;
  
  if ( list[Loff]->type == SPACE )
    ++Loff;
  
  if ( list[Loff]->type != STRING ) {
    printf("\nERROR: Username missing\n\n");
  }
  else {
    if ( sscanf(list[Loff]->start, "%s %s", gUsername, gPassword) == 2 )
      success = true;
    else
      printf("\nERROR: Username and Password not available\n");
  }
  
  return success;
}


bool ProcessCommand() {
  Token_s* first = list[0];
  
  switch ( first->type )
  {
    case EXIT: {
      
      if ( authState == PROC_LOGIN )
        return true;
      else
        ProcessLogout();
      
      break;
    }
    
    case TCONNECT:
    {
      Token_s* second = list[1];
      
      if ( !second )
        printf("\nERROR: additional tokens needed\n\n");
      else if ( second->type == IP_ADDR ) 
      {
        unsigned int offset = 2;
        char username[MAX_CMD_SIZE];
        char password[MAX_CMD_SIZE];
        
        if ( ProcessIpAddress() ) {
           if ( ProcessUsername() )
             ProcessLogin( gServerIpAddress, gUsername, gPassword );
        }
      }
      else
        printf("\nERROR: Invalid TCONNECT request: \"%s\"\n\n", second->start );
      
      // Clear out the variables after use to avoid data leakage
      memset( gUsername, 0, MAX_CMD_SIZE );
      memset( gPassword, 0, MAX_CMD_SIZE );
      gServerIpAddress = INVALID_IP;
      
      break;
    }
    
    case TGET: {
      Token_s* second = list[1];
      
      // Check for a space before the filename
      if ( !second )
        printf("\nERROR: additional tokens needed\n\n");
      else if ( second->type == SPACE )
        second = list[2];
      
      if ( !second )
        printf("\nERROR: additional tokens needed\n\n");
      else if ( second->type == STRING )
      {
        ProcessGetFile( second->start );
      }
      else
        printf("ERROR: No filename provided for TPUT command\n\n");
        
      for ( int i = 0; i < list.size(); i++ )
        printf("list[%d] - type %d string %s\n", i, list[i]->type, list[i]->toChar() );
      printf("\n");
      
      break;
    }
    
    case TPUT: {
      Token_s* second = list[1];
      
      // Check for a space before the filename
      if ( !second ) {
        printf("\nERROR: additional tokens needed\n\n");
        break;
      }
      else if ( second->type == SPACE )
        second = list[2];
      
      if ( !second )
        printf("\nERROR: additional tokens needed\n\n");
      else if ( second->type == STRING )
      {
        ProcessPutFile( second->start );
      }
      else
        printf("ERROR: No filename provided for TPUT command\n\n");

      for ( int i = 0; i < list.size(); i++ )
        printf("list[%d] - type %d string %s\n", i, list[i]->type, list[i]->toChar() );
      printf("\n");
      
      
      break;
    }
    
    case TLIST: {
      // Call back to the students code to list the server's current directory
      ProcessListDir();
      break;
    }
    
    case TCWD: {
      Token_s* second = list[1];
      
      // Check for a space before the filename
      if ( !second ) {
        printf("\nERROR: additional tokens needed\n\n");
        break;
      }
      else if ( second->type == SPACE )
        second = list[2];
      
      if ( !second )
        printf("\nERROR: additional tokens needed\n\n");
      else if ( second->type == STRING )
      {
        // Call the students code to force server to change directories
        ProcessChangeDir( second->start );
      }
      else
        printf("ERROR: No filename provided for TCWD command\n\n");
      
      break;
    }
                  
    default:
      printf("\nERROR: Unknown command\n\n");
      break;
  }
  
  return false;
}

void ProcessHelpCommand() {
  
  Token_s* second = list[1];
  
  if ( second ) {
    // Help on a lower level command
    switch ( second->type )
    {
      case TCONNECT:{
        printf(
          "\n"
          "  Connect to the server\n"
          "\n"
          "\tUsage:  TCONNECT [IPv4 Address] [username] [password]\n"
          "\n"
          "  Use this command to connect to a known server at the user provided IPv4 Address.\n"
          "  The IPv4 Address of the server must be provided. The port number is predefined and\n"
          "  requires that the developer have the same Port Number defined in both the client\n"
          "  and server code. The IPv4 address MUST be followed by a Username and Password. The\n"
          "  username and password must be used for authentication with the server.  When the \n"
          "  responds with a TSUCCESS message, the FTP statemachine shifts to CONNECTED. If the\n"
          "  authentication fails with a TFAIL message, the FTP statemachine remains in the \n"
          "  LOGIN state to query for the next TCONNECT command. \n"
          "\n"
        );
        break;
      }
        
      case EXIT:{
        printf(
          "\n"
          "  Exit authenticated connection or application\n"
          "\n"
          "\tUsage:  EXIT\n"
          "\n"
          "  When entered at the command line, this command should cause the the applpication to\n"
          "  to disconnect from the server, if already connected, or exit the application if the\n"
          "  client is not connected to the server.\n"
          "\n"
        );
        break;
      }
        
      case TPUT:{
        printf(
          "\n"
          "  Push a file to the server\n"
          "\n"
          "\tUsage:  TPUT [filename] <BIN/ASCII>\n"
          "\n"
          "  This command sends a file from the local system to the server. The file transfer is\n"
          "  performed using ASCII characters by default, or if the \"ASCII\" option is appended\n"
          "  after the filename. If the \"BIN\" option is appended after the filename, then the\n"
          "  file transfer transfers binary data. The user must be connected and authenticated\n"
          "  with the server in order to execute this command.\n"
          "\n"
        );
        break;
      }
        
      case TGET:{
        printf(
          "\n"
          "  Retrieve a remote file from the server\n"
          "\n"
          "\tUsage:  TGET [filename]\n"
          "\n"
          "  This command causes the client to request the download of a file from the server's\n"
          "  local filesystem. The client must be connected to and authenticated to the server\n"
          "  use this command. The filename may be a relative pathname or an absolute pathname.\n"
          "  The server will default the data format as a BINARY transfer to ensure data transfers\n"
          "  regardless of the original file's format.\n"
          "\n"
        );
        break;
      }
      
      case TLIST:{
        printf(
          "\n"
          "  List the contents of the server's current working directory\n"
          "\n"
          "\tUsage:  TLIST\n"
          "\n"
          "  This command queries the server for a listing of its current working directory.\n"
          "  The server shall respond with the listing of the directory using a standard \"ls\"\n"
          "  or \"ls -al\" type output (develoeprs choice). The user must be logged in and\n"
          "  authenticated in order for this command to function.\n"
          "\n"
        );
        break;
      }

      case TCWD:{
        printf(
          "\n"
          "  Change server's working directory\n"
          "\n"
          "\tUsage:  TCWD [dir]\n"
          "\n"
          "  This command changes the current working directory of the server. The DIR value\n"
          "  in the command may be an absolute or relative path. It may use the \".\",\"..\",\n"
          "  \"~\" relative paths. The server shall respond with a success or failure notification\n"
          "  \"TSUCCESS\" or \"TFAIL\" string message so the iuser is aware of the TCWD success.\n"
          "\n"
        );
        break;
      }

      default:{
        // Top level help - i.e. usage
        printf(
          "\n"
          "TigerFTP Interface Help / Usage: \n"
          "\n"
          "  Choose from the following help commands.\n"
          "\n"
          "\tTCONNECT\n"
          "\tTGET\n"
          "\tTPUT\n"
          "\tTLIST\n"
          "\tTCWD\n"
          "\n"
        );
        break;
      }
    }
    
  }
  else {
    // Top level help - i.e. usage
    printf(
      "\n"
      " **** NO SECOND TOKEN ****\n\n"
      "TigerFTP Interface Help / Usage: \n"
      "\n"
      "  Choose from the following help commands.\n"
      "\n"
      "\tTCONNECT\n"
      "\tTGET\n"
      "\tTPUT\n"
      "\tTLIST\n"
      "\tTCWD\n"
      "\n"
    );
  }
}

bool ProcessCommandString( UserInputBuffer* inbuf ) {
  bool validCommand = false;
  bool isHelp = false;

  Token_s* first = GetNextCommandToken( inbuf );
  TokenType_e type = first->type;
  
  switch ( type ) 
  {
    case HELP:
      validCommand = true;
      isHelp = true;
      break;
      
    case UNKNOWN:
      printf("\nERROR: UNKNOWN first command token: %u\n\n", type);
      break;
      
    case TERMINATOR:
      printf("\nERROR: TERMINATOR as first command\n\n");
      break;
      
    case EXIT:
      validCommand = true;
      break;
      
    default:
    {
      switch ( authState ) 
      {
        case PROC_LOGIN:
        {
          if ( type == TCONNECT )
            validCommand = true;

          break;
        }
        
        case PROC_AUTHENTICATED:
        {
          if (( type == TPUT ) ||
              ( type == TGET ) ||
              ( type == TLIST ) ||
              ( type == TCWD ))
          {
            validCommand = true;
          }
          break;
        }
        
        default:
        {
          printf("\nERROR: Invalid FSM state: %d\n\n", authState );
          break;
        }
      }
    }
  }

  
  list.push_back( first );
  
  while ( inbuf->getIndex < inbuf->putIndex ) 
  {
    Token_s* temp = GetNextCommandToken( inbuf );  
    
    type = temp->type;
//    printf("\t\tToken type: %d\n", type);
    list.push_back( temp );
  }

  bool programDone = false;
  
  if ( list.size() > 0 ) {
    if ( isHelp ) 
      ProcessHelpCommand();
    else {
      if ( validCommand )
        programDone = ProcessCommand();
    }
  }
  
  list.clear();
  
  return programDone;
}

void ProceessLoop() {
  UserInputBuffer *buf = &userInputBuf;
  char* command = buf->buf;
  
  setvbuf( stdin, NULL, _IONBF, BUFSIZ );
  
  while(true)
  {
    switch ( authState ) 
    {
      case PROC_LOGIN:
        printf("TigerC> ");
        break;
        
      case PROC_AUTHENTICATED:
        printf("Connected> ");
        break;
        
      default:
        printf("ERROR - Invalid ASCII FSM state\n");
        exit(20);
        break;
    }
    
    bool cmdComplete = false;
    
    do 
    {
      char c = getc( stdin );
        
      switch(c)
      {
        case EOF:
        {
          // If the STDIN input stream is closed, you need to perform an 
          // emergency shutdown. Don't forget to close existing sessions.
          EmergencyShutdown();
          return;
        }

        case '\r':
        case '\n':
        {        
          command[buf->putIndex] = '\0';
          
          // If the Processing of the Command tokens returns an EXIT
          // token, end the application
          if ( ProcessCommandString( buf ) ) 
          {
            EmergencyShutdown();
            return;
          }

          // Reset the PUT pointer for future line input
          buf->putIndex = 0;
          buf->getIndex = 0;
          cmdComplete = true;
          buf->Clear();
          break;
        }
        
        case '\b':
        {
          printf("hit backspace\n");
          buf->putIndex--;
          command[buf->putIndex] = '\0';
          break;
        }
        
        default:
        {
          // Only use printable ASCII characters
          if(isprint(c))
          {
              if( buf->putIndex < MAX_CMD_LENGTH )
              {
                command[buf->putIndex++] = c;
                command[buf->putIndex]   = '\0';

                if( false )    // echo - was old variable used
                  printf(":: %c", c);

              }
          }
          else
          {
              printf("ERR: non-print char encountered: %u decimal, 0x%02x \n",
                      (unsigned char)c, (unsigned char)c );
          }
          break;
        }
      }
    } while ( !cmdComplete );
    
  }
}

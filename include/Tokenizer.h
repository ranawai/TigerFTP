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
 * This headerfile comntains the C++ contructs, enumerations, and declares
 * functions shared across the lexer.cpp and processor.cpp files.
 * 
 * Do not change this file unless you really know what you're doing.  You 
 * don't need to include this file in your main.cpp or main.c file.
 */

#ifndef  Tokenizer_H
#define  Tokenizer_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#define YYSKIP()      
#define YYPEEK()      *(p = &( command->buf[ command->getIndex++ ] ));
#define YYGETCONDITION() (currentCondition)
#define YYSETCONDITION(c) (currentCondition = yyc ## c)
#define YYMAXFILL 1

#define MAX_CMD_SIZE    1024
#define MAX_CMD_LENGTH  1023    // Used to avoid writing \0 to invalid memory at end of buf

enum FtpStates_e
{
   PROC_LOGIN, 
   PROC_AUTHENTICATED, 
   NUM_STATES
};

static const size_t SIZE = 4096;

struct UserInputBuffer
{
   UserInputBuffer() : putIndex(0), getIndex(0)
   {
      memset(buf, 0, sizeof(buf));
   }
   
   void Clear() { memset( buf, 0, sizeof(buf)); putIndex = 0; getIndex = 0; };
   unsigned putIndex;
   unsigned getIndex;
   char buf[MAX_CMD_SIZE];
};

//#define YYGETSTATE()  in.state
//#define YYSETSTATE(s) in.state = s
//#define YYFILL(n)     do { in.need = n; return NEED_MORE_INPUT; } while (0)
//#define YYLESSTHAN(n) (in.need < n)
#define YYBACKUP()
#define YYRESTORE()

enum YYCONDTYPE {
  yyc0,
  yycCOMMAND,
  yycSTRING_START,
  yycSTRING_FINISH,
};

extern YYCONDTYPE currentCondition;

enum TokenType_e             
{
  // GLOBAL commands
  UNKNOWN            = 0,
  TERMINATOR         = 1,
  EXIT               = 2,   // EOL, Terminator
  HELP               = 45,  // Not really needed

  // TOP LEVEL commands
  TCONNECT           = 3,   // tconnect [IPv4] [username] [password]
  TPUT               = 4,   // tput <filename>
  TGET               = 5,   // tget <filename>
  TLIST              = 6,   // tlist 
  TCWD               = 7,   // tcwd [dir]
  

  EOL                = 40,

  // Support Tokens
  SPACE              = 50,
  STRING             = 53,

  IP_ADDR            = 56,  
  UNSIGNED           = 57,  
  INTEGER            = 58,
  HEX_INT            = 59
};

struct Token_s
{
   Token_s(): type(UNKNOWN), start(0), end(0) {}

   std::string toString()
   {
      std::string val(start);
      return val.substr(0, (end - start));
   }

   const char *toChar()
   {
      return start;
   }

   TokenType_e type;
   char* start;
   char* end;
};

typedef std::vector<Token_s*> TokenList;

extern Token_s*  GetNextCommandToken( UserInputBuffer* command );
extern Token_s*  GetParameterToken( UserInputBuffer* command );

#endif // Tokenizer_H


#include <stdio.h>
#include "Tokenizer.h"

Token_s*  GetNextCommandToken( UserInputBuffer* command )
{
   char *p = &( command->buf[ command->getIndex ] );
   char *m;
   char *start;
   Token_s *token = new Token_s;

   while( 1 )
   {
      start = p;

      /*!re2c
         re2c:define:YYCTYPE  = "unsigned char";
         re2c:define:YYCURSOR = p;
         re2c:define:YYMARKER = m;
         re2c:yyfill:enable   = 0;
         re2c:yych:conversion = 1;
         re2c:indent:top      = 2;

      DIGIT     = [0-9] ;
      HEX_DIGIT = [0-9a-fA-F] ;
         
      /* Odd Balls.. */
      TAB                 = "\t"                  ;
      SPACE               = ( " " | TAB | "\v" )+ ;
      WHITESPACE          = ( " " | TAB | "\v" )* ;
      TERMINATOR          = [\000]                ;
      EOL                 = ( "\n" | "\r" )+;

      /* MUTATION operations (i.e. checks write level) */
      TCONNECT = WHITESPACE 'TCONNECT' ; /* case-insensitive matching */
      TGET     = WHITESPACE 'TGET'     ; /* case-insensitive matching */
      TPUT     = WHITESPACE 'TPUT'     ; /* case-insensitive matching */
      TLIST    = WHITESPACE 'TLIST'    ; /* case-insensitive matching */
      TCWD     = WHITESPACE 'TCWD'     ; /* case-insensitive matching */

      /* NON-MUTATION operations (i.e. checks read level) */
      HELP     = WHITESPACE 'HELP' ; /* case-insensitive matching */

            
      /* Exits the current config mode - needs validation of config level */
      EXIT     = WHITESPACE 'EXIT'       WHITESPACE TERMINATOR  ; /* case-insensitive matching */
      
      octet = [0-9] | [1-9][0-9] | [1][0-9][0-9] | [2][0-4][0-9] | [2][5][0-5];
      dot = [.];
      
      STRING   = [\.\/\\a-zA-Z_][\.\/\\a-zA-Z_0-9]*;
      
      IP_ADDR  = (octet) dot (octet) dot (octet) dot (octet) ;

      TCONNECT
      {
         token->type = TCONNECT;
         token->start = start; token->end = p;
         return token;
      }
      TGET
      {
         token->type = TGET;
         token->start = start; token->end = p;
         return token;
      }
      TPUT
      {
         token->type = TPUT;
         token->start = start; token->end = p;
         return token;
      }
      TLIST
      {
         token->type = TLIST;
         token->start = start; token->end = p;
         return token;
      }
      TCWD
      {
         token->type = TCWD;
         token->start = start; token->end = p;
         return token;
      }


      HELP
      {
         token->type = HELP;
         token->start = start; token->end = p-1;
         return token;
      }

      EXIT
      {
         token->type = EXIT;
         token->start = start; token->end = p;
         return token;
      }

      SPACE
      {
         token->type = SPACE;
         token->start = start; token->end = p;
         return token;
      }

      STRING
      {
         token->type = STRING;
         token->start = start; token->end = p;
         return token;
      }

      IP_ADDR
      {
         token->type = IP_ADDR;
         token->start = start; token->end = p;
         return token;
      }
      
      EOL
      {
         token->type = EOL;
         token->start = start; token->end = p;
         return token;
      }

      /* Terminating cases */
      TERMINATOR
      {
         token->type = TERMINATOR;
         token->start = start; token->end = p;
         return token;
      }
      [^]
      {
         token->type = UNKNOWN;
         token->start = start; token->end = p;
         return token;
      }

      */
   }

   return 0;
}

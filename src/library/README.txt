This folder contains the simple lexer and command line interface parser for the
TigerC application.  Note that we are not usiong termios.h.  Ergo, we perform 
all operations on the STDIN stream, which is a stream I/O rather than character 
driven I/O. This means that the command line is parsed after a <CR> is performed 
by the user, and we do not support any TAB-Completion operation.

To build the lexer, one must first install the latest version of RE2C.  I used 
ver. 1.0.1 as installed by KUbuntu 18.04.  To create the lexer.cppc code:


prompt\> re2c --input custom -o lexer.cpp CommandLex.re

NOTE: The re2c does not complete the .cpp file, and thus I had to modify some 
of the file. Ensure that the top of the GetNetCommandToken() looks like this....


  Token_s*  GetNextCommandToken( UserInputBuffer* command )
  {
    char *p = &( command->buf[ command->getIndex ] );
    char *m;
    char *start;
    Token_s *token = new Token_s;

  #define YYSKIP()                                                        <------
  #define YYPEEK()      *(p = &( command->buf[ command->getIndex++ ] ));  <------

To avoid constantly copy-pasting said lines into the lexer.cpp file, the lines 
are placed in the Tokenizer.h headerfile.


Once the lexer code is generated, we can then create the application using the 
top level Makefile.



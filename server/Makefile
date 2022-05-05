###########################################
# the make file for tcp server and client #
# author: Tianran Cui                     #
###########################################

SERVER_SRC	= TigerS.c
HEADER		= ../include/ftpTiger.h 

EXE		= $(CLIENT_EXE) $(SERVER_EXE)
SERVER_EXE	= TigerS

all: $(CLIENT_EXE) $(SERVER_EXE)

clean:
	-rm -f $(EXE)

help: 
	@echo "make all    - compile tcp client and server"
	@echo "make clean  - delete the old executables" 
	@echo "make help   - read this again"
	@echo "make server - compile the server and print usage"

server: $(SERVER_EXE)
	@echo "<usage: the tcp server program"
	@echo ">./$(SERVER_EXE) <ip address>"
	@echo ""

$(SERVER_EXE): $(SERVER_SRC) $(HEADER)
	@echo "compiling the tcp server..."
	@gcc -pthread -std=c99 -Wall -Wextra -pedantic $(SERVER_SRC) -o $(SERVER_EXE)


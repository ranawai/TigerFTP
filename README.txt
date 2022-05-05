*** disclaimer, the string tokenizer will create segmentation fault
*** when user input: tconnect <ip>
*** without providing furthur info
*** please revisit and fix that int he future

This is a ftp server / client implemented using tcp.
(didn't say they are good.)
@author Tianran Cui

To compile the client, type in:
	make
in the root folder, then execute the program typing:
	./build/TigerC

To compile the server:
step1:	cd ./server
step2:	make
then the program can be executed in the root folder by:
	./server/TigerS <a valid ip address to start the server on>

Please read the documentation for more details. 

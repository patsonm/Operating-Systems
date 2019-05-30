/*Michael Patson 
**Oregon State
**homework 4

****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>

#define BUFFER_SIZE 1024
#define MESSAGE_SIZE 75000

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int getLength(const char* filename){
	int cINT;
	int count = 0;
	FILE* fileptr = fopen(filename, "r");

    while (1) {
        cINT = fgetc(fileptr);

        if (cINT == '\n')
            break;
		if(!isupper(cINT) && cINT != ' '){

			error("File contains illegal characters\n");
		}
        ++count;
    }
	fclose(fileptr);
	return count;
}

int main(int argc, char *argv[]) //where communication between client and server happens
{
	int socketFD, portNumber, charsSentCount, charsRecv, sizeRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[BUFFER_SIZE];
	char codedmessage[MESSAGE_SIZE];
    
    //if there are not enoguh args
	if (argc < 3) { fprintf(stderr, "Missing Arguments", argv[0]); exit(0); }
    
    
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address
    
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network socket
    
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); 
	if (serverHostInfo == NULL) { fprintf(stderr, "Should not get this if using local hsot\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
																										
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("Cannot open socket");
	int yes = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); //make socket reusable

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		error("Cannot connec to to socket");
	}

    //callfunction to find length in files
	int fileLength = getLength(argv[1]);
	int keyLength = getLength(argv[2]);

	if(fileLength > keyLength)
    {
		error("Key is too-short\n");
	}
    
    /*
    dont this obiouws
    if(keyLength > fileLength)
    {
		error("File is too short!\n");
	}
	*/
	
    //send name to server
	char* myName = "otp_enc";
	charsSentCount = send(socketFD, myName, strlen(myName), 0);
    
	memset(buffer, '\0', sizeof(buffer)); //send the length of file
	if (charsSentCount < 0) 
		error("Cannot write to socket");
	charsRecv = 0;
	while(charsRecv == 0)
        //read daata from socket, ingor trailig 0, from hw prompt 
		charsRecv = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
    
	if (charsRecv < 0) error("Cannot read from socket");

	if(strcmp(buffer, "N") == 0){
        
		fprintf(stderr, "ERROR:otp_enc cannot use otp_dec_d\n");
        //print error 2
		exit(2);
	}
    
    if(strcmp(buffer, "Y") != 0){
        
		fprintf(stderr, "ERROR:otp_enc cannot use otp_dec_d\n");
        //print error 2
		exit(2);
	}
    

	memset(buffer, '\0', sizeof(buffer)); //clear buffer
	sprintf(buffer, "%d", fileLength);

	charsSentCount = send(socketFD, buffer, sizeof(buffer), 0); //send the length of file
	memset(buffer, '\0', sizeof(buffer)); //clear buffer
	charsRecv = 0;
	while(charsRecv == 0)
		charsRecv = recv(socketFD, buffer, sizeof(buffer)-1, 0);

	if(strcmp(buffer, "C") == 0){
		int fd = open(argv[1], 'r');
		charsSentCount = 0;

		while(charsSentCount <= fileLength){
			memset(buffer, '\0', sizeof(buffer)); // Clear out for reuse
			sizeRead = read(fd, buffer, sizeof(buffer)-1);// Read data from the socket, leaving \0 at end
			charsSentCount = charsSentCount+send(socketFD, buffer, strlen(buffer), 0);
			
		}
        memset(buffer, '\0', sizeof(buffer)); //clear buffer
		fd = open(argv[2], 'r');
		charsSentCount = 0;
		while(charsSentCount <= fileLength){
			memset(buffer, '\0', sizeof(buffer)); // Clear out for reuse
			sizeRead = read(fd, buffer, sizeof(buffer)-1);
			charsSentCount = charsSentCount+ send(socketFD, buffer, strlen(buffer), 0);
			memset(buffer, '\0', sizeof(buffer)); //clear buffer
		}
	}
	memset(buffer, '\0', sizeof(buffer)); //clear buffer

	//for incoming message
	charsRecv = 0;
	int charsSent = 0;

	while(charsRecv < fileLength){
		memset(buffer, '\0', BUFFER_SIZE); //clear buffer
		charsSent = recv(socketFD, buffer, sizeof(buffer)-1, 0);// get message back, rember to cut off trailing 0
		charsRecv = charsRecv + charsSent;
		charsSent = 0;
		strcat(codedmessage, buffer);
		memset(buffer, '\0', BUFFER_SIZE); //clear buffer
	}
	strcat(codedmessage, "\n");
	printf("%s", codedmessage);
	close(socketFD); // Close the socket
	return 0;
}


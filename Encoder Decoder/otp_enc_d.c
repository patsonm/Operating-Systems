/*Michael Patson 
**Oregon State
**homework 4
****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

#define BUFFER_SIZE 1024
#define MESSAGE_SIZE 75000


//from description in hw assignment
int charToInt (char c){
	if (c == ' ')
    {
		return 26;
	}
	else 
    {
		return (c - 'A');
	}
	//return 0;
}

char intToChar(int i){
	if (i == 26)
    {
		return ' ';
	}
	else 
    {
		return (i + 'A');
	}
}


//newline ends plainMsg
void encode(char* plainMsg, char* key)
{ 
	int i;
	char c;
	for (i=0; plainMsg[i] != '\n'; i++)
    {
	  		c = (charToInt(plainMsg[i]) + charToInt(key[i])) % 27;
            //now convert ints back to chars for code
	  		plainMsg[i] = intToChar(c);
	  }
	  plainMsg[i] = '\0';
	  return;
}

int main(int argc, char *argv[]) //where communication between client and server happens
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, status;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid;

    //copied from class notes
    
	if (argc < 2) { fprintf(stderr, "Missing Arguments", argv[0]); exit(1); } 
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // clears address struct
	portNumber = atoi(argv[1]); // Get the port numb
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(portNumber); // Store the port number, convert to ls from ms
	serverAddress.sin_addr.s_addr = INADDR_ANY; 

    
    //create the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocketFD < 0) error("cannot open socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("cannot bind socket");
    
    //this makes an infinite loops waiting to receive, just like in class notes
	while(1)
    {
        //can run 5 connections, per class assignment
		listen(listenSocketFD, 5); 

        //get client ifo and open
        //what is happening, why are you stuck here
		sizeOfClientInfo = sizeof(clientAddress);
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
		if (establishedConnectionFD < 0) error("cannot accept");

		//lifted this from hw two ago
		pid = fork();
		switch (pid){
			case -1:
                {
				error("Danger Will Robinson!\n");
			}
			case 0:{
				char buffer[BUFFER_SIZE];
				char plainMsg[MESSAGE_SIZE];
				char key[MESSAGE_SIZE];
				int charsWritten = 0;
				memset(buffer, '\0', sizeof(buffer)); //clear buffer
				charsRead = 0;
				while(charsRead == 0)
					charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); 
                    //read plainMsg
				if (charsRead < 0) error("cannot read from ");
		
				if(strcmp(buffer, "otp_enc") != 0){
					charsRead = send(establishedConnectionFD, "N", 2, 0);
					exit(2);
				}
                if(strcmp(buffer, "otp_enc") == 0)
                //yes right code
                {
					memset(buffer, '\0', sizeof(buffer)); //clear buffer
					charsRead = send(establishedConnectionFD, "Y", 3, 0); //send any resonse to say not sae file
					charsRead = 0;
					while(charsRead == 0)
                    {
						charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //leave of trailing 0
					}
					int size = atoi(buffer);

					charsRead = send(establishedConnectionFD, "C", 4, 0);//continue
					charsRead = 0;
					int charsSent = 0;
                    
					while(charsRead < size){
						memset(buffer, '\0', sizeof(buffer)); //clear buffer
						charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);//remember to cut off trail
						charsRead = charsRead + charsSent;
						charsSent = 0;
						strcat(plainMsg, buffer);
				
					}
				    //clear buff
                    memset(buffer, '\0', sizeof(buffer)); //clear buffer
					charsRead = 0;
					charsSent = 0;
					
                    
					while(charsRead < size){
						memset(buffer, '\0', sizeof(buffer)); //clear buffer
						charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0);
						charsRead = charsRead+charsSent;
						charsSent = 0;
						strcat(key, buffer);
						 
					}
                    //clearbuff
                    memset(buffer, '\0', sizeof(buffer)); //clear buffer
					
                    //actually ecnode
					encode(plainMsg, key);
								
                    
					charsWritten = 0;
					while(charsWritten < size){
						memset(buffer, '\0', sizeof(buffer)); //clear buffer
						charsWritten = charsWritten + send(establishedConnectionFD, plainMsg, sizeof(plainMsg), 0);
						memset(buffer, '\0', sizeof(buffer)); //clear buffer					
					}	

					exit(0);

				}
			}
			default:{
				pid_t actualpid = waitpid(pid, &status, WNOHANG);
			}
		}
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}

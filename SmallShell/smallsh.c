/*****************************
Filename: smallsh.c
Michael patson
Oregon State 2018 CS344
Assignment 3: Create a smal shell with basic functions
*******************************/



#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>



//set constants, value taken from HW prompt

#define MAX_ARGS 512
#define MAX_LENGTH 2048



int runInBackGround = 1;


void getInput(char* arr[], int* backgroundProc, char inputCommand[], char outputText[], int pid) {
	
	char input[MAX_LENGTH];
	

	// print colon and get input
	printf(": ");
    
    //flush stout, remember error you were getting
	fflush(stdout);
	fgets(input, MAX_LENGTH, stdin);

    
    // Remove the new line token
    //as always... i look
    //https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
	int command = 0;
    int i;
	for (i=0; !command && i< MAX_LENGTH; i++) 
    {
		if (input[i] == '\n') 
        {
			input[i] = '\0';
            //sets bool to stop looking
            
			command = 1;
		}
	}

	//if there is no text, compare it to no text and end
	if (!strcmp(input, "")) 
    {
		arr[0] = strdup("");
		return;
	}

	//if there is more than one part of the string, seperated by a space, we need to split that up.  we did something similar to this in 165, but i couldnt find my code.
    //did have this reference saved
    //http://www.cplusplus.com/reference/cstring/strtok/
    
    
	char *pch = strtok(input, " ");
    int x, y;

	for (x=0; pch; x++)
    {
		//if running in background
		if (!strcmp(pch, "&")) 
        {
			*backgroundProc = 1;
		}
		//if this is an inputfile
		else if (!strcmp(pch, "<"))
        {
			pch = strtok(NULL, " ");
			strcpy(inputCommand, pch);
		}
		//if this is an outputfile
		else if (!strcmp(pch, ">")) 
        {
			pch = strtok(NULL, " ");
			strcpy(outputText, pch);
		}
		//if it is not one of the above indicators, might be part of command
		else {
            //need to replace the $$ with the PID
			arr[x] = strdup(pch);
            
			for (y=0; arr[x][y]; y++) {
				if (arr[x][y] == '$' && arr[x][y+1] == '$') 
                {
					arr[x][y] = '\0';
					snprintf(arr[x], 256, "%s%d", arr[x], pid);
				}
			}
		}
		
		pch = strtok(NULL, " ");
	}
}



void modifiedMainFromLec(char* arr[], int* newProcExitStatus, struct sigaction signal, int* backgroundProc, char inputArray[], char outputArray[])
{
    
    //this is the setup from minute 10 of video 3.1, about forking
    
    pid_t spawnPid = -5;
    spawnPid = fork();
    
    int inputInt, duplicate, outputInt;
    switch (spawnPid)
    {
        case -1:
            perror("Danger Will Robinson!!!\n");
            exit(1);//exits with error
            break;
        
            
        //Tried this part before 3.4 lecture... yikes
        case 0:
            signal.sa_handler = SIG_DFL;//sets default behavior
            sigaction(SIGINT, &signal, NULL);
            
       
            if (strcmp(inputArray, ""))
            {
                ///int inputInt;
                inputInt= open(inputArray, O_RDONLY);
                    if (inputInt == -1)
                    {
                        perror("Cannot open file \n");
                        exit(1);
                    }
                    //duplicate file header
                    //int duplicate;
                    duplicate = dup2(inputInt, 0);
                
                    //need to set to negative one if error, piazza forums helped. cannot remmeber which  
                    if (duplicate == -1) 
                    {
                        
                        //getting stuck here. w
                        perror("Cannot duplicate file descriptor stuck\n");
                        exit(1);
                    }
                //from lecture min 43
                fcntl(inputInt, F_SETFD, FD_CLOEXEC);
                
            }
            
            	if (strcmp(outputArray, "")) {
				// open it
				outputInt = open(outputArray, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    //all have rw only (6)
                    
				if (outputInt == -1)
                {
					perror("Cannot open file\n");
					exit(1);
				}
                    
				
                    
				duplicate = dup2(outputInt, 1);
				if (duplicate == -1) 
                {
					perror("Cannot duplicate file descriptor2\n");
					exit(1);
				}
                
				//close, same as input
				fcntl(outputInt, F_SETFD, FD_CLOEXEC);
			}
			
                // why is the char* demands it 
			if (execvp(arr[0], (char* const*)arr)) {
				printf("%s: no file or directy is found\n", arr[0]);
				fflush(stdout);
				exit(1);
			}
			break;
        default:
            if (*backgroundProc && runInBackGround)
            //will run only if bool value is set to allow to run in background
            {
				pid_t newPid = waitpid(spawnPid, newProcExitStatus, WNOHANG);
				printf("background pid is %d\n", spawnPid);
				fflush(stdout);
			}

			else 
            {
				pid_t newPid = waitpid(spawnPid, newProcExitStatus, 0);
			}

		// the reutn status must be greater than 0, otherwise failure somewhere else
        //must be greater thatn 0
            
            //print status
		while ((spawnPid = waitpid(-1, newProcExitStatus, WNOHANG)) > 0) 
        {
			printf("Child %d terminated\n", spawnPid);
			
            {
	
            if (WIFEXITED(newProcExitStatus)) 
            {
                printf("exit value %d\n", WEXITSTATUS(newProcExitStatus));
            } else 
            {
                printf("terminated by signal %d\n", WTERMSIG(newProcExitStatus));
            }
            }

			fflush(stdout);
		}
    }
}



/*The below function is in the outline for hw, it helps allow the program in to run in the background
*/
void STPSIGTSTP(int signo) {

	// If it's 1, set it to 0 and display a message reentrantly
	if (runInBackGround == 1) {
		char* output = "Entering foreground-only mode (& is now ignored)\n";
		write(1, output, 49);
		fflush(stdout);
		runInBackGround = 0;
	}

	// If it's 0, set it to 1 and display a message reentrantly
	else {
		char* output = "Exiting foreground-only mode\n";
		write (1, output, 29);
		fflush(stdout);
		runInBackGround = 1;
	}
}




/***************
The main function does the following
Reads input and directs input from termial from terminal
*****************/

int main() {

    //call getPid funtion and set it equal to pid
    int pid = getpid();

    
    int exitStatus = 0;// no error
    //set boolean values
    int backgroundProc = 0;

    //set the files to blanks
    char inputFile[256] = "";
    char outputFile[256] = "";
    char* input[MAX_ARGS];
    
    //create an array to hold all of the args, like regularsh
    int i;
	for (i=0; i < MAX_ARGS; i++) 
    {
		input[i] = NULL;
	}

	
	// Ignore ^C
    // referenced http://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html
    
    //copied style from https://oregonstate.instructure.com/courses/1692912/pages/3-dot-3-advanced-user-input-with-getline
    
	struct sigaction sa_IGN_C = {0};
	sa_IGN_C.sa_handler = SIG_IGN;
	sigfillset(&sa_IGN_C.sa_mask);
	sa_IGN_C.sa_flags = 0;
	sigaction(SIGINT, &sa_IGN_C, NULL);

	// change ^Z behavior
    //referenced
    //https://stackoverflow.com/questions/42449124/can-successfully-trap-ctrl-z-but-not-sigtstp
    //
    
	struct sigaction sigstop_Z = {0};
	sigstop_Z.sa_handler = STPSIGTSTP;
	sigfillset(&sigstop_Z.sa_mask);
	sigstop_Z.sa_flags = 0;
	sigaction(SIGTSTP, &sigstop_Z, NULL);

    //this keeps the input looping, until no longer true
    //checks for commands and redirects them to appropriate function
    
    int loop = 1;
    
	do {
		//Reads using input
		getInput(input, &backgroundProc, inputFile, outputFile, pid);

		//will ignore # think it is a comment and blank input, inducated by EOL \0
		
        if (input[0][0] == '#') 
        {
            
            int i;
		      for (i=0; input[i]; i++) 
            {
              input[i] = NULL;
		      }
			continue;
		}
		
        else if (input[0][0] == '\0')
        {
            continue;
        }
		// exits if string exit is read
        //must be lower case
		else if (strcmp(input[0], "exit") == 0) 
        {
			loop = 0; //exit loop
		}
	
		// changed directy if string cd is detected
        // must be lower case
        
		else if (strcmp(input[0], "cd") == 0) 
        {
			// Change to the directory specified
			if (input[1]) 
            {
				if (chdir(input[1]) == -1) 
                {
					printf("No Directory Found.\n");
					fflush(stdout);
				}
			} 
            else 
            {
			//go to home if not specified (like regular CD)
				chdir(getenv("HOME"));
			}
		}

		//calls print status if string reads status,
        //remember must be lower case
		else if (strcmp(input[0], "status") == 0) 
        {
			if (WIFEXITED(exitStatus)) 
            {
                printf("exit value %d\n", WEXITSTATUS(exitStatus));
            } else 
            {
                printf("terminated by signal %d\n", WTERMSIG(exitStatus));
            }
        }
		

        //else it must be another command
		else 
        {
			modifiedMainFromLec(input, &exitStatus, sa_IGN_C, &backgroundProc, inputFile, outputFile);
		}

		// repeats loop to reset all the arg variables
        int i;
		for (i=0; input[i]; i++) 
        {
			input[i] = NULL;
		}
		
        backgroundProc = 0;
        
        
		inputFile[0] = '\0';
		outputFile[0] = '\0';

	} while (loop);
	
	return 0;
}

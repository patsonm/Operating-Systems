/***

keygen: This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard UNIX randomization methods. Do not create spaces every five characters, as has been historically done. Note that you specifically do not have to do any fancy random number generation: we’re not looking for cryptographically secure random number generation! rand() is just fine. The last character keygen outputs should be a newline. All error text must be output to stderr, if any.

****/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>



void validArgc(int argc)
{
    if(argc == 1){
        fprintf(stderr,"KeyGen: Enter key length. Need additional info \n");
        exit(1);
    }
}


int main(int argc, char * argv[]){
    
    //validate or terminate
    validArgc(argc);
    
    //read key from arg
    int keyLength = atoi(argv[1]);
    
    //from prompt, srand is ok to use
    srand(time(NULL));
    
    
    
    int i;
    int key;
    
    
    //key generation
    
    for(i=0; i < keyLength; i++)
    {
    
        //from prompt
        key = rand() % 27;


        if(key != 26)
        {
            printf("%c", 'A'+(char)key);
        }
        else
        {
            // if it is 26...space
            printf(" ");
        }
    }
   //print new line, from prompt
    printf("\n");
    return 0;
}
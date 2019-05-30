/************************
**Name: Michael Patson
**Assignment 2 OSU CS344
**Oct-19-2018
************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>


#define STRING_ROOM_MAX 8
#define ROOM_COUNT 7
#define MAX_ROOM 10
#define MAX_CONNECTION 6
#define BUFFER_SIZE 512


//in th first part i set these with an array, for analayis and file reading,
//enum was suggested on piazza and is much easier.
enum ROOM_TYPES{START_ROOM, MID_ROOM, END_ROOM};

//define room structure
//this is the same as the .buildroom
struct Room 
{
    int actualConnections;
    struct Room* roomConnect[MAX_CONNECTION];
    enum ROOM_TYPES typeROOM;
    char roomName[STRING_ROOM_MAX];
};

//global variable, foldername 
char *currentTime = "currentTime.txt";
char FolderName[BUFFER_SIZE];
struct Room RoomList[ROOM_COUNT];


//http://pubs.opengroup.org/onlinepubs/000095399/functions/readdir_r.html
//This functiong sets the folderNamef 
void setFolder()
{
    time_t time;
    DIR* directory = opendir(".");
    struct dirent* dp;
    struct stat statbuffer;
    memset(FolderName, '\0', sizeof(FolderName));
    int i = 0;
    //uses time to check for the most recent folder

    if(directory != NULL)
    {
        while(dp = readdir(directory))
        {
            if(stat(dp->d_name, &statbuffer) == 0 && S_ISDIR(statbuffer.st_mode) && strncmp(dp->d_name, "patsonm.rooms", 12)==0) 
            {
                time = statbuffer.st_mtime;
                if(time > i)
                {
                    strcpy(FolderName, dp->d_name);
                    i = time;
                }
            }
        }
            closedir(directory);                      
    }
}





//returns the index of the room in roomList, based on a name
int RoomIndex(char *name)
{
    int i;

    for(i = 0;i < ROOM_COUNT; i++ ){
        if( strcmp(RoomList[i].roomName, name) == 0 )
        { 
            return i;
        }
    }
    return -1; // error
}


//This breaks the string at the colon and cuts the leading space. 
//also truncates the line string
void BreakString(char *Line, char *Val)
{

    int i;
    strtok(Line, ":");
    strcpy(Val, strtok(NULL, ""));
    Line[strlen(Line) - 1] = '\0';
    Val[strlen(Val) - 1 ] = '\0';
    

    
    //cut leading whitespace, remeber what this does to line(shortens)
    for(i = 0; i < strlen(Val); i++)
    {
        Val[i] = Val[i+1];
    }
}



//creats defailt room structure array
//set connects to null and actual connections to 0
//copy name from in file
void CreateRoomStruct()
{
    DIR *directory;
    struct dirent *ent;
//creats defailt room structure array
//set connects to null and actual connections to 0
    int i;
    for(i = 0;i < ROOM_COUNT;i++)
    {
        memset(RoomList[i].roomName, '\0', sizeof(RoomList[i].roomName)); //clears anyhting elese in array
        RoomList[i].actualConnections = 0; 
        int j;
        for(j = 0; j < MAX_CONNECTION; j++)
        {
            RoomList[i].roomConnect[j] = NULL;
        }
    }
    
    //copy name from in file
    int x;
    if ((directory = opendir (FolderName)) != NULL) {

        while ((ent = readdir (directory)) != NULL) {

            if(strlen(ent->d_name) > 2)
            {
                strcpy(RoomList[x].roomName,ent->d_name);
                x++;
            }
        }
    }
}

//CreateRoomsFromFiles
//Creates a fole roomlist, loops thru and grabs the room names
//sets the room type

void CreateRoomsFromFile()
{
    char Line[50];
    char Value[50];

    FILE *RoomListFile;
    int i;

    CreateRoomStruct();
    chdir(FolderName); // return to directory

    //copy list into RoomListFile
    for(i = 0; i < ROOM_COUNT; i++){
        RoomListFile = fopen(RoomList[i].roomName, "r");


        memset(Line, '\0', sizeof(Line)); //from the lecture slides, good idea to clear, i was getting garbage with shorter room
        memset(Value, '\0', sizeof(Value)); //from the lecture slides, good idea to clear, i was getting garbage with shorter room

        // get each line from the file.
        while(fgets(Line, sizeof(Line), RoomListFile) != NULL){

            BreakString(Line,Value);
             //printf("%s\n",Line);
            //printf("%s\n",Value);
            
            //cant figure out why i am cutting type,  knwo it has to do with white sapce
            if(strcmp(Line, "ROOM TYP") == 0)
            { 

                if(strcmp(Value, "START_ROOM") == 0)
                {
                    RoomList[i].typeROOM = START_ROOM;
                }
                else if(strcmp(Value, "END_ROOM") == 0)
                {
                    RoomList[i].typeROOM = END_ROOM;
                }
                else if(strcmp(Value, "MID_ROOM") == 0)
                {
                    RoomList[i].typeROOM = MID_ROOM;
                }
            }
            else if(strcmp(Line,"CONNECTION ") == 0)
            {
                //this will loop thru and get the connection for each
                
                int connectRoomIndex = RoomIndex(Value);
                RoomList[i].roomConnect[RoomList[i].actualConnections] = &RoomList[connectRoomIndex];
                RoomList[i].actualConnections++;
            }
        }
        fclose(RoomListFile);
    }
    //we need to get up out of this directory for writing the time file, 
    //if not here, it will appear int he room folder
    chdir("..");
    
}

int StartRoomIndex(){
    int i;
    for(i =0;i< ROOM_COUNT; i++)
    {
        if(RoomList[i].typeROOM == START_ROOM){ 
            return i; // return room position.
        }
    }
    return -1; //error if cant find
}

//takes an array of ints that reprents the roomlist[]num, print out the name
void printRoute(int *route,int steps)
{
    //skips starting room, per directions
    int x;
    for(x = 1; x < steps + 1; x++)
    {
        printf("%s",RoomList[route[x]].roomName);
        printf("\n");
    }
}


//used https://linux.die.net/man/3/strftime for time info
//creats updates checktimetext
void* CreateCurrentTimeFile()
{
    char timeString[BUFFER_SIZE];
    time_t curTime;
    struct tm * timer;
    FILE *TimeFile;

    memset(timeString, '\0', sizeof(BUFFER_SIZE)); //from the lecture slides, good idea to clear, i was getting garbage with shorter room

    time(&curTime);
    timer = localtime(&curTime);
    
    strftime(timeString, BUFFER_SIZE, "%I:%M%P %A, %B %d, %Y", timer); 
    
    //make sure use w, not r
    TimeFile = fopen(currentTime,"w");
    fprintf(TimeFile,"%s\n",timeString);
    fclose(TimeFile);

    return NULL;
}
//reacds the created file
void ReadCurrentTimeFile()
{
    char buffer[BUFFER_SIZE];
    FILE *TimeFile;

    memset(buffer, '\0', sizeof(BUFFER_SIZE)); //clears anyhting elese in array

    TimeFile = fopen(currentTime, "r");
    if(TimeFile == NULL)
    {
        printf("%s was not found.\n", currentTime);
        return;
    }

    while(fgets(buffer, BUFFER_SIZE, TimeFile) != NULL)
    {
        printf("%s\n", buffer);
    }
    fclose(TimeFile);
}



//Run game makes a struct of the current room
//
void playGame()
{
    int curStepCount = 0;
    int stepString[100];
    int curRoomIndex;
    
    struct Room curRoomStruct;
    char InputBuffer[BUFFER_SIZE];

    //calls a function that loops thru rooms to look for strt room, set it to first step
    stepString[curStepCount] = StartRoomIndex();
    
    do{
       
        curRoomIndex = stepString[curStepCount];
        curRoomStruct = RoomList[curRoomIndex];
         printf("\n");
        printf("CURRENT LOCATION: %s\n", curRoomStruct.roomName);

        printf("POSSIBLE CONNECTIONS:");
        //loop thru connections and print out name
        
        int i;
        for(i = 0; i < curRoomStruct.actualConnections - 1; i++)
        {
            printf(" %s,", curRoomStruct.roomConnect[i]->roomName);
        }
        printf(" %s.", curRoomStruct.roomConnect[i]->roomName);
        printf("\n");
        
        memset(InputBuffer, '\0', sizeof(InputBuffer));//clears input buffer//was getting some STrange results with know names, had to do with shorter names in the array, like NYC for examoke.
        
        
        printf("WHERE TO? >");
        scanf("%511s" ,InputBuffer);
        printf("\n");

        int found = 0;//will reset with each new search
        //check if input matches the name of a room.
        for(i = 0; i < curRoomStruct.actualConnections; i++)
        {
            if(strcmp(InputBuffer, curRoomStruct.roomConnect[i]->roomName) == 0)
            { 
                
                curStepCount++;
                found=1;//set found equal to 1 to avoid iff statement "huh i dont know"
                stepString[curStepCount] = RoomIndex(InputBuffer);//if success on finding room in list, add room as next step in array
                curRoomIndex = stepString[curStepCount];
                curRoomStruct = RoomList[curRoomIndex];//set the new room as the found room
                
                if(curRoomStruct.typeROOM == END_ROOM)
                {
                    printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
                    
                    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", curStepCount);
                    printRoute(stepString,curStepCount);
                    printf("\n");
                    return;
                }
            }
        
        }


        if(strcmp(InputBuffer, "time") == 0)
        {
            CreateCurrentTimeFile();
            ReadCurrentTimeFile();    
        }
         //mismatched room
        else if (found==0)
        {
            printf("HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
            printf("\n");
        }
        
       
       
    }
    while(1);
}


int main(void)
{
    setFolder();//finds most recent folder
    CreateRoomsFromFile();    //creates the roomlist array, which is an array of rooms, gets infor from found folder
    playGame();//loops until rooms is found
    return 0;
}
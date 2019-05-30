/************************
**Name: Michael Patson
**Assignment 2 OSU CS344
**Oct-19-2018
************************/

#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>


//game Rules


#define ROOM_COUNT 7
#define MAX_ROOM 10
#define MIN_CONNECTION 3
#define MAX_CONNECTION 6

struct Room
{
    char* roomName;
    char* typeROOM; //mid last first
    int roomConnections[MAX_CONNECTION];
    int actualConnections; // will generate all connections and then pick some    
};


char * roomListNames[MAX_ROOM] = 
    {
    "Roseland", "Austin", "NYC", "Tulsa", "Denver", "Wayne", "Midland", "Manayunk", "Chatham", "Essex"
    };

char * roomType[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};

char foldName[50];

//make folder to how specis, set permission
char* makeRoomFolder()
{  
  sprintf(foldName, "patsonm.rooms.%d", getpid()); 
  mkdir(foldName, 0770); 
  
        return foldName; 
}

//make rooms, we need to assign a room name, connections, and type of room
//so first make thr oom and count set default values
void makeRoom(struct Room madeRoom[ROOM_COUNT])
{
    int z;
    for (z = 0; z < ROOM_COUNT; z++)
    {
        madeRoom[z].actualConnections=0;//deafault room connection is 0   
        
    }
    
  //this is time complex for sure, buy with 7 rooms not sure it matters.
    //generate a random number between 0-9 which will get a name from the roomListNames.
    //if the room is in the list already, break out and move on
    int x;
    for (x = 0; x < ROOM_COUNT; x++)
    {
        int found = 1;
        int index;// to hold the room id
        
        while (found==1)
        {
            index = rand() % MAX_ROOM;
            
            found = 0; //this will break loop if we cannot find it
            
            int y;
            for (y = 0; y < ROOM_COUNT; y++)
            {
                if (madeRoom[y].roomName==roomListNames[index])
                {
                    found=1; //already in list, generate a new room "seed"
                }
            }
            
        }
        
        
        
        //room was not ofund in list already, now we can add a it to our new list
        
        madeRoom[x].roomName = roomListNames[index];
        
        
        
        //made the first room the start type
        
        //make last room the last type
        
        //make all other rooms, the mid type
        
        
        if (x==0)
        {
            madeRoom[x].typeROOM  = roomType[0];      //first room type  
        }
        else if (x==6)
        {
            madeRoom[x].typeROOM  = roomType[1]; //last room type
        }
        else
        {
            madeRoom[x].typeROOM  = roomType[2]; //these are the middle rooms
        } 
        
        int thisRoomConnection = rand() % (MAX_CONNECTION + 1 - MIN_CONNECTION) + MIN_CONNECTION; //from cs165 to give between 3-6 connectections
         
        while (madeRoom[x].actualConnections < thisRoomConnection)
        {
           
            int randConnection;
            int found = 1;
            
            
            //same logic as checking for room name
              while(found==1)
              {               
                found = 0;
                randConnection = rand() % ROOM_COUNT;
                if(randConnection == x)
                {   
                    found = 1;
                }
                //check to see if the room is in the list
                  int i;
                for(i = 0; i <madeRoom[x].actualConnections; i++)
                {
                    if(madeRoom[x].roomConnections[i] == randConnection)
                    {
                      found = 1;
                    }
                }
            }
            
            //now add the connection, the room adding to and the room being conencted each  
            //need to be increase for total connection count
            
            
            
            madeRoom[x].roomConnections[madeRoom[x].actualConnections]=randConnection;
            
            madeRoom[randConnection].roomConnections[madeRoom[randConnection].actualConnections]=x;
            
            madeRoom[x].actualConnections++;
            
            madeRoom[randConnection].actualConnections++;
              
        }
        
      //end of room make  
    }   
}
    
void saveFiles(struct Room rooms[], char* currfolder)
{
  
    chdir(currfolder);
    int i;
    for(i = 0; i < ROOM_COUNT; i++)
    {
        //remember -a Appends to a file. Writing operations, append data at the end of the file. The file is created if it does not exist.
        FILE* myFile = fopen(rooms[i].roomName, "a");
       
        //print room name
        fprintf(myFile, "ROOM NAME: %s\n", rooms[i].roomName);
        int j;
        for(j = 1; j < rooms[i].actualConnections; j++)
        {
            //list connections
            fprintf(myFile, "CONNECTION %d: %s\n", j, rooms[rooms[i].roomConnections[j-1]].roomName);
        }
        //prnt room tyoe
        fprintf(myFile, "ROOM TYPE: %s\n", rooms[i].typeROOM);
        //close file
        fclose(myFile);  
    }
}

int main()
{
    srand(time(NULL));                //Seed the randomization
    char* roomsFolderSave = makeRoomFolder();
    struct Room rooms[ROOM_COUNT];
    makeRoom(rooms);             //Create the rooms

    saveFiles(rooms, roomsFolderSave); //Write athe files in the folder

        return 0;  
}


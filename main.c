/* 
 * File:   main.c
 * Author: griffin fujioka
 * ID: 11044124
 * Course: CptS 360 - Systems Programming I 
 * Project: EXT2 File System Simulator 
 * Created on March 28, 2012, 11:02 AM
 * 
 * Disclaimer: This was an available partner project and I previously worked
 * with Matt Karcher and submitted it along with the culminating interview. 
 * Afterwards, still feeling crappy about my understanding, I went back and did 
 * the entire project by myself however parts of it will reflect work that was
 * done in the previously submitted partner-version. 
 */

#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "Global.c"
#include "functions.h"
#include "init.h"

/* ===========================================================================
 * The global variable N will be used in a for-loop in the findCmd function 
  where N represents the number of an entry in the command table. This way it
   it is easy to add and index new commands in the command table 
 =========================================================================*/
#define N 14  

char pathname[64]; 
char device[256]; 
char* name[16];                         // stores pathname components 
char* argArray[20];                     // stores the arguments 
char lineBuffer[256];                   // store the (raw) user inputed line 
char command[16];                       // store the command 


/* -----------------------------------------------
 * parseArgs: 
 *      Parameter: the entire line read in by fgets
 *              in the form of "command arg1 arg2 .... argn"
 * 
 */
int parseArgs(char* theLine)
{
    int i=0; 
    char* temp; 
    char lineCopy[256]; 
    
 
    clearCommandBuff(); 
    strncpy(lineCopy, theLine, 256);    // make a copy of the line 
    
    temp = strtok(lineCopy, " ");       // put the command in temp 
    if(temp == NULL)  // nothing was entered 
    {
        return 0; 
    }
    
    // Copy the command into global command buffer 
    strncpy(command, temp, strlen(temp));
    
    //printf("command: %s\n", command); 

    for(temp = strtok(NULL, " "); temp != NULL; temp = strtok(NULL, " "))
    {
         
        argArray[i] = strdup(temp); 
        i++;
    }
    
    return i; 
}

void clearCommandBuff()
{
    int i; 
    for(i=0; i<16; i++)
    {
        command[i] = NULL; 
    }
}

void clearlineBuff()
{
    int i; 
    for(i=0; i<256; i++)
    {
        lineBuffer[i] = NULL; 
    }
}

void clearArgArray()
{
    int i; 
    for(i=0; i<20; i++)
    {
        argArray[i] = NULL; 
    }
}
// command table defined as global variable 
char* cmdTable[] = {                    // An array of strings
    "menu", "mkdir","rmdir", "ls","cd",
    "pwd", "stat", "creat", "rm", "link", "symlink", "unlink","quit", "touch", 0}; 

/* =========== FindCmd function definition =======================
recall that N is defined as the number of commands in the table, so this for-loop 
 will always iterate through all of the commands in  cmdTable
 returning the command's index in the table or -1 if not found 
 =============================================================================*/
int findCmd()   // use the global command buff
{
    int i; 
    //printf("Searching for command %s\n", command); 
    for(i=0; i<N; i++)          
    {
        // If you find the command, return it's index 
        if((strcmp(cmdTable[i], command)==0))
        {
            //printf("Command %s found at cmdtable[%d]!\n", cmdTable[i], i);
            return i; 
        }
        
    }
    // If we've iterated through cmdTable and not found the command
    printf("Error: not a valid command %s\n", command); 
    return -1; 
    
}

int main(int argc, char** argv) {
    
    printf("Welcome to Griffin.OS360!\n"
            "---------------------------------------------\n"); 
   
    int cmdIndex;               // catch return value from findCmd, use in switch statement 
    int hack;                   // used as counter to remove newline from fgets
    int numArgs;              
    clearArgArray();            // clear the argument array 
    
  
    // Mount the root and and initialize processes 
    init(); 
    
    while(1)
    {
        // Prompt for user input 
        printf("enter a command [P%i]: ", running->pid); 
        
        clearlineBuff(); 
        clearArgArray();            // clear the argument array 
        
        // read command into lineBuffer
        fgets(lineBuffer, 256, stdin); 
        // remove new line from fgets
        hack = 0; 
        while (lineBuffer[hack] != '\n') 
        {
            hack++;
        }
        lineBuffer[hack] = '\0'; 
        
        //printf("read in: %s\n", lineBuffer);
        
        /*
         Send the entire line to parseArgs
                Store the first word in the global command buffer
                 as the command
         */
        numArgs = parseArgs(lineBuffer); 
        
        // Arguments are now held in argArray
        if(argArray[0] != NULL)
        {
            // argArray[0] will hold pathname 
            token_path(argArray[0], name);
        }
         
        //printf("numArgs = %i\n", numArgs); 
        
       // The global command buffer was filled with the entered command 
           // in the parseArgs function
        cmdIndex = findCmd();   // use global command buffer     
        
        // This is called an ugly switch statement. Function Ptrs would have been better
        switch(cmdIndex)
        {
            case 0: menu(); break;                              // menu 
            case 1: makdir_driver(argArray); break;             // mkdir 
            case 2: rmdir_driver(argArray); break;              // rmdir 
                case 3: ls(argArray, numArgs); break;           // ls 
            case 4: change_dir(argArray); break;                // cd 
            case 5: pwd(); break;                               // pwd 
            case 6: stat_driver(argArray); break;               // stat 
            case 7: creat_driver(argArray); break;              // creat 
            case 8: ulink(argArray); break;                                      // rm 
            case 9: link_(argArray); break;                     // link 
            case 10: symlink_(argArray); break;                       // symlink 
            case 11: ulink(argArray);  break;                         // unlink 
            case 12: quit(); exit(0);  break;                         // quit 
            case 13: mytouch(argArray); break;                  // touch 
            case -1: continue; 
            
        }
    }
    return (EXIT_SUCCESS);
}


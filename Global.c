/* 
 * File:   Global.c
 * Author: griffin fujioka
 * ID: 11044124
 * Course: CptS 360 - Systems Programming I 
 * Project: EXT2 File System Simulator 
 * Created on March 28, 2012, 11:02 AM
 * 
 * -> Store global variables to be used in the program 
 */

#include "type.h"

MINODE* root;           // root minode
PROC *running;          // currently running process

// each mountTable entry represents a mounted device (on a DIR)
MOUNT mountTable[NMOUNT];       // stores all mounted devices                    
MINODE minode[NMINODES];        // load inodes from disk into memory 
OFT oft[NOFT];                  // open files table 
PROC proc[NPROC];               // running processes table 

char* name[16];                 // store pathname components 
char bname[64];                         // basename 
char dname[64];                         // dirname
char dBuf[BLOCK_SIZE];                // holds inode data 
char buff[BLOCK_SIZE];                // read blocks into this variable 
int nPathComponents; 
int iNodesCount; 
int InodesBeginBlock;           // represents the inode of root directory (INODE#2)
int ninodes;                    // total number of inodes available 
int fd; 
int ino; 
int nextBlock; 
int nextDir; 


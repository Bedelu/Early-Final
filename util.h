/* 
 * File:   util.h
 * Author: griffin
 *
 * Created on March 29, 2012, 6:14 AM
 */


#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "type.h"
#include "Global.c"
    


//void printInode(MINODE* mip); 
    
int get_block(int dev, int block, char* buff);

void mount_root(); 

void put_block(int dev, int block, char* buff);

int token_path(char* pathname, char** name); 

char* Dirname(); 

char* Basename(); 

unsigned long getino(int* dev, char* pathname); 

unsigned long search(MINODE* mip, char *name); 

char* search_ino(MINODE *mip, int ino);

MINODE* iget(int dev, unsigned long ino); 

void iput(MINODE *mip);

char* findmyname(MINODE* parent, unsigned long ino);

void setBit(char* x, int bitNum); 

int getBit(char* x, int bitNum);

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */


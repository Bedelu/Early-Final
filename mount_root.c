/* 
 * File:   mount_root.c
 * Author: griffin fujioka
 * ID: 11044124
 * Course: CptS 360 - Systems Programming I 
 * Project: EXT2 File System Simulator 
 * Created on March 28, 2012, 11:02 AM
 * 
 * -> Mount a user supplied filesystem 
 *      - check if unix fs
 *      - setup mount table
 *      - initialize proc[0] and proc[1] 
 */
#ifndef MOUNTROOT_C
#define	MOUNTROOT_C

#include "type.h"

void mount_root()
{
    // Load inode #2 of the root device into memory as MINODE[0]
        // Entails getting INODE, device, ino#, refCount and dirty 
    
    int fd, i; 
    char dev_name[256]; 
    char buff[BLOCK_SIZE]; 
    struct ext2_super_block *sb; 
   
    // Prompt user for mount device 
    printf("Enter device to mount root on (return for default ext2image): "); 
    fgets(dev_name, 256, stdin);        // get user input 
    
    if(strncmp(dev_name,"\n", strlen(dev_name))==0)
    {
        // User didn't enter anything, use default 
        strncpy(dev_name, "ext2image", 10); 
    }
    
    
}

#endif /* MOUNTROOT_C */ 
/* 
 * File:   init.h
 * Author: griffin
 *
 * Created on April 1, 2012, 2:11 AM
 */

#ifndef INIT_H
#define	INIT_H

#include "util.h"

/* ------------------------------------------------
 * mount_root(): mount the root device in mountTable[0]
 *      when a file system starts, it must mount a device on the Root DIR / 
 * 
 * 
 * this means: load the root INODE on device into memory and flag is root directory / 
 * 
 *      - check if FS is EXT2 
 *      - setup the mount table
 *      - initialize proc[0] & proc[1]      
 */
void mount_root()
{
    // Load inode #2 of the root device into memory as MINODE[0]
        // Entails getting INODE, device, ino#, refCount and dirty 
    SUPER* sb; 
    MINODE* mip;
    int dev; 
    char buf[BLOCK_SIZE]; 
    char line[256];                 // store the device name 
                             
 
   
    // Prompt user for mount device 
    printf("Enter device to mount root on (return for default diskimage): "); 
    fgets(line, 256, stdin);        // get line from user input  
    if(line[strlen(line)-1] == '\n')
    {
        line[strlen(line)-1] = 0; // add NULL to end of dev_name
    }
    
    // Check if the user entered something 
    if(strncmp(line,"\n", strlen(line))==0)
    {
        // If nothing was entered, us the default image stored in this directory 
        printf("Using default diskimage...\n");
        strncpy(line, "/home/griffin/NetBeansProjects/FinalProject/diskimage", 256); 
    }
    
    
    printf("Opening %s....\n", line);               
    
    // Open the device for Read/Write
    dev = open(line, O_RDWR);
    if (dev < 0) {                                // open failed 
        printf("error - could not open device\n");
    } else {                    
        printf("successfully opened device \n");
    }
    
    if(!get_block(dev,SUPERBLOCK, buf))
    {
        printf("failed to read SUPERBLOCK\n"); 
        exit(1); 
    }
    sb = (SUPER *)buf;                 // sb points to beginning of buff, aka SUPERBLOCK 
    
    // s_magic for EXT2 FS is 0xEF53. Verify that this is an EXT2 system 
    if(sb->s_magic != SUPER_MAGIC) // Magic# = 0xEF53
    {
        // the magic number is not EXT2 verified 
        printf("Error: device %s is not EXT2 File System\n", line); 
        exit(-1); 
    }
    
    if(!get_block(dev, GDBLOCK, buf))
    {
        printf("failed to read group descriptor block\n"); 
        exit(1); 
    }
    gp = (GD *)buf; 
    
    // We get here, we've verified EXT2 status and can rock and roll 
    // Initialize the global variables so this program is not limited 
        // to EXT2 floppy disks and will work on any EXT2 system 
    bg_inode_table = gp->bg_inode_table; 
    ibitmap = gp->bg_inode_bitmap; 
    bbitmap = gp->bg_block_bitmap; 
    
    printf("Inode table begins at %i \n", bg_inode_table);
    printf("ibitmap is at block location %i\n", ibitmap); 
    printf("bbitmap is at block location %i\n", bbitmap); 
    
    // Setup first mount-table entry 
    mountTable[0].dev = dev;     // which device it is 
    // knowing the device, we can always access the device to get it's SUPERBLOCK, bitmaps, etc
    mountTable[0].busy = BUSY;  // device currently in use 
    strncpy(mountTable[0].mount_name,line, 256);            // set mount_name 
    
    
    mip = iget(dev, 2); // get root inode, initalize global root; 
    
    root = mountTable[0].mounted_inode = mip; 
    strncpy(mountTable[0].name, "/", 1); 
    
    mountTable[0].ninodes = sb->s_inodes_count; 
    mountTable[0].nblocks = sb->s_blocks_count; 
    ninodes = sb->s_inodes_count; 
    printf("Mounted %s on %s....\n", mountTable[0].name, mountTable[0].mount_name); 
    printf("root inode device: %d\n", mountTable[0].mounted_inode->dev); 
    printf("# blocks = %i \tfree blocks=%i\n", mountTable[0].nblocks, sb->s_free_blocks_count); 
    printf("#inodes = %d \tfree inodes = %d\n", mountTable[0].ninodes, sb->s_free_inodes_count); 
    
}

void init(){
        int i;


        // initialize minode array (minode)
        for(i = 0; i < NMINODES; i++){
                minode[i].refCount = 0;
        }

        //initialize mount array (mountTable)
        for(i = 0; i < NMOUNT; i++){
                mountTable[i].busy = FREE;
        }

        //initialize proc table
        for(i = 0; i < NPROC; i++){
                proc[i].status = FREE;
        }

        mount_root();
        printf("Initializing processes..."); 
        // proc 0 = root (super user)
        proc[0].uid = 0; // uid 0 means super user, non-zero means other
        proc[0].gid = 0; // group ID
        proc[0].pid = 0; // process ID
        proc[0].cwd = root;//iget(mtable[0].dev, 2);

        running = &proc[0]; //set running process to root
        //printf("cwd refcount: %d\n", procstruct[0].cwd->refCount);
        printf(" complete.\n"); 
}



#endif	/* INIT_H */


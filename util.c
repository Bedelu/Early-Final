/* 
 * File:   util.c
 * Author: griffin fujioka
 * ID: 11044124
 * Course: CptS 360 - Systems Programming I 
 * Project: EXT2 File System Simulator 
 * Created on March 28, 2012, 11:02 AM
 * 
 * Disclaimer: I previously worked on this final project with Matt Karcher and 
 * submitted it along with the interview. Afterwards, feeling crappy about 
 * my understanding, I went back and did the entire project by myself however
 * parts of it will reflect work that was done in the previously submitted 
 * partner-version. 
 * ---> Contains utility functions needed for this program. 
 */

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>     // for dirname() and basename() 
#include <ext2fs/ext2_fs.h>


/*-------------------------------------------------------
 * isreg:
 *      accept a MINODE and return TRUE if it's a regular file
 * or FALSE if it's not 
 -------------------------------------------------------*/
int isreg(MINODE* mip)
{
    int isReg = S_ISREG(mip->INODE.i_mode); 
    //printf("Is Regular = %i\n", isReg); 
    return isReg; 
}

/* -----------------------------------------------------
 isdir:
 *      accept a MINODE and return TRUE if it's a directory 
 * or false if it's not 
 */

int isdir(MINODE* mip)
{
    int isDir = S_ISDIR(mip->INODE.i_mode);
    //printf("Is Directory = %i\n", isDir); 
    return isDir; 
}


/* ============================================
 * get_block: 
 *      3 parameters: 
 *              (1) dev = device to read from 
 *              (2) block = block location 
 *              (3) buffer to read into 
 =================================================*/
int get_block(int dev, int block, char buff[])
{
    // Find the block 
    if(lseek(dev, (long)BLOCK_SIZE*block, 0)==-1)
    {
        printf("Error in get_block: Seek error\n");
        return 0; 
    }
    
    // Read the block into buffer 
    if(read(dev, buff, BLOCK_SIZE) != BLOCK_SIZE)
    {
        printf("Error in get_block: Read error\n"); 
        return 0; 
    }
    return 1; 
}

void put_block(int dev, int block, char* buff)
{
    // Find the block 
    lseek(dev, (long)block*BLOCK_SIZE, 0); 
    
    // Write buff to the file 
    write(dev, buff, BLOCK_SIZE); 
    
}


/* ---------------------------------------------------------------------
 * token_path: 
 *      Input: pathname and container for storing path components 
 *      Output: number of components and the char** container is filled with them
 * 
 * I.e., 
 *      token_path("/a/big/pathname", name) = 3 and 
 *              name[0] = a
 *              name[1] = big
 *              name[2] = pathname
 * 
 *      - makes a copy, to not destruct the original pathname 
 -----------------------------------------------------------------------------*/
int token_path(char* pathname, char** name)
{
    // make a copy of the pathname 
    char* pathCopy = strdup(pathname); 
    int i=0; 
    char* p; 
    
    //printf("parsing pathname %s\n", pathname); 
    
    p = strtok(pathCopy,"/");           // tokenize by "/"
    while(p!= NULL)
    {
        name[i] = p;        // store the component in the char* array
        //printf("name[%d] = %s\n", i, p); 
        i++; 
        p=strtok(NULL, "/"); 
    }
    //printf("%d path components\n", i); 
    return i; 
    
    
}

/* Dirname and Basename
 * 
 * Pathname = /a/big/pathname
 * Dirname = /a/big
 * Basename = pathname
 * 
 * Use global variables: dirname and basename 
 * Be careful! Both functions destroy the parameter string. 
 * 
 * Dirname: 
 *      accept the pathname as a parameter 
 *      copy it, to not destroy it, and return directory path
 */
char* Dirname(char* pathname)
{
   
    char* retDirname;           // hold return value from dirname() call 
    retDirname = strdup(pathname); 
    retDirname = dirname(retDirname); 
    //printf("Dirname = %s\n", retDirname); 
    return retDirname; 
}

/* Basename:
 *      accept the pathname as a parameter
 *      copy the pathname and return base name of the path
 */
char* Basename(char* pathname)
{
    char* retBasename;                  // for return value 
    retBasename = strdup(pathname); 
    retBasename = basename(retBasename); 
    //printf("Basename = %s\n", retBasename);
    return retBasename; 
}
/* ----------------------------------------------------
 *              MOST IMPORTANT FUNCTION
 * 
 * Convert a pathname into it's (dev,inumber)
 * // Return inumber if pathname exists 
    // return 0 if not 
 * 
 */
unsigned long getino(int *dev, char *pathname) {
    int i;
    unsigned long ino = 0;
    MINODE* mip;
    char* name[32];     // hold 32 path components
    int nComponents; 

    //printf("entering getino with path: %s\n", pathname); 
    if(strlen(pathname) ==1 && pathname[0] == '/')
        return root->ino;
    
    if (strlen(pathname) == 1 && pathname[0] == '.')
        return root->ino;
    
     

    //get the pathname and number of tokens
    nComponents = token_path(pathname, name);
    //printf("number of path components = %d\n", nComponents); 
    if (nComponents == 0)       // no pathname components
    { 
        printf("WARNING: NOTHING IN COMPONENT ARRAY\n");
    } 
    else 
    {
        if (pathname[0] == '/') // Absolute pathname 
        {
            mip = iget(*dev, root->ino); // start from root 
        } 
        else                    // Relative pathname 
        {
            mip = iget(*dev, running->cwd->ino); // start from cwd 
        }
    }

    // search for each path component 
    for (i = 0; i < nComponents; i++) 
    {
        ino = search(mip, name[i]);
        if (ino == 0) 
        {                               // not found 
            break; // dont return, just exit this for-loop 
        }

        iput(mip); // release mip
        mip = iget(*dev, ino); // reload mip with MINODE of found name 
    }
    
    // Clear the path component array
    for(i=0; i<nComponents; i++)
    {
        name[i] = NULL; 
    }
    iput(mip);
    return ino;
}

/* =======================================================
 * Search: search the entire direct block for the name 
 *      two parameters: 
 *      (1) INODE 
 *      (2) the name we're looking for 
 *      
 *      look in each INODE block and search the dir entries for the name 
 * 
 *      Assume DIRECT datablocks only 
 * =========================================================*/
unsigned long search(MINODE* mip, char* name)
{
    char* cp;
    DIR* dp;
    char sBuff[BLOCK_SIZE]; 
    int i = 0; 
    char nameBuff[512];
    int inodeNumber; 
    
    
    for(; i<12 && mip->INODE.i_block[i] != 0; i++)         // look in direct blocks 
    {
        // Read the block[0] into sBuff into sBuff 
        if(!get_block(mip->dev, mip->INODE.i_block[i], sBuff)) 
        {
            printf("Error in get block\n"); 
            return 0; 
        }
        
        cp = sBuff;             // cp points to start of sBuff 
        dp = (DIR *)cp;        // directory pointer for traversing 
        //printf("Searching for %s\n", name);
        
        while (cp< &sBuff[BLOCK_SIZE])
        {
            strncpy(nameBuff, dp->name, dp->name_len); // copy directory name into nameBuff  
            nameBuff[dp->name_len] =0; 
            if (dp->name_len == strlen(name) && !strncmp(dp->name, name, dp->name_len)) 
            {
                //printf("Found %s\n", name); 
                // Found the node!
                inodeNumber = dp->inode;
                
                return inodeNumber;     // return the inodeNumber 
                
            }
           
            // if not found, advance dp to keep looking 
            cp += dp->rec_len;
            dp = (DIR *) cp;
        }
        
        // If you didn't find it, return 0
        return 0; 
    }
}


/* ====================================================
 iget: 
 * Load the specified INODE into minode table and return pointer 
 *      Parameters: 
 *              (1) device to look in 
 *              (2) inode number to look for 
 * 
 * Search for an empty slot in minode table then use
 * inode number and mailman's algorithm to find the inode block
 *  
 * Once you have (dev, ino) of an inode, 
 load the inode into a slot in the MINODE[] array. 
        - search MINODE[] to ensure it's not already there 
                - if exists: increment refCount and return 
                - else: Allocate space in MINODE[]
                        and load the INODE from disk into 
                        MINODE[i].INODE. initialize everything
                        and return it's address
 */
MINODE* iget(int dev, unsigned long ino)
{
    int i=0;
    int blocknum, inonumber; 
    MINODE* mp; 
    char igetBuff[BLOCK_SIZE]; 
    
    
    // Search the in memory inodes first, increment refcount if found 
    for(i=0; i<NMINODES; i++)
    {
        if(minode[i].dev == dev && minode[i].ino == ino && minode[i].refCount > 0)
        {
            // Same ino and same device, we've found the minode
            minode[i].refCount++;       // increment refCount
            return &minode[i];          // return pointer to that MINODE
        }
    }
    
    // If we reach here, the MINODE does not exist in the minode[] table
    // So we search for an empty location and store it 
    for(i=0; i<NMINODES; i++)           // iterate all elements 
    {
        if(minode[i].refCount == 0)     // empty slot! 
        {
            // set mp to point to the empty slot 
            mp = &minode[i]; 
            
            // Mailman's algorithm
            blocknum = ((ino-1)/INODES_PER_BLOCK) + INODEBLOCK; 
            inonumber = (ino-1) % INODES_PER_BLOCK; 
            
            if(!get_block(dev, blocknum, igetBuff))
            {
                printf("Warning: get_block error!\n"); 
            }
            
            // ip: global inode pointer 
            // Now points to the inode . Why do you do this? 
            ip = (INODE*)igetBuff + inonumber; 
            
            // Create the minode entry 
            
            // Copy into the the elements INODE struct 
            memcpy(&minode[i].INODE, igetBuff + sizeof(INODE)*inonumber, sizeof(INODE)); 
            // Use mp, we already set it to point here!
            mp->dev = dev; 
            mp->ino = ino; 
            mp->refCount = 1; 
            mp->dirty = 0; 
            mp->mounted = 0; 
            mp->mountptr = NULL; 
            mp->INODE.i_size = BLOCK_SIZE; 
            return mp; // pointer to new minode entry 
        }
    }
    
    // We make it to here without returning, 
        // there are no free inodes 
    printf("No free MINODES, returning NULL...\n"); 
    return NULL; 
}

/* 
 * Release an entry from the MINODE table 
 *      - decrement refCount 
 *      if refCount < 1: remove the minode 
 *              if dirty == 0: do nothing
 *              if dirty == 1: write back to the disk 
 *                      read a block
 *                      copy minode's INODE
 *                      write block to disk 
 */
void iput(MINODE *mip)
{
    int blocknum, inonumber; 
    char iputBuff[BLOCK_SIZE]; 
    // Decrement refCount FIRST THING 
    mip->refCount--; 
    
    // If refCount > 0, mip is still in use --> do nothing
    if(mip->refCount > 0)
    {
        return; 
    }
    // If it's not dirty, just erase it from memory 
    if((mip->dirty == 0))       // not dirty 
    {
        bzero(mip, sizeof(MINODE));     // clear the memory space 
        return; 
    }
    
    // By here, the INODE is dirty so we read, copy and rewrite a block 
    // Mailman's algorithm 
    blocknum = INODEBLOCK + ((mip->ino-1)/ INODES_PER_BLOCK); 
    inonumber = (mip->ino-1) % INODES_PER_BLOCK; 
    
    get_block(mip->dev, blocknum, iputBuff);            // read into buffer 
    
    // Add mip's INODE
    memcpy(iputBuff+sizeof(INODE)*inonumber, &(mip->INODE), sizeof(INODE)); 
    
    // Write it to the disk 
    put_block(mip->dev, blocknum, iputBuff); 
    

    bzero(mip, sizeof(MINODE)); 
}

/* -----------------------------------------------------------------
 * Given the parent DIR and inumber, find the string of myino
 * in the parent's data block 
 * Similar to Search() 
 * used exclusively in pwd 
  ---------------------------------------------------------------------*/
char* findmyname(MINODE* parent, unsigned long ino)
{
    DIR* dp;                    // For traversing through the data block         
    char* cp; 
    char buf[BLOCK_SIZE]; 
    int i=0; 
    char nameBuff[256]; 
    char* retName; 
    
    // Go through direct data blocks 
    for(i=0; i<12; i++)
    {
        if(parent->INODE.i_block[i] == 0)       // empty i_block 
            break;                              // exit for loop 
        
        // Read the i_block into buf
        get_block(parent->dev, parent->INODE.i_block[i], buf); 
        
        cp = buf; 
        dp = (DIR *)cp; 
        
        while(cp < &buf[BLOCK_SIZE])
        {
            // We've found the inode 
            if(dp->inode == ino)
            {
                // Copy directory name into nameBuff
                strncpy(nameBuff, dp->name, dp->name_len); 
                nameBuff[dp->name_len] = 0;     // add null!!
                // Make a copy and store in return value 
                retName = strdup(nameBuff); 
                //printf("Found %s\n", retName); 
                return retName; 
                
            }
            // Advice directory pointers
            cp += dp->rec_len; 
            dp = (DIR *)cp; 
        }
        // Made it through data blocks 
    }
    // Made it through ALL direct datablocks 
    
    return NULL; 
    
}



int groupdesc(char* device) {
    int fd, blksize, inodesize;
    clearBuff(); 
    fd = open(device, O_RDONLY);
    if (fd < 0) {
        printf("open failed\n");
        exit(1);
    }

    // read the GROUP DESCRIPTOR block at byte offset 2048
    lseek(fd, (long) 2048, 0); // find the block using offset
    read(fd, buff, 1024); // read the block in 
    gp = (GD *) buff;



    // determine block size 
    blksize = 1024 * (1 << sp->s_log_block_size);
    printf("blocksize = %d\n", blksize); 

    printf("********** GROUP DESCRIPTOR info **********\n");
    printf("block bitmap = %d\n", gp->bg_block_bitmap);
    printf("block imap = %d\n", gp->bg_inode_bitmap);
    printf("inodes begin at: %d\n", gp->bg_inode_table);
    InodesBeginBlock = gp->bg_inode_table;
    printf("free blocks = %d\n", gp->bg_free_blocks_count);
    printf("free inodes = %d\n", gp->bg_free_inodes_count);
    

}
// Get superblock information
int super(char* device) {
    int blksize, inodesperblock;
    clearBuff(); 
    fd = open(device, O_RDONLY);
    if (fd < 0) {
        printf("open failed");
        exit(1);
    }

    // Read the SUPERblock at byte offset 1024
    lseek(fd, (long) 1024, 0);
    read(fd, buff, 1024);
    sp = (SUPER *) buff;
    // Determine FS block size 
    blksize = 1024 * (1 << sp->s_log_block_size);
    inodesperblock = blksize / sp->s_inode_size; 


    // Check the fs magic number; must be 0xef53 hex = 61267 decimal
    if (sp->s_magic == 0xEF53)
        printf("ext2 status confirmed\n");
    else {
        printf("device not ext2");
        return;
    }


    printf("********** SUPERBLOCK info **********\n");
    printf("inodes count = %d\n", sp->s_inodes_count);
    iNodesCount = sp->s_inodes_count; 
    printf("super blocks count = %d\n", sp->s_blocks_count);
    printf("free inodes count = %d\n", sp->s_free_inodes_count);
    printf("free blocks count = %d\n", sp->s_free_blocks_count);
    printf("s_log_block_size = %d\n", sp->s_log_block_size);
    printf("blocks per group = %d\n", sp->s_blocks_per_group);
    printf("inodes per group = %d\n", sp->s_inodes_per_group);
    printf("mount count = %d\n", sp->s_mnt_count);
    printf("max mounts = %d\n", sp->s_max_mnt_count);
    printf("magic = 0x%x\n", sp->s_magic); // print magic number as hex 
    printf("inode size = %d\n", sp->s_inode_size);
    printf("inodes per block = %d\n", inodesperblock); 
}

// Clear global buff variable (used for reading in blocks of data)
void clearBuff()
{
    int i=0; 
    for(i=0; i<1024; i++)
    {
        buff[i] = NULL; 
    }
}

// Clear the global name array (array of char* containing path components 
void clearNameArray()
{
    int i=0;
    for(i=0; i<16; i++)
    {
        name[i] = NULL; 
    }
}

void setBit(char* x, int bitNum)
{
    // shift to the bitnum and set it to 0 
    *x |= 1 << (bitNum); 
}

int getBit(char* x, int bitNum)
{
    // shift to the bitNum that is of interest
    return (*x >> (bitNum) & 1);        // return the bit number 
}

/* ------------------------------------------------------------
     clear the entire INODE, return a pointer to the empty one 
  ---------------------------------------------------------------*/
MINODE* clearInode(MINODE* mp)
{
    memset(&mp->INODE, 0x00, sizeof(INODE)); 
    mp->dirty = 1; 
    return mp; 
}

/* -------------------------------------------------------------
 * updateGD(): 
 *      increase used DIRS count by one in GroupDescriptor block
 * 
 -----------------------------------------------------------------*/
void updateGD() {
        char buf[1024];
        GD *gp;

        // ensure the buffer is empty 
        bzero(buf, BLOCK_SIZE);
        // Read GroupDescriptor block into  buf
        get_block(root->dev, GDBLOCK, buf);
        gp = (GD *)buf;
        gp->bg_used_dirs_count++;
        put_block(root->dev, GDBLOCK, buf);
}

/* -------------------------------------------------------------
 * ialloc: allocate an inode on a dev
 * 
 *      (1) Look in imap for a FREE inode 
 *              imap is just a series of bits, 
 *              look for a 0 bit 
 * 
 *              So we need to read in imap into buf[BLOCK_SIZE]
 *      (2) Look through all bits in buf[] for a 0 bit 
 *      (3) Once you find the 0 bit, turn it to 1 
 *      and remember the index (counting from 0)
 *      (4) Write the modified imap block back to the disk 
 *      (5) Decrement the number of free inodes 
 *              in both SuperBlock and GroupDescriptor 
 *      (6) return bit_index + 1, since bits count from 0 
 *              but inumbers count from 1 
 ----------------------------------------------------------------*/
unsigned long ialloc(int dev){
        int i, inum, index;
        unsigned char buf[BLOCK_SIZE];
        unsigned char inode_byte;
        int found = 0;
        SUPER * sbp;
        MINODE* mp;
        GD * gdp;

        //read imap block into buf
        if(!get_block(dev, IBITMAP, buf))
        {
                printf("WARNING - ialloc: getblock() failed\n");
        }

        //check each byte in buf to find one that has at least 1 empty bit
        for(i = 0; (i < BLOCK_SIZE); i++)
        {
                // 0xFF = hexadecimal a full byte 
                if(buf[i] != 0xFF)              // entry is not full 
                { //if the byte in buf[i] isn't completely full
                        found =1;
                        // Load the empty byte into inode_byte 
                        inode_byte = buf[i];
                        
                        // 8 bits = 1 byte 
                        // inum = location of free bit 
                        inum = (i * 8) + 1;
                        index = i;
                        break;          // exit for loop 
                }
        }

        if(!found){
                printf("ialloc() - no free bits found\n");
                return 0;
        }

        //discover 1st unused bit within inode_byte by iterating through the byte 
        for(i = 0; i < 8; i++)
        {
                if( getBit(&inode_byte, i) == 0) // if the ith bit in inode_byte is free
                {
                        inum = inum + i; //inum now exact inum
                        //mark inode (bit) as used
                        setBit(&inode_byte, i); //set the ith bit to 1
                                                // bc you're about to allocate it 

                        //clear that inode and write to disk
                        mp = iget(dev, inum); //get minode from that ino 
                        clearInode(mp); //clear the minode 
                        iput(mp); //put back MINODE

                        // Since we've modified 
                        //save back to IBITMAP and write to disk
                        buf[index] = inode_byte; //store new byte in the buffer to put back
                        put_block(dev, IBITMAP, buf); //put back IBITMAP

                        //now update superblock and group descriptor with free inodes
                        get_block(dev, SUPERBLOCK, buf); //get superblock, to update
                        sbp = (SUPER *)buf; //typecast the superblock buff
                        sbp->s_free_inodes_count--; //subtract from 1 free inode count
                        put_block(dev, SUPERBLOCK, buf); //put back superblock

                        get_block(dev, GDBLOCK, buf); //get groupdescriptor block
                        gdp = (GD *)buf; //typecast the gd block
                        gdp->bg_free_inodes_count--; // 1 from subtract free inode count
                        put_block(dev, GDBLOCK, buf);// put back gdblock
                        break;          // exit for loop 
                }
        }
        return inum; //return 1 or 0 depending on change respectively
}

/* -----------------------------------------------------------
 * balloc: 
 *      Search the block bitap for a block with a 0 bit 
 *      Once found, change it to 1 and allocate the block 
 *      at the corresponding location 
 ----------------------------------------------------------------*/
unsigned long balloc(int dev)
{
            int i, index = 0, found = 0;
        unsigned long bnum = 0;
        unsigned char buf[BLOCK_SIZE], block_byte;
        SUPER *sbp;
        GD *gdp;

        get_block(dev, BBITMAP, buf);
        //search for first unused inode's block
        for(i = 0; (i < BLOCK_SIZE) && (!found); i++)
        {
                //if all bits not used in this byte
                if( buf[i] != 0xff)
                {
                        found = 1;
                        index = i;
                        block_byte = buf[i];
                        //set inum to start of found block ie: 1,9,17...
                        bnum = (i * 8) + 1;
                        break;
                }
        }
        if(!found)
        {
                return 0;
        }
        //discover 1st unused inode within found byte
        for(i = 0; i < 8; i++)
        {
                if( getBit(&block_byte, i) == 0)
                {
                        bnum = bnum + i; //inum now exact inum

                        //mark bit as used
                        setBit(&block_byte, i);
                        //save back to BBITMAP & write to disk
                        buf[index] = block_byte; //save the byte
                        put_block(dev, BBITMAP, buf); //put back BBITMAP

                        //now update superblock and group descriptor with free inodes
                        get_block(dev, SUPERBLOCK, buf); //get superblock
                        sbp = (SUPER *)buf; //typecast
                        sbp->s_free_blocks_count--; //subtract 1 from free block count
                        put_block(dev, SUPERBLOCK, buf); //put back superblock

                        get_block(dev, GDBLOCK, buf); //get gdblock
                        gdp = (GD *)buf; //typecast
                        gdp->bg_free_blocks_count--; //subtract 1 from free block count
                        put_block(dev, GDBLOCK, buf); //put back gdblock
                        break;
                }
        }

        //erase previous data in block
        get_block(dev, bnum, buf);
        memset(buf, 0x00, BLOCK_SIZE);
        put_block(dev, bnum, buf);
        return bnum;
}
void bdealloc(int dev, int bnum){
        int bytenum, bitnum;
        unsigned char buf[BLOCK_SIZE];
        unsigned char* b;
        SUPER* sbp;
        GD* gdp;

        bytenum = (bnum - 1) / 8;               // determine byte number 
        bitnum = (bnum - 1) % 8;                // determine bit number 

        //remove from inode bitmap
        get_block(dev, BBITMAP, buf);
        b = buf + bytenum;              // get to beginning of the byte 

        
        // create a byte where b points to the bit you'd like to dellocate 
        //clear bit b
        *b &= ~(1 << (bitnum));         // change the 1 bit 

        //write back to disk
        put_block(dev, BBITMAP, buf);

        // update superblock and group descriptor
        get_block(dev, SUPERBLOCK, buf);
        sbp = (SUPER* )buf;
        sbp->s_free_blocks_count++;
        put_block(dev, SUPERBLOCK, buf);

        get_block(dev, GDBLOCK, buf);
        gdp = (GD *)buf;
        gdp->bg_free_blocks_count++;
        put_block(dev, GDBLOCK, buf);
}
/* ------------------------------------------------------------------
 * idealloc: deallocate the inode on dev specified by inum (same as bdealloc)
 *      inum - 1 = the bit in bmap that must be cleared
 *      Increment GD and SuperBlock 
 --------------------------------------------------------------*/

void idealloc(int dev, int inum){
        int bytenum, bitnum;
        unsigned char buf[BLOCK_SIZE];
        unsigned char* b;
        SUPER* sbp;
        GD* gdp;

        bytenum = (inum - 1) / 8;
        bitnum = (inum - 1) % 8;

        //remove from inode bitmap
        get_block(dev, IBITMAP, buf);
        b = buf + bytenum;

        //clear bit b
        *b &= ~(1 << (bitnum));

        //write back to disk
        put_block(dev, IBITMAP, buf);

        // update superblock and group descriptor
        get_block(dev, SUPERBLOCK, buf);
        sbp = (SUPER* )buf;
        sbp->s_free_inodes_count++;
        put_block(dev, SUPERBLOCK, buf);

        get_block(dev, GDBLOCK, buf);
        gdp = (GD *)buf;
        gdp->bg_free_inodes_count++;
        put_block(dev, GDBLOCK, buf);
}

/*--------------------------------------------------------
 * isempty: 
 *      takes an MINODE directory and determine if it's empty
 --------------------------------------------------------*/
int isempty(MINODE* parent)
{
    int i=0; 
    int dev = parent->dev; 
    int contentCtr = 0;         // count the cotents of the directory 
    char buf[BLOCK_SIZE]; 
    char* cp; 
    DIR* dp; 
    
    for(i=0; i<12; i++)
    {
        if(parent->INODE.i_block[i] != 0)
        {
            get_block(dev, parent->INODE.i_block[i], buf); 
            cp = buf; 
            dp = (DIR *)cp; 
            
            while(cp < &buf[BLOCK_SIZE])
            {
                contentCtr++; 
                if(contentCtr>2)             // dir isn't empty 
                    return 0;   
               
                cp += dp->rec_len; 
                dp = (DIR *)cp; 
            }
        }
    }
    return 1;   // Directory has no contents! 
}
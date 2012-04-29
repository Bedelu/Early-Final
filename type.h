/* 
 * File:   type.h
 * Author: griffin
 *
 * Created on March 29, 2012, 4:34 AM
 */


#ifndef TYPE_H
#define	TYPE_H

#ifdef	__cplusplus
extern "C" {
#endif

/*	type.h for CS360 Project             */

#include <stdio.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>  // for dirname() and basename() 
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
    

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc  GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;    // need this for new version of e2fs

GD    *gp;       // pointer for Group Descripter block 
SUPER *sp;      // pointer for Superblock 
INODE *ip;      
DIR   *dp; 

/* these variables allow this program to be extendable past EXT2 floppy disks
 and available on other EXT2 file systems. They are initialized from data contained in 
 * the SUPERBLOCK in init.h */
int bg_inode_table; 
int ibitmap; 
int bbitmap; 


#define BLOCK_SIZE        1024
#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           bbitmap
#define IBITMAP           ibitmap
#define INODEBLOCK        bg_inode_table
#define ROOT_INODE        2

// Default dir and regular file modes
#define DIR_MODE          0040777       // Directory mode
#define FILE_MODE         0100644       // File mode 
#define SUPER_MAGIC       0xEF53        // EXT2 magic 
#define SUPER_USER        0             // Super User 

// Proc status - Free, Busy or Killed 
#define FREE              0
#define BUSY              1
#define KILLED            2

// Table sizes
#define NMINODES          50    // # of available In Memory Inodes
#define NMOUNT            10    // # devices mounted 
#define NPROC             10    // # of processes running 
#define NFD               10
#define NOFT              50    // # open files 

// Open File Table
typedef struct Oft{
  int   mode;
  int   refCount;
  struct Minode *inodeptr;
  long  offset;
} OFT;

// PROC structure
typedef struct Proc{
  int   uid;            // user id      
  int   pid;            // process id 
  int   gid;            // group id 
  int   ppid;           // parent process id 
  struct Proc *parent;          // pointer to parent process 
  int   status;                 // what's the status of this process? running? dead? failed? 

  struct Minode *cwd;
  OFT   *fd[NFD];
} PROC;
      
// In-memory inodes structure
typedef struct Minode{		
  INODE    INODE;               // disk inode
  ushort   dev;                 // device 
  unsigned long ino;            // inode number
  ushort   refCount;            // times this iNode is reference  
  ushort   dirty;               // has the file been modified?
  ushort   mounted;             
  struct Mount *mountptr;
  char     name[128];           // name string of file
} MINODE;

// Mount Table structure
typedef struct Mount{
        int    ninodes;
        int    nblocks;
        int    dev, busy;   
        int first_datablock;    // first available datablock 
        struct Minode *mounted_inode;
        char   name[256]; 
        char   mount_name[64];
} MOUNT;


#ifdef	__cplusplus
}
#endif

#endif	/* TYPE_H */


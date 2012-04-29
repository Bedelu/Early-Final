/* 
 * File:   functions.c
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
 * ---> Contains the command function definitions
 */

#include "functions.h"

void menu()
{
    printf("Menu:\n");
    printf("======================================\n"); 
    printf("mkdir\t rmdir\t ls\t\n");
    printf("cd\t pwd\t creat\t\n");
    printf("link\t unlink\t syslink\t\n"); 
    printf("stat\t chmod\t touch\t\n"); 
    //printf("rm\t save\t\n");
    //printf("save\t reload\t\n");
    printf("quit\n");
    printf("======================================\n");  
    return; 
}

/* ------------------------------------------------------------
 * change_dir: 
 *      accept a pathname and change to that directory 
 * 
 *      (1) Check pathname for NULL, set to root if so 
 *      (2) get INODE of pathname into a MINODE
 *      (3) get INODE into minode[] table 
 *      (4) check DIR type, return if not a DIR 
 *      (5) dispose of running->cwd, change to found MINODE* dir
 * 
 -----------------------------------------------------------------*/
void change_dir(char** args)
{
    MINODE* dir; 
    int ino; 
    int dev = running->cwd->dev;  
    
    
    // (1) Check for pathname 
    if(args[0] == NULL)         // no path provided 
    {
        iput(running->cwd);     // Dispose of cwd 
        running->cwd = root;    // change to root 
        return; 
    }
    
    // (2) get INODE of pathname into a MINODE
    ino = getino(&dev, args[0]);                // (2.1) get inumber 
    dir = iget(dev, ino);                       // (3) get INODE into minode[] table 
    
    if(dir == NULL)             // unable to find directory 
    {                   
        printf("No such file or directory %s\n", args[0]); 
        return; 
    }
    else                        // Release running->cwd and update to dir
    {
        // (4) Check DIR type, return if not directory 
        if(isreg(dir))          // if dir is a regular file 
        {
            printf("Error: Not a directory %s\n", args[0]);
            iput(dir);          // release minode[] entry 
            return;  
        }
        
        // (5) We've found the (valid) directory 
        iput(running->cwd);                 // dispose the current cwd 
        running->cwd = iget(dir->dev, dir->ino); // load new cwd by calling iget
        iput(dir);      // write back to disk 
     
    }  
}
/* -----------------------------------------------------
 * mkdir_driver:
 *      allows mkdir to act just like Linux by calling 
 *      mak_dir() on each argument
 * 
 * i.e., mkdir dir1 dir2 dir3 
 will create all of directories 
 --------------------------------------------------------*/
void makdir_driver(char** args)
{
    int i=0; 
    
    if(args[0] == NULL)
    {
        printf("Error: mkdir requires arguments\n"); 
        return; 
    }
    
    // Create a directory for each of the arguments 
    for(i=0; args[i] != NULL; i++)
    {
        make_dir(args[i]); 
    }
    
}

/* -------------------------------------------------------
 * make_dir: 
 *      must understand perfectly!!!
 * 
 *      Takes a pathname as parameter 
 * 
 *      (1) set the device 
 *      (2) get parent MINODE by using dirname with 
 *              getino 
 *          Load MINODE using iget 
 *      (3) call my_mkdir(MINODE* parent, char* name)
 * 
 --------------------------------------------------------*/
void make_dir(char* path)
{
    MINODE* pip;                // parent MINODE* 
    int dev, pino;              // device, parent ino 
    
    // parent = path to parent directory, child = basename 
    char* parent, *child;        
    
    // (1) Set device according to relative or absolute path 
    if(path[0] == '/')
        dev = root->dev; 
    else 
        dev = running->cwd->dev; 
    
    // (2) Separate path into dirname and basename 
    parent = strdup(Dirname(path));     // make copies 
    child = strdup(Basename(path)); 
    
    // Debug print 
    //printf("parent: %s\n", parent);
    //printf("child: %s\n", child);
    
    // (3) get in memory MINODE of parent directory 
    pino = getino(&dev, parent); // First, get parent ino 
    pip = iget(dev, pino); // Then use it to load INODE into minode[] table
    
    
    // (4) Error checking on found MINODE
    if(pip == NULL)                     // (4.1) ensure the MINODE was found 
    {
        printf("Error: unable to locate %s\n", parent); 
    }
    else if(!isdir(pip))                // (4.2) is DIR 
    {
        printf("Error: %s is not a directory\n", parent);
    }
    else if(search(pip, child) != 0)    // (4.3) child does not already exist    
    {
        // the child was already found 
        printf("Error: %s already exists in %s\n", child, parent); 
    }
    // (5) We've verified that parent path exists, is a directory 
        // and child does not exist in it, so add to it  
    else
        my_mkdir(pip,child); 
    
    // No matter what, dont forget to write back! 
    // Release parent from minode[] table and write back to disk 
    iput(pip);
}

/* ----------------------------------------------------------------------
 * my_mkdir: 
 *      (1) Allocate INODE space on device using ialloc, which will
 *              return the new directory's inumber 
 *      (2) Allocate a block on the device using balloc, which will
 *              return the new directory's block_number 
 *      (3) get a newMinode for the new directory using 
 *              newdir = iget(dev, inumber)
 *      (4) Setup newdir information 
 *      (5) iput(newdir) - release from MINODE table and write to disk 
 *      (6) Write . and .. into buf and put the buf to the disk block    
 *      (7) Add the name to parent dir 
 *              May require "trimming" the length of the last dir entry 
 *              in the block. 
 * 
 *              To trim, calculate needed_len and ideal_len and trim 
 *              the last entry's size to ideal_len 
 *      (8) write data back to the disk using put_block(dev, block_number, buf)
 ------------------------------------------------------------------ */
void my_mkdir(MINODE* pip, char* name) 
{
    // (1) pip points to the parent directory, which we'll be adding to 
    
    int inumber, bnumber;       // ino number, blocknumber 
    MINODE* newdir;             // directory to be added
    int i;
    char* cp;                   // for iterating through the block
    DIR* dp;                    // points to DIR structs within the block 
    int ideallen, neededlen;    // ideal_length, needed_length 
    char buf[BLOCK_SIZE];
    int dev = pip->dev;         // same dev as parent's directory 
    int count = 0; 

    // (2) Allocate inode and a disk block 
    inumber = ialloc(dev);      // allocate inumber 
    // Block number will be i_block[0], where this directory's data block begins 
    bnumber = balloc(dev);      // allocate blocknumber
    

    if (inumber < 1) 
    {
        printf("my_mkdir() -- ialloc failed with dev %d", dev);
    }
    if (bnumber < 1) 
    {
        printf("my_mkdir() -- balloc failed with dev %d", pip->dev);
    }
        
    // (3) load inode into minode[] in 
        // order to write contents to intended inode in memory 
    newdir = iget(dev, inumber); 

    // (4) Write contents into newdir->INODE 
    newdir->dirty = 1;
    
    // Initialize all i_blocks to 0 
    for (i = 0; i < 15; i++) {
        newdir->INODE.i_block[i] = 0;
    }
    
    // Directories data blocks start at i_block[0], block number of this directory
    newdir->INODE.i_block[0] = bnumber;
    newdir->INODE.i_mode = DIR_MODE;
    newdir->INODE.i_uid = running->uid;
    newdir->INODE.i_gid = running->gid;
    newdir->INODE.i_size = BLOCK_SIZE;          // size in bytes 
    newdir->INODE.i_atime = newdir->INODE.i_ctime = newdir->INODE.i_mtime = time(0L);
    newdir->INODE.i_blocks = 2;  // 2-512 bytes
    newdir->INODE.i_links_count = 2;

    // (5) write inode to disk
    iput(newdir);

    // (6) now write the . and .. entries into a buf[BLOCK_SIZE]
    // then write buf back to disk block allocated to this directory
    bzero(buf, BLOCK_SIZE);             // clear buf
    
    // doing "."
    dp = (DIR*)buf; // get dir pointer
    dp->inode = inumber; //inumber = same inumber as new dir
    strncpy(dp->name, ".", 1);
    dp->name_len = 1;
    dp->rec_len = 12; // 8 + multiple of 4 closest and greater than name length
    
    // doing ".." 
    dp = (DIR *)(buf + dp->rec_len);    
    dp->inode = pip->ino;       // inumber = parent inumber
    dp->name_len = 2;
    strncpy(dp->name, "..", 2);
    dp->rec_len = (BLOCK_SIZE - 12); //take up remainder of block

    //write block to disk:
    put_block(dev, bnumber, buf);

    // (7) Finally, write the directory to it's parent's data blocks 
    //this is the minimum length we need for our new directory 
    
    /* -------------- NOTES -----------------------------------
     *  - each EXT2 DIR entry has rec_len and name_len 
     *  - the rec_len of the LAST entry in the datablock is to the end of the block 
     *  - When entering a new directory with name_len = n
     *          need_length = 4*((8 + name_len + 3)/4) 
     *  - Step to the last entry in the data block, it's IDEAL_LENGTH is 
     *          IDEAL_LENGTH = 4 *((8 + name_len + 3)/4)
     *          
     *          if(rec_len - IDEAL_LENGTH >= need_length)
     *                  Enter new directory as the last entry and trim the
     *                  previous entry to its IDEAL_LENGTH 
     *          else 
     *                  allocate a new data block and enter new directory as 
     *                  first entry in new data block 
    ---------------------------------------------------------------- */
    
    // neededlen = #bytes needed by this new directory 
    neededlen = 4 * ((8 + strlen(name) + 3) / 4); 
    bzero(buf, BLOCK_SIZE); //clear buf

    // check all the direct blocks for free space 
    for (i = 0; i < 12; i++) 
    { 
        count = 0; 
       // break if the iblock is empty 
        if(pip->INODE.i_block[i] == 0)
            break; 
        
        // read parent's data block into buf
        get_block(dev, pip->INODE.i_block[i], buf); 
        

        cp = buf;
        dp = (DIR*) buf;

        // Step to the last entry of the data block 
        while (cp < &buf[BLOCK_SIZE]) 
        {
            // ideallen = minimum length that last record takes up? 
            ideallen = 4 * ((8 + dp->name_len + 3) / 4); // calculate ideal length of record (to end of block)

            if ((dp->rec_len - ideallen) >= neededlen) // There's room in this block 
            {   
                DIR* lastDir;
                int lastBlockBool;

                if ((cp + dp->rec_len) < &buf[BLOCK_SIZE])
                    lastBlockBool = 0; //not the last block
                else
                    lastBlockBool = 1; //it is the last block!

                // Trim the length of the last record 
                dp->rec_len = ideallen;

                //move to beginning of new last entry
                cp += ideallen;
                count += ideallen;

                // add last entry
                lastDir = (DIR*)cp;
                lastDir->inode = inumber;
                lastDir->name_len = strlen(name);
                strncpy(lastDir->name, name, lastDir->name_len);

                // If this is the last entry in the data block
                        // it's length extends to the end of the block 
                if (lastBlockBool) 
                    lastDir->rec_len = BLOCK_SIZE - count;
                else
                    lastDir->rec_len = neededlen;

                // (8) write parent's data block back to disk
                put_block(dev, pip->INODE.i_block[i], buf);

                //update parent inode link count
                pip->INODE.i_links_count++;
                pip->INODE.i_atime = time(0L); 
                pip->dirty = 1;
                
                
                iput(pip);      // Write back to disk 
                // Update group descriptor free_inodes and such 
                updateGD();
                return;

            }

            //advance dp to next dir entry in it's block:
            count += dp->rec_len;
            cp += dp->rec_len;
            dp = (DIR*) cp;
        }
    }

    return;
}


/* -----------------------------------------------------
 * rmdir_driver:
 *      allows rmdir to act just like Linux by calling 
 *      rm_dir() on each argument
 * 
 * i.e., rmdir dir1 dir2 dir3 
 will remove all of directories 
 --------------------------------------------------------*/
void rmdir_driver(char** args)
{
    int i=0; 
    
    if(args[0] == NULL)
    {
        printf("Error: rmdir requires arguments\n"); 
        return; 
    }
    
    
    for(i=0; args[i] != NULL; i++)
    {
        rm_dir(args[i]); 
    }
}

/* ---------------------------------------------------------------
 * rm_dir: 
 *      (1) ask for pathname to rmdir 
 *      (2) get rmdir's ino 
 *              ino = getino(&dev, pathname)
 *      (3) get a pointer to its MINODE[]
 *              mip = iget(dev, ino)
 *      (4) check DIR type && not busy && is empty 
 *      (5) get parent directory into memory
 -----------------------------------------------------------------*/
void rm_dir(char* path)
{
    // (1) Asks for  pathname
    MINODE* mip;        // directory to remove 
    MINODE* pip;        // parent minode 
    int i; 
    int ino, pino;      // inumber and parent inumber 
    int dev = running->cwd->dev; 
    
    // (2) get inumber 
    if((ino = getino(&dev, path)) == 0)
    {
        printf("Could not find directory %s\n", path); 
        return; 
    }
    // get pointer to MINODE[] of file to Dir to remove 
    mip = iget(dev, ino); 
    
    if(mip == NULL)
    {
        printf("Error: Unable to find directory %s\n", path); 
        return; 
    }
    
    // (3) Check DIR, not BUSY, and is empty 
    if(!isdir(mip))
    {
        printf("Error: %s is not a directory\n", path); 
        iput(mip); 
        return; 
    }                                   // (3.1) not BUSY 
    if(mip->refCount > 1 || mip->mounted || mip == running->cwd)
    {
        printf("Error: %s is in use\n", path); 
        iput(mip); 
        return; 
    }
    if(!isempty(mip))                   // (3.2) is empty 
    {
        printf("Error: %s is not empty\n", path); 
        iput(mip); 
        return; 
    }
    
    
    char* parentPath = strdup(Dirname(path)); 
    printf("parent path = %s\n", parentPath); 
    
    
    char* base = strdup(Basename(path)); 
    printf("base (to remove) = %s\n", base);
    
        // (5) get parent directory into memory
    pino = getino(&dev, parentPath);    // get parent ino 
    pip = iget(dev, pino);              // get parent MINODE*
    
    rm_child(pip, base); 
    
    // (6) Passed all the checks, deallocate block and inode
        // (6.1) deallocate block 
    for(i=0; i<12; i++)
    {
        if(mip->INODE.i_block[i] == 0)          // block already 0 
            continue; 
        
        // Deallocate the block 
        bdealloc(mip->dev, mip->INODE.i_block[i]); 
        
    }
    
    // (6.2) Deallocate the INODE and inumber
    idealloc(mip->dev, mip->ino); 
   
    mip->refCount=0;    // Free minode[] entry 
 
    pip->INODE.i_links_count--; 
    pip->dirty = 1; 
    
    iput(pip); 
}

/* ------------------------------------------------------------------
 * rm_child: 
 *      Remove the INODE with name from the parent INODE 
 *      
 *      (1) Search parent INODE's data blocks for entry name
 *      (2) Erase name from parent directory by 
 *              if NOT first entry 
 *                      - move all following entries in the data block forward
 *                      - add removed rec_len to the LAST entry of the block 
 *              else if it is first entry 
 *                      Deallocate the data block and modify the parent's file size 
 *      (3) Write the parent's data block back to disk 
 -------------------------------------------------------------------*/
void rm_child(MINODE* parent, char* name)
{
    int i, count = 0; 
    char buf[BLOCK_SIZE]; 
    char nameBuff[256]; 
    DIR* dp, *dPrev; 
    char* cp, *cPrev; 
    
    // (1) Search parent INODE's data blocks for entry name
    for(i=0; i<12; i++)
    {
        if(parent->INODE.i_block[i] == 0)
            break;      // exit the for loop 
        
        // Read i_block into buf 
        get_block(running->cwd->dev, parent->INODE.i_block[i], buf); 
        
        cp = buf; 
        cPrev = buf; 
        dp = (DIR *)cp; 
        dPrev += dp->rec_len;   // dPrev starts at the second block 
        
        
        while((count < parent->INODE.i_size) && (cp < &buf[BLOCK_SIZE]))
        {
          
            strncpy(nameBuff, dp->name, dp->name_len); 
            nameBuff[dp->name_len] = 0; 
            /*
            printf("name: %s\n", name); 
            printf("nameBuff: %s\n", nameBuff); 
            printf("Currently comparing %s to %s\n", name, nameBuff); 
            */
            
            
            if(strcmp(nameBuff,name)==0)
            {
                // Found the entry in the datablocks, so remove it 
                dp->inode = 0; 
                dp = (DIR *)cp; 
                dPrev->rec_len += dp->rec_len;  // absorb previous entry 
                put_block(parent->dev, parent->INODE.i_block[i], buf); 
                return; 
            }
            count += dp->rec_len; 
            cp += dp->rec_len; 
            //cPrev += dPrev->rec_len; 
            dp = (DIR *)cp; 
            dPrev += dp->rec_len; 
        }
    }
    return; 
}

/* ----------------------------------------------------------
 * list_dir: 
 *      print the directories and files of cwd or a pathname 
 * 
 *      (1) if pathname == NULL, use CWD
 *      (2) print INODE info 
 --------------------------------------------------------------*/
void list_dir(char* pathname)
{
    MINODE* dir; 
    int ino, dev; 
     
    // (1) pathname == NULL, use CWD 
    if(pathname == 0)           
    {
        ino = running->cwd->ino; 
        dev = running->cwd->dev; 
    }
    else
    {
        // Absolute path, start from root 
        if(strncmp(pathname, "/", 1)==0)
        {
            dev = root->dev; 
        }
        else    // Relative path, start from cwd 
        {
            dev = running->cwd->dev; 
        }
        
        // Get the ino number
        if(pathname[0] == '/' && strlen(pathname) == 1)  // only the root was entered
        {
            // Get ino from root 
            ino = root->ino; 
        }
        else
        {
            // Get ino from path 
            ino = getino(&dev, pathname); 
        }
    }
    
    // Could not find valid ino 
    if(ino == 0)
    {
        printf("%s is not a valid path\n", pathname); 
        return NULL; 
    }
    else
    {   
        dir = iget(dev, ino);   // Load MINODE* into minode[] table 
    } 
    
    if(dir == NULL)
    {
        printf("Unable to access %s\n no such file or directory", pathname); 
        return; 
    }
    
    printInode(dir); 
    iput(dir); 

}

/* ----------------------------------------------------------
 * ls:
 *      driver for ls 
 */
void ls(char** args, int numArgs)
{
    int i=0; 
    
    if(numArgs ==0)
    {
        list_dir(0); 
    }
    
    // if >1 arguments are passed on, do ls for each 
    for(i=0; i<numArgs; i++)
    {
        list_dir(args[i]); 
        if(i<numArgs-1)
            printf("\n\n"); 
    }
}

void printPermissionString(INODE i)
{
    char permissions[] = "---------\0"; 
    

    if((i.i_mode & 0040000) == 0040000) // directory 
        permissions[0] = 'd'; 
    else if((i.i_mode & 0120000) == 0120000) // symbolic 
        permissions[0] = 'l'; 
    else if((i.i_mode & 0060000) == 0060000) // block device 
        permissions[0] = 'b'; 
    else if((i.i_mode & 0020000) == 0020000) // character device
        permissions[0] = 'c'; 
    
    // owner 
    if((i.i_mode & 00400) == 00400)
        permissions[1] = 'r'; 
    
    if((i.i_mode & 00200) == 00200)
        permissions[2] = 'w'; 
    
    if((i.i_mode & 00100) == 00100)
        permissions[3] = 'x'; 
    
    // group 
    if((i.i_mode & 00040) == 00040)
        permissions[4] = 'r'; 
    if((i.i_mode & 00020) == 00020)
        permissions[5] = 'w'; 
    if((i.i_mode & 00010) == 00010)
        permissions[6] = 'x'; 
    
    // others 
    if((i.i_mode & 0004) == 0004)
        permissions[7] = 'r'; 
    if((i.i_mode & 0002) == 0002)
        permissions[8] = 'w';
    if((i.i_mode & 0001) == 0001)
        permissions[9] = 'x';
    
    printf("%s", permissions); 
}

void printInode(MINODE* mip)
{
    char* cp;   // current block location 
    char tBuff[BLOCK_SIZE];     // points to start of block 
    DIR* dp; 
    int i = 0; 
    MINODE* in; 
    
    char namebuff[256]; 
    char timebuff[256]; 
    
   
    if(isreg(mip))
    {
        printf("Not a directory\n"); 
        return; 
    }
    printf("------------------------------------------------------\n"); 
    
    // read blocks in tBuff
                // for each of the Direct Blocks 
    for(;i<12 && mip->INODE.i_block[i] != 0; i++)
    {
        if(!get_block(mip->dev,mip->INODE.i_block[i], tBuff))
        {
            printf("Error in get_block\n"); 
            return; 
        }
        cp = tBuff; 
        dp = (DIR *)cp;         // This is not correct 
        
        // Go through all records in a block 
        while(cp < &tBuff[BLOCK_SIZE])
        {
            
            in = iget(mip->dev, dp->inode);             // get inode
            bzero(namebuff, sizeof(namebuff));          // clear namebuff 
            strncpy(namebuff, dp->name, dp->name_len);  // fill namebuff
            namebuff[dp->name_len]=0;                   // add null 
            printPermissionString(in->INODE);           // permissions 
            printf(" "); 
            printf("%3d", in->INODE.i_links_count);             // links 
            printf("%3d", in->INODE.i_uid);                     // user 
            printf("%3d", in->INODE.i_gid);                     // group 
            ctime_r(&in->INODE.i_mtime, timebuff); 
            timebuff[24] = 0;   // remove newline 
            printf("%26s", timebuff); 
            printf("%8d", in->INODE.i_size); 
            printf("%8s", namebuff); 
            printf("\n"); 
           
            
            cp += dp->rec_len; 
            dp = (DIR *)cp; 
            
            iput(in); 
            
        }
        
    }
    
    printf("------------------------------------------------------\n"); 
   
}

/* -----------------------------------------------------
 * stat_driver: 
 *      Driver behind the stat function 
 *      that allows it to behave just as in Linux. 
 * 
 * i.e., stat dir1 dir2 dir3
 * 
 * will print the stats for all of the directories
 -----------------------------------------------------*/
void stat_driver(char** args)
{
    int i=0; 
    
    if(args[0] == NULL)
    {
        printf("Error: stat requires arguments\n"); 
        return; 
    }
    
    for(i=0; args[i] != NULL; i++)
    {
        
        do_stat(args[i]); 
        
    }
}

/* -------------------------------------------------------------
 * do_stat: print all important inode info for the path INODE 
 *      (1) get INODE of path into a minode[] table 
 *      (2) print all important info 
 * 
 * Note: This is the lazy way. KC's recommended way utilizes
 * the stat struct 
 --------------------------------------------------------------*/
void do_stat(char* path)
{
    MINODE* dir; 
    char timebuf[256]; 
    int ino; 
    int dev = running->cwd->dev; 
    
    //(1) get INODE of path into a minode[table]
    ino = getino(&dev, path);   // get ino 
    dir = iget(dev, ino);       // get MINODE* 
    
    if(dir == NULL)
    {
        printf("Error: unable to stat %s\n", path); 
        return; 
    }
    
    // Copy dir's modified time into timebuf
    ctime_r(&dir->INODE.i_mtime, timebuf); 
    timebuf[24] = 0;    // add NULL terminator 
    
    
    printf("-------------------------------------------------------\n"); 
    printf("file: %s\n", Basename(path)); 
    printf("dev: %d\t\tinode number: %i\tmode:%3x\n", dir->dev, dir->ino, dir->INODE.i_mode); 
    printf("uid: %i\tgid: %i\tlink count: %d\n", running->uid, running->gid, dir->INODE.i_links_count); 
    printf("size: %d\t\t%5s\n", dir->INODE.i_size, timebuf); 
    printf("-------------------------------------------------------\n"); 
    
    iput(dir); 
    
    
}

/* -----------------------------------------------
 * creat_driver: create files just as in Linux 
 -----------------------------------------------------*/
void creat_driver(char** args)
{
    int i=0; 
    
    if(args[0] == NULL)
    {
        printf("Error: creat requires arguments\n"); 
        return; 
    }
    
    for(i=0; args[i] != NULL; i++)
    {
        
        creat_r(args[i]); 
        
    }
}

/* ---------------------------------------------------------------
 * creat_r: 
 *      Given path of a file to be made, 
 *      get the parent's MINODE and ensure that it is a DIR
 *      and that the new file does not already exist in it 
 -------------------------------------------------------------------*/
void creat_r(char* path)
{
    MINODE* pip; 
    char* parent, *child; 
    int dev, pino; 
    
    // (1) Set device according to relative or absolute pathname 
    if(path[0] == '/')
        dev = root->dev; 
    else 
        dev = running->cwd->dev; 
    
    parent = strdup(Dirname(path)); 
    child = strdup(Basename(path));
    
    // (3) get in memory MINODE of parent directory 
        // First, get parent ino 
    pino = getino(&dev, parent); 
        // Then use it to load MINODE into minode[] table
    pip = iget(dev, pino);
    
    // (4)Verify pip
        // (4.1) was found 
    if(pip == NULL)
    {
        printf("Error: unable to locate %s\n", parent); 
    }
        // (4.2) is directory 
    else if(!isdir(pip))
    {
        printf("Error: %s is not a directory\n", parent);
    }
        // (4.3) does not already contain child
    else if(search(pip, child) != 0)    // the child was already found 
    {
        printf("Error: %s already exists in %s\n", child, parent); 
    }
    // (5) We've verified that parent path exists, is a directory 
        // and child does not exist in it, so add to it  
    else
        my_creat(pip,child); 
    
    iput(pip); 
    
}

/* ---------------------------------------------------------
 * my_creat: Creates a new file in pip
 *      very similar to my_mkdir, except that we're creating a file 
 *      instead of a directory so it's much easier. NTM we've already
 *      done my_mkdir... 
 * 
 * pip: pointer to parent MINODE
 * name: name of new file 
 -----------------------------------------------------------*/
void my_creat(MINODE* pip, char* name)
{
    int inumber, bnumber; 
    MINODE* newfile; 
    int i; 
    DIR* dp; 
    char* cp; 
    int ideallen, neededlen; 
    char buf[BLOCK_SIZE]; 
    int dev = pip->dev;         // use parent's device 
    int count; 
    
    inumber = ialloc(dev);      // allocate space, get an inumber for the file 
    if(inumber <1)
    {
        printf("Error: could not allocate space on %i\n", dev); 
    }
    
    // Load MINODE into minode[] table so we can modify INODE contents 
    newfile = iget(dev, inumber);       
    
    newfile->INODE.i_mode = FILE_MODE; 
    newfile->INODE.i_uid = running->uid; 
    newfile->INODE.i_gid = running->gid; 
    newfile->INODE.i_size = 0; 
    newfile->INODE.i_atime = newfile->INODE.i_ctime = newfile->INODE.i_mtime = time(0L); 
    newfile->INODE.i_blocks = 0; 
    newfile->INODE.i_links_count = 1; 
    
    // Write the inode to the disk 
    newfile->dirty = 1; 
    iput(newfile); 
    
    // Add to the parent's directory. Same idea as my_mkdir except we don't allocate
        // a block for it 
    neededlen = 4*((8+strlen(name)+3)/4); 
    
    for(i=0; i<12; i++)
    {
        if(pip->INODE.i_block[i] == 0)
            break; 
        
        count = 0; 
        // Read parent's data block into buf
        get_block(dev, pip->INODE.i_block[i], buf); 
        
        cp = buf; 
        dp = (DIR *)cp; 
        
        while(count < pip->INODE.i_size && cp < &buf[BLOCK_SIZE])
        {
            // Calculate the ideal length of the currently pointed to record 
            ideallen = 4*((8+dp->name_len + 3)/4); 
            
            if((dp->rec_len - ideallen) >= neededlen)
            {   //There's room in the datablocks! No need to allocate more 
                DIR* lastDir; 
                int lastDirBool; 
                
                if((cp + dp->rec_len) < &buf[BLOCK_SIZE])
                    lastDirBool = 0;    // not the last block 
                else 
                    lastDirBool = 1;    // dp points to the last data block 
                
                // Trim last entry's length to it's ideallength 
                dp->rec_len = ideallen; 
                
                // Traverse to beginning of NEW last entry 
                cp += ideallen; 
                count += ideallen; 
                
                // Add the new entry 
                lastDir = (DIR *)cp; 
                lastDir->inode = inumber;       // set inumber 
                lastDir->name_len = strlen(name);       // set name length 
                strncpy(lastDir->name, name, strlen(name));     // copy name
                
                if(lastDirBool)
                    lastDir->rec_len = BLOCK_SIZE - count; 
                else 
                    lastDir->rec_len = neededlen; 
                
                // Write the modified block back to disk 
                put_block(dev, pip->INODE.i_block[i], buf); 
                return; 
            }
            count+=dp->rec_len; 
            cp += dp->rec_len; 
            dp = (DIR *)cp; 
        }
    }
    
    
}

/* --------------------------------------------------------
 do_pwd: 
 *      recursively follow a pathname back to a root 
 * 
 *      base case: dir is a ROOT 
 *      if not base case: 
 *              (1) get_block and point DIR* dp to start 
                   of second block, which is ..
 *              (2) use dp to get parent's ino 
 *              (3) load parent MINODE using iget 
 *              (4) Recusively call do_pwd(parent) 
 *              (5) print / and directory name 
 -------------------------------------------------------------*/
void do_pwd(MINODE* dir)
{
    MINODE* parent; 
    int pino, ino;  
    char* cp; 
    DIR* dp; 
    char buf[BLOCK_SIZE]; 
    
    // (1) Base case: DIR is root 
    if(dir == root)
    {
        printf("/"); 
        return; 
    }
    
    // Read in i_block[0], 
    // which contains all of the directories contained in this DIR
    get_block(dir->dev, dir->INODE.i_block[0], buf); 
    
    // Point to the beginning of the datablock 
    cp = buf; 
    dp = (DIR*) cp; 
    
    // Get ino number of current directory 
    ino = dp->inode; 
    
    // go to second data block, get ino of .. 
    cp += dp->rec_len;            
    dp = (DIR* )cp; 
    
    // dp now points to .., the parent's directory 
    pino = dp->inode;           // get parent's ino
    
    // Load the parent MINODE*
    parent = iget(dir->dev, pino);
    
    // Call pwd with parent's MINODE pointer 
    do_pwd(parent);
    
    if(parent == NULL)
    {
        printf("Error: could not load MINODE %s", dp->name); 
        return; 
    }
    
    // (3) Print name followed by /
            // Search parent DIR for an entry with this ino 
            // Get the name associated with this ino 
    char* dirName = findmyname(parent, ino); 
    printf("%s/", dirName);
   
    iput(parent); 
}

/* -------------------------------------------------------------
 * link: Create a hard link between 2 files 
 * Usage: link oldfile newfile
 * 
 *      (1) get INODE of oldfile into memory 
 *      (2) check that oldfile is file, not DIR 
 *      (3) check newfile's parent directory exists but newfile
 *              doesn't exist in it
 *      (4) Add an entry to the data block of newfile's parent
 *              with same ino as oldfile 
 *      (5) increment the i_links of INODE by 1 
 *      (6) write INODE back to disk 
 ------------------------------------------------------------------*/
void link_(char** args)
{
    char* oldPath, *newPath;            // I.e., /a/b/c and /x/y/z
    int oldino, newPino;            // Ino of /a/b/c and /x/y !!
    int i; 
    int dev = running->cwd->dev; 
    MINODE* omip, *ndp;         // oldPath, newDirectoryParent (/x/y)
    char buf[BLOCK_SIZE]; 
    char* cp; 
    DIR* dp; 
    char nameBuff[256]; 
    
    
    // Link requires two arguments, if either are NULL inform user
    if(args[0] == NULL || args[1] == NULL)
    {
        printf("Error: link requires two pathnames\n"); 
        return; 
    }
    
  
    oldPath = strdup(args[0]);          // Copy names
    newPath = strdup(args[1]);  
    printf("Creating link from old file (%s) to new file (%s)....\n", oldPath, newPath); 
    
    // (1) Get INODE of oldfile ("/a/b/c") into memory 
    oldino = getino(&dev, oldPath);       // get ino 
    omip = iget(dev, oldino);             // get MINODE* 
    
    if(omip == NULL)
    {
        printf("Error: could not find file %s\n", args[0]); 
        return; 
    }                   
    if(!isreg(omip))  // (2) Check that oldfile is a file, not DIR 
    {
        printf("Error: cannot link directories \n"); 
        iput(omip); 
        return; 
    }
    
    // (3) check newfile's parent directory exists but newfile
                // doesn't exist in it
    newPino = getino(&dev, Dirname(newPath));   // get ino to "/x/y"
    ndp = iget(dev, newPino); 
    
    if(ndp == NULL)
    {
        printf("Error: could not find directory %s\n", Dirname(newPath));  
        return;
    }
    else if(!isdir(ndp))
    {
        printf("Error: %s is not a directory\n", Dirname(newPath)); 
        iput(ndp); 
        return; 
    }                  
    else if(search(ndp, Basename(newPath))!=0)          // Search for 'z' in /x/y
    {
        printf("Error: %s already exists in %s\n", Basename(newPath), Dirname(newPath)); 
        iput(ndp); 
        return; 
    }
     /* (4) Now add a new entry to ndp's data block with same ino as oldfile */
    my_creat(ndp, Basename(newPath)); 
    
    // The entry has been created in ndp's data blocks 
        // Now we've got to find it and change it's ino
    for(i=0; i<12; i++)
    {
        if(ndp->INODE.i_block[i] != 0)
        {
            // Read i_blocks into buf 
            get_block(dev, ndp->INODE.i_block[i], buf); 
            cp = buf;   // points to start of buf 
            dp = (DIR *)cp; 
            while(cp < &buf[BLOCK_SIZE])
            {
                strncpy(nameBuff, dp->name, dp->name_len); 
                nameBuff[dp->name_len] = 0; 
                if(strcmp(nameBuff, Basename(newPath)) == 0)    
                {
                    // Found the entry, change it's inode
                    dp->inode = oldino; 
                    put_block(dev, ndp->INODE.i_block[i], buf); 
                }
                cp += dp->rec_len;      // go to next entry 
                dp = (DIR *)cp; 
            }
        }
    }
    iput(ndp); 
    
    omip->INODE.i_links_count++;        // (5) Increment link count of oldfile
    omip->dirty =1 ; 
    iput(omip);                         //(6) write INODE back to disk 
}

/* --------------------------------------------------------------------
 * symlink: Create a symbolic link between two files 
 *      Similar to link 
 * 
 * When you've created 'z' in /x/y, set z's type to S_IFLNK
 *      Write the name of oldfile to z's i_block
 *      Write inode of /x/y/z back to disk 
 ----------------------------------------------------------------------*/
void symlink_(char** args)
{
    /* To be completed - lower priority */
}

/*-----------------------------------------------------------------------
 * unklink: Remove the link between two files 
 *      (1) get /a/b/c's INODE into memory 
 *              ino = iget(dev, filename); 
 *              rm = getino(dev, ino);
 *      (2) Verify that rm is a file (NOT DIR)
 *      (3) Decrement rm->INODE's link count by 1 
 *      (4) If rm->links_count == 1, remove the file
 *              Deallocate data blocks 
 *              Deallocate INODE 
 *      (5) Remove c from directory by 
 *              rm_child(rm, 'c')
 -----------------------------------------------------------------------*/
void ulink(char** args)
{
    MINODE* rm; 
    MINODE* parent; 
    char* filename; 
    int dev, ino, pino; 
    int i; 
    
    if(args[0] == NULL)
    {
        printf("Error: unlink requires a pathname\n"); 
        return; 
    }
    
    filename = strdup(args[0]); 
    
    // (1) Load MINODE of file to be unlinked 
    ino = getino(&dev, filename);           // get it's inumber 
    rm = iget(dev, ino);                // and load the MINODE
    
    if(rm == NULL)
    {
        printf("Error: Invalid path %s\n", filename); 
        return; 
    }
    // (2) Verify file is regular (not DIR)
    if(!isreg(rm))
    {
        printf("Error: unlink cannot be used on directories \n"); 
        iput(rm); 
        return; 
    }
    
    rm->INODE.i_links_count--;  // (3) Decrement links count 
    dev = rm->dev; 
    ino = rm->ino; 
    
    // No remaining links 
    if(rm->INODE.i_links_count < 1)
    {
        // Deallocate datablocks 
        for(i=0; i<12; i++)
            bdealloc(dev, rm->INODE.i_block[i]); 
        
        // Deallocate inode 
        for(i=0; i<12; i++)
            idealloc(dev, ino); 
        
    }
    
    // Remove from parent directory 
    pino = getino(&dev, Dirname(filename));         // parent's directory 
    parent = iget(dev, pino); 
    
    // Remove child from parent's data blocks 
    rm_child(parent, Basename(filename)); 
    
    parent->dirty = 1; 
    rm->dirty = 1; 
    iput(parent); 
    iput(rm); 
}

void mytouch(char** args)
{
    char* filename; 
    MINODE* mp; 
    int ino; 
    int dev = running->cwd->dev; 
    
    if(args[0] == NULL)
    {
        printf("Error: touch requires a filename\n"); 
        return; 
    }
    
    filename=strdup(args[0]); 
    
    ino = getino(&dev, filename);       // get inumber 
    mp = iget(dev, ino);                // get MINODE* 
    
    if(mp != NULL)      // we found it 
    {
        mp->INODE.i_atime = mp->INODE.i_mtime = time(0L); 
        mp->dirty = 1; 
        iput(mp); 
    }
    else        // Create the file 
    {
        creat_r(filename); 
        ino = getino(&dev, filename); 
        mp = iget(dev, ino); 
        
        if(mp == NULL)
        {
            printf("Failed to create file %s\n", filename); 
            return; 
        }
        mp->INODE.i_atime = mp->INODE.i_mtime = time(0L); 
        mp->dirty = 1; 
        iput(mp); 
        
    }
    
}

/* --------------- driver to for recursive do_pwd() --------------*/
void pwd()
{   
    do_pwd(running->cwd); 
    printf("\n"); 
}

void quit()
{
    iput(root); 
    printf("Unmounting root....complete.\n"); 
    iput(running->cwd); 
    printf("Unmounting cwd.... complete.\n"); 
    printf("Quitting..."); 
}

 
/* 
 * File:   functions.h
 * Author: griffin
 *
 * Created on March 29, 2012, 10:22 PM
 * 
 *  -> contains function headers for all commands as well as some helper functions
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "Global.c"
#include "util.h"
    
    void printPermissionString(INODE i);
    void printInode(MINODE* mip); 
    
    MINODE* getino_from_path(char* path); 
    
    void menu();        // print the menu 

    /* --------- mkdir functions --------- */
    void makdir_driver(char** args); 
    void do_mkdir(char* path); 
    void my_mkdir(MINODE* pip, char* name); 
    
    /* --------- rmdir functions --------- */
    void rmdir_driver(char** args); 
    void rm_dir(char* path);
    void rm_child(MINODE* parent, char* name); 
    
    /* --------- ls functions --------- */
    void list_dir(char* pathname);
    void ls(char** args, int numArgs); 
    
    /* --------- stat functions --------- */
    void stat_driver(char** args); 
    void do_stat(char* path); 
    
    /* --------- cd function --------- */
    void change_dir(char** args); 
    
    /* --------- creat functions --------- */
    void creat_driver(char** args); 
    void creat_r(char* path);
    void my_creat(MINODE* pip, char* name); 
    
    /* --------- pwd functions --------- */
    void do_pwd(MINODE* dir); 
    void pwd(); 
    
    /* --------- link functions --------- */
    void link_(char** args); 
    void symlink_(char** args); 
    void ulink(char** args); 
    
    /* --------- touch function  --------- */
    void mytouch(char** args); 
    
    void quit(); 
#ifdef	__cplusplus
}
#endif

#endif	/* FUNCTIONS_H */


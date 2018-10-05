#ifndef INODES_FUNCTIONS_H
#define INODES_FUNCTIONS_H


#include "filesystems.h"
#include "general_functions.h"
#include "SuperBlock_functions.h"


/* Create file Function */
/************************/
int pnlfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl);


/* Create directry Function */
/****************************/
int pnlfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode);


/* Unlink Function */
/*******************/
int pnlfs_unlink( struct inode* dir, struct dentry *dentry);


/* Delete directry Function */
/****************************/
int pnlfs_rmdir(struct inode *dir, struct dentry *dentry);



/* Rename Function without changement of dentry */
/*********************************************/
int normal_rename (struct inode * old_dir, struct dentry *old_dentry, struct inode *new_dir, struct dentry *new_dentry, unsigned int flags);


/* Rename Function with changement of dentry */
/*********************************************/
int cross_rename (struct inode * old_dir, struct dentry *old_dentry, struct inode *new_dir, struct dentry *new_dentry, unsigned int flags);


/*
 * old_dir : inode de l'ancien répertoire
 * old_dentry : dentry actuel du fichier / répertoire
 * new_dir : inode du nouveau répertoire
 * new_dentry : nouvelle dentry du fichier avec le nouveau noms
 *
 * Les différents flags possibles :
 * RENAME_EXCHANGE : échange le nom des 2 fichiers
 * RENAME_NOREPLACE : ne supprime/remplace pas le fichier dest s'il existe déjà
 * RENAME_WITHOUT : euh quelque chose avec des systèmes en "oignion"
 */
/* Rename Function */
/*******************/
int pnlfs_rename (struct inode *old_dir, struct dentry *old_dentry, struct inode *new_dir, struct dentry *new_dentry, unsigned int flags);


#endif

#ifndef GENERAL_FUNCTIONS_H
#define GENERAL_FUNCTIONS_H


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/statfs.h>
#include <linux/proc_fs.h>
#include <linux/types.h>

#include "filesystems.h"


struct pnlfs_inode_info *pnlfs_inode_info_of(struct inode *vfs_inode);
int first_empty_inode (void);
int first_empty_block (void);
int delete_entry( struct inode * dir, struct dentry * dentry);


/*
 * Delete a file
 * Release inode associate to bitmap
 * Release all blocks of this inode
 * Clear these blocks
 * Decrement the number of inode_index
 * WARNING : File have to be deleted of parent directory !
 *
 *
 * Supprime un fichier
 * Libère l'inode associe dans bitmap
 * Libère tous les blocs de l'inode dans bloc
 * CLEAR LES BLOCS
 * Met à jour index_inode
 * ATTENTION : Suppose que le fichier a déjà été supprimé du répertoire parent
 */
int delete_file(struct inode *ino, int nr_ino);


/*
 * Delete Empty directory
 * Release inode associate to bitmap
 * Clear the block of the directory
 * Decrement the number of inode_index
 * WARNING : Directory have to be deleted of parent directory !
 *
 *
 * Supprime un répertoire VIDE
 * Libère l'inode associe dans bitmap
 * CLEAR LE BLOC DU RÉPERTOIRE
 * Met à jour index_inode
 * ATTENTION : Suppose que le fichier a déjà été supprimé du répertoire parent */
int delete_dir(struct inode *ino, int nr_ino);
struct buffer_head *find_entry( struct inode * dir, struct dentry * dentry);


/* Verify if directory is empty */
/********************************/
int dir_is_empty( struct inode *ino);


/* Return the number of directories in specified inode */
/*******************************************************/
int nb_entries_dir( struct inode *ino);
int add_dir_entry( struct inode *dir, struct dentry *dentry, struct inode *file_ino);
int setent( struct inode *dir, struct dentry *new_dentry, struct dentry *old_dentry);



uint64_t static const CONST_T = 1;
#define EXTRACT_BIT(a,i) a & ( CONST_T << i )
#define IS_SET(a,i) ((EXTRACT_BIT(a,i)) > 0)
#define SET_BIT(a,i) a |= ( CONST_T << i)
#define UNSET_BIT(a,i) a = a & ~( CONST_T << i)


#endif

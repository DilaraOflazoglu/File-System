#ifndef PERSISTENCE_FUNCTIONS_H
#define PERSISTENCE_FUNCTIONS_H


#include "filesystems.h"
#include "general_functions.h"
#include "SuperBlock_functions.h"



/*
 * synchronize une inode
 * écrit les informations dans l'inode store
 */
int pnlfs_write_inode(struct inode *inode, struct writeback_control *wbc);


/*
 * Synchronise le super bloc et les bitmaps
 * Synchronise le superblock
 * Synchronise les blocs de bitmaps
 */
int pnlfs_sync_fs(struct super_block *sb, int wait);

#endif

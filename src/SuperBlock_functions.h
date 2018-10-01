#ifndef FONCTIONSQ15
#define FONCTIONSQ15

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/statfs.h>
#include <linux/proc_fs.h>

#include <linux/types.h>

#include "pnlfs.h"


/* called when the FS wishes to free the superblock */
/*****************************************************/
void pnl_put_super(struct super_block *sb);

/* Allocation of an Inode in the specified SuperBlock */
/******************************************************/
struct inode *pnl_alloc_inode(struct super_block *sb);


/* Destruction of specified Inode */
/**********************************/
void pnl_destroy_inode(struct inode *inode);


/* Retourn the Inode specified by ino */
/**************************************/
struct inode *pnlfs_iget(struct super_block *sb, unsigned long ino);


/* Called in partition mount function to initialize SuperBlock */
/***************************************************************/
int fill_super(struct super_block *sb, void *data, int silent);


#endif

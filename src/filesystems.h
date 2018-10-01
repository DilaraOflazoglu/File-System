#ifndef FILESYSTEMS_H
#define FILESYSTEMS_H

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


/* Cache of created SuperBlock Inode */
/**************************************/
extern struct kmem_cache *kmem_cache_inode;	


/* Have to stay, used in sync_fs fucntion to synchronized the SuperBlock */
/*************************************************************************/
extern struct buffer_head *buffer_h;


/* Structure to store some data */
/************************/
extern struct pnlfs_sb_info *sb_info;

/* SuperBlock structure */
/************************/
extern struct pnlfs_superblock *super_block;


/* This structure contains SuperBlock's functions */
/**************************************************/
static const struct super_operations pnlfs_ops;


/* This structure contains Inode's functions */
/*********************************************/
static const struct inode_operations pnlfs_file_iops;


/* This structure contains directory's functions */
/*************************************************/
static const struct file_operations pnlfs_dir_fops;


/* This structure contains file's functions */
/********************************************/
static const struct file_operations pnlfs_file_fops;


#endif

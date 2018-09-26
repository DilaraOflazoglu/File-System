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


extern struct kmem_cache *kmem_cache_inode;
extern struct buffer_head *buffer_h; // doit rester, utilisé dans sync_fs pour synchroniser le superblock
extern struct pnlfs_sb_info *sb_info;
extern struct pnlfs_superblock *super_block;


//OK
static const struct super_operations pnlfs_ops ;


//OK
static const struct file_operations pnlfs_file_fops;

static const struct file_operations pnlfs_dir_fops;

//OK
static const struct inode_operations pnlfs_file_iops;




#endif



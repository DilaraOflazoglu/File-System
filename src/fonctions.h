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


void pnl_put_super(struct super_block *sb);
struct inode *pnl_alloc_inode(struct super_block *sb);
void pnl_destroy_inode(struct inode *inode);
struct inode *pnlfs_iget(struct super_block *sb, unsigned long ino);
int fill_super(struct super_block *sb, void *data, int silent);


#endif

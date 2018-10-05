#include "filesystems.h"


struct kmem_cache *kmem_cache_inode;	/* Cache of created SuperBlock Inode */
struct buffer_head *buffer_h; 			/* Have to stay, used in sync_fs fucntion to synchronized the SuperBlock */
struct pnlfs_sb_info *sb_info;			/* Structure to store some data */
struct pnlfs_superblock *super_block;	/* SuperBlock structure */


/* This structure contains SuperBlock's functions */
/**************************************************/
static const struct super_operations pnlfs_ops = {
	.put_super = pnl_put_super,
	.alloc_inode = pnl_alloc_inode,
	.destroy_inode = pnl_destroy_inode,
	.sync_fs = pnlfs_sync_fs,
	.write_inode = pnlfs_write_inode,
	.statfs = pnlfs_statfs,
};


/* This structure contains file's functions */
/********************************************/
static const struct file_operations pnlfs_file_fops = {
	.owner = THIS_MODULE,
	.read = pnlfs_read,
	.write = pnlfs_write,
};


/* This structure contains directory's functions */
/*************************************************/
static const struct file_operations pnlfs_dir_fops = {
	.owner = THIS_MODULE,
	.iterate_shared	= pnlfs_iterate_shared,
};


/* This structure contains Inode's functions */
/*********************************************/
static const struct inode_operations pnlfs_file_iops = {
	.create		= pnlfs_create,
	.unlink		= pnlfs_unlink,
	.mkdir		= pnlfs_mkdir,
	.rmdir		= pnlfs_rmdir,
	.rename		= pnlfs_rename,
	.lookup = pnlfs_lookup,
};


/* Called in "kmem_cache_create" function to create the root Inode */
/*******************************************************************/
static void pnlfs_init_once(void *p)
{
	struct pnlfs_inode_info *ei = p;
	inode_init_once(&ei->vfs_inode);
}


/* File System Mount Function */
/******************************/
static struct dentry *pnlfs_mount(struct file_system_type *fs_type, int flags,
		const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, fill_super);
}


/* Called in "sys_unmount" function to destroy the partition */
/*************************************************************/
void kill_pnlfs_super(struct super_block *sb)
{
	brelse(buffer_h);
	kill_block_super(sb);
}


/* File system structure */
/*************************/
static struct file_system_type pnl_fs_type = {
	.owner = THIS_MODULE,
	.name     = "filesystems",
	.mount    = pnlfs_mount,
	.kill_sb  = kill_pnlfs_super,
	.next = NULL,
};


/* Module Init Function */
/************************/
static int filesystems_init(void)
{
	int err;

	kmem_cache_inode = kmem_cache_create("kmem_cache_inode",
		sizeof(struct pnlfs_inode_info), 0,
		(SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD |  SLAB_ACCOUNT | SLAB_HWCACHE_ALIGN),
		pnlfs_init_once);

	if (!kmem_cache_inode){
		pr_err("Error kmeme cache create");
		goto error_kmem_cache_inode;
	}

	err = register_filesystem(&pnl_fs_type);	/* File system registration */

	if(err){
		pr_err("Error register filesystems");
		goto error_register_filesystem;
	}

	pr_info("file_system loaded\n");

	return 0;


error_register_filesystem :
	kmem_cache_destroy(kmem_cache_inode);

error_kmem_cache_inode :
	return -ENOMEM;
}
module_init(filesystems_init);


/* Module exit Function */
/************************/
static void filesystems_exit(void)
{
	unregister_filesystem(&pnl_fs_type);	/* File system unregistration */
	pr_info("file_system unloaded\n");
}
module_exit(filesystems_exit);

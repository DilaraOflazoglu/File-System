#include "persistence_functions.h"


/*
 * synchronize une inode
 * écrit les informations dans l'inode store
 */
int pnlfs_write_inode(struct inode *inode, struct writeback_control *wbc)
{
	int block;
	struct buffer_head *bh;
	struct pnlfs_inode *tab;
	struct pnlfs_inode_info *p_info;

	pr_info("synchronise l'inode %ld \n",inode->i_ino);

	p_info = pnlfs_inode_info_of(inode);
	block = 1 + (inode->i_ino / PNLFS_INODES_PER_BLOCK);
	bh = sb_bread(inode->i_sb, block);
	if(!bh){
		pr_err("error sb_bread\n");
		return -1;
	}

	tab = (struct pnlfs_inode*) bh->b_data;
	tab[inode->i_ino % PNLFS_INODES_PER_BLOCK].mode = cpu_to_le32(inode->i_mode);
	tab[inode->i_ino % PNLFS_INODES_PER_BLOCK].index_block =  cpu_to_le32(p_info->index_block);
	tab[inode->i_ino % PNLFS_INODES_PER_BLOCK].filesize = cpu_to_le32(inode->i_size);
	if(S_ISDIR(inode->i_mode)){
		tab[inode->i_ino % PNLFS_INODES_PER_BLOCK].nr_entries = cpu_to_le32(p_info->nr_entries);
	}

	else{
		tab[inode->i_ino % PNLFS_INODES_PER_BLOCK].nr_used_blocks = cpu_to_le32(p_info->nr_entries);
	}

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	return 0;
}

/*
 * Synchronise le super bloc et les bitmaps
 * Synchronise le superblock
 * Synchronise les blocs de bitmaps
 */
int pnlfs_sync_fs(struct super_block *sb, int wait)
{
	int block;
	int i,j;
	struct buffer_head *bh = NULL;
	uint64_t *tab;

/* test si shutdown */
	if(test_bit(1,sb->s_fs_info)){
		pr_info("forced shutdown !\n");
		return 0;
	}

	if(atomic_read(&buffer_h->b_count) > 0){
	/* synchronise le super_block */
		if(sync_dirty_buffer(buffer_h)){
			pr_err("erreur sync_dirty_buffer 1\n");
			return -1;
		}
	}
/* synchronise les bitmaps lit les blocks correspondants au bitmap ifree */
	block = 1 + sb_info->nr_istore_blocks;
	for(j = 0; j < sb_info->nr_ifree_blocks; j++){
		bh = sb_bread(sb, block+j);
		if(!bh){
			pr_err("erreur sb_bread\n");
			return -1;
		}
	/* écrit le block */
		tab = (uint64_t *)bh->b_data;
		for(i=0;i < super_block->nr_inodes / 64 ; i++)
			tab[i] = cpu_to_le64( sb_info->ifree_bitmap[i]);

		mark_buffer_dirty(bh);
		if(sync_dirty_buffer(bh)){
			pr_err("erreur sync_dirty_buffer 2\n");
			goto end_sync;
		}

		brelse(bh);
	}

	// lit les blocks correspondants au bitmap bfree
	block = 1 + sb_info->nr_istore_blocks + sb_info->nr_ifree_blocks;
	for(j= 0 ; j < sb_info->nr_ifree_blocks ; j++){
		pr_info("lis le block bfree %d\n",block + j);
		bh = sb_bread(sb, block+j);
		if(!bh){
			pr_err("erreur sb_bread\n");
			return -1;
		}
		// écrit le block
		tab = (uint64_t *)bh->b_data;
		for(i = 0;i < super_block->nr_inodes / 64; i++){
			tab[i] = cpu_to_le64( sb_info->bfree_bitmap[i]);
		}
		mark_buffer_dirty(bh);
		if(sync_dirty_buffer(bh)){
			pr_err("erreur sync_dirty_buffer 3\n");
			goto end_sync;
		}
		brelse(bh);
	}
	pr_info("success\n");

	return 0;

end_sync:
	brelse(bh);
	return -1;
}

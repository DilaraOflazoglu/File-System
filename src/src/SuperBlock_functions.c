#include "./../include/filesystems.h"
#include "./../include/SuperBlock_functions.h"
#include "./../include/general_functions.h"


MODULE_DESCRIPTION("Creation of a File System");
MODULE_AUTHOR("Dilara OFLAZOGLU <DilaraOflazoglu@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1");


/* called when the FS wishes to free the superblock */
/*****************************************************/
void pnl_put_super(struct super_block *sb)
{
	kfree(sb_info->ifree_bitmap);
	kfree(sb_info->bfree_bitmap);
	kfree(sb_info);
	sb->s_fs_info = NULL;
	pr_info("je suis dans put super\n");
}


/* Allocation of an Inode in the specified SuperBlock */
/******************************************************/
struct inode *pnl_alloc_inode(struct super_block *sb)
{
	struct pnlfs_inode_info *inode_info;

	inode_info = kmem_cache_alloc(kmem_cache_inode, GFP_KERNEL);

	if (!inode_info){
		pr_err("Error kmem_cache_alloc");
		return NULL;
	}

	return &inode_info->vfs_inode;
}


/* Destruction of specified Inode */
/**********************************/
void pnl_destroy_inode(struct inode *inode)
{
	struct pnlfs_inode_info *inode_info;
	pr_info("inode est détruite\n");
	inode_info = pnlfs_inode_info_of(inode);
	kmem_cache_free(kmem_cache_inode, inode_info);
}


/* Retourn the Inode specified by ino */
/**************************************/
struct inode *pnlfs_iget(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;
	struct buffer_head *bh;
	struct pnlfs_inode *pnlfs_inode;
    struct pnlfs_inode_info *pnlfs_inode_info;

	inode = iget_locked(sb, ino); // augmente le compteur de référence quand lock l'inode

	if (!inode){
		pr_err("Error iget locked");
		goto error_iget_locked;
	}

/* Si l'inode n'est pas présente dans le cache */
	if (inode->i_state & I_NEW) {

		bh = sb_bread(sb, (ino/PNLFS_INODES_PER_BLOCK) + 1);
		if (!bh){
			pr_err("Error sb_bread");
			goto error_sb_bread;
		}

		pnlfs_inode = (struct pnlfs_inode *)(bh->b_data +
					((ino % PNLFS_INODES_PER_BLOCK) * sizeof(struct pnlfs_inode)));


	 	inode->i_mode = le32_to_cpu(pnlfs_inode->mode);
	 	if(S_ISDIR(inode->i_mode)){
			inode->i_op = &pnlfs_file_iops;
			inode->i_fop = &pnlfs_dir_fops;
		}

		else{
			inode->i_op = &pnlfs_file_iops;
			inode->i_fop = &pnlfs_file_fops;
		}

	 	inode->i_ino = ino;
	 	inode->i_sb = sb;
	 	inode->i_size = le32_to_cpu(pnlfs_inode->filesize);
	 	inode->i_blocks = le32_to_cpu(pnlfs_inode->nr_entries);

		inode->i_mtime = CURRENT_TIME;
		inode->i_ctime = CURRENT_TIME;
		inode->i_atime = CURRENT_TIME;

		inode->i_private = pnlfs_inode;
		pnlfs_inode_info = pnlfs_inode_info_of(inode);
        pnlfs_inode_info->nr_entries = le32_to_cpu(pnlfs_inode->nr_entries);
        pnlfs_inode_info->index_block = le32_to_cpu(pnlfs_inode->index_block);

		brelse(bh);
		unlock_new_inode(inode);
	}

	return inode;


error_sb_bread :
	iget_failed(inode);

error_iget_locked :
	return ERR_PTR(-ENOENT);
}


/* Called in partition mount function to initialize SuperBlock */
/***************************************************************/
int fill_super(struct super_block *sb, void *data, int silent)
{
	int i, j;
	uint64_t t;
	struct inode *inode_racine;
	struct buffer_head *buffer_tmp;
	int block_ifree, block_bfree, taille_bitmap_ifree, taille_bitmap_bfree, index;

	sb->s_root = NULL;

	sb->s_magic = PNLFS_MAGIC;				/* nombre magic */
	sb->s_blocksize = PNLFS_BLOCK_SIZE;		/* taille max d'un fichier */
	sb->s_maxbytes = PNLFS_MAX_FILESIZE;	/* Taille max d'un fichier */

	sb->s_op = &pnlfs_ops;

	buffer_h = sb_bread(sb, 0);				/* super bloc dans le bloc 0 */
	if (!buffer_h){
		if(!silent)
			pr_err("Error read bloc");
		goto error_read_block;
	}

	super_block = (struct pnlfs_superblock*)buffer_h->b_data;

	sb_info = kmalloc(sizeof(struct pnlfs_sb_info), GFP_KERNEL);
	if(!sb_info){
		if(!silent)
			pr_err("Error malloc sb_info");
		goto error_malloc_sb_info;
	}

	sb->s_fs_info = sb_info;

/* Copie des données récuperer par la struct superBlock dans le sb_info */
	sb_info->nr_blocks = le32_to_cpu(super_block->nr_blocks);
	sb_info->nr_inodes = le32_to_cpu(super_block->nr_inodes);

	sb_info->nr_istore_blocks = le32_to_cpu(super_block->nr_istore_blocks);
	sb_info->nr_ifree_blocks = le32_to_cpu(super_block->nr_ifree_blocks);
	sb_info->nr_bfree_blocks = le32_to_cpu(super_block->nr_bfree_blocks);

	sb_info->nr_free_inodes = le32_to_cpu(super_block->nr_free_inodes);
	sb_info->nr_free_blocks = le32_to_cpu(super_block->nr_free_blocks);


	taille_bitmap_ifree = super_block->nr_inodes / 64 ; // chaque entrée de la bitmap a 64 bits
	sb_info->ifree_bitmap = kmalloc(taille_bitmap_ifree*sizeof(unsigned long), GFP_KERNEL);
	pr_info("taille %d \n",taille_bitmap_ifree);
	if(!sb_info->ifree_bitmap){
		if(!silent)
			pr_err("Error malloc ifree");
		goto error_malloc_sb_info;
	}

	block_ifree = 1 + super_block->nr_istore_blocks; // 1er block qui contient la bitmap ifree, c'est ça d'après le sujet ?
	pr_info("se place au bloc %d\n",block_ifree);
	pr_info("apres modif\n");
	index = 0;
	for(i=0; i < sb_info->nr_ifree_blocks ; i++){ // pour chaque ifree block
		buffer_tmp = sb_bread(sb, block_ifree + i);
		if(!buffer_tmp){
			pr_err("error bitmap_ifree read");
			goto error_malloc_sb_info;
		}
		for(j=0; j < PNLFS_BLOCK_SIZE; j+=8){
			memcpy(&t, buffer_tmp->b_data + j, sizeof(unsigned long)); // pour stocker le nombre dans un uint64
			sb_info->ifree_bitmap[index] = le64_to_cpu(t); // pour faire la conversion que le prof veut
			index ++;
			if(j== taille_bitmap_ifree) // finit de lire la bitmap, ce qui suit dans le block est vide
				break;
		}
		brelse(buffer_tmp);
	}



	taille_bitmap_bfree = super_block->nr_blocks / 64 ; // chaque entrée de la bitmap a 64 bits
	sb_info->bfree_bitmap = kmalloc(taille_bitmap_bfree*sizeof(unsigned long), GFP_KERNEL);
	if(!sb_info->bfree_bitmap){
		if(!silent)
			pr_err("Error malloc ifree");
		goto error_malloc_bfree;
	}

	block_bfree = 1 + super_block->nr_istore_blocks + super_block->nr_ifree_blocks; // 1er block qui contient la bitmap ifree, c'est ça d'après le sujet ?

	index = 0;
	for(i=0; i < sb_info->nr_bfree_blocks ; i++){ // pour chaque ifree block
		buffer_tmp = sb_bread(sb, block_bfree + i);
		if(!buffer_tmp){
			pr_err("error bitmap_bfree read");
			goto error_malloc_sb_info;
		}
		for(j=0; j < PNLFS_BLOCK_SIZE; j+=8){
			memcpy(&t, buffer_tmp->b_data+j, sizeof(unsigned long)); // pour stocker le nombre dans un uint64
			sb_info->bfree_bitmap[index] = le64_to_cpu(t); // pour faire la conversion voulue
			index ++;
			if(j== taille_bitmap_bfree) // finit de lire la bitmap, ce qui suit dans le block est vide
				break;
		}
		brelse(buffer_tmp);
	}


/* inode racine  = inode 0 */
	inode_racine = pnlfs_iget(sb, 0);

	if (IS_ERR(inode_racine)){
		if(!silent)
			pr_err("Error pnlfs_iget");
		goto error_last_out;
	}
	inode_init_owner(inode_racine, NULL, inode_racine->i_mode);
	sb->s_root = d_make_root(inode_racine);

	if (!sb->s_root){
		if(!silent)
			pr_err("Error d_make_root");
		goto error_last_out;
	}
	return 0;

error_last_out :
	kfree(sb_info->bfree_bitmap);

error_malloc_bfree :
	kfree(sb_info->ifree_bitmap);

error_malloc_sb_info :
	brelse(buffer_h);

error_read_block :
	return -EIO;
}

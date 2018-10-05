#include "general_functions.h"


struct pnlfs_inode_info *pnlfs_inode_info_of(struct inode *vfs_inode)
{

	int ecart;
    struct pnlfs_inode_info c;

	ecart = (void *)&(c.vfs_inode)-(void *)&(c);
	return  (struct pnlfs_inode_info *)((void*)vfs_inode-ecart);
}


int first_empty_inode ()
{
	char tab[65];
	int i, j, k, remplissage;

	remplissage = (super_block->nr_inodes % 64 == 0) ? 64 : super_block->nr_inodes % 64;
	pr_info("seek first empty inode\n");

	for(i=0;i< super_block->nr_inodes/64 - 1 ;i++){ // pour chaque u64 de la bitmap (sauf dernier qui n'est peut-être pas tout à fait remplit)
		pr_info("%ld\n",sb_info->ifree_bitmap[i]);
		for(j=0;j<64;j++){ // pour chaque bit
			if(j==0){
				for(k=0;k<64;k++){
					tab[k] = IS_SET(sb_info->ifree_bitmap[0],(63-k)) + '0';
				}
				tab[64] = 0;
				pr_info("entrée 0 : %s\n",tab);
			}
			if(EXTRACT_BIT( sb_info->ifree_bitmap[i], j)){ // ATTENTION D'APRES MKFS_PNLFS 1ERE INODE DU LONG INT VIDE -> POSITION 0 ET PAS 63 comme dans ((sb_info->ifree_bitmap[i] >>(63-j) )&1) == 1) // bit j de l'emplacement i
				UNSET_BIT(sb_info->ifree_bitmap[i], j);
				super_block->nr_free_inodes --;
				sb_info->nr_free_inodes--;
				mark_buffer_dirty(buffer_h);
				sync_dirty_buffer(buffer_h);
				// synchronise la bitmaps dans sync
				return i*64+j;
			}
		}
	}
	// pour le dernier block
	for(j=0;j < remplissage;j++){ // pour chaque bit
		if(EXTRACT_BIT( sb_info->ifree_bitmap[i], j)){ // bit j de l'emplacement i
			SET_BIT(sb_info->ifree_bitmap[i], j);
			super_block->nr_free_inodes --;
			sb_info->nr_free_inodes--;
			mark_buffer_dirty(buffer_h);
			sync_dirty_buffer(buffer_h);
			// synchronise la bitmaps dans sync
			return i*64+j;
		}
	}
	return -EINVAL;
}


int first_empty_block ()
{
	int i, j, remplissage;

	remplissage = (super_block->nr_blocks % 64 == 0) ? 64 : super_block->nr_blocks % 64;

	for(i=0;i< super_block->nr_blocks/64 - 1 ;i++){ // pour chaque u64 de la bitmap (sauf dernier qui n'est peut-être pas tout à fait remplit)
		for(j=0;j<64;j++){ // pour chaque bit
			if(EXTRACT_BIT(sb_info->bfree_bitmap[i], j)){ // ATTENTION D'APRES MKFS_PNLFS 1ERE INODE DU LONG INT VIDE -> POSITION 0 ET PAS 63 comme dans ((sb_info->ifree_bitmap[i] >>(63-j) )&1) == 1) // bit j de l'emplacement i
				UNSET_BIT(sb_info->bfree_bitmap[i], j);
				super_block->nr_free_blocks --;
				sb_info->nr_free_blocks --;
				mark_buffer_dirty(buffer_h);
				sync_dirty_buffer(buffer_h);
				// synchronise la bitmaps dans sync
				return i*64+j;
			}
		}
	}
	// pour le dernier block
	for(j=0;j < remplissage;j++){ // pour chaque bit
		if( EXTRACT_BIT( sb_info->bfree_bitmap[i], j)){ // bit j de l'emplacement i
			SET_BIT(sb_info->bfree_bitmap[i], j);
			super_block->nr_free_blocks --;
			sb_info->nr_free_blocks --;
			mark_buffer_dirty(buffer_h);
			sync_dirty_buffer(buffer_h);
			// synchronise la bitmaps dans sync
			return i*64+j;
		}
	}
	return -EINVAL;
}


int delete_entry( struct inode * dir, struct dentry * dentry)
{
	int i,ino;
	struct pnlfs_dir_block *directory;
	struct pnlfs_inode_info *p_ino = pnlfs_inode_info_of(dir);
	struct buffer_head *bh = sb_bread(dir->i_sb, p_ino->index_block);

	if(IS_ERR(bh)){
		pr_err("error sb_bread\n");
		return PTR_ERR(bh);
	}
/* cherche l'entrée dans le répertoire */
	directory = (struct pnlfs_dir_block*) bh->b_data;

	for(i=0;i<p_ino->nr_entries;i++){
		if (strncmp( directory->files[i].filename, dentry->d_name.name, dentry->d_name.len) == 0){
			ino = directory->files[i].inode;
			// ici prend la dernière entrée et la met à la place du fichier supprimé
			// le répertoire a au moins un fichier (celui quu'on supprime)
			directory->files[i] = directory->files[p_ino->nr_entries-1];
			memset(directory->files +p_ino->nr_entries - 1, 0, sizeof(__le32) + PNLFS_FILENAME_LEN);
			p_ino->nr_entries --;
			mark_buffer_dirty(bh);
			sync_dirty_buffer(bh);
			mark_inode_dirty(dir); // nrentries--
			brelse(bh);
			return ino;
		}
	}
	// a parcouru le dossier sans trouver le répertoire
	brelse(bh);
	return -ENOENT;
}


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
int delete_file(struct inode *ino, int nr_ino)
{
	int i;
	struct buffer_head *bh, *bh2;
	struct pnlfs_file_index_block *occupied_blocks;
	struct pnlfs_inode_info *p_ino = pnlfs_inode_info_of(ino);

/* libère l'inode associé */
	bitmap_set( sb_info->ifree_bitmap, nr_ino, 1);

/* lit le bloc contenant les blocs utilisés par l'inode pour les libérer */
	pr_info("read block %d\n",p_ino->index_block);
	bh = sb_bread(ino->i_sb,  p_ino->index_block);
	if(!bh){
		pr_err("error sb_bread\n");
		return -EINVAL;
	}
	occupied_blocks = (struct pnlfs_file_index_block*) bh->b_data;
	i = 0;
	while(occupied_blocks->blocks[i]){
		pr_info ("read block %d\n",occupied_blocks->blocks[i]);
		bh2 = sb_bread(ino->i_sb, occupied_blocks->blocks[i]);
		if(!bh2){
			pr_err("error sb_bread2");
			brelse(bh);
			return 1;
		}
		memset(bh2->b_data, 0, PNLFS_BLOCK_SIZE);
		mark_buffer_dirty(bh2);
		sync_dirty_buffer(bh2);
		brelse(bh2);
		bitmap_set( sb_info->bfree_bitmap, occupied_blocks->blocks[i], 1);
		i++;
		super_block->nr_free_blocks++;
		sb_info->nr_free_blocks++;
	}
	memset(bh->b_data, 0, PNLFS_BLOCK_SIZE);
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

/* pas besoin de mettre à jour l'inode_index car des données seront réécrites
sur la place lorsque l'inode sera réallouée, met à jour l'index_inode */

	super_block->nr_free_inodes++;
	sb_info->nr_free_inodes++;
	mark_buffer_dirty(buffer_h);
	sync_dirty_buffer(buffer_h);

	return 0;
}



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
 * ATTENTION : Suppose que le repertoire a déjà été supprimé du répertoire parent
 */
int delete_dir(struct inode *ino, int nr_ino)
{
	struct buffer_head *bh;
	struct pnlfs_inode_info *p_ino;

	p_ino = pnlfs_inode_info_of(ino);

/* libère l'inode associé */
	bitmap_set( sb_info->ifree_bitmap, nr_ino, 1);

/* lit le bloc contenant les blocs utilisés par l'inode pour les libérer */
	bh = sb_bread(ino->i_sb,  p_ino->index_block);
	if(!bh){
		pr_err("sb_bread");
		return -1;
	}

	memset(bh->b_data, 0, PNLFS_BLOCK_SIZE);
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

/*  pas besoin de mettre à jour l'inode_index car des données seront réécrites
 sur la place lorsque l'inode sera réallouée met à jour l'index_inode */
	super_block->nr_free_blocks++;
	sb_info->nr_free_blocks++;
	super_block->nr_free_inodes++;
	sb_info->nr_free_inodes++;
	mark_buffer_dirty(buffer_h);
	sync_dirty_buffer(buffer_h);
	return 0;
}



//OK
struct buffer_head *find_entry(struct inode *dir, struct dentry *dentry){
	int i;
	struct buffer_head *bh;
	struct pnlfs_inode_info *p_ino;
	struct pnlfs_dir_block *directory;

	p_ino = pnlfs_inode_info_of(dir);
	bh = sb_bread(dir->i_sb, p_ino->index_block);

	if(IS_ERR(bh)){
		pr_err("sb_bread");
		return ERR_PTR(EINVAL);
	}
/* cherche l'entrée dans le répertoire */
	directory = (struct pnlfs_dir_block*) bh->b_data;

	for(i=0;i<p_ino->nr_entries;i++){
		if (strncmp( directory->files[i].filename, dentry->d_name.name, dentry->d_name.len) == 0)
			return bh;
	}

/* a parcouru le dossier sans trouver le répertoire */
	brelse(bh);
	return ERR_PTR(ENOENT);
}


/* Verify if directory is empty */
/********************************/
int dir_is_empty( struct inode *ino){
	struct pnlfs_inode_info *p_ino = pnlfs_inode_info_of(ino);
	return p_ino->nr_entries == 0;
}


/* Return the number of directories in specified inode */
/*******************************************************/
int nb_entries_dir( struct inode *ino){
	struct pnlfs_inode_info *p_ino = pnlfs_inode_info_of(ino);
	return p_ino->nr_entries;
}


int add_dir_entry( struct inode *dir, struct dentry *dentry, struct inode *file_ino){
	struct buffer_head *bh;
	struct pnlfs_inode_info *p_ino;
	struct pnlfs_dir_block *directory;

	p_ino = pnlfs_inode_info_of(dir);
	bh = sb_bread(dir->i_sb, p_ino->index_block);

	if(IS_ERR(bh)){
		pr_err("error sb_bread");
		return PTR_ERR(bh);
	}
	/* cherche l'entrée dans le répertoire */
	directory = (struct pnlfs_dir_block*) bh->b_data;

	if(p_ino->nr_entries >= 127){ // Dictionnaire est remplit
		brelse(bh);
		pr_err("error sb_bread");
		return -EMLINK;
	}

	directory->files[p_ino->nr_entries].inode = file_ino->i_ino;
	strncpy(directory->files[p_ino->nr_entries].filename, dentry->d_name.name, PNLFS_FILENAME_LEN);
	p_ino->nr_entries ++;

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	mark_inode_dirty(file_ino); // marque dirty car a changé la struct pnlfs_inode_info ?? ou faut le mettre dans l'inode store ??
	dir->i_ctime = CURRENT_TIME;
	dir->i_mtime = CURRENT_TIME;
	dir->i_version++;
	mark_inode_dirty(dir);
	brelse(bh);
	return 0;
}


//OK
// cherche l'entrée existante et la remplace par le nouveau fichier
int setent( struct inode *dir, struct dentry *new_dentry, struct dentry *old_dentry){
	int i;
	struct buffer_head *bh;
	struct pnlfs_inode_info *p_ino;
	struct pnlfs_dir_block *directory;

	p_ino = pnlfs_inode_info_of(dir);
	bh = sb_bread(dir->i_sb, p_ino->index_block);

	if(IS_ERR(bh)){
		pr_err("sb_bread");
		return PTR_ERR(bh);
	}

/* cherche l'entrée dans le répertoire */
	directory = (struct pnlfs_dir_block*) bh->b_data;

	for(i=0;i<p_ino->nr_entries;i++){
		if (strncmp( directory->files[i].filename, old_dentry->d_name.name, PNLFS_FILENAME_LEN) == 0)
			break;
	}

/* a parcouru le dossier sans trouver le répertoire */
	if(i == p_ino->nr_entries){
		brelse(bh);
		return -ENOENT;
	}

	else{
		strncpy( directory->files[i].filename, new_dentry->d_name.name, PNLFS_FILENAME_LEN);
		directory->files[i].inode = old_dentry->d_inode->i_ino;
	}

	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	dir->i_ctime = CURRENT_TIME;
	dir->i_mtime = CURRENT_TIME;
	dir->i_version++;
	mark_inode_dirty(dir);
	brelse(bh);
	return 0;
}

#include "filesystems.h"
#include "general_fonctions.h"
#include "SuperBlock_functions.h"


/* Create file Function */
/************************/
int pnlfs_create(struct inode *dir, struct dentry *dentry,
										umode_t mode, bool excl)
{
	struct inode *inode;
	struct buffer_head *bh;
	struct pnlfs_inode_info *p_ino;
	struct pnlfs_inode* index_inode;
	int inode_libre, block_libre, nr_inode_store, err;

	pr_info("ENTRE DANS CREATE\n");

	inode_libre = first_empty_inode();
	if(inode_libre == -EINVAL){
		pr_err("No inodes empty");
		return inode_libre;
	}

	pr_info("inode libre %d\n",inode_libre);

	block_libre = first_empty_block();
	if(block_libre == -EINVAL){
		pr_err("No empty block");
		err = block_libre;
		goto error_max_block;
	}

	pr_info("inode libre %d block_libre %d\n",inode_libre,block_libre);

	inode = pnlfs_iget( dir->i_sb,inode_libre);
	if(IS_ERR(inode)){
		err = PTR_ERR(inode);
		pr_err("Error get inode");
		goto error_get_inode;
	}

	inode->i_op = &pnlfs_file_iops;
	inode->i_fop = &pnlfs_file_fops;
	inode->i_mode = mode;


/* ajoute l'entrée au répertoire parent */
	err = add_dir_entry(dir, dentry, inode);
	if(err){
		pr_err("error add_dir_entry");
		goto error_add_dir_entry;
	}

/* met à jour son pnlfs_iode_info */
	p_ino = pnlfs_inode_info_of(inode);
	p_ino->index_block = block_libre;
	p_ino->nr_entries = 0;


/* met à jour l'inode store
 * Va chercher le bloc de l'inode store dans lequel sera stocké l'inode
 * Met à jour l'inode store pour être sûr de pouvoir lire dedans tout de
 * suite et ne pas avoir besoin d'attendre qu'il y ai un sync
*/

	nr_inode_store = 1+inode_libre / PNLFS_INODES_PER_BLOCK;
	bh = sb_bread( dir->i_sb, nr_inode_store);

	if(!bh){
		pr_err("error sb_bread inode_store");
		err = PTR_ERR(bh);
		goto error_add_dir_entry;
	}

	index_inode = (struct pnlfs_inode *)bh->b_data;
	index_inode[inode_libre].mode = cpu_to_le32(inode->i_mode);
	index_inode[inode_libre].index_block = cpu_to_le32(block_libre);
	index_inode[inode_libre].filesize = cpu_to_le32(0);
	index_inode[inode_libre].nr_used_blocks = cpu_to_le32(1);
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	d_instantiate(dentry,inode);

	return 0;

	error_add_dir_entry:
		iput(inode);
	error_get_inode:
		iget_failed(inode);
		UNSET_BIT(sb_info->bfree_bitmap[inode_libre/64], block_libre %64);

	error_max_block:
		UNSET_BIT(sb_info->ifree_bitmap[inode_libre/64], inode_libre %64);

	return err;
}


/* Create directry Function */
/****************************/
int pnlfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
	struct buffer_head *bh;
	struct pnlfs_inode* index_inode;
	int inode_libre, block_libre, nr_inode_store;
	struct inode *inode;
	struct pnlfs_inode_info *p_ino;
	int err;

	inode_libre = first_empty_inode();
	if(inode_libre == -EINVAL ){
		pr_err("No inodes empty");
		return inode_libre;
	}

	pr_info("inode libre %d\n",inode_libre);

	block_libre = first_empty_block();
	if(block_libre == -EINVAL ){
		pr_err("No empty block");
		err = block_libre;
		goto error_max_block;
	}

	pr_info("inode libre %d block_libre %d\n",inode_libre,block_libre);

	inode = pnlfs_iget( dir->i_sb,inode_libre);
	if(IS_ERR(inode)){
		err = PTR_ERR(inode);
		pr_err("Error get inode");
		goto error_get_inode;
	}

	inode->i_op = &pnlfs_file_iops;
	inode->i_fop = &pnlfs_dir_fops;
	inode->i_mode = S_IFDIR | mode ;


	// ajoute l'entrée au répertoire parent
	err = add_dir_entry(dir, dentry, inode);
	if(err)	{
		pr_err("error add_dir_entry");
		goto error_add_dir_entry;
	}

/* met à jour son pnlfs_iode_info */
	p_ino = pnlfs_inode_info_of(inode);
	p_ino->index_block = block_libre;
	p_ino->nr_entries = 0;


/* met à jour l'inode store
 * Va chercher le bloc de l'inode store dans lequel sera stocké l'inode
 * Met à jour l'inode store pour être sûr de pouvoir lire dedans tout de
 * suite et ne pas avoir besoin À attendre qu'il y ai un sync
*/

	nr_inode_store = 1+inode_libre / PNLFS_INODES_PER_BLOCK;
	bh = sb_bread( dir->i_sb, nr_inode_store);
	if(!bh){
		pr_err("error sb_bread inode_store");
		err = PTR_ERR(bh);
		goto error_add_dir_entry;
	}
	index_inode = (struct pnlfs_inode *)bh->b_data;
	index_inode[inode_libre].mode = cpu_to_le32(inode->i_mode);
	index_inode[inode_libre].index_block = cpu_to_le32(block_libre);
	index_inode[inode_libre].filesize = cpu_to_le32(PNLFS_BLOCK_SIZE);
	index_inode[inode_libre].nr_used_blocks = cpu_to_le32(1);
	mark_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

	// unlock_new_inode(inode); A faire ou pas ??????
	d_instantiate(dentry,inode);
	//~ iput(inode); // ne fait pas de iput ici car inode est liée au dentry
	return 0;

	error_add_dir_entry:
		iput(inode);
	error_get_inode:
		UNSET_BIT(sb_info->bfree_bitmap[inode_libre/64], block_libre %64);

	error_max_block:
		UNSET_BIT(sb_info->ifree_bitmap[inode_libre/64], inode_libre %64);

	return err;
}


/* Unlink Function */
/*******************/
int pnlfs_unlink( struct inode* dir, struct dentry *dentry)
{
	int err = -ENOENT;
	struct buffer_head *bh;
	struct pnlfs_dir_block *directory;
	int ino;


	bh = sb_bread(dir->i_sb, dir->i_ino);
	directory = (struct pnlfs_dir_block*) bh->b_data;
	// supprime l'entrée dans le répertoire parent
	if((ino = delete_entry(dir,dentry) ) == -1){
		pr_err("fichier non contenu !\n");
		brelse(bh);
		return -1;
	}

	// cherche si doit supprimer le fichier
	if(dentry->d_inode->i_nlink == 0){
		pr_err("Deleting file %s with no links\n", dentry->d_name.name);
		set_nlink(dentry->d_inode,1);
	}

	dir->i_ctime = dir->i_mtime = CURRENT_TIME;
	drop_nlink(dentry->d_inode);
	mark_inode_dirty(dir);

	if (!dentry->d_inode->i_nlink){
		err = delete_file(dentry->d_inode,ino); // supprime le fichier
		if(err){
			brelse(bh);
			return -1;
		}
	}
	dentry->d_inode->i_ctime = CURRENT_TIME;
	mark_inode_dirty(dentry->d_inode);
	brelse(bh);

	return 0;
}


/* Delete directry Function */
/****************************/
int pnlfs_rmdir(struct inode *dir, struct dentry *dentry)
{
	struct buffer_head *bh;
	int err = 0;

	// cherche si l'entrée existe dans le répertoire
	bh = find_entry(dir,dentry);
	if (IS_ERR(bh)){
		pr_err("Entry not find\n");
		err = PTR_ERR(bh);
		goto err_find_entry;
	}

	//check si l'image est vide
	if (!dir_is_empty(dentry->d_inode)){
		pr_err("dir_not_empty\n");
		err = ENOTEMPTY;
		goto end_rmdir;
	}

	// faut supprimer l'entrée du répertoire
	if( !delete_entry(dir,dentry)){
		pr_err("cannot delete entry\n");
		err =ENOENT;
		goto end_rmdir;
	}

	dir->i_ctime = dir->i_mtime = CURRENT_TIME;
	drop_nlink(dentry->d_inode);
	mark_inode_dirty(dir);

	pr_info("libere l'inode %ld\n",dentry->d_inode->i_ino);
	err = delete_dir(dentry->d_inode,dentry->d_inode->i_ino);
	if (err){ // supprime le fichier représentant le répertoire

		pr_err("error delete_dir");
		goto end_rmdir;
	}

	dentry->d_inode->i_ctime = CURRENT_TIME;
	mark_inode_dirty(dentry->d_inode);

	brelse(bh);
	return err;

	end_rmdir:
		brelse(bh);
	err_find_entry:
		return err;
}


/* Rename Function without changement of dentry */
/*********************************************/
int normal_rename (struct inode * old_dir, struct dentry *old_dentry, struct inode *new_dir, struct dentry *new_dentry, unsigned int flags)
{
	struct buffer_head *bh_old = NULL, *bh_new = NULL;
	int retval;

	pr_info("affiche une erreur %d\n",IS_ERR( ERR_PTR(ENOENT)));
	bh_old = find_entry(old_dir, old_dentry); // regarde si le fichier existe
	if(IS_ERR(bh_old))
		return PTR_ERR(bh_old);
	if(!bh_old){
		pr_err("File doesn't exist\n");
		retval = ENOENT;
		return retval;
	}

	bh_new = find_entry(new_dir, new_dentry); // regarde si le fichier renommé existe déjà
	if(PTR_ERR(bh_new) == EINVAL){
		retval = PTR_ERR(bh_new);
		bh_new = NULL;
		goto end_rename;
	}

	if(bh_new != ERR_PTR(ENOENT)){ // le fichier dest existe
		pr_info("ne doit pas entrer ici\n");
		if(!new_dentry->d_inode){ // inode négative pour la nouvelle entrée ->n'existe pas encore
			brelse(bh_new);
			bh_new = NULL;
		}
	}

	if (S_ISDIR(old_dentry->d_inode->i_mode)) // si le fichier à renommer est un répertoire
	{
		if (new_dentry->d_inode) // le fichier dest existe déjà
		{
			// test si le répertoire est vide
			if (!dir_is_empty(new_dir))
			{
				retval = -ENOTEMPTY;
				goto end_rename;
			}
		}
		else
		{
			retval = -EMLINK;
			if (new_dir != old_dir && nb_entries_dir(new_dir) >= PNLFS_MAX_DIR_ENTRIES)
				goto end_rename;
		}
	}

	if (PTR_ERR(bh_new) == EINVAL) // faut créer l'entrée dans le répertoire dest
	{
		retval = add_dir_entry( new_dir, new_dentry, old_dentry->d_inode);
		if(retval)
			goto end_rename;
	}
	else
	{
		retval = setent(new_dir, new_dentry, old_dentry);
		if(retval)
			goto end_rename;
	}

	old_dentry->d_inode->i_ctime = CURRENT_TIME;
	mark_inode_dirty(old_dentry->d_inode);

	if (new_dentry->d_inode)
	{
		drop_nlink(new_dentry->d_inode);
		new_dentry->d_inode->i_ctime = CURRENT_TIME;

	}

	old_dir->i_ctime = old_dir->i_mtime = CURRENT_TIME;

	if (S_ISDIR(old_dentry->d_inode->i_mode)) // rename un répertoire
	{
		if (new_dentry->d_inode)
			clear_nlink(new_dir); // a check au-dessus si le répertoire est vide
		else
		{
			inc_nlink(new_dir);
		}
		mark_inode_dirty(new_dir);

	}
	mark_inode_dirty(old_dir);
	new_dentry->d_inode = old_dentry->d_inode;
	retval = 0;

	pr_info("new _dentry inode %ld\n",new_dentry->d_inode->i_ino);

	pr_info("la val qui bug nouveau est vrai %d\n", !new_dentry->d_u.d_alias.pprev);
	//~ d_add(new_dentry,new_dentry->d_inode);

	end_rename:
		brelse(bh_old);
		if(bh_new && PTR_ERR(bh_new) != ENOENT)
			brelse(bh_new);
		pr_info("end rename!!!!!!!!!! \n");
		return retval;
}


/* Rename Function with changement of dentry */
/*********************************************/
int cross_rename (struct inode * old_dir, struct dentry *old_dentry, struct inode *new_dir, struct dentry *new_dentry, unsigned int flags)
{
	struct buffer_head *bh_old = NULL, *bh_new = NULL;
	struct inode * ino;
	int retval;

	bh_old = find_entry(old_dir, old_dentry); // regarde si le fichier existe
	if(IS_ERR(bh_old))
		return PTR_ERR(bh_old);


	if(!bh_old)
	{
		pr_err("File doesn't exist\n");
		retval = ENOENT;
		goto end_rename;
	}

	bh_new = find_entry(new_dir, new_dentry); // regarde si le fichier renommé existe déjà
	if(IS_ERR(bh_new))
	{
		retval = PTR_ERR(bh_new);
		bh_new = NULL;
		goto end_rename;
	}

	if(bh_new == ERR_PTR(ENOENT)) // les deux doivent exister pour pouvoir les échanger
	{
		pr_err("File doesn't exist\n");
		retval = ENOENT;
		goto end_rename;
	}

	retval = setent(new_dir, old_dentry, new_dentry);
	if(retval)
		goto end_rename;
	retval = setent(old_dir, new_dentry, old_dentry);
	if(retval)
		goto end_rename;

	old_dentry->d_inode->i_ctime = CURRENT_TIME;
	new_dentry->d_inode->i_ctime = CURRENT_TIME;
	mark_inode_dirty(old_dentry->d_inode);
	mark_inode_dirty(new_dentry->d_inode);

	ino = old_dentry->d_inode;
	old_dentry->d_inode = new_dentry->d_inode;
	new_dentry->d_inode = ino;
	retval = 0;

	end_rename:
		brelse(bh_old);
		brelse(bh_new);

		return retval;
}


/*
 * old_dir : inode de l'ancien répertoire
 * old_dentry : dentry actuel du fichier / répertoire
 * new_dir : inode du nouveau répertoire
 * new_dentry : nouvelle dentry du fichier avec le nouveau noms
 *
 * Les différents flags possibles :
 * RENAME_EXCHANGE : échange le nom des 2 fichiers
 * RENAME_NOREPLACE : ne supprime/remplace pas le fichier dest s'il existe déjà
 * RENAME_WITHOUT : euh quelque chose avec des systèmes en "oignion"
 */
/* Rename Function */
/*******************/
int pnlfs_rename (struct inode *old_dir, struct dentry *old_dentry, struct inode *new_dir, struct dentry *new_dentry, unsigned int flags)
{
	/* Si des flags non permis sont set */
	if (flags & ~(RENAME_NOREPLACE | RENAME_EXCHANGE | RENAME_WHITEOUT))
	{
		pr_info("FALSE FLAG\n");
		return -EINVAL;
	}
	else if (RENAME_WHITEOUT & flags)
	{
		pr_info("Without not supported\n");
		return -EINVAL;
	}
	pr_info("RENAME ENTER\n");
	if(flags & RENAME_EXCHANGE) // échange les 2 fichiers
		return cross_rename(old_dir, old_dentry, new_dir, new_dentry, flags);

	pr_info("normal rename\n");

	pr_info("la val qui bug est vrai %d\n", !old_dentry->d_u.d_alias.pprev);
	pr_info("la val qui bug nouveau est vrai %d\n", !new_dentry->d_u.d_alias.pprev);
	return normal_rename(old_dir, old_dentry, new_dir,new_dentry, flags);
}

#include "SuperBlock_functions.h"
#include "filesystems.h"
#include "general_fonctions.h"


/*
  lookup: called when the VFS needs to look up an inode in a parent
	directory. The name to look for is found in the dentry. This
	method must call d_add() to insert the found inode into the
	dentry. The "i_count" field in the inode structure should be
	incremented. If the named inode does not exist a NULL inode
	should be inserted into the dentry (this is called a negative
	dentry). Returning an error code from this routine must only
	be done on a real error, otherwise creating inodes with system
	calls like create(2), mknod(2), mkdir(2) and so on will fail.
	If you wish to overload the dentry methods then you should
	initialise the "d_dop" field in the dentry; this is a pointer
	to a struct "dentry_operations".
	This method is called with the directory inode semaphore held
	*
	*
	*
	* Appelée quand le VFS cherche un inode dans le répertoire parent.
	* Le nom à chercher se trouve dans dentry.
	* Cette méthode doit appeler d_add() pour insérer l'inode trouvée dans le dentry
	* Si l'inode de ce nom n'existe pas, une inode NULL devrait être inséré dans le dentry (appelé negative dentry)
	* Retourner un code d'erreur dans cette routine doit uniquement être fait lors de vrais erreurs, sinon
	* créer des inodes avec create,mknod,mkdir vont également échouer?
	*
*/
struct dentry *pnlfs_lookup(struct inode *directory, struct dentry *dentry, unsigned int flags){
	int i;
	struct inode *inode;
	struct buffer_head *bh;
	struct pnlfs_dir_block *dir;
	struct pnlfs_inode_info *p_ino;

	p_ino = pnlfs_inode_info_of(directory);

	if(dentry->d_name.len > PNLFS_FILENAME_LEN){
		return ERR_PTR(-ENAMETOOLONG);
	}

	bh = sb_bread(directory->i_sb, p_ino->index_block);
	if(!bh){
		pr_err("Error read bloc");
		return NULL;
	}

	dir = (struct pnlfs_dir_block*) bh->b_data;

	for(i=0 ; i < p_ino->nr_entries ; i ++){
		pr_info("file : %s inode : %d\n",dir->files[i].filename, dir->files[i].inode);
		inode = pnlfs_iget(directory->i_sb, dir->files[i].inode);

		if(IS_ERR(inode)){
			pr_err("error iget error\n");
			brelse(bh);
			return NULL;
		}

		if(strncmp(dir->files[i].filename, dentry->d_name.name, PNLFS_FILENAME_LEN) == 0){
			pr_info("Fichier trouvé ! \n");
			d_add(dentry, inode);
			iput(inode);
			return dentry;
		}
		else
			iput(inode);
	}
	pr_info("rien trouvé\n");
	d_add(dentry,NULL);
	brelse(bh);
	return dentry;
}


/* This function is the implementation of "ls" shell command */
/*************************************************************/
int pnlfs_iterate_shared(struct file *file, struct dir_context *actor){
    int i = 0;
    struct buffer_head *bh;
    struct pnlfs_inode_info *inode_info;
    struct pnlfs_dir_block *list_blocks;

/* Ajout des fichiers particuliers "." et ".." */
    if (actor->pos < 2) {
        if (!dir_emit_dots(file, actor))
            return 0;
    }

    inode_info = pnlfs_inode_info_of(file->f_inode);
    mutex_lock(&file->f_pos_lock);
    if(actor->pos == 2){
        bh = sb_bread(file->f_inode->i_sb, inode_info->index_block);
        if(!bh)
            goto error_read_block;

        list_blocks = (struct pnlfs_dir_block*) bh->b_data;

		pr_info("nr entries %d\n",inode_info->nr_entries);
        while(i != inode_info->nr_entries){
    /* Ajout des autres fichiers */
            dir_emit(actor, list_blocks->files[i].filename, PNLFS_FILENAME_LEN,
                          list_blocks->files[i].inode, DT_UNKNOWN);
            i++;
            actor->pos++;
        }
    }
    mutex_unlock(&file->f_pos_lock);
    return 0;

error_read_block :
    pr_err("Error read bloc");
    return -ENOMEM;
}

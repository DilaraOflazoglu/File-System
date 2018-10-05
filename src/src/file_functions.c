#include "file_functions.h"

/*
 *	@file: the file to read from
 *	@buf : the buffer to read to
 *	@size: the maximum number of bytes to read
 *	@ppos: the current position in the file
*/
ssize_t pnlfs_read(struct file *file, char __user *buf,
												size_t size, loff_t *ppos)
{
	ssize_t ret;
	int num_block;
	struct inode *inode;
	struct pnlfs_inode_info *p_ino;
    struct pnlfs_file_index_block *index_block;
    struct buffer_head *bh_file_inode, *bh_block_pos;

/* Valeur de retour */
    ret = 0;
    inode = file->f_inode;

/* On récupère le pnlfs_inode_info de l'inode */
	p_ino = pnlfs_inode_info_of(inode);

/* Lecture de l'inode pour récupérer le tableau d'index des blocks */
	bh_file_inode = sb_bread(inode->i_sb, p_ino->index_block);
	if(!bh_file_inode){
		pr_err("error sb_bread inode\n");
		goto error_sb_bread_inode;
	}

	index_block = (struct pnlfs_file_index_block*) bh_file_inode->b_data;

/* offset supérieur à la fin de l'offset du fichier */
	if(inode->i_size <= *ppos){
		goto error_last_out;
	}

/* Sinon offset correcte */

/* Lire que le premier block car l'appel système read va être
	appelé tant qu'on n'a pas lu les size octets */
	num_block = *ppos / PNLFS_BLOCK_SIZE;

/* index_block->blocks[num_block] contient le numéro de l'inode */
	bh_block_pos = sb_bread(inode->i_sb, index_block->blocks[num_block]);
	if(!bh_block_pos){
		pr_err("error sb_bread\n");
		goto error_last_out;
	}

	ret = scnprintf(buf, size, bh_block_pos->b_data);

/* Mise à jour de la position de l'offset dans le fichier */
	*ppos += ret;

	mark_inode_dirty(inode);
	mark_buffer_dirty(bh_file_inode);
	brelse(bh_file_inode);

 	return ret;


error_last_out :
	brelse(bh_file_inode);

error_sb_bread_inode :
	return ret;
}



/*
 *	@file: the file to read from
 *	@buf : the buffer to read to
 *	@size: the maximum number of bytes to read
 *	@ppos: the current position in the file
*/
ssize_t pnlfs_write(struct file *file, const char __user *buf,
													size_t size, loff_t *ppos)
{
	struct inode *inode;
	struct pnlfs_inode_info *p_ino;
    struct pnlfs_file_index_block *index_block;
    struct buffer_head *bh_file_inode, *bh_block_pos;
    int max_bcopie, bytes_copie = 0, pos_max, num_block, block_libre;

	pr_info("****************pnlfs_write********************\n");
	if(file->f_flags & O_TRUNC)
		pr_info("TRUNCATE FILE \n");

    if(size != 0){
	    inode = file->f_inode;
		p_ino = pnlfs_inode_info_of(inode); /* On récupère le pnlfs_inode_info de l'inode */

	/* Lecture de l'inode pour récupérer le tableau d'index des blocks */
		bh_file_inode = sb_bread(inode->i_sb, p_ino->index_block);
		if(!bh_file_inode){
			pr_err("error sb_bread inode\n");
			goto error_sb_bread_inode;
		}

		index_block = (struct pnlfs_file_index_block*) bh_file_inode->b_data;

		/* Fichier a sa taille max */
		if(inode->i_size == (PNLFS_BLOCK_SIZE >> 2)){
			pr_err("Fichier rempli a block\n");
			goto error_out_2;
		}

		pr_info("inode->i_size : %llu *ppos : %llu size %lu\n", inode->i_size, *ppos,size);
		/* offset supérieur à la fin de l'offset du fichier */
		if((inode->i_size != 0) && (inode->i_size < *ppos)){
			pr_err("error invalid offset\n");
			goto error_out_2;
		}

		/* Ecriture en fin de fichier */
		if(file->f_flags & O_APPEND)
			*ppos = inode->i_size;

		num_block = *ppos / PNLFS_BLOCK_SIZE; // cherche le bloc dans lequel écrire

		/* num_block n'est pas encore alloué*/
		if(num_block >= inode->i_blocks){
			pr_info("doit allouer un nouveau block\n");

			block_libre = first_empty_block();
			if(block_libre < 0 ){
				pr_err("error de block libre\n");
				goto error_out_2;
			}
			index_block->blocks[num_block] = block_libre;
			inode->i_blocks++;
			mark_buffer_dirty(bh_file_inode);
			mark_inode_dirty(inode);
		}

	/* index_block->blocks[num_block] contient le numéro du block */
		bh_block_pos = sb_bread(inode->i_sb, index_block->blocks[num_block]);

	/* choisit le nombre de caractères à copier, essaie au plus de remplir le block */
		max_bcopie = PNLFS_BLOCK_SIZE - (*ppos%PNLFS_BLOCK_SIZE);
		bytes_copie = ( max_bcopie < size ) ? max_bcopie : size;

		if(copy_from_user(bh_block_pos->b_data + (*ppos % PNLFS_BLOCK_SIZE), buf, bytes_copie )){
			pr_err("error copy_from_user\n");
			goto error_out_2;
		}

		pr_info("a copié %d\n",bytes_copie);
	/* Mise à jour de la position de l'offset dans le fichier */
		pos_max = (*ppos + bytes_copie > inode->i_size) ? *ppos + bytes_copie : inode->i_size;
		*ppos = pos_max;
		inode->i_size = pos_max;
		file->f_pos = pos_max;

		pr_info("inode->i_size : %llu *ppos : %llu\n", inode->i_size, *ppos);

		mark_buffer_dirty(bh_block_pos);
		sync_dirty_buffer(bh_file_inode);
		sync_dirty_buffer(bh_block_pos);
		mark_inode_dirty(inode);
		brelse(bh_file_inode);
		brelse(bh_block_pos);
	}
	return bytes_copie;

error_out_2 :
	brelse(bh_file_inode);

error_sb_bread_inode :
	return bytes_copie;
}

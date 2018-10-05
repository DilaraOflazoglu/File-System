#include "statfs_function.h"


/* fonction statfs, remplit la structure
   struct statfs {
   long    f_type;      type de système de fichiers
   long    f_bsize;     Taille optimale de bloc
   long    f_blocks;    Nombre total de blocs
   long    f_bfree;     Blocs libres
   long    f_bavail;    Blocs libres pour utilisateurs
   long    f_files;     Nombres de nœuds
   long    f_ffree;     Nombre de nœuds libres
   fsid_t  f_fsid;      ID du système de fichiers
   long    f_namelen;   Longueur maxi des noms de fichier
}
*/
int pnlfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	buf->f_type = PNLFS_MAGIC;
	buf->f_bsize = PNLFS_BLOCK_SIZE;
	buf->f_blocks = sb_info->nr_blocks;
	buf->f_bfree = sb_info->nr_free_blocks;
	pr_info("nr free blocks %d\n",sb_info->nr_free_blocks);
	pr_info("files %d\n",sb_info->nr_inodes - sb_info->nr_free_inodes);
	pr_info("free inodes %d\n",sb_info->nr_free_inodes);

	buf->f_bavail = sb_info->nr_free_blocks;
	buf->f_files = sb_info->nr_inodes - sb_info->nr_free_inodes; // puisque 1 inode correspond à un fichier ( et que les répertoires sont considérés comme des fichiers )
	buf->f_ffree = sb_info->nr_free_inodes;
	buf->f_fsid.val[0] = 0;
	buf->f_fsid.val[1] = 0;
	buf->f_namelen = PNLFS_FILENAME_LEN;
	return 0;
}

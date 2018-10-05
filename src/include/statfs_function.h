#ifndef STATFS_FUNCTION_H
#define STATFS_FUNCTION_H

#include "filesystems.h"
#include "general_functions.h"
#include "SuperBlock_functions.h"


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
int pnlfs_statfs(struct dentry *dentry, struct kstatfs *buf);


#endif

#ifndef DIRECTORY_FUNCTIONS_H
#define DIRECTORY_FUNCTIONS_H

#include "SuperBlock_functions.h"
#include "filesystems.h"
#include "general_functions.h"

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
struct dentry *pnlfs_lookup(struct inode *directory, struct dentry *dentry, unsigned int flags);


/* This function is the implementation of "ls" shell command */
/*************************************************************/
int pnlfs_iterate_shared(struct file *file, struct dir_context *actor);

#endif

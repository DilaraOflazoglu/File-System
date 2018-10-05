#ifndef FILE_FUNCTIONS_H
#define FILE_FUNCTIONS_H

#include "filesystems.h"
#include "general_functions.h"
#include "SuperBlock_functions.h"


/*
 *	@file: the file to read from
 *	@buf : the buffer to read to
 *	@size: the maximum number of bytes to read
 *	@ppos: the current position in the file
*/
ssize_t pnlfs_read(struct file *file, char __user *buf,	size_t size, loff_t *ppos);
			

/*
 *	@file: the file to read from
 *	@buf : the buffer to read to
 *	@size: the maximum number of bytes to read
 *	@ppos: the current position in the file
*/
ssize_t pnlfs_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos);

											
#endif

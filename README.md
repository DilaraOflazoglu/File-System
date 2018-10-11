# FILE SYSTEM

# INTRODUCTION
Implementation of a File System as a module compatible with Linux 4.9.83.


# BUILDING
	-	git clone https://github.com/DilaraOflazoglu/File-System.git
	-	cd File-System/src
	-	make

Then you have to load the module into the Kernel : 
	-	insmod filesystems.ko
You have to create a directory to mount the disk image :
	-	mkdir dir
	-	mount -t filesystems -o loop disk.img dir/
	-	cd dir
	
	
# FEATURES
What this File System does :
	-	Create and Remove a file or a directory : "touch", "mkdir" and "rm" Commands
	-	Find a file : "cd" and "ls" commands
	-	Write and Read to file : "echo > FileName" and "cat" commands
	-	Rename a file : "mv" command


# DESTRUCTION
	-	cd ..
	-	unmount dir
	-	rmmod filesystems.ko
	

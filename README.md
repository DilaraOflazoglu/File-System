# FILE SYSTEM

# INTRODUCTION
Implementation of a File System as a module compatible with Linux 4.9.83.


# BUILDING
	-	git clone https://github.com/DilaraOflazoglu/File-System.git
	-	cd File-System/src
	-	make

Then you have to load the module into the Kernel : <br />
	-	insmod filesystems.ko  <br />
You have to create a directory to mount the disk image :  <br />
	-	mkdir dir  <br />
	-	mount -t filesystems -o loop disk.img dir/  <br />
	-	cd dir  <br />
	
	
# FEATURES
What this File System does :
	-	Create and Remove a file or a directory : "touch", "mkdir" and "rm" Commands <br />
	-	Find a file : "cd" and "ls" commands <br />
	-	Write and Read to file : "echo > FileName" and "cat" commands <br />
	-	Rename a file : "mv" command


# DESTRUCTION
	-	cd ..
	-	unmount dir
	-	rmmod filesystems.ko
	

# FILE SYSTEM

# INTRODUCTION
Implementation of a File System as a module compatible with Linux 4.9.83.


# BUILDING
	-	git clone https://github.com/DilaraOflazoglu/File-System.git
	-	cd File-System/src
	-	make

Then you have to load the module into the Kernel : <br />
&nbsp; &nbsp; &nbsp;-&nbsp; &nbsp; &nbsp;insmod filesystems.ko  <br />  <br />
You have to create a directory to mount the disk image :  <br />
&nbsp; &nbsp; &nbsp;-&nbsp; &nbsp; &nbsp;mkdir dir  <br />
&nbsp; &nbsp; &nbsp;-&nbsp; &nbsp; &nbsp;mount -t filesystems -o loop disk.img dir/  <br />
&nbsp; &nbsp; &nbsp;-&nbsp; &nbsp; &nbsp;cd dir  <br />
	
	
# FEATURES
What this File System does : <br />
&nbsp; &nbsp; &nbsp;-&nbsp; &nbsp; &nbsp;Create and Remove a file or a directory : "touch", "mkdir" and "rm" Commands <br />
&nbsp; &nbsp; &nbsp;-&nbsp; &nbsp; &nbsp;Find a file : "cd" and "ls" commands <br />
&nbsp; &nbsp; &nbsp;-&nbsp; &nbsp; &nbsp;Write and Read to file : "echo > FileName" and "cat" commands <br />
&nbsp; &nbsp; &nbsp;-&nbsp; &nbsp; &nbsp;Rename a file : "mv" command


# DESTRUCTION
	-	cd ..
	-	unmount dir
	-	rmmod filesystems.ko
	

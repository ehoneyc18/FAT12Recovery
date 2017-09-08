Earl Honeycutt (ehoneyc@g.clemson.edu) and Keven Fuentes (kfuente@clemson.edu)

This project includes a single .c file:

notjustcats.c

notjustcats.c is a FAT12 recovery program that finds and recreates files in FAT12 filesystems. It has logic to handle both existing and "deleted" files and directories.

KNOWN PROBLEMS:

An issue with deleted files - output doesn't match that produced by the autograder. 

DESIGN:

notjustcats.c navigates the provided FAT12 filesystem through a call to goThroughDir, which recursively checks for either file or directory status and reads the data as is appropriate (while avoiding already read directories). goThroughDir makes use of retrieveClusters to decode the byte entries (through some byte manipulation and sprintf) and find the appropriate logical clusters for the object. Finally, a call to writeFile is made to actually create the file with the appropriate data and file name.


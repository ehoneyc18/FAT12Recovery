#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(){
    
    //Get the file descriptor for the image
    int fd = open("random.img",O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;
    
    //Get the side of the file and saave it to sb
    if (fstat(fd,&sb) == -1){
        perror("couldn't get file size.\n");
    }
    
    //Map the file into memory
    char *filemappedpage = mmap(NULL, sb.st_size,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd, 0);
    
    // Printing the 9728 byte
    printf("%x",filemappedpage[9728]);
    
    
    
    
    //Check to see if attribute byte is 20
    
    //if 20 then it is a file else if it is 10 it is a directory
    
    //then look at last four bytes to get size and divide by 512 to get number of clusters.
    //make array of size of clusters
    //go to FAT and follow it until EOF to fill in array with the cluster numbers
    //then calculate location of each cluster and write 512 bytes to file
    
    
    
    
    
    
    
    return(0);
    
}
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <math.h>

#define ROOT_START 9728
#define ATT_OFFSET 11
#define FIRST_CLUSTER_OFFSET 27
#define FILE_TYPE 0x20
#define FILE_ENTRY_SIZE 31
#define FAT_START 512
#define CLUSTER_SIZE 512


unsigned char *filemappedpage;

void retrieveClusters(int currentEntry, long int* clusterNum, long int** clusters, long int* size){
      /* ---- Get the int value for the file size----- */

  char filesize_Hex[9];
  int nibbleNum;
  int byteNum = 0;

  //Get the bytes in reverse due to little endian ordering and put them in an array
  for(nibbleNum = 0; nibbleNum < 8; nibbleNum = nibbleNum + 2){
    int currentByte = (int)filemappedpage[currentEntry + FILE_ENTRY_SIZE - byteNum];
    sprintf(filesize_Hex+nibbleNum, "%02x", currentByte);
    byteNum++;
  }
  filesize_Hex[8] = '\0';

  //Convert the array to an int value which will be the size
  long int filesize = strtol(filesize_Hex, NULL, 16);

  //Save the size to the passed in parameter
  *size = filesize;


       /*----- Get the first cluster of the file ----- */

  //Get the bytes in reverse order from the cluster offset and put them in array
  char clusterNum_Hex[5];
  byteNum = 0;
  for(nibbleNum = 0; nibbleNum < 4; nibbleNum = nibbleNum + 2){
    int currentByte = (int)filemappedpage[currentEntry + FIRST_CLUSTER_OFFSET - byteNum];
    sprintf(filesize_Hex+nibbleNum, "%02x", currentByte);
    byteNum++;
  }
  filesize_Hex[4] = '\0';

  //Convert the array of hex values to an int which will be the first cluster number
  long int firstCluster = strtol(filesize_Hex, NULL, 16);

  //Find out how many clusters we need and make array of that size
  long int totalCluster = ceil((double)filesize / (double)CLUSTER_SIZE);
  *clusterNum = totalCluster;
  long int* clusterList = (long int*)malloc(sizeof(long int) * totalCluster);

  //Set the first cluster in the list
  clusterList[0] = firstCluster;
  long int nextCluster = firstCluster;

  //Fill in the rest of the list
  int i;
  for(i = 1; i < totalCluster; i++){

    /* Go through the FAT */

    //Calculate location of the index in the FAT
    bool even = true;
    if(nextCluster%2 != 0){
      nextCluster = nextCluster - 1;
      even = false;
    }
    nextCluster = nextCluster / 2;
    nextCluster = nextCluster * 3;

    //Read the entry at that index of the FAT
    char nextCluster_Hex[5];
    int currentByte;
    if(even){
      currentByte = (int)filemappedpage[FAT_START + nextCluster + 1];
      currentByte = currentByte & 0x0F;
      sprintf(nextCluster_Hex, "%02x", currentByte);

      currentByte = (int)filemappedpage[FAT_START + nextCluster];
      sprintf(nextCluster_Hex+ 2, "%02x", currentByte);
      nextCluster = strtol(nextCluster_Hex, NULL, 16);
    } else{

      currentByte = (int)filemappedpage[FAT_START + nextCluster + 2];
      sprintf(nextCluster_Hex, "%02x", currentByte);

      currentByte = (int)filemappedpage[FAT_START + nextCluster + 1];
      sprintf(nextCluster_Hex+2, "%02x", currentByte);
      nextCluster = strtol(nextCluster_Hex, NULL, 16);
      nextCluster = nextCluster >> 4;
    }
    clusterList[i] = nextCluster;
  }

  //Save the finised cluster list to the parameter passed in
  *clusters = clusterList;

}

void writeFile(long int* clusterList, long int totalClusters, long int filesize){
   int i;
   long int* clusters = clusterList;

   //Open a file to write to
   FILE *fptr;
   fptr = fopen("programtest.txt", "w");
   if(fptr == NULL)
   {
      printf("Error!");
      exit(1);
   }

   // Write the bytes to the file as characters
   int count = 0;
    for(i = 0; i < totalClusters; i++){
      int clusterRoot = (33 + clusters[i] - 2) * 512;
      int j;
      for(j = 0; (j < CLUSTER_SIZE); j++){
        fprintf(fptr, "%c", filemappedpage[clusterRoot+j]);
        count++;
        filesize--;
        if(filesize == 0) break;
      }
    }
}



int main(){

    //Get the file descriptor for the image
    int fd = open("random.img",O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;

    //Get the side of the file and saave it to sb
    if (fstat(fd,&sb) == -1){
        perror("couldn't get file size.\n");
    }

    //Map the file into memory
    filemappedpage = mmap(NULL, sb.st_size,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd, 0);

    //Start at the first entry of the root directory
    int currentEntry = ROOT_START;


    //Check the attribute byte to see if it is a file or a directory
    if(filemappedpage[currentEntry + ATT_OFFSET] == FILE_TYPE){ // If it is a file
      long int* clusterList;
      long int totalClusters;
      long int filesize;
      retrieveClusters(currentEntry, &totalClusters, &clusterList, &filesize);
      writeFile(clusterList, totalClusters, filesize);
    } else{

    }

    return 0;










    //Check to see if attribute byte is 20

    //if 20 then it is a file else if it is 10 it is a directory

    //then look at last four bytes to get size and divide by 512 to get number of clusters.
    //make array of size of clusters
    //go to FAT and follow it until EOF to fill in array with the cluster numbers
    //then calculate location of each cluster and write 512 bytes to file



}

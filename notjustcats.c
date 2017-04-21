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
#define DIR_OFFSET 26
#define FILE_TYPE 0x20
#define DIR_TYPE 0x10
#define FILE_ENTRY_OFFSET 31
#define DIR_ENTRY_SIZE 32
#define FAT_START 512
#define CLUSTER_SIZE 512
#define NUM_ROOT_ENTRIES 224
#define FILE_NAME_SIZE 8
#define EMPTY_CHAR 32
#define FILE_EXT_SIZE 3
#define FILE_SIZE_OFFSET 28


unsigned char *filemappedpage;
int dirCount = 0;
int usedRoot[500];
char *dirName;
char currDir[1024];
char finalName[1024];
char fullPath[1024];
char *newPath;
int numFiles = -1;

void retrieveClusters(int currentEntry, long int* clusterNum, long int** clusters, long int* size, char** name, char** ext, int deleted){

      /* ----- Get the file name --- */
  char* filename = (char*) malloc(sizeof(char) * FILE_NAME_SIZE + 1);
  int charnum;
  for(charnum = 0; charnum < FILE_NAME_SIZE; charnum++){
    int currentByte = (int)filemappedpage[currentEntry + charnum];
    if (currentByte == EMPTY_CHAR){
      break;
    }
    if (charnum == 0 && deleted == 1)
      sprintf(filename+charnum, "_");
    else
      sprintf(filename+charnum, "%c", currentByte);
  }
  *name = filename;

        /*------Get the file extension -----*/
  char* fileext = (char*) malloc(sizeof(char) * FILE_EXT_SIZE + 1);
  for(charnum = 0; charnum < FILE_EXT_SIZE; charnum++){
    int currentByte = (int)filemappedpage[currentEntry + FILE_NAME_SIZE + charnum];
    sprintf(fileext+charnum, "%c", currentByte);
  }
  fileext[charnum] = '\0';
  *ext = fileext;


      /* ---- Get the int value for the file size----- */
  char filesize_Hex[9];
  int nibbleNum;
  int byteNum = 0;

  //Get the bytes in reverse due to little endian ordering and put them in an array
  for(nibbleNum = 0; nibbleNum < 8; nibbleNum = nibbleNum + 2){
    int currentByte = (int)filemappedpage[currentEntry + FILE_ENTRY_OFFSET - byteNum];
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
  //char clusterNum_Hex[5];
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

  if (deleted == 0)
  {
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
  else //for deleted files
  {
    //Fill in the rest of the list
    int i;
    nextCluster++;
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
      long int nextClusterChecker = 0;
      if(even){
        currentByte = (int)filemappedpage[FAT_START + nextCluster + 1];
        currentByte = currentByte & 0x0F;
        sprintf(nextCluster_Hex, "%02x", currentByte);

        currentByte = (int)filemappedpage[FAT_START + nextCluster];
        sprintf(nextCluster_Hex+ 2, "%02x", currentByte);
        nextClusterChecker = strtol(nextCluster_Hex, NULL, 16);
      } else{

        currentByte = (int)filemappedpage[FAT_START + nextCluster + 2];
        sprintf(nextCluster_Hex, "%02x", currentByte);

        currentByte = (int)filemappedpage[FAT_START + nextCluster + 1];
        sprintf(nextCluster_Hex+2, "%02x", currentByte);
        nextClusterChecker = strtol(nextCluster_Hex, NULL, 16);
        nextClusterChecker = nextClusterChecker >> 4;
      }
      if (nextClusterChecker == 0)
      {
        clusterList[i] = nextCluster;
        nextCluster++;
      }
      else
        break;
    }

  //Save the finised cluster list to the parameter passed in
  *clusters = clusterList;
  }
}

void writeFile(long int* clusterList, long int totalClusters, long int filesize, char* filename, char* extension, int deleted, char *path){
   int i;
   long int* clusters = clusterList;
   int extensionSize = 4;

   //Construct the whole name of the file
   char *fullname = malloc(strlen(filename) + extensionSize + 1); //+1 for the zero-terminator
   strcpy(fullname, filename);
   strcat(fullname, ".");
   strcat(fullname, extension);

   char nameBuild[1024] = "file";
   char numBuild[1024]; sprintf(numBuild, "%d", numFiles);
   strcat(nameBuild, numBuild);
   strcat(nameBuild, ".");
   strcat(nameBuild, extension);

   struct stat chk = {0};
   if (stat(currDir, &chk) == -1)
      mkdir(currDir, 0777);

   chdir(currDir);

   //Open a file to write to
   FILE *fptr;
   fptr = fopen(nameBuild, "w");
   if(fptr == NULL)
   {
      printf("Error!");
      exit(1);
   }

   char lastName[1024];
   strcpy(lastName, path);
   strcat(lastName, fullname);

   char *delText = "";

   if (deleted == 1)
     delText = "DELETED";
   else
     delText = "NORMAL";

   printf("FILE\t%s\t%s\t%ld\n", delText, lastName, filesize);

   // Write the bytes to the file as characters
   int count = 0;
    for(i = 0; i < totalClusters; i++){
      int clusterRoot = (33 + clusters[i] - 2) * 512;
      int j;
      for(j = 0; (j < CLUSTER_SIZE); j++){
        fwrite(filemappedpage+clusterRoot+j, sizeof(char),1, fptr);
        count++;
        filesize--;
        if(filesize == 0) break;
      }
    }
}


void goThroughDir(int root, int rootsize, char *path){
  int currentEntry = root;
  usedRoot[dirCount] = root;
  dirCount++;

  if (root == ROOT_START)
    newPath = malloc(1000);

  //Check the attribute byte to see if it is a file or a directory
  int i;
  for(i = 0; i < rootsize; i++){
    if(filemappedpage[currentEntry + ATT_OFFSET] == FILE_TYPE){ // If it is a file
      long int* clusterList;
      long int totalClusters;
      long int filesize;
      char* filename;
      char* extension;
      int deleted = 0;

      numFiles++;

      //0x00E5 == 229 -> deleted
      if (filemappedpage[currentEntry] == 229)
        deleted = 1;
      retrieveClusters(currentEntry, &totalClusters, &clusterList, &filesize, &filename, &extension, deleted);
      writeFile(clusterList, totalClusters, filesize, filename, extension, deleted, path);
    } else if (filemappedpage[currentEntry + ATT_OFFSET] == DIR_TYPE){

      int newRoot = filemappedpage[currentEntry + DIR_OFFSET];
      newRoot = (newRoot + 33 - 2) * 512;

      long int* clusterList;
      long int totalClusters;
      long int filesize;
      char* filename;
      char* extension;
      int deleted = 0;

      //i implemented this check because the reader was getting into infinite
      //loops with reappearing folders
      int x;
      int flag = 0;
      for (x = 0; x < dirCount; x++)
      {
        if (newRoot == usedRoot[x])
          flag = 1;
      }

      strcpy(fullPath, path);
      char temp[1024];
      strcpy(temp, fullPath);

      if (!flag)
      {
        if (filemappedpage[currentEntry] == 229)
          deleted = 1;
        retrieveClusters(currentEntry, &totalClusters, &clusterList, &filesize, &filename, &extension, deleted);
        strcat(newPath, filename);
        strcat(newPath, "/");

        goThroughDir(newRoot, 16, newPath);
      }
      strcpy(fullPath, temp);
      newPath = fullPath;
    }
    currentEntry = currentEntry + DIR_ENTRY_SIZE;
  }
}

int main(int argc, char* argv[]){

    //Get the file descriptor for the image
    int fd = open(argv[1],O_RDWR, S_IRUSR | S_IWUSR);
    struct stat sb;

    dirName = argv[2];
    getcwd(currDir, sizeof(currDir));

    char finalName[1024];

    strcpy(finalName, "/");
    strcat(finalName, dirName);
    strcat(currDir, finalName);

    //Get the side of the file and save it to sb
    if (fstat(fd,&sb) == -1){
        perror("couldn't get file size.\n");
    }

    //Map the file into memory
    filemappedpage = mmap(NULL, sb.st_size,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd, 0);

    strcpy(fullPath, "/");

    goThroughDir(ROOT_START, NUM_ROOT_ENTRIES, fullPath);

    return 0;
}

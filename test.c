#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int main(){
  FILE *fp;
 int c;

 fp = fopen("random.img","r");

 fseek(fp,9728,SEEK_SET);
  c = fgetc(fp);


    printf("%x", c);

 fclose(fp);


 //Check to see if attribute byte is 20

    //if 20 then it is a file else if it is 10 it is a directory

        //then look at last four bytes to get size and divide by 512 to get number of clusters.
        //make array of size of clusters
        //go to FAT and follow it until EOF to fill in array with the cluster numbers
        //then calculate location of each cluster and write 512 bytes to file







 return(0);

}

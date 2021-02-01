#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>


int
main(int argc, char*argv[]){

    DIR             *dp;
    struct dirent   *dirp;

    if ( argc != 2)
      return (-1);

    if ( (dp = opendir(argv[1])) == nullptr){
     
        printf("can't open %s\n",argv[1]);
        return -1;
    }

    while ( (dirp=readdir(dp)) != nullptr )
        printf("%s \t",dirp->d_name);

    return 0;
}
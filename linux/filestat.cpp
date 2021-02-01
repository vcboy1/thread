#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char* argv[]){

    int             i =0;
    struct   stat   info;
    char*           ptr;

    for  (int i=1; i< argc; ++i){

        printf("%s: ", argv[1]);
        lstat( argv[i], &info);

        if      ( S_ISREG(info.st_mode))        ptr = "regular";
        else if ( S_ISDIR(info.st_mode))        ptr = "dir";
        else if ( S_ISCHR(info.st_mode))        ptr = "ccharacter";
        else if ( S_ISLNK(info.st_mode))        ptr = "link";

        printf("%s  uid:%d, gid: %d\n",ptr, info.st_uid, info.st_gid);
    }
}
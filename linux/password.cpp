#include <sys/cdefs.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 1024

int
main(void){

    FILE*       fp = fopen("/etc/passwd","rt");
    char       buf[MAXLINE];

    if ( fp != nullptr){

        while ( fgets(buf, MAXLINE, fp) != nullptr)
           write(1,buf, strlen(buf));
           // puts(buf);
    } 

    // 关闭
    fclose(fp);
}
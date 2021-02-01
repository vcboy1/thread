#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 1024

void sig_int(int signo){

    printf("interrupt\n%%");
}

int
main(void){

    char    buf[MAXLINE];
    pid_t   pid;
    int     status;

    if ( signal(SIGINT, sig_int) == SIG_ERR)
        return -1;

    printf("#  ");
    while ( fgets(buf,MAXLINE, stdin) != nullptr){

        buf[ strlen(buf)-1] = 0;

        if ( (pid = fork()) < 0 )
            return -1;

        if ( pid == 0 ){

            // child process
            execlp(buf, buf, (char*)0);
            return 127;
        }
        
        if ( (pid =waitpid(pid,&status,0)) < 0 )
            printf("waitpid error\n");
        printf("#  ");
    }
    return 0;
}
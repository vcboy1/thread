#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void pr_ids(char * name){

    printf("%s: pid =%d, ppid = %d, pgrp = %d\n",name,getpid(), getppid(),getpgrp());
    fflush(stdout);
}

static void sig_hup(int signo){

    printf("SIGHUP received, pid =%d\n",getpid());
}



int
main(void){

    char    c;
    pid_t   pid;

    pr_ids("parent");

    if ( (pid = fork()) < 0 )
        return -1;

    // parent
    if ( pid > 0 ){

        sleep(5);
        return 0 ;
    }

    // child
    pr_ids("child");
    //setsid();
    signal(SIGHUP, sig_hup);
    //kill( getpid(), SIGTSTP);
    raise(SIGTSTP);
    sleep(6);
    pr_ids("child");

    return 0;
}
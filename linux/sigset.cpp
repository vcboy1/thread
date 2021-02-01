#include <signal.h>
#include <unistd.h>
#include <stdio.h>

int  SIGNO = SIGQUIT;

static void  sig_quit(int sig_no){

    printf("Recived SIGQUIT message\n");

    signal(SIGNO, SIG_DFL);
}


int
main(void){

    printf("Pid = %d\n",getpid() );

    sigset_t    new_mask,old_mask,pend_mask;

    signal(SIGNO, sig_quit);

    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGNO);
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);

    sleep(5);

    sigpending(&pend_mask);
    if ( sigismember(&pend_mask,SIGNO))
        printf("\nSIGQUIT pending\n");

    sigprocmask(SIG_SETMASK,&old_mask,0);
    printf("SIGQUIT unblocked\n");

    sleep(5);

}
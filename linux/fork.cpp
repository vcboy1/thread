#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

int
main(void){

    pid_t   pid = fork();
    if ( pid <0 )
      return -1;

    // parent
    if ( pid > 0 )
       assert( waitpid(pid, 0,0) == pid );
    else
    {   // first child
        pid = fork();
        if ( pid != 0 )
            return 0;

        // second child
        sleep(2);
        printf("second child, parent pid = %d\n", getppid() );
        return 0;
    }
    
    return 0;
}
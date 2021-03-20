/*
 * studentID.c
 * Use two thread to printf pid of parent and child
 * Use program of problem2 two print relationship of parent and child
 */
#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<unistd.h>

int main(){
    pid_t pid = fork();
    if (pid < 0) {
        printf("Failed to fork.\n");
        return -1;
    }
    /* children */
    if (pid == 0) {
        pid_t child = getpid();
        printf("519021911094 Children is %d.\n", child);
        /* <unistd.h>
         * int execl( const char * path,
         *            const char * arg0,
         *            ...,
         *            NULL );
         */
        execl("/data/misc/os/ptreeARM", "ptreeARM", NULL);
        exit(0);
    } else{
        /* parent */
        pid_t parent = getpid();
        printf("519021911094 Parent is %d.\n", parent);
        wait(0);
    }
}
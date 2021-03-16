 #include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<string.h>

#define __NR_pstreecall 356
#define N_TASK 1024

struct prinfo {
    pid_t parent_pid;           /* process id of parent */
    pid_t pid;                  /* process id*/
    pid_t first_child_pid;      /* pid of youngest child */
    pid_t next_sibling_pid;     /* pid of older sibling */
    long state;                 /* curren state of process */
    long uid;                   /* user id of process owner */
    char comm[64];              /* name of program executed */
};

void printTree(struct prinfo* buf, int * nr){
    
    int cur = 0;
    pid_t parent[N_TASK] = {0};
    parent[0] = buf[0].pid;
    struct prinfo p = buf[0];
    printf("%s,%d,%ld,%d,%d,%d,%d\n", 
        p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
    int i,j;
    for (i = 1; i < (*nr); ++i) {
        p = buf[i];
        if (p.parent_pid == parent[cur]) {
            ++cur;
            parent[cur] = p.pid;
        } else {
            for (j = cur; j >= 0; --j) {
                if (parent[j] == p.parent_pid) {
                    cur = j + 1;
                    parent[cur] = p.pid;
                }
            }
        }
        for (j = 0; j < cur; ++j) {
            printf("\t");
        }
        printf("%s,%d,%ld,%d,%d,%d,%d\n", 
            p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);

    }
    /*
    struct prinfo p;
    int i = 0;
    while (i < (*nr)) {
        p = buf[i];
        printf("%s,%d,%ld,%d,%d,%d,%d\n", 
            p.comm, p.pid, p.state, p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
        ++i;
    }
    return;
    */
}

int main() {
    struct prinfo* buf;
    int* nr;
    if (!(buf = malloc(N_TASK * sizeof(struct prinfo)))) {
        printf("Failed to malloc buffer.\n");
    }
    if (!(nr = malloc(sizeof(int)))) {
        printf("Failed to malloc nr.\n");
    }
    syscall(__NR_pstreecall, buf, nr);
    printTree(buf, nr);
    free(buf);
    free(nr);
    return 0;
}
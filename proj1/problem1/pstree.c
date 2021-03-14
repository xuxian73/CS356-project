#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/unistd.h>
#include<linux/list.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/syscalls.h>
#include<linux/gfp.h>

MODULE_LICENSE("Dual BSD/GPL");

#define __NR_pstreecall 356     /* define syscall number */
#define N_TASK 1024             /* define maximum number of entries */

struct prinfo {
    pid_t parent_pid;           /* process id of parent */
    pid_t pid;                  /* process id*/
    pid_t first_child_pid;      /* pid of youngest child */
    pid_t next_sibling_pid;     /* pid of older sibling */
    long state;                 /* curren state of process */
    long uid;                   /* user id of process owner */
    char comm[64];              /* name of program executed */
};

void write2buf(struct task_struct * task, struct prinfo* ker_buf) {
    ker_buf->parent_pid = task->parent->pid;
    ker_buf->pid = task->pid;
    ker_buf->state = task->state;
    ker_buf->uid = task->cred->uid;
    get_task_comm(ker_buf->comm, task);

    if (!list_empty(&(task->children))) {
        ker_buf->first_child_pid = list_entry((task->children).next, struct task_struct, sibling)->pid;
    } else {
        ker_buf->first_child_pid = 0;
    }
    if (!list_empty(&(task->sibling))) {
        pid_t next_sibling = list_entry((task->sibling).next, struct task_struct, sibling)->pid;
        pid_t parent_ch = list_entry((task->sibling).next, struct task_struct, children)->pid;
        if ( next_sibling == parent_ch) {
            ker_buf->next_sibling_pid = 0;
        } else ker_buf->next_sibling_pid = next_sibling;
    } else {
        ker_buf->next_sibling_pid = 0;
    }
}

void DFS(struct task_struct* task, struct prinfo* ker_buf, int *ker_n) {
    write2buf(task, ker_buf + (*ker_n));
    ++(*ker_n);
    if (ker_buf->first_child_pid == 0) {
        DFS(list_entry((task->children).next, struct task_struct, sibling), ker_buf + (*ker_n), ker_n);
    } else {
        if (ker_buf->next_sibling_pid) {
            DFS(list_entry((task->sibling).next, struct task_struct, children), ker_buf + (*ker_n), ker_n);
        } else return;
    }
}

int pstree(struct prinfo * buf, int *nr)
{
    struct prinfo* ker_buf;
    int* ker_n;
    /* kcalloc(size_t n, size_t size, gfp_t flags) */
    if (!(ker_buf = kcalloc(N_TASK, sizeof(struct prinfo), GFP_KERNEL))) {
        printk("Failed to allocate kernel buffer.\n");
        return -1;
    }
    if (!(ker_n = kzalloc(sizeof(int), GFP_KERNEL))) {
        printk("Fialed to allocate kernal nr.\n");
        return -1;
    }
    *ker_n = 0;
    
    read_lock(&tasklist_lock);
    DFS(&init_task, ker_buf, ker_n);
    read_unlock(&tasklist_lock);

    /* unsigned long copy_to_user(void __user * to, const void * from, unsigned long n); */
    if (!copy_to_user(buf, ker_buf, N_TASK * sizeof(struct prinfo))) {
        printk("Failed to copy kernel buffer to user.\n");
        return -1;
    }
    if (!copy_to_user(nr, ker_n, sizeof(int))) {
        printk("Failed to copy kernel nr to user.\n");
        return -1;
    }
    kfree(ker_buf);
    kfree(ker_n);
    return 0;
}

static int (*oldcall)(void);
static int addsyscall_init(void) {
    long *syscall = (long*)0xc000d8c4;
    oldcall = (int(*)(void))(syscall[__NR_pstreecall]);
    syscall[__NR_pstreecall] = (unsigned long)pstree;
    printk(KERN_INFO "module load!\n");
    return 0;
}

static void addsyscall_exit(void) {
    long *syscall = (long*)0xc000d8c4;
    syscall[__NR_pstreecall] = (unsigned long)oldcall;
    printk(KERN_INFO "module exit\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);
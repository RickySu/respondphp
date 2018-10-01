#include <sys/types.h>
#include <sys/wait.h>

#include "respondphp.h"
#include "task_manager.h"

static uint rp_task_count = 0;

static int makeForks(int n)
{
    int i, pid = 1;
    for(i = 0; i < n; i++) {
        pid = fork();
        if(pid <= 0) {
            break;
        }
        rp_task_count++;
    }
    return pid;
}

static void signal_chld_handler(uv_signal_t* signal, int signum)
{
    rp_task_manager_t *task_manager = (rp_task_manager_t *) signal;
    switch(signum) {
        case SIGCHLD:
            wait_all_children();

            if(makeForks(RP_TASK_MAX - rp_task_count) > 0) {
                return;
            }

            uv_signal_stop(&task_manager->signal);
            uv_stop(&task_manager->loop);
            break;
        case SIGHUP:
            fprintf(stderr, "MANAGER HUP %d %d %d\n", SIGHUP, signum, getpid());
            uv_signal_stop(&task_manager->signal);
            exit(0);
        default:
            break;
    }
}

static void wait_all_children()
{
    int child_pid;
    while((child_pid = waitpid(-1, NULL, WNOHANG)) > 0){
        rp_task_count--;
        printf("task dead: %d %d\n", child_pid, rp_task_count);
    }
}

static void rp_do_init_task_manager()
{
    rp_task_manager_t task_manager;
    DETTACH_SESSION();
    if(makeForks(RP_TASK_MAX - rp_task_count) > 0) { //Task Manager
        rp_set_task_type(TASK_MANAGER);
        uv_loop_init(&task_manager.loop);
        uv_signal_init(&task_manager.loop, &task_manager.signal);
        uv_signal_start(&task_manager.signal, signal_chld_handler, SIGCHLD);
#ifdef HAVE_PR_SET_PDEATHSIG
        uv_signal_start(&task_manager.signal, signal_chld_handler, SIGHUP);
        prctl(PR_SET_PDEATHSIG, SIGHUP);
#endif
        uv_run(&task_manager.loop, UV_RUN_DEFAULT);
        uv_loop_close(&task_manager.loop);
    }
    rp_set_task_type(TASK);
}

int rp_init_task_manager()
{
    int fd[2];
    int pid;

    if(rp_get_task_type() != ACTOR) {
        return 0;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
        return -1;
    }

    pid = fork();

    if(pid < 0){
        return pid;
    }

    if(pid > 0){ // ACTOR
        close(fd[0]);
        return fd[1];
    }

    close(fd[1]);
    rp_do_init_task_manager();
    return fd[0];
}

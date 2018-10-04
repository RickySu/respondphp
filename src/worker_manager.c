#include <sys/types.h>
#include <sys/wait.h>

#include "respondphp.h"
#include "worker_manager.h"
static uint rp_worker_count = 0;

static int makeForks(int n)
{
    int i, pid = 1;
    for(i = 0; i < n; i++) {
        pid = fork();
        if(pid <= 0) {
            break;
        }
        rp_worker_count++;
    }
    return pid;
}

static void signal_chld_handler(uv_signal_t* signal, int signum)
{
    rp_worker_manager_t *worker_manager = (rp_worker_manager_t *) signal;
    switch(signum) {
        case SIGCHLD:
            wait_all_children();

            if(makeForks(RP_WORKER_MAX - rp_worker_count) > 0) {
                return;
            }

            uv_signal_stop(&worker_manager->signal);
            uv_stop(&worker_manager->loop);
            break;
        case SIGHUP:
            uv_signal_stop(&worker_manager->signal);
            exit(0);
        default:
            break;
    }
}

static void wait_all_children()
{
    int child_pid;
    while((child_pid = waitpid(-1, NULL, WNOHANG)) > 0){
        rp_worker_count--;
        printf("worker dead: %d %d\n", child_pid, rp_worker_count);
    }
}

static void rp_do_init_worker_manager()
{
    rp_worker_manager_t worker_manager;
    DETTACH_SESSION();
    if(makeForks(RP_WORKER_MAX - rp_worker_count) > 0) { //Worker Manager
        rp_set_task_type(WORKER_MANAGER);
        uv_loop_init(&worker_manager.loop);
        uv_signal_init(&worker_manager.loop, &worker_manager.signal);
        uv_signal_start(&worker_manager.signal, signal_chld_handler, SIGCHLD);
        rp_register_pdeath_sig(&worker_manager.loop, SIGINT, signal_chld_handler);
        uv_run(&worker_manager.loop, UV_RUN_DEFAULT);
        uv_loop_close(&worker_manager.loop);
    }
    rp_set_task_type(WORKER);
}

int rp_init_worker_manager()
{
    int fd[2];
    int pid;

    if(rp_get_task_type() != ACTOR){
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
    rp_do_init_worker_manager();
    return fd[0];
}

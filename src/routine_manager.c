#include <sys/types.h>
#include <sys/wait.h>

#include "respondphp.h"
#include "routine_manager.h"

static uint rp_routine_count = 0;
static void rp_do_init_routine_manager();
static int makeForks(int n);
static void signal_chld_handler(uv_signal_t* signal, int signum);
static void wait_all_children();

static int makeForks(int n)
{
    int i, pid = 1;
    for(i = 0; i < n; i++) {
        pid = fork();
        if(pid <= 0) {
            break;
        }
        rp_routine_count++;
    }
    return pid;
}

static void signal_chld_handler(uv_signal_t* signal, int signum)
{
    rp_routine_manager_t *routine_manager = (rp_routine_manager_t *) signal;
    switch(signum) {
        case SIGCHLD:
            wait_all_children();

            if(makeForks(RP_ROUTINE_MAX - rp_routine_count) > 0) {
                return;
            }

            uv_signal_stop(&routine_manager->signal);
            uv_stop(&routine_manager->loop);
            break;
        case SIGHUP:
            uv_signal_stop(&routine_manager->signal);
            exit(0);
        default:
            break;
    }
}

static void wait_all_children()
{
    int child_pid;
    while((child_pid = waitpid(-1, NULL, WNOHANG)) > 0){
        rp_routine_count--;
        fprintf(stderr, "routine dead: %d %d\n", child_pid, rp_routine_count);
    }
}

static void rp_do_init_routine_manager()
{
    rp_routine_manager_t routine_manager;
    DETTACH_SESSION();
    if(makeForks(RP_ROUTINE_MAX - rp_routine_count) > 0) { //Routine Manager
        rp_set_task_type(ROUTINE_MANAGER);
        uv_loop_init(&routine_manager.loop);
        uv_signal_init(&routine_manager.loop, &routine_manager.signal);
        uv_signal_start(&routine_manager.signal, signal_chld_handler, SIGCHLD);
#ifdef HAVE_PR_SET_PDEATHSIG
        uv_signal_start(&routine_manager.signal, signal_chld_handler, SIGHUP);
        prctl(PR_SET_PDEATHSIG, SIGHUP);
#endif
        uv_run(&routine_manager.loop, UV_RUN_DEFAULT);
        uv_loop_close(&routine_manager.loop);
    }
    rp_set_task_type(ROUTINE);
}

void rp_init_routine_manager(int *routine_fd)
{
    int fd[2];
    int pid;

    if(rp_get_task_type() != ACTOR) {
        *routine_fd = 0;
        return;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0) {
        *routine_fd = -1;
        return;
    }

    pid = fork();

    if(pid < 0){
        *routine_fd = pid;
        return;
    }

    if(pid > 0){ // ACTOR
        close(fd[0]);
        *routine_fd = fd[1];
        return;
    }

    close(fd[1]);
    rp_do_init_routine_manager();
    *routine_fd = fd[0];
}

#ifndef _RP_ROUTINE_MANAGER_H
#define _RP_ROUTINE_MANAGER_H

#define RP_ROUTINE_MAX 3

typedef struct {
    uv_signal_t signal;
    uv_loop_t loop;
} rp_routine_manager_t;

static void rp_do_init_routine_manager();
static int makeForks(int n);
static void signal_chld_handler(uv_signal_t* signal, int signum);
static void wait_all_children();
#endif

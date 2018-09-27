#ifndef _RP_TASK_MANAGER_H
#define _RP_TASK_MANAGER_H

#include "reactor.h"

#define RP_TASK_MAX 10

extern rp_task_type_t rp_task_type;

typedef struct {
    uv_signal_t signal;
    uv_loop_t loop;
} rp_task_manager_t;

static void rp_do_init_task_manager();
static int makeForks(int n);
static void signal_chld_handler(uv_signal_t* signal, int signum);
static void wait_all_children();
#endif

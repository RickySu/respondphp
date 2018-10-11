#ifndef _RP_WORKER_MANAGER_H
#define _RP_WORKER_MANAGER_H

#define RP_WORKER_MAX 5

typedef struct {
    uv_signal_t signal;
    uv_loop_t loop;
} rp_worker_manager_t;
#endif

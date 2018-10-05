#ifndef _RP_ROUTINE_MANAGER_H
#define _RP_ROUTINE_MANAGER_H

#define RP_ROUTINE_MAX 1

typedef struct {
    uv_signal_t signal;
    uv_loop_t loop;
} rp_routine_manager_t;
#endif

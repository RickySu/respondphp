#include "respondphp.h"
static rp_reactor_t *rp_reactor_head = NULL;
static uv_pipe_t ipc_pipe;
static rp_reactor_t *rp_reactor_get_head()
{
    return rp_reactor_head;
}

int rp_init_reactor(int fd)
{
    uv_loop_init(&main_loop);
    uv_pipe_init(&main_loop, &ipc_pipe, 1);
    int ret = uv_pipe_open(&ipc_pipe, fd);
    return ret;
}
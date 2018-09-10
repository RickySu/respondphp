#ifndef _RP_RESPONDPHP_H
#define _RP_RESPONDPHP_H

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef NDEBUG
    #define RESPOND_ASSERT(exp)
#else
    #define RESPOND_ASSERT(exp) ZEND_ASSERT(exp)  
#endif

#ifdef HAVE_JEMALLOC
    #include <jemalloc/jemalloc.h>
#endif

#include <php.h>
#include <ext/standard/info.h>
#include <uv.h>
#include "types.h"
#include "common.h"

extern uv_loop_t main_loop;

PHP_MINIT_FUNCTION(respondphp);
PHP_MSHUTDOWN_FUNCTION(respondphp);
PHP_MINFO_FUNCTION(respondphp);
PHP_RINIT_FUNCTION(respondphp);
PHP_RSHUTDOWN_FUNCTION(respondphp);

extern zend_module_entry respondphp_module_entry;

DECLARE_CLASS_ENTRY(respond_event_loop);

int rp_init_worker_manager();
int rp_init_reactor(int fd);
rp_task_type_t rp_get_task_type();
void rp_set_task_type(rp_task_type_t type);
#endif

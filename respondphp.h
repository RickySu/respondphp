#ifndef _RESPONDPHP_H
#define _RESPONDPHP_H

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#ifdef NDEBUG
  #define RESPOND_ASSERT(exp)
#else
  #define RESPOND_ASSERT(exp) ZEND_ASSERT(exp)  
#endif

#include <php.h>
#include <ext/standard/info.h>
#include <uv.h>
#include "common.h"

extern uv_loop_t main_loop;

PHP_MINIT_FUNCTION(respondphp);
PHP_MSHUTDOWN_FUNCTION(respondphp);
PHP_MINFO_FUNCTION(respondphp);
PHP_RINIT_FUNCTION(respondphp);
PHP_RSHUTDOWN_FUNCTION(respondphp);

extern zend_module_entry respondphp_module_entry;

DECLARE_CLASS_ENTRY(respond_event_loop);
#endif

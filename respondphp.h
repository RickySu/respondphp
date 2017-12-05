#ifndef _RESPONDPHP_H
#define _RESPONDPHP_H

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include <php.h>
#include <ext/standard/info.h>
#include <uv.h>


PHP_MINIT_FUNCTION(respondphp);
PHP_MSHUTDOWN_FUNCTION(respondphp);
PHP_MINFO_FUNCTION(respondphp);
PHP_RSHUTDOWN_FUNCTION(respondphp);

extern zend_module_entry respondphp_module_entry;
extern uv_loop_t default_loop;
#endif


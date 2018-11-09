dnl THIRDPARTY_BUILD_DIR="$srcdir/thirdparty/build"

PHP_ADD_INCLUDE("$srcdir/include")
PHP_ADD_INCLUDE("$srcdir/thirdparty/picohttpparser")

AC_DEFUN([AC_RESPOND_HAVE_PR_SET_PDEATHSIG],
[
    AC_MSG_CHECKING([for pcntl PR_SET_PDEATHSIG])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include <sys/prctl.h>
        #include <signal.h>
    ]], [[
        prctl(PR_SET_PDEATHSIG, SIGHUP);
    ]])],[
        AC_DEFINE(HAVE_PR_SET_PDEATHSIG, 1, [have PR_SET_PDEATHSIG?])
        AC_MSG_RESULT([yes])
    ],[
        AC_MSG_RESULT([no])
    ])
])

AC_DEFUN([AC_RESPOND_HAVE_REUSEPORT],
[
    AC_MSG_CHECKING([for socket REUSEPORT])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include <sys/socket.h>
    ]], [[
        int val = 1;
        setsockopt(0, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));
    ]])],[
        AC_DEFINE(HAVE_REUSEPORT, 1, [have SO_REUSEPORT?])
        AC_MSG_RESULT([yes])
    ],[
        AC_MSG_RESULT([no])
    ])
])

AC_DEFUN([AC_RESPOND_HAVE_TLS_METHOD],
[
    PHP_CHECK_LIBRARY(ssl, TLS_method, [
        AC_DEFINE(HAVE_OPENSSL_TLS_METHOD, 1, [Have openssl tls method?])
    ], [
    ], [
    ])
])

AC_DEFUN([AC_RESPOND_HAVE_TLSV1_METHOD],
[
    PHP_CHECK_LIBRARY(ssl, TLSv1_method, [
        AC_DEFINE(HAVE_OPENSSL_TLSV1_METHOD, 1, [Have openssl tls v1 method?])
    ], [
    ], [
    ])
])

AC_DEFUN([AC_RESPOND_HAVE_TLSV1_1_METHOD],
[
    PHP_CHECK_LIBRARY(ssl, TLSv1_1_method, [
        AC_DEFINE(HAVE_OPENSSL_TLSV1_1_METHOD, 1, [Have openssl tls v1_1 method?])
    ], [
    ], [
    ])
])

AC_DEFUN([AC_RESPOND_HAVE_TLSV1_2_METHOD],
[
    PHP_CHECK_LIBRARY(ssl, TLSv1_2_method, [
        AC_DEFINE(HAVE_OPENSSL_TLSV1_2_METHOD, 1, [Have openssl tls v1_2 method?])
    ], [
    ], [
    ])
])

AC_DEFUN([AC_RESPOND_HAVE_TLSV1_3_METHOD],
[
    PHP_CHECK_LIBRARY(ssl, TLSv1_3_method, [
        AC_DEFINE(HAVE_OPENSSL_TLSV1_3_METHOD, 1, [Have openssl tls v1_3 method?])
    ], [
    ], [
    ])
])

AC_DEFUN([AC_RESPOND_HAVE_TLS_ALPN],
[
    PHP_CHECK_LIBRARY(ssl, SSL_CTX_set_alpn_protos, [
        AC_DEFINE(HAVE_OPENSSL_TLS_ALPN, 1, [Have openssl tls alpn?])
    ], [
    ], [
    ])
])

PHP_ARG_WITH(respondphp, for respondphp support,
dnl Make sure that the comment is aligned:
[  --with-respondphp       Include respondphp support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(respondphp, whether to enable respondphp support,
dnl Make sure that the comment is aligned:
[  --enable-respondphp     Enable respondphp support])

PHP_ARG_WITH(uv-dir, for libuv installation prefix,
[  --with-uv-dir[=DIR] libuv installation prefix], no, no)

PHP_ARG_WITH(jemalloc, for jemalloc support,
[  --with-jemalloc         Include jemalloc support], no, no)

PHP_ARG_WITH(jemalloc-dir, for jemalloc installation prefix,
[  --with-jemalloc-dir[=DIR] jemalloc installation prefix], no, no)

PHP_ARG_WITH(openssl, for OpenSSL support in event,
[  --with-openssl          Include OpenSSL support], yes, no)

PHP_ARG_WITH(openssl-dir, for OpenSSL installation prefix,
[  --with-openssl-dir[=DIR]  openssl installation prefix], no, no)

PHP_ARG_ENABLE(debug-respondphp, whether to enable debug respondphp support,
[  --enable-debug-respondphp Enable debug respondphp support])

PHP_ARG_WITH(phpconfig-path, for PHP config path ,
[  --with-phpconfig-path[=PATH] phpconfig path], no, no)

if test "$PHP_RESPONDPHP" != "no"; then

  AC_RESPOND_HAVE_REUSEPORT
  AC_RESPOND_HAVE_PR_SET_PDEATHSIG

  modules="   
    thirdparty/picohttpparser/picohttpparser.c
    src/respondphp.c
    src/event_loop.c
    src/reactor.c
    src/routine_manager.c
    src/worker_manager.c
    src/network/resolver.c
    src/system/timer.c
    src/system/timer_promise.c
    src/server/udp.c
    src/server/tcp.c
    src/server/pipe.c
    src/server/routine.c
    src/server/secure.c
    src/connector/tcp.c
    src/connector/pipe.c
    src/connector/secure.c
    src/stream/connection.c
    src/stream/secure.c
    src/internal/event_emitter.c
    src/internal/stream_connection.c
    src/internal/socket_connector.c
    src/interface/event_event_emitter_interface.c
    src/interface/stream_server_interface.c
    src/interface/stream_writable_stream_interface.c
    src/interface/stream_readable_stream_interface.c
    src/interface/stream_connection_interface.c
    src/interface/socket_connector_interface.c
    src/interface/async_promise_interface.c
  "

  PHP_NEW_EXTENSION(respondphp, $modules, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
  if test "$PHP_DEBUG_RESPONDPHP" != "yes"; then
    NO_DEBUG_RESPONDPHP="-DNDEBUG -O3"    
    CFLAGS="$CFLAGS $NO_DEBUG_RESPONDPHP"
  else
    AC_DEFINE(HAVE_DEBUG, 1, [Enable debug])
  fi
  
  dnl {{{ --with-phpconfig-path
  if test "$PHP_PHPCONFIG_PATH" != "no"; then
    PHP_CONFIG=$PHP_PHPCONFIG_PATH
  else
    PHP_CONFIG=`which php-config`
  fi
  if test ! -x "$PHP_CONFIG"; then
    AC_MSG_ERROR([php-config not found. Check the path given to --with-phpconfig-path and output in config.log])
  fi
  PHP_PATH=`$PHP_CONFIG --php-binary`
  if test "x$PHP_PATH" == "x" || test ! -x "$PHP_PATH"; then
    AC_MSG_ERROR([php-binary not found. Please install PHP CLI])
  fi

  PHP_PATH=$PHP_PATH make -f Makefile.predefined clean all
  
  dnl {{{ --with-uv
  if test "$PHP_UV_DIR" != "no"; then
    UV_INCLUDE="$PHP_UV_DIR/include"
    PHP_ADD_INCLUDE("${UV_INCLUDE}")
    PHP_ADD_LIBRARY(uv, 1, RESPONDPHP_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(uv, "${PHP_UV_DIR}/${PHP_LIBDIR}")
  else
    UV_SEARCH_PATHS="/usr /usr/local /opt /usr/local/uv"
    for i in $UV_SEARCH_PATHS; do        
      for j in include ""; do
        if test -r "$i/$j/uv.h"; then
          UV_INCLUDE="$i/$j"
        fi
      done
    done
    PHP_CHECK_LIBRARY(uv, uv_pipe_getsockname, [
      AC_DEFINE(HAVE_UV, 1, [Have libuv 1.3.0 or later])
      PHP_ADD_INCLUDE("${UV_INCLUDE}")
    ], [
      AC_MSG_ERROR([libuv libraries not found. Check the path given to --with-uv-dir and output in config.log])
    ], [
    ])
    PHP_ADD_INCLUDE("${UV_INCLUDE}")
    PHP_ADD_LIBRARY(uv, 1, RESPONDPHP_SHARED_LIBADD)
    AC_MSG_RESULT(libuv include success)
  fi    
  dnl }}}

  dnl {{{ --with--jemalloc
  if test "$PHP_JEMALLOC" != "no" || test "$PHP_JEMALLOC_DIR" != "no"; then
    if test "$PHP_JEMALLOC_DIR" != "no"; then
      AC_MSG_RESULT(jemalloc include success)
      JEMALLOC_INCLUDE="$PHP_JEMALLOC_DIR/include"
      PHP_ADD_LIBRARY_WITH_PATH(jemalloc, "${PHP_JEMALLOC_DIR}/${PHP_LIBDIR}")
    else
      JEMALLOC_SEARCH_PATHS="/usr /usr/local /usr/local/jemalloc"
      for i in $JEMALLOC_SEARCH_PATHS; do        
        for j in include ""; do
          if test -r "$i/$j/jemalloc/jemalloc.h"; then
            JEMALLOC_INCLUDE="$i/$j"
            AC_MSG_RESULT(jemalloc.h found in $JEMALLOC_INCLUDE)
            PHP_ADD_LIBRARY(jemalloc, 1, RESPONDPHP_SHARED_LIBADD)
            break
          fi
        done
      done
      if test -z "$JEMALLOC_INCLUDE"; then
        AC_MSG_ERROR([jemalloc libraries not found. Check the path given to --with-jemalloc-dir and output in config.log])
      fi
    fi
    PHP_ADD_INCLUDE("${JEMALLOC_INCLUDE}")
    AC_DEFINE(HAVE_JEMALLOC, 1, [have jemalloc?])
  fi
  dnl }}}
  
  dnl {{{ --with--openssl
  if test "$PHP_OPENSSL" != "no" || test "$PHP_OPENSSL_DIR" != "no"; then
    if test "$PHP_OPENSSL_DIR" != "no"; then
      OPENSSL_INCLUDE="$PHP_OPENSSL_DIR/include"
      PHP_ADD_LIBRARY_WITH_PATH(ssl, "${PHP_OPENSSL_DIR}/${PHP_LIBDIR}")
      PHP_ADD_LIBRARY_WITH_PATH(crypto, "${PHP_OPENSSL_DIR}/${PHP_LIBDIR}")
    else
      OPENSSL_SEARCH_PATHS="/usr /usr/local /usr/local/openssl"
      for i in $OPENSSL_SEARCH_PATHS; do        
        for j in include ""; do
          if test -r "$i/$j/openssl/ssl.h"; then
            OPENSSL_INCLUDE="$i/$j"
            AC_MSG_RESULT(ssl.h found in $OPENSSL_INCLUDE)
            break
          fi
        done
      done
      PHP_CHECK_LIBRARY(ssl, SSL_connect, [
        AC_DEFINE(HAVE_OPENSSL, 1, [Have openssl?])
        PHP_ADD_INCLUDE("${OPENSSL_INCLUDE}")
      ], [
        AC_MSG_ERROR([openssl libraries not found. Check the path given to --with-openssl-dir and output in config.log])
      ], [
      ])
      PHP_ADD_INCLUDE("${OPENSSL_INCLUDE}")
      PHP_ADD_LIBRARY(ssl, 1, RESPONDPHP_SHARED_LIBADD)
      PHP_ADD_LIBRARY(crypto, 1, RESPONDPHP_SHARED_LIBADD)
      AC_MSG_RESULT(libuv include success)
    fi
    AC_RESPOND_HAVE_TLS_ALPN
    AC_RESPOND_HAVE_TLS_METHOD
    AC_RESPOND_HAVE_TLSV1_METHOD
    AC_RESPOND_HAVE_TLSV1_1_METHOD
    AC_RESPOND_HAVE_TLSV1_2_METHOD
    AC_RESPOND_HAVE_TLSV1_3_METHOD
  fi
  dnl }}}


  CFLAGS="$CFLAGS -Wall -pthread -fPIC"
  RESPONDPHP_SHARED_LIBADD="-lrt -lpthread $RESPONDPHP_SHARED_LIBADD"  
  dnl shared_objects_respondphp="$THIRDPARTY_BUILD_DIR/lib/libuv.a $shared_objects_respondphp"
  PHP_SUBST(RESPONDPHP_SHARED_LIBADD)
fi

dnl PHP_ADD_MAKEFILE_FRAGMENT([Makefile.thirdparty])

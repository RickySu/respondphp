dnl $Id$
dnl config.m4 for extension respondphp

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(respondphp, for respondphp support,
dnl Make sure that the comment is aligned:
[  --with-respondphp       Include respondphp support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(respondphp, whether to enable respondphp support,
dnl Make sure that the comment is aligned:
[  --enable-respondphp     Enable respondphp support])


PHP_ARG_WITH(openssl, for OpenSSL support in event,
[  --with-openssl          Include OpenSSL support], yes, no)


PHP_ARG_WITH(openssl-dir, for OpenSSL installation prefix,
[  --with-openssl-dir[=DIR]  openssl installation prefix], no, no)

PHP_ARG_WITH(uv-dir, for libuv installation prefix,
[  --with-uv-dir[=DIR]       libuv installation prefix], no, no)

if test "$PHP_RESPONDPHP" != "no"; then
  dnl Write more examples of tests here...

  modules="
    respondphp.c
  "

  if test -z "$UV_DIR"; then
    for i in /usr /usr/local /opt; do
      if test -f $i/include/uv.h; then
        UV_DIR=$i
      fi
    done
  fi

  PHP_CHECK_LIBRARY(uv, uv_pipe_getsockname, [
    AC_DEFINE(HAVE_UV, 1, [Have libuv 1.3.0 or later])
    PHP_ADD_LIBRARY_WITH_PATH(uv, $UV_DIR, RESPONDPHP_SHARED_LIBADD)
  ], [
    AC_MSG_ERROR([libuv libraries not found. Check the path given to --with-uv-dir and output in config.log])
  ], [
    -L$UV_DIR
  ])

  dnl {{{ --with--openssl
  if test "$PHP_OPENSSL" != "no"; then
    test -z "$PHP_OPENSSL" && PHP_OPENSSL=no

    if test -z "$PHP_OPENSSL_DIR" || test $PHP_OPENSSL_DIR == "no"; then
      PHP_OPENSSL_DIR=yes
    fi
    PHP_SETUP_OPENSSL(RESPONDPHP_SHARED_LIBADD, [
      AC_DEFINE(HAVE_OPENSSL,1,[ ])
    ], [
      AC_MSG_ERROR([OpenSSL libraries not found.
          Check the path given to --with-openssl-dir and output in config.log
      ])
    ])
    PHP_ADD_LIBRARY(ssl, RESPONDPHP_SHARED_LIBADD)
  fi
  dnl }}}

  RESPONDPHP_SHARED_LIBADD="-lrt -lpthread $RESPONDPHP_SHARED_LIBADD"
  PHP_NEW_EXTENSION(respondphp, $modules, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
  PHP_SUBST(RESPONDPHP_SHARED_LIBADD)

fi

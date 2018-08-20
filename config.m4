THIRDPARTY_BUILD_DIR="$srcdir/thirdparty/build"

PHP_ADD_INCLUDE("$srcdir/thirdparty/libuv/include")

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

PHP_ARG_WITH(debug-respondphp, whether to enable debug respondphp support,
[  --with-debug-respondphp   include debug respondphp support], no, no)

if test "$PHP_RESPONDPHP" != "no"; then
  dnl Write more examples of tests here...

  modules="
    respondphp.c
    src/respond_event_loop.c
  "

  if test "$PHP_DEBUG_RESPONDPHP" == "no"; then
      DEBUG_RESPONDPHP="-DNDEBUG"
      CPPFLAGS="$CPPFLAGS $DEBUG_RESPONDPHP"
      PHP_SUBST(DEBUG_RESPONDPHP)

  fi
  
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
  PHP_ADD_MAKEFILE_FRAGMENT([Makefile.thirdparty])
  shared_objects_respondphp="$THIRDPARTY_BUILD_DIR/lib/libuv.a $shared_objects_respondphp"
  PHP_SUBST(RESPONDPHP_SHARED_LIBADD)

fi

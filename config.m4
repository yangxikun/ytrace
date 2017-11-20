dnl $Id$
dnl config.m4 for extension ytrace

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(ytrace, for ytrace support,
dnl Make sure that the comment is aligned:
dnl [  --with-ytrace             Include ytrace support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(ytrace, whether to enable ytrace support,
dnl Make sure that the comment is aligned:
[  --enable-ytrace           Enable ytrace support])

if test "$PHP_YTRACE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-ytrace -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/ytrace.h"  # you most likely want to change this
  dnl if test -r $PHP_YTRACE/$SEARCH_FOR; then # path given as parameter
  dnl   YTRACE_DIR=$PHP_YTRACE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for ytrace files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       YTRACE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$YTRACE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the ytrace distribution])
  dnl fi

  dnl # --with-ytrace -> add include path
  dnl PHP_ADD_INCLUDE($YTRACE_DIR/include)

  dnl # --with-ytrace -> check for lib and symbol presence
  dnl LIBNAME=ytrace # you may want to change this
  dnl LIBSYMBOL=ytrace # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $YTRACE_DIR/$PHP_LIBDIR, YTRACE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_YTRACELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong ytrace lib version or lib not found])
  dnl ],[
  dnl   -L$YTRACE_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(YTRACE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(ytrace, ytrace.c ytrace_execute.c ytrace_opcode_handler_override.c ytrace_var.c ytrace_str.c ytrace_file.c ytrace_filter.c, $ext_shared)
fi

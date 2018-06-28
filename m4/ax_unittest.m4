AC_DEFUN([AX_UNITTEST],
[
    AC_PATH_PROG([XARGS],[xargs])
    AC_SUBST([XARGS])
    AC_CHECK_HEADER([setjmp.h], [HAS_SETJMP=true], [HAS_SETJMP=false])
    AC_CHECK_HEADER([stdarg.h], [HAS_STDARG=true], [HAS_STDARG=false])

    AM_CONDITIONAL([HAVE_UNITTEST], [test x$XARGS != x -a x$HAS_SETJMP = xtrue])

    AS_IF([test x$XARGS = x], [AX_RED_WARN([xargs not found, disabled unit tests])])
    AS_IF([test x$HAS_SETJMP != xtrue], [AX_RED_WARN([setjmp.h header not found, disabled unit tests])])
    AS_IF([test x$HAS_STDARG != xtrue], [AX_RED_WARN([stdarg.h header not found, disabled unit tests])])
])

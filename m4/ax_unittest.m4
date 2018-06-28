AC_DEFUN([AX_UNITTEST],
[
    AC_PATH_PROG([XARGS],[xargs])
    AC_SUBST([XARGS])
    AM_CONDITIONAL([HAVE_UNITTEST], [test x$XARGS != x])

    AS_IF([test x$XARGS = x], [AX_RED_WARN([xargs not found, disabled unit tests])])
])

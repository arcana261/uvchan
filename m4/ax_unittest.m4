AC_DEFUN([AX_UNITTEST],
[
    AC_PATH_PROG([XARGS],[xargs])
    AC_SUBST([XARGS])
    AC_CHECK_HEADER([setjmp.h], [HAS_SETJMP=true], [HAS_SETJMP=false])
    AC_CHECK_HEADER([stdarg.h], [HAS_STDARG=true], [HAS_STDARG=false])
    AC_CHECK_HEADER([sys/time.h], [HAS_SYS_TIME=true], [HAS_SYS_TIME=false])
    AC_CHECK_HEADER([unistd.h], [HAS_UNISTD=true], [HAS_UNISTD=false])
    AC_CHECK_HEADER([signal.h], [HAS_SIGNAL=true], [HAS_SIGNAL=false])
    AC_CHECK_HEADER([errno.h], [HAS_ERRNO=true], [HAS_ERRNO=false])

    AC_MSG_CHECKING([availability of strtok_r function])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include <string.h>

        char buff[128];
        char* tok;
        char* saveptr;
    ]], [[
        strcpy(buff, "h e l l o");
        tok = strtok_r(buff, " ", &saveptr);
        tok = strtok_r(NULL, " ", &saveptr);
        return 0;
    ]])], [HAS_STRTOK_R=true], [HAS_STRTOK_R=false])
    AC_MSG_RESULT([$HAS_STRTOK_R])
    
    # resolve pthread
    AX_PTHREAD

    AC_CHECK_HEADER([sched.h], [HAS_SCHED=true], [HAS_SCHED=false])

    AM_CONDITIONAL([HAVE_UNITTEST], [test x$XARGS != x -a x$HAS_SETJMP = xtrue -a x$ax_pthread_ok = xyes -a x$HAS_SCHED = xtrue -a x$HAS_SYS_TIME = xtrue -a x$HAS_UNISTD = xtrue -a x$HAS_SIGNAL = xtrue -a x$HAS_ERRNO = xtrue -a x$HAS_STRTOK_R = xtrue])

    AS_IF([test x$XARGS = x], [AX_RED_WARN([xargs not found, disabled unit tests])])
    AS_IF([test x$HAS_SETJMP != xtrue], [AX_RED_WARN([setjmp.h header not found, disabled unit tests])])
    AS_IF([test x$HAS_STDARG != xtrue], [AX_RED_WARN([stdarg.h header not found, disabled unit tests])])
    AS_IF([test x$ax_pthread_ok != xyes], [AX_RED_WARN([pthread not found, disabled unit tests])])
    AS_IF([test x$HAS_SCHED != xtrue], [AX_RED_WARN([sched.h header not found, disabled unit tests])])
    AS_IF([test x$HAS_SYS_TIME != xtrue], [AX_RED_WARN([sys/time.h header not found, disabled unit tests])])
    AS_IF([test x$HAS_UNISTD != xtrue], [AX_RED_WARN([unistd.h header not found, disabled unit tests])])
    AS_IF([test x$HAS_SIGNAL != xtrue], [AX_RED_WARN([signal.h header not found, disabled unit tests])])
    AS_IF([test x$HAS_ERRNO != xtrue], [AX_RED_WARN([errno.h header not found, disabled unit tests])])
    AS_IF([test x$HAS_STRTOK_R != xtrue], [AX_RED_WARN([strtok_r function not found in string.h, disabled unit tests])])
])

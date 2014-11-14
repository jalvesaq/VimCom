AC_INIT(vimcom, 1.1-0)

AC_ARG_ENABLE([clientserver],
              [AS_HELP_STRING([--enable-clientserver],[support Vim clientserver feature @<:@yes@:>@])],
              [want_clientserver="${enableval}"],
              [want_clientserver="yes"])

if test "x${want_clientserver}" = xyes; then
    AC_PATH_XTRA
    XCFLAGS="$X_CFLAGS"
    XLIBS="-lX11 $X_EXTRA_LIBS $X_LIBS"
    AC_SUBST(XCFLAGS)
    AC_SUBST(XLIBS)
    echo ""
    echo "Information about X11:"
    echo "    x_includes     : $x_includes"
    echo "    x_libraries    : $x_libraries"
    echo "    X_CFLAGS       : $X_CFLAGS"
    echo "    X_PRE_LIBS     : $X_PRE_LIBS"
    echo "    X_EXTRA_LIBS   : $X_EXTRA_LIBS"
    echo "    X_LIBS         : $X_LIBS"
    echo ""
else
    XCFLAGS="-DNEOVIM_ONLY $X_CFLAGS"
    XLIBS="$X_LIBS"
    AC_SUBST(XLIBS)
    AC_SUBST(XCFLAGS)
fi

AC_OUTPUT(src/Makevars)


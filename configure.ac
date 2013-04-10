AC_INIT(vimcom, 0.9-8)

AC_PATH_XTRA

XCFLAGS="$X_CFLAGS"

XLIBS="-lX11 $X_EXTRA_LIBS $X_LIBS"

AC_SUBST(XCFLAGS)
AC_SUBST(XLIBS)

AC_OUTPUT(src/Makevars)

echo ""
echo "Information about X11:"
echo "    x_includes     : $x_includes"
echo "    x_libraries    : $x_libraries"
echo "    X_CFLAGS       : $X_CFLAGS"
echo "    X_PRE_LIBS     : $X_PRE_LIBS"
echo "    X_EXTRA_LIBS   : $X_EXTRA_LIBS"
echo "    X_LIBS         : $X_LIBS"
echo ""


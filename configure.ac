AC_INIT([libisi], [0.0.0], [smartphones-userland@linuxtogo.org], [libisi])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([dist-bzip2])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AM_PROG_CC_C_O
AC_GNU_SOURCE
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

# pkg-config
PKG_PROG_PKG_CONFIG
VALA_REQUIRED=0.9.7
GLIB_REQUIRED=2.18.0

PKG_CHECK_MODULES(GLIB,
                  glib-2.0 >= $GLIB_REQUIRED)

AC_ARG_ENABLE(tests,
              [--enable-tests           Enable tests(default=disabled)],
              [enable_tests=$enableval],
              [enable_tests="no"])

AM_CONDITIONAL([WANT_TESTS], [test x"$enable_tests" = "xyes"])
if test "$enable_tests" != "no" ; then
    # vala
    AC_PATH_PROG(VALAC, valac, valac)
    AC_SUBST(VALAC)
    m4_ifdef([AM_PROG_VALAC], [AM_PROG_VALAC([$VALA_REQUIRED])])

    PKG_CHECK_MODULES(GOBJECT,
                      [gobject-2.0 >= $GLIB_REQUIRED])
    AC_SUBST(GOBJECTC_FLAGS)
    AC_SUBST(GOBJECTC_LIBS)
fi


AC_ARG_ENABLE(wireshark-plugin,
              [--enable-wireshark-plugin      Enable wireshark plugin (default=disabled)],
              [wireshark=$enableval],
              [wireshark="no"])
AM_CONDITIONAL([WANT_WIRESHARK], [test x"$wireshark" = "xyes"])

if test "x$WANT_WIRESHARK" = "xyes" ; then
    PKG_CHECK_MODULES(GMODULE,
                      [gmodule-2.0 >= $GLIB_REQUIRED])
fi

AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

AC_PATH_PROG([GLIB_MKENUMS],[glib-mkenums])

AC_CONFIG_FILES([
                 Makefile
                 isi/Makefile
                 data/Makefile
                 data/libisi.pc
                 test/Makefile])
                 #wireshark-plugin/Makefile])

AC_OUTPUT

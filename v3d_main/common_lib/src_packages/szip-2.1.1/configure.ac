##                                               -*- Autoconf -*-
## Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([SZIP], [2.1.1], [help@hdfgroup.org])
AC_CONFIG_SRCDIR([src/rice.h])
AC_CONFIG_AUX_DIR([bin])
AC_CONFIG_HEADER([src/SZconfig.h])

## Tell automake that this is not a GNU package (and doesn't need NEWS,
## AUTHORS, etc. files)
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

AC_CANONICAL_HOST
AC_PREFIX_DEFAULT([`pwd`/szip])

## Source any special files that we need. These files normally aren't
## present but can be used by the maintainers to fine tune things like
## turning on debug or profiling flags for the compiler. The search order
## is:
##
##         CPU-VENDOR-OS
##         VENDOR-OS
##         CPU-OS
##         CPU-VENDOR
##         OS
##         VENDOR
##         CPU
##
## If the `OS' ends with a version number then remove it. For instance,
## `freebsd3.1' would become `freebsd'
##
case "$host_os" in
  aix4.*)       host_os_novers="aix4.x"     ;;
  aix5.*)       host_os_novers="aix5.x"     ;;
  freebsd*)     host_os_novers="freebsd"    ;;
  irix5.*)      host_os_novers="irix5.x"    ;;
  irix6.*)      host_os_novers="irix6.x"    ;;
  osf4.*)       host_os_novers="osf4.x"     ;;
  osf5.*)       host_os_novers="osf5.x"     ;;
  solaris2.*)   host_os_novers="solaris2.x" ;;
  *)            host_os_novers="$host_os"   ;;
esac

host_config="none"
for f in $host_cpu-$host_vendor-$host_os        \
         $host_cpu-$host_vendor-$host_os_novers \
         $host_vendor-$host_os                  \
         $host_vendor-$host_os_novers           \
         $host_cpu-$host_os                     \
         $host_cpu-$host_os_novers              \
         $host_cpu-$host_vendor                 \
         $host_os                               \
         $host_os_novers                        \
         $host_vendor                           \
         $host_cpu ; do
  AC_MSG_CHECKING([for config $f])
  if test -f "$srcdir/config/$f"; then
    host_config=$srcdir/config/$f
    AC_MSG_RESULT([found])
    break
  fi
  AC_MSG_RESULT([no])
done
if test "X$host_config" != "Xnone"; then
  CC_BASENAME="`echo $CC | cut -f1 -d' ' | xargs basename 2>/dev/null`"
  . $host_config
fi

## Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL

## Fix up the INSTALL macro if it's a relative path. We want the
## full-path to the binary instead.
case "$INSTALL" in
  *install-sh*)
    INSTALL='\${top_srcdir}/bin/install-sh -c'
    ;;
esac

## ----------------------------------------------------------------------
## Check which archiving tool to use. This needs to be done before
## the AC_PROG_LIBTOOL macro.
##
if test -z "$AR"; then
  AC_CHECK_PROGS([AR], [ar xar], [:], [$PATH])
fi
AC_SUBST([AR])

## Export the AR macro so that it will be placed in the libtool file
## correctly.
export AR

AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

## Post processing to patch up some deficiencies in libtool
case $host_os in
  linux*)
    # If gcc is not used, need to set $wl to use "-Wl,"
    if $CC -v 2>&1 | grep '^gcc' > /dev/null ; then
      : using gcc
    else
      echo 'fixing $wl in' $ofile
ed - $ofile <<EOF 2> /dev/null
/^wl=""/s//wl="-Wl,"/
w
q
EOF
    fi
    ;;
esac



## Are we building this in debug or production mode? (Remove the -g flag
## in production mode.)
AC_MSG_CHECKING(for build mode)
AC_ARG_ENABLE(production,
              [AC_HELP_STRING([--enable-production],
                              [Determines how to run the compiler.])])

case "X-$enable_production" in
  X-|X-yes)
    AC_MSG_RESULT([production])

    # Remove the "-g" flag from compile line if it's in there.
    CFLAGS_temp=""
    if test -n "$CFLAGS"; then
      for d in $CFLAGS ; do
        if test "X$d" != "X-g"; then
          CFLAGS_temp="$CFLAGS_temp $d"
        fi
      done
      CFLAGS=$CFLAGS_temp
    fi


    CFLAGS="$CFLAGS $PROD_CFLAGS"
    CPPFLAGS="$CPPFLAGS $PROD_CPPFLAGS"
    ;;
  X-no)
    AC_MSG_RESULT([development])
    CFLAGS="$CFLAGS $DEBUG_CFLAGS"
    CPPFLAGS="$CPPFLAGS $DEBUG_CPPFLAGS"
    ;;
  *)
    AC_MSG_RESULT([user-defined])
    ;;
esac

## ======================================================================
## Checks for library functions
## ======================================================================

AC_MSG_CHECKING([for math library support])
AC_TRY_LINK([#include <math.h>], [volatile x = 37.927; k=1.2; sinh(x*k/cosh(x/k)],
            [AC_MSG_RESULT([yes])],
            [AC_MSG_RESULT([no]); LIBS="$LIBS -lm"])


## Checks for libraries.

## Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h])

## Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

## Checks for library functions.
AC_FUNC_ERROR_AT_LINE
##AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memset])

## ======================================================================
## Checks if encoding is enabled
## ======================================================================
AC_MSG_CHECKING([if encoding enabled])
AC_ARG_ENABLE([encoding],
             [AC_HELP_STRING([--enable-encoding],
                             [Enable SZIP encoding [default=yes]])],
             [ENCODING=$enableval],
             [ENCODING=yes])
if test "X$ENCODING" = "Xyes"; then
     echo "yes"
     AC_DEFINE([HAVE_ENCODING], [1], [Define if encoding is enabled])
   else
     echo "no"
fi
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 test/Makefile])
AC_OUTPUT

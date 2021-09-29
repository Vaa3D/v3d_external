#                                                   -*- shell-script -*-
#
# This file is part of the HDF4 build script. It is processed shortly
# after configure starts and defines, among other things, flags for
# the various compilation modes.

# Choosing C, Fortran, and C++ Compilers
# --------------------------------------
#
# The user should be able to specify the compiler by setting the CC, F77,
# and CXX environment variables to the name of the compiler and any
# switches it requires for proper operation. If CC is unset then this
# script may set it. If CC is unset by time this script completes then
# configure will try `gcc' and `cc' in that order (perhaps some others
# too).
#
# Note: Code later in this file may depend on the value of $CC_BASENAME
#       in order to distinguish between different compilers when
#       deciding which compiler command-line switches to use.  This
#       variable is set based on the incoming value of $CC and is only
#       used within this file.

if test "X-$CC" = "X-"; then
  CC=xlc
  CC_BASENAME=xlc
fi


# C, Fortran, and C++ Compiler and Preprocessor Flags
# ---------------------------------------------------
#
# - Flags that end with `_CFLAGS' are always passed to the C compiler.
# - Flags that end with `_FFLAGS' are always passed to the Fortran
#   compiler.
# - Flags that end with `_CXXFLAGS' are always passed to the C++ compiler.
# - Flags that end with `_CPPFLAGS' are passed to the C and C++ compilers
#   when compiling but not when linking.
#
#   DEBUG_CFLAGS
#   DEBUG_FFLAGS
#   DEBUG_CXXFLAGS
#   DEBUG_CPPFLAGS  - Flags to pass to the compiler to create a
#                     library suitable for use with debugging
#			          tools. Usually this list will exclude
#                     optimization switches (like `-O') and include
#                     switches that turn on symbolic debugging support
#                     (like `-g').
#
#   PROD_CFLAGS
#   PROD_FFLAGS
#   PROD_CXXFLAGS
#   PROD_CPPFLAGS   - Flags to pass to the compiler to create a
#                     production version of the library. These
#                     usualy exclude symbolic debugging switches (like
#                     `-g') and include optimization switches (like
#                     `-O').
#
#   PROFILE_CFLAGS
#   PROFILE_FFLAGS
#   PROFILE_CXXFLAGS
#   PROFILE_CPPFLAGS- Flags to pass to the compiler to create a
#                     library suitable for performance testing (like
#                     `-pg').  This may or may not include debugging or
#                     production flags.
#			
#   FFLAGS
#   CFLAGS          - Flags can be added to these variable which
#                     might already be partially initialized. These
#                     flags will always be passed to the compiler and
#                     should include switches to turn on full warnings.
#
#                     WARNING: flags do not have to be added to the CFLAGS
#                     or FFLAGS variable if the compiler is the GNU gcc
#                     and g77 compiler.
#
#                     FFLAGS and CFLAGS should contain *something* or else
#                     configure will probably add `-g'. For most systems
#                     this isn't a problem but some systems will disable
#                     optimizations in favor of the `-g'. The configure
#                     script will remove the `-g' flag in production mode
#                     only.
#
# These flags should be set according to the compiler being used.
# There are two ways to check the compiler. You can try using `-v' or
# `--version' to see if the compiler will print a version string.  You
# can use the value of $FOO_BASENAME which is the base name of the
# first word in $FOO, where FOO is either CC, F77, or CXX (note that the
# value of CC may have changed above).

case $CC_BASENAME in
  gcc)
    CFLAGS="$CFLAGS"
    DEBUG_CFLAGS="-g -fverbose-asm"
    DEBUG_CPPFLAGS=
    PROD_CFLAGS="-O3 -fomit-frame-pointer"
    PROD_CPPFLAGS=
    PROFILE_CFLAGS="-pg"
    PROFILE_CPPFLAGS=
    ;;

  *)
    CFLAGS="$CFLAGS -D_ALL_SOURCE -qlanglvl=ansi"
    DEBUG_CFLAGS="-g"
    DEBUG_CPPFLAGS=
    PROD_CFLAGS="-O"
    PROD_CPPFLAGS=
    PROFILE_CFLAGS="-pg"
    PROFILE_CPPFLAGS=
    ;;
esac

# Overriding Configure Tests
# --------------------------
#
# Values for overriding configuration tests when cross compiling.

# Set this to `yes' or `no' depending on whether the target is big
# endian or little endian.
#ac_cv_c_bigendian=${ac_cv_c_bigendian='yes'}

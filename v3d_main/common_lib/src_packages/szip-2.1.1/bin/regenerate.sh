#! /bin/sh
#
# Run this script in the top source directory of HDF4 to regenerate
# all of the necessary files.
#
# This is a helper script to regenerate the aclocal.m4, h4config.h.in,
# Makefile.in, and configure files all in one go. The versions for the
# tools need to be according to this table or better:
#
#       Name            Version
#       ----            -------
#       autoreconf        2.57
#
#       aclocal           1.7.7
#       automake          1.7.7
#       autoheader        2.57
#       autoconf          2.57
#
echo /usr/bin/autoreconf -v
env AUTOMAKE="/usr/bin/automake --foreign" /usr/bin/autoreconf -v
#echo /usr/bin/autoheader
#/usr/bin/autoheader
#echo /usr/bin/aclocal
#/usr/bin/aclocal
#echo /usr/bin/autoconf
#/usr/bin/autoconf
#echo /usr/bin/automake --foreign
#/usr/bin/automake --foreign

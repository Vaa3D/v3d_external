#!/bin/csh
#
# Teem: Tools to process and visualize scientific data and images              
# Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
# Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
# Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# (LGPL) as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# The terms of redistributing and/or modifying this software also
# include exceptions to the LGPL that facilitate static linking.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

# This script is something GLK uses to test for consistent use of Teem
# library name-spaces; it isn't likely to be useful for a wider
# audience, but its under version control for posterity's sake

# this is where GLK puts his Teem build
cd ~/teem-install/include

set D=`pwd`

echo "======================"
echo "looking in $D; assuming its up-to-date"
echo "Should see only TEEM-related #defines below"
echo "======================"

cd ~/teem-install/include

# (TEEM_LIB_LIST)
grep "#define" teem/*.h | grep -v _export \
  | grep -v " AIR"    | grep -v " _AIR" \
  | grep -v " HEST"   | grep -v " _HEST" \
  | grep -v " BIFF"   | grep -v " _BIFF" \
  | grep -v " NRRD"   | grep -v " _NRRD" \
  | grep -v " ELL"    | grep -v " _ELL" \
  | grep -v " UNRRDU" | grep -v " _UNRRDU" \
  | grep -v " ALAN"   | grep -v " _ALAN" | grep -v " alan" \
  | grep -v " MOSS"   | grep -v " _MOSS" \
  | grep -v " TIJK"   | grep -v " _TIJK" \
  | grep -v " GAGE"   | grep -v " _GAGE" \
  | grep -v " DYE"    | grep -v " _DYE" \
  | grep -v " BANE"   | grep -v " _BANE" \
  | grep -v " LIMN"   | grep -v " _LIMN" | grep -v " limn" \
  | grep -v " ECHO"   | grep -v " _ECHO" | grep -v " echo" \
  | grep -v " HOOVER" | grep -v " _HOOVER" \
  | grep -v " SEEK"   | grep -v " _SEEK" \
  | grep -v " TEN"    | grep -v " _TEN" \
  | grep -v " ELF"    | grep -v " _ELF" \
  | grep -v " PULL"   | grep -v " _PULL" \
  | grep -v " COIL"   | grep -v " _COIL" | grep -v " coil" \
  | grep -v " PUSH"   | grep -v " _PUSH" \
  | grep -v " MITE"   | grep -v " _MITE" | grep -v "mite_" \
  | grep -v " MEET"   | grep -v " _MEET" 

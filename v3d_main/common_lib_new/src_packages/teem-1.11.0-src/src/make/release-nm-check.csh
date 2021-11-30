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
cd ~/teem-install/lib

set D=`pwd`

echo "======================"
echo "looking in $D; assuming its up-to-date"
echo "Should not see any symbols listed below"
echo "======================"

# | grep -v "         U "    : an undefined symbol

# (TEEM_LIB_LIST)
nm libteem.a \
  | grep -v "libteem.a(" \
  | grep -v "         U " \
  | grep -v \\\?\\\?\\\?\\\?\\\?\\\?\\\?\\\?\ t\  \
  | grep -v \\\?\\\?\\\?\\\?\\\?\\\?\\\?\\\?\ s\  \
  | grep -v \\\?\\\?\\\?\\\?\\\?\\\?\\\?\\\?\ d\  \
  | grep -v \\\?\\\?\\\?\\\?\\\?\\\?\\\?\\\?\ b\  \
  | grep -v " _air"    | grep -v " __air" \
  | grep -v " _hest"   | grep -v " __hest" \
  | grep -v " _biff"   | grep -v " __biff" \
  | grep -v " _nrrd"   | grep -v " __nrrd" \
  | grep -v " _ell"    | grep -v " __ell" \
  | grep -v " _unrrdu" | grep -v " __unrrdu" \
  | grep -v " _alan"   | grep -v " __alan" \
  | grep -v " _moss"   | grep -v " __moss" \
  | grep -v " _tijk"   | grep -v " __tijk" \
  | grep -v " _gage"   | grep -v " __gage" \
  | grep -v " _dye"    | grep -v " __dye" \
  | grep -v " _bane"   | grep -v " __bane" \
  | grep -v " _limn"   | grep -v " __limn" \
  | grep -v " _echo"   | grep -v " __echo" \
  | grep -v " _hoover" | grep -v " __hoover" \
  | grep -v " _seek"   | grep -v " __seek" \
  | grep -v " _ten"    | grep -v " __ten" \
  | grep -v " _elf"    | grep -v " __elf" \
  | grep -v " _pull"   | grep -v " __pull" \
  | grep -v " _coil"   | grep -v " __coil" \
  | grep -v " _push"   | grep -v " __push" \
  | grep -v " _mite"   | grep -v " __mite" \
  | grep -v " _meet"   | grep -v " __meet" \
  | uniq


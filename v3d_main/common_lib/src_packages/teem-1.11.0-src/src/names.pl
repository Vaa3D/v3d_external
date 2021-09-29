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

# GLK uses this to make sure that there are no filename clashes
# between different libraries, a constraint introduced with the
# Windows port, and then enforced with the practice of putting all
# object files into per-architecture "obj" directories.
#
# Currently this is very simple!  Usage is:
#
# (TEEM_LIB_LIST)
# ls -1 {air,hest,biff,nrrd,ell,unrrdu,alan,moss,tijk,gage,dye,bane,limn,echo,hoover,seek,ten,elf,pull,coil,push,mite,meet}/*.c | perl names.pl | sort | wc -l
# ls -1 {air,hest,biff,nrrd,ell,unrrdu,alan,moss,tijk,gage,dye,bane,limn,echo,hoover,seek,ten,elf,pull,coil,push,mite,meet}/*.c | perl names.pl | sort | uniq | wc -l
#
# and then make sure the two numbers are the same.

while (<>) {
    chomp;
    ($dir, $name) = split '/', $_, 2;
    print "$name\n";
}

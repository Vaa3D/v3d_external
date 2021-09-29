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

=========================
User-set environment variables which effect global things:
=========================

TEEM_ARCH: the architecture you're compiling with and for
  >>> This is the only environment variable which MUST be set <<<

TEEM_ROOT: the top-level "teem" directory, under which are the
  directories where object, library, and include files will be
  installed.  If not set, the top-level directory is taken to be
  "../..", when inside the source directory for the individual
  libraries

TEEM_LINK_SHARED: if set to "true" then binaries, when linked, will be
  linked with shared libraries, and not static libraries. If not set,
  we link against static libraries, in order to produce
  stand-alone-ish binaries

=========================
The variables that can/must be set by the individual architecture
.mk files.  Those which must be set are marked by a (*):
=========================

TEEM_ENDIAN (*): some things in the air library are too annoying to do
  if the endianness is determined only at run-time, so just setting
  here simplifies things
  1234: Little Endian (Intel and friends)
  4321: Big Endian (everyone else)

CC, LD, AR, RM, INSTALL, CHMOD (*): programs used during make

SHEXT: the extension on the name of shared libraries (.so, .sl, .dll)

SHARED_CFLAG, STATIC_CFLAG (*): flags which are passed to $(CC) when
  used to create binaries, so as to control whether shared or static
  libraries are linked against.

BIN_CFLAGS: any flags to $(CC) which should be used for compiling 
  binaries (in addition to the SHARED_CFLAG, STATIC_CFLAG flags above)

OPT_CFLAG: how to control optimization (if desired)

ARCH_CFLAG: any flags to $(CC) which are important for compiling
  particular to the target architecture

CFLAGS: any flags to $(CC) for both .o and binary compiliation, in
  addition to $(OPT_CFLAG) $(ARCH_CFLAG)

ARCH_LDFLAG: any architecture-specific flags to $(LD) which are
  important for making a shared library on the target architecture

SHARED_LDFLAG: the flag to $(LD) which causes a shared library
  generated to be produced, not a static one

LDFLAGS: any flags to $(LD) for making shared libraries, in addition
  to $(ARCH_LDFLAG) $(SHARED_LDFLAG)

OTHER_CLEAN: other files that might have been created automatically 
  as part of compilation, but which should be deleted if "make clean"
  is to be true to its word (e.g. "so_locations" on SGI)

=========================
The variables that can be set by the individual library Makefile's
=========================

LIB: the name of the library being compiled.  If this isn't set, the
  assumption is that there is no new library to compile, but simply
  a set of binaries which depend on other libraries

LIB_BASENAME: the base name of the archive and shared library files; 
  by default this is set to "lib$(LIB)", but setting this allows one
  to over-ride that.

HEADERS: the "public" .h files for this library; these will be installed

PRIV_HEADERS: .h files needed for this library, but not installed
  
LIBOBJS: the .o files (created from .c files) which comprise this library

TEST_BINS: executables which are used to debug a library, but which will
  not be installed

BINS: executables which will be installed

BINLIBS: all the libraries (-l<name> ...) against which $(BINS) and
  $(TEST_BINS) collectively depend, for example "-lnrrd -lbiff -lair"
  is used for unrrdu stuff.  Unfortunately, warning messages of the
  sort " ... not used for resolving any symbol" are to be expected due
  to the current simplistic nature of the Teem makefiles

IPATH, LDPATH: the "-I<dir>" and "-L<dir>" flags for the compiler and linker.
  Values set here will be suffixed by the common makefile

LDLIBS: when making shared libraries, it is sometimes necessary to
  link against other libraries.  $(LDLIBS) is the last argument to the
  $(LD) call which creates a shared library, and should be used like
  $(BINLIBS). $(LPATH) can be set with the "-L<dir." flags non-Teem
  libraries, the Teem library flags will be suffixed on.

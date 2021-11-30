=============== 
  Teem: Tools to process and visualize scientific data and images              
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

=============== License information

See above.  This preamble should appear on all released files. Full
text of the Simple Library Usage License (SLUL) should be in the file
"LICENSE.txt" in the "src" directory.  The SLUL is the GNU Lesser
General Public License, plus an exception: statically-linked binaries
that link with Teem can be destributed under the terms of your choice,
with very modest provisions.  

=============== How to compile

Use CMake to compile Teem.  CMake is available from:

http://www.cmake.org/

There are some instructions for building Teem with CMake at:

http://teem.sourceforge.net/build.html

=============== Directory Structure

* src/
  With one subdirectory for each of the teem libraries, all the 
  source for the libraries is in here.  See library listing below.
  The src/CODING.txt file documents Teem coding conventions.
  * src/make
    Files related to compiling Teem with src/GNUmakefile, the old way
    of making Teem prior to CMake.  This is still unofficially in use.
  * src/bin
    Source files for Teem command-line tools, including "unu" and "tend"

* include/
  Some short header files that are used to check the setting of compiler
  variables
  * include/teem/
    When using the old GNU make system, the include (.h) files for all the
    libraries (such as nrrd.h) get put here (but don't originate here).

* CMake/
  Files related to compiling Teem with CMake

* Testing/
  Tests run by CTest.  More are being added.

* data/
  Small reference datasets; more will be added for testing

* arch/
  When using the old GNU make system, objects and binaries are put
  in the cygin, darwin.32, linux.32, etc, architecture-dependent
  subdirectories, with a name which exactly matches valid settings
  for the environment variable TEEM_ARCH. Within these directories are:
  * lib/
    all libraries put both their static/archive (.a) and 
    shared/dynamic (.so) library files here (such as libnrrd.a)
  * bin/ 
    all libraries put their binaries here, hopefully in a way which
    doesn't cause name clashes
  * obj/
    make puts all the .o files in here, for all libraries. When
    compiling "dev", it also puts libraries here, so that "tests"
    can link against them there

* python/
  For python wrappings
  * python/ctypes
    Bindings for python via ctypes

* Examples/
  Place for examples of Teem-using programs, but unfortunately 
  not populated by much right now.  A work in progress.

=============== Teem libraries

Teem is a coordinated collection of libraries, with a stable
dependency graph.  Below is a listing of the libraries (with
indication of the libraries upon which it depends).  (TEEM_LIB_LIST)

* air: Basic utility functions, used throughout Teem

* hest: Command-line parsing (air) 

* biff: Accumulation of error messages (air)

* nrrd: Nearly Raw Raster Data- library for raster data manipulation,
and support for NRRD file format (biff, hest, air) 

* ell: Linear algebra: operations on vectors, matrices and quaternions,
and solving cubic polynomials. (nrrd, biff, air)

* unrrdu: internals of "unu" command-line tool, and some machinery used
in other multi-command tools (like "tend") (nrrd, biff, hest, air)

* alan: Reaction-diffusion textures (nrrd, ell, biff, air)

* moss: Processing of 2D multi-channel images (ell, nrrd, biff, hest, air)

* tijk: Spherical harmonics and higher-order tensors (ell, nrrd, air)

* gage: Convolution-based measurement of 3D fields, or 4D scale-spacem
(ell, nrrd, biff, air)

* dye: Color spaces and conversion (ell, biff, air)

* bane: Implementation of Semi-Automatic Generation of Transfer Functions
(gage, unrrdu, nrrd, biff, air)

* limn: Basics of computer graphics, including polygonal data representation
and manipulation (gage, ell, unrrdu, nrrd, biff, hest, air)

* echo: Simple ray-tracer, written for class (limn, ell, nrrd, biff, air)

* hoover: Framework for multi-thread ray-casting volume renderer
(limn, ell, nrrd, biff, air)

* seek: Extraction of polygonal features from volume data, including
Marching Cubes and ridge surfaces (limn, gage, ell, nrrd, biff, hest, air)

* ten: Visualization and analysis of diffusion imaging and diffusion tensor
fields (echo, limn, dye, gage, unrrdu, ell, nrrd, biff, air)

* elf: Visualization/processing of high-angular resolution diffusion imaging
(ten, tijk, limn, ell, nrrd, air)

* pull: Particle systems for image feature sampling in 3D or 4D scale-space
(ten, limn, gage, ell, nrrd, biff, hest, air)

* coil: Non-linear image filtering (ten, ell, nrrd, biff, air)

* push: Original implmentation of glyph packing for DTI
(ten, gage, ell, nrrd, biff, air)

* mite: Hoover-based volume rendering with gage-based transfer functions
(ten, hoover, limn, gage, ell, nrrd, biff, air)

* meet: Uniform API to things common to all Teem libraries 
(mite, push, coil, pull, elf, ten, seek, hoover, echo, limn, bane, dye,
gage, tijk, moss, alan, unrrdu, ell, nrrd, biff, hest, air)

=============== Teem comand-line tools

The easiest way to access the functionality in Teem is with its
command-line tools.  Originally intended only as demos for the Teem
libraries and using their APIs, the command-line tools have become a
significant way of getting real work done.  Source for the tools is in
teem/src/bin.  The most commonly used tools are:

* unu: uses the "nrrd" library; a fast way to do raster data processing
and visualization

* tend: uses the "ten" library; for DW-MRI and DTI processing

* gprobe (and vprobe, pprobe): uses the "gage" library, allows measuring
gage items in scalar, vector, tensor, and DW-MRI volumes.

* miter: uses the "mite" library; a flexible volume renderer

* overrgb: for compositing an RGBA image over some background

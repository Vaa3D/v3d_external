#
# Teem: Tools to process and visualize scientific data and images             .
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

MACRO(CREATE_EXEC name sources)
  ADD_EXECUTABLE(${name} ${sources})
  TARGET_LINK_LIBRARIES(${name} teem)
  INSTALL(TARGETS ${name}
    RUNTIME DESTINATION bin
    )
  SET_TARGET_PROPERTIES(${name} PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
    )
ENDMACRO(CREATE_EXEC)

# These binaries should not be depending on BUILD_EXPERIMENTAL_LIBS
CREATE_EXEC(nrrdSanity nrrdSanity.c)
CREATE_EXEC(overrgb overrgb.c)
CREATE_EXEC(ilk ilk.c)
CREATE_EXEC(mrender mrender.c)
CREATE_EXEC(miter miter.c)
CREATE_EXEC(vprobe vprobe.c)
CREATE_EXEC(gprobe gprobe.c)
CREATE_EXEC(unu unu.c)
CREATE_EXEC(puller puller.c)
CREATE_EXEC(tend tend.c)

# NOTE: The BUILD_EXPERIMENTAL_LIBS-dependent inclusion of binaries has to be
# done with knowledge of how BUILD_EXPERIMENTAL_LIBS behaves in the top-level
# CMakeLists.txt file
IF(BUILD_EXPERIMENTAL_APPS)
  CREATE_EXEC(airSanity airSanity.c)
  CREATE_EXEC(cubic cubic.c)
  CREATE_EXEC(undos undos.c)
  CREATE_EXEC(qbert qbert.c)
  CREATE_EXEC(emap emap.c)
  CREATE_EXEC(talkweb talkweb.c)
  CREATE_EXEC(pprobe pprobe.c)
  CREATE_EXEC(ninspect ninspect.c)
  CREATE_EXEC(ungantry ungantry.c)
  CREATE_EXEC(deconv deconv.c)
  IF(BUILD_EXPERIMENTAL_LIBS)
    CREATE_EXEC(gkms gkms.c)
    CREATE_EXEC(spots spots.c)
  ENDIF(BUILD_EXPERIMENTAL_LIBS)
ENDIF(BUILD_EXPERIMENTAL_APPS)

#IF(BUILD_TESTING)
#  ADD_TEST(nrrd.Sanity ${EXECUTABLE_OUTPUT_PATH}/nrrdSanity)
#ENDIF(BUILD_TESTING)

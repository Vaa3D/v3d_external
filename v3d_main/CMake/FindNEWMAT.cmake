# - Find NEWMAT library
# Find the native NEWMAT includes and library
# This module defines
#  NEWMAT_INCLUDE_DIR, where to find tiff.h, etc.
#  NEWMAT_LIBRARIES, libraries to link against to use NEWMAT.
#  NEWMAT_FOUND, If false, do not try to use NEWMAT.
# also defined, but not for general use are
#  NEWMAT_LIBRARY, where to find the NEWMAT library.

#=============================================================================
# Copyright 2002-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

FIND_PATH(NEWMAT_INCLUDE_DIR tiff.h)

SET(NEWMAT_NAMES ${NEWMAT_NAMES} newmat libnewmat )
FIND_LIBRARY(NEWMAT_LIBRARY NAMES ${NEWMAT_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set NEWMAT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NEWMAT  DEFAULT_MSG  NEWMAT_LIBRARY  NEWMAT_INCLUDE_DIR)

IF(NEWMAT_FOUND)
  SET( NEWMAT_LIBRARIES ${NEWMAT_LIBRARY} )
ENDIF(NEWMAT_FOUND)

MARK_AS_ADVANCED(NEWMAT_INCLUDE_DIR NEWMAT_LIBRARY)

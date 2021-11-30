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

ADD_EXECUTABLE(test_trand trand.c)
TARGET_LINK_LIBRARIES(test_trand teem)
ADD_TEST(trand ${EXECUTABLE_OUTPUT_PATH}/test_trand)

ADD_EXECUTABLE(test_tload tload.c)
TARGET_LINK_LIBRARIES(test_tload teem)
ADD_TEST(tload ${EXECUTABLE_OUTPUT_PATH}/test_tload)

ADD_EXECUTABLE(test_tskip tskip.c)
TARGET_LINK_LIBRARIES(test_tskip teem)
# Note the different file names; tests are run in parallel
ADD_TEST(tskip11p ${EXECUTABLE_OUTPUT_PATH}/test_tskip -s 101 102 103 -p 66 81     -o tsA.raw tsA.nhdr)
ADD_TEST(tskip11n ${EXECUTABLE_OUTPUT_PATH}/test_tskip -s 101 102 103 -p 66 81 -ns -o tsB.raw tsB.nhdr)
ADD_TEST(tskip01p ${EXECUTABLE_OUTPUT_PATH}/test_tskip -s 101 102 103 -p 0 99      -o tsC.raw tsC.nhdr)
ADD_TEST(tskip01n ${EXECUTABLE_OUTPUT_PATH}/test_tskip -s 101 102 103 -p 0 99 -ns  -o tsD.raw tsD.nhdr)
ADD_TEST(tskip10p ${EXECUTABLE_OUTPUT_PATH}/test_tskip -s 101 102 103 -p 77 0      -o tsE.raw tsE.nhdr)
ADD_TEST(tskip10n ${EXECUTABLE_OUTPUT_PATH}/test_tskip -s 101 102 103 -p 77 0 -ns  -o tsF.raw tsF.nhdr)

ADD_EXECUTABLE(test_sanity sanity.c)
TARGET_LINK_LIBRARIES(test_sanity teem)
ADD_TEST(sanity ${EXECUTABLE_OUTPUT_PATH}/test_sanity)

ADD_EXECUTABLE(test_macros macros.c)
TARGET_LINK_LIBRARIES(test_macros teem)
ADD_TEST(macros ${EXECUTABLE_OUTPUT_PATH}/test_macros)

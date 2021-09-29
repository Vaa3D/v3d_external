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

ADD_EXECUTABLE(test_miscAir miscAir.c)
TARGET_LINK_LIBRARIES(test_miscAir teem)
ADD_TEST(misc ${EXECUTABLE_OUTPUT_PATH}/test_miscAir)

ADD_EXECUTABLE(test_mtrand mtrand.c)
TARGET_LINK_LIBRARIES(test_mtrand teem)
ADD_TEST(mtrand ${EXECUTABLE_OUTPUT_PATH}/test_mtrand)

ADD_EXECUTABLE(test_amath amath.c)
TARGET_LINK_LIBRARIES(test_amath teem)
ADD_TEST(amath ${EXECUTABLE_OUTPUT_PATH}/test_amath)

ADD_EXECUTABLE(test_string string.c)
TARGET_LINK_LIBRARIES(test_string teem)
ADD_TEST(string ${EXECUTABLE_OUTPUT_PATH}/test_string)

ADD_EXECUTABLE(test_strtok strtok.c)
TARGET_LINK_LIBRARIES(test_strtok teem)
ADD_TEST(strtok ${EXECUTABLE_OUTPUT_PATH}/test_strtok)

ADD_EXECUTABLE(test_pptest pptest.c)
TARGET_LINK_LIBRARIES(test_pptest teem)
ADD_TEST(pptest ${EXECUTABLE_OUTPUT_PATH}/test_pptest)

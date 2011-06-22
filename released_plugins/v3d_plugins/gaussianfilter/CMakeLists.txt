project( $$qtLibraryTarget(gaussianfilter) )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -w )
find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
	../../../v3d_main/basic_c_fun)
set ( LIBRARY_OUTPUT_PATH ../../v3d/plugins/image_filters/Gaussian_Filter )
set ( $$qtLibraryTarget(gaussianfilter)_HDRS
	
	)

set ( $$qtLibraryTarget(gaussianfilter)_SRCS
	gaussianfilterplugin.cpp
	)

set ( $$qtLibraryTarget(gaussianfilter)_MOCS
	gaussianfilterplugin.h
	)
QT4_WRAP_CPP(MOCS ${$$qtLibraryTarget(gaussianfilter)_MOCS})

add_library ( $$qtLibraryTarget(gaussianfilter) MODULE ${$$qtLibraryTarget(gaussianfilter)_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( $$qtLibraryTarget(gaussianfilter)  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

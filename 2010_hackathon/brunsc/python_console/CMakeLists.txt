cmake_minimum_required(VERSION 2.6)

# Embedded python on Linux might need some special linking to get all the python symbols
# http://docs.python.org/py3k/extending/embedding.html
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # message("Linux")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker -export-dynamic")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Xlinker -export-dynamic")
    find_library(DL_LIBRARY "dl")
endif()

# To support local test building, conditionally set certain variables
if(NOT FinishedPlugins)
    add_custom_target(FinishedPlugins)
endif()
if(NOT INSTALLATION_DIRECTORY)
    set(INSTALLATION_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
include(../../../released_plugins/CMake/PluginConfiguration.cmake)

# Default to building fat binaries, even on Leopard
if(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
         set(CMAKE_OSX_ARCHITECTURES i386 x86_64)
    endif()
endif()

# Python libraries are needed to create a python interpreter
FIND_PACKAGE( PythonLibs REQUIRED )
include_directories(${PYTHON_INCLUDE_PATH})

# Boost::python is needed to expose our C++ classes in python
SET(Boost_ADDITIONAL_VERSIONS "1.44.0" "1.44" "1.45.0" "1.45")
find_package(Boost COMPONENTS python REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# Qt is needed for GUI window.
find_package(Qt4 REQUIRED)
include( ${QT_USE_FILE} )

QT4_WRAP_UI(QT_UI_SRCS
  main_python_console.ui
)

QT4_WRAP_CPP(QT_MOC_SRCS
  PythonConsoleWindow.h
  PythonInterpreter.h
  wrappable_v3d.h
)

QT4_ADD_RESOURCES(QT_RCC_SRCS python_console.qrc)

# Qt dynamically generates some headers here
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB PYPLUSPLUS_SRCS "generated_code/*.cpp")
list(REMOVE_ITEM PYPLUSPLUS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/generated_code/v3d.main.cpp")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/generated_code")
# message(${PYPLUSPLUS_SRCS})

set(V3D_PYTHON_CONVERSION_HEADERS
    convert_qlist.h
    convert_qhash.h
    convert_c_array_struct.h
)

set(V3D_PYTHON_CONSOLE_SRCS
    PythonConsoleWindow.cpp
    PythonInterpreter.cpp
    V3DPythonModule.cpp
    ${QT_UI_SRCS}
    ${QT_MOC_SRCS}
    ${PYPLUSPLUS_SRCS}
    ${QT_RCC_SRCS}
    wrappable_v3d.cpp
    convert_qstring.cpp
    convert_qbool.cpp
)

##################################################################
# pythonConsoleTest is a test program for use during development #
##################################################################

add_executable(pythonConsoleTest
    pythonConsoleTest.cpp
    ${V3D_PYTHON_CONSOLE_SRCS}
)

target_link_libraries(pythonConsoleTest
    ${Boost_PYTHON_LIBRARY}
    ${PYTHON_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)
if (DL_LIBRARY)
    target_link_libraries(pythonConsoleTest ${DL_LIBRARY})
endif()


################################################
# pythonConsole is a V3D plugin shared library #
################################################
# ...which also doubles as a python module
# TODO - create separate libraries for python module and plugin-only code

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../v3d_main/basic_c_fun/"
    )

set(PLUGIN_SOURCES 
    PythonConsolePlugin.cpp
    ${V3D_PYTHON_CONSOLE_SRCS}
)
set(PLUGIN_HEADERS 
    PythonConsolePlugin.h
    # PythonConsoleWindow.h // already MOCed in this file above
)

set(PLUGIN_DIRECTORY_NAME python_scripting/python_console)

configure_v3d_plugin(PythonConsolePlugin)

target_link_libraries(PythonConsolePlugin
    ${Boost_PYTHON_LIBRARY}
    ${PYTHON_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)
if (DL_LIBRARY)
    target_link_libraries(pythonConsoleTest ${DL_LIBRARY})
endif()

find_path(PLUGINS_PARENT_PATH plugins
    PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../build_v3d_cmake_eclipse/v3d"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../build_cmake/v3d"
    PATH_SUFFIXES
        "Mac_Fat/v3d.app/Contents/MacOS"
        "Linux_32"
        "Linux_64"
    NO_DEFAULT_PATH
    )
    
if(PLUGINS_PARENT_PATH)
    set(PLUGINS_PATH "${PLUGINS_PARENT_PATH}/plugins")
else()
    set(PLUGINS_PATH "${CMAKE_CURRENT_BINARY_DIR}/plugins")
endif()

set(V3D_PYTHON_CONSOLE_PLUGIN_OUTPUT_DIR
    "${PLUGINS_PATH}/${PLUGIN_DIRECTORY_NAME}"
    CACHE PATH "Where to build python console plugin")

set_target_properties(PythonConsolePlugin PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${V3D_PYTHON_CONSOLE_PLUGIN_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${V3D_PYTHON_CONSOLE_PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${V3D_PYTHON_CONSOLE_PLUGIN_OUTPUT_DIR}"
)

# Set output name of plugin to v3d.so or whatever, so it can be loaded as a module in python,
# ... so we can use epydoc to generate documentation.
set_target_properties(PythonConsolePlugin PROPERTIES
    OUTPUT_NAME v3d
    PREFIX ""
)
if(WIN32)
    set_target_properties(PythonConsolePlugin PROPERTIES SUFFIX "pyd")
endif()

#####################################
# Generate bindings with pyplusplus #
#####################################

set(V3D_PYTHON_CONSOLE_GENERATE_WRAP_CODE FALSE CACHE BOOL 
    "Whether to run pyplusplus to generate boost python wrappers")
if(V3D_PYTHON_CONSOLE_GENERATE_WRAP_CODE)
    find_package(PythonInterp REQUIRED)
    file(GLOB V3D_PLUGIN_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../v3d_main/basic_c_fun/*.h")
    set(WRAP_DEPEND_SRCS
        ${V3D_PLUGIN_HEADERS}
        ${V3D_PYTHON_CONVERSION_HEADERS}
        wrappable_v3d.h
        doxygen_doc_extractor.py
        wrap_v3d_boost.py)
    # message(${WRAP_DEPEND_SRCS})
    set(V3DGEN_STAMP "${CMAKE_CURRENT_SOURCE_DIR}/generated_code/generate_v3d.stamp")
    add_custom_command(
        COMMAND "${PYTHON_EXECUTABLE}" wrap_v3d_boost.py
            > "${CMAKE_CURRENT_BINARY_DIR}/wrap_v3d_boost.log"
            2> "${CMAKE_CURRENT_BINARY_DIR}/wrap_v3d_boost.err"
        COMMAND $(CMAKE_COMMAND) -E touch "${V3DGEN_STAMP}"
        OUTPUT "${V3DGEN_STAMP}"
        DEPENDS ${WRAP_DEPEND_SRCS}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Creating V3D wrapper source files.  Examine wrap_v3d_boost.err file for details"
    )
    add_custom_target(GenerateV3DWrapperSources
        DEPENDS "${V3DGEN_STAMP}"
        SOURCES ${WRAP_DEPEND_SRCS}
    )
    
    add_dependencies(PythonConsolePlugin GenerateV3DWrapperSources)
endif()

#######################################
# Generate documentation using epydoc #
#######################################

set(V3D_PYTHON_CONSOLE_GENERATE_API_DOCS FALSE CACHE BOOL
    "Whether to (re)generate documentation for the V3D python programming API")
if(V3D_PYTHON_CONSOLE_GENERATE_API_DOCS)
    find_package(PythonInterp REQUIRED)
    find_program(EPYDOC_EXECUTABLE NAMES epydoc epydoc.py)
    if(EPYDOC_EXECUTABLE)
        find_program(DOT_EXECUTABLE dot)
        if(DOT_EXECUTABLE)
            set(EPYDOC_OPTIONS ${EPYDOC_OPTIONS} --graph all --dotpath ${DOT_EXECUTABLE})
        endif()
        set(DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doc")
        file(MAKE_DIRECTORY "${DOC_DIR}")
        configure_file(run_epydoc.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/run_epydoc.cmake @ONLY)
        add_custom_command(
            TARGET PythonConsolePlugin POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/run_epydoc.cmake"
            COMMENT "Creating V3D python API documentation using epydoc..."
        )
    endif()
endif()


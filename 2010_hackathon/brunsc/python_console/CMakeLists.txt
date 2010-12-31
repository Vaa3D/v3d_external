cmake_minimum_required(VERSION 2.6)

# To support local test building, conditionally set certain variables
if(NOT FinishedPlugins)
    add_custom_target(FinishedPlugins)
endif()
if(NOT INSTALLATION_DIRECTORY)
    set(INSTALLATION_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
include(../../../released_plugins/CMake/PluginConfiguration.cmake)

# Python libraries are needed to create a python interpreter
FIND_PACKAGE( PythonLibs REQUIRED )
include_directories(${PYTHON_INCLUDE_PATH})

# Boost::python is needed to expose our C++ classes in python
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
)

##################################################################
# pythonConsoleTest is a test program for use during development #
##################################################################

add_executable(pythonConsoleTest
    pythonConsoleTest.cpp
    ${V3D_PYTHON_CONSOLE_SRCS}
)

target_link_libraries(pythonConsoleTest
    ${PYTHON_LIBRARY}
    ${Boost_PYTHON_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)


################################################
# pythonConsole is a V3D plugin shared library #
################################################

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

configure_v3d_plugin(PythonConsolePlugin)

target_link_libraries(PythonConsolePlugin
    ${PYTHON_LIBRARY}
    ${Boost_PYTHON_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

find_path(PLUGINS_PARENT_PATH plugins
    PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../build_v3d_cmake_eclipse/v3d/Mac_Fat/v3d.app/Contents/MacOS"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../build_v3d_cmake_eclipse/v3d/Linux_32"
    NO_DEFAULT_PATH
    )
    
if(PLUGINS_PARENT_PATH)
    set(PLUGINS_PATH "${PLUGINS_PARENT_PATH}/plugins")
else()
    set(PLUGINS_PATH "${CMAKE_CURRENT_BINARY_DIR}/plugins")
endif()

set(PYTHON_CONSOLE_PLUGIN_OUTPUT_DIR
    "${PLUGINS_PATH}/python_console"
    CACHE PATH "Where to build python console plugin")

set_target_properties(PythonConsolePlugin PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${PYTHON_CONSOLE_PLUGIN_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PYTHON_CONSOLE_PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PYTHON_CONSOLE_PLUGIN_OUTPUT_DIR}"
)

#####################################
# Generate bindings with pyplusplus #
#####################################

set(PYCONSOLE_GENERATE_WRAP_CODE FALSE CACHE BOOL 
    "Whether to run pyplusplus to generate boost python wrappers")
if(PYCONSOLE_GENERATE_WRAP_CODE)
    find_package(PythonInterp REQUIRED)
    file(GLOB V3D_PLUGIN_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../v3d_main/basic_c_fun/*.h")
    set(WRAP_DEPEND_SRCS
        ${V3D_PLUGIN_HEADERS}
        convert_qstring.h
        wrappable_v3d.h
        qlist_py_indexing.h
        doxygen_doc_extractor.py
        wrap_v3d_boost.py)
    message(${WRAP_DEPEND_SRCS})
    set(V3DGEN_STAMP "${CMAKE_CURRENT_SOURCE_DIR}/generated_code/generate_v3d.stamp")
    add_custom_command(
        COMMAND "${PYTHON_EXECUTABLE}" wrap_v3d_boost.py
            > "${CMAKE_CURRENT_BINARY_DIR}/wrap_v3d_boost.log"
            2> "${CMAKE_CURRENT_BINARY_DIR}/wrap_v3d_boost.err"
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

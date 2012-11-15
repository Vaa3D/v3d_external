project(TestApp3D)
cmake_minimum_required(VERSION 2.8)

# Use Qt for user interface
file(GLOB TEST3D_UIS "*.ui")
set(QT_USE_QTOPENGL TRUE)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
QT4_WRAP_UI(TEST3D_UI_HDRS ${TEST3D_UIS})
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # where ui headers get generated
set(LINK_LIBS ${LINK_LIBS} ${QT_LIBRARIES})

# OpenGL for hard core graphics programming
find_package(OpenGL REQUIRED)
set(LINK_LIBS ${LINK_LIBS} ${OPENGL_LIBRARIES})

# GLUT for teapot actor example
find_package(GLUT REQUIRED)
set(LINK_LIBS ${LINK_LIBS} ${GLUT_glut_LIBRARY})

# Boost to make C++ programming less excruciating
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

QT4_WRAP_CPP(TEST3D_MOC_SRCS
    Camera3D.h
)

# Create application for testing 3D viewer prototype
file(GLOB TEST3D_SRCS "*.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}) # so ui headers can include local files
add_executable(TestApp3D 
    ${TEST3D_SRCS} 
    ${TEST3D_MOC_SRCS} 
    ${TEST3D_UI_HDRS}
    "../geometry/Rotation3D.cpp"
    "../render/ActorGL.cpp"
)
target_link_libraries(TestApp3D ${LINK_LIBS})

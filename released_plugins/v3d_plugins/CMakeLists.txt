# Visit plugin subdirectories if and only if they have a CMakeLists.txt file
file(GLOB CMAKE_DIRS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*/CMakeLists.txt")
# message("CMAKE_DIRS = ${CMAKE_DIRS}")
foreach(CMAKE_DIR ${CMAKE_DIRS})
    string(REPLACE "//CMakeLists.txt" "" CMAKE_DIR "${CMAKE_DIR}")
    string(REPLACE "/CMakeLists.txt" "" CMAKE_DIR "${CMAKE_DIR}")
    # message(${CMAKE_DIR})
    add_subdirectory(${CMAKE_DIR})
endforeach()

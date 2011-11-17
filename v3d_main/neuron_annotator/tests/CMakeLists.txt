# Put test binaries in the build_cmake directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_executable(test_load_chk4 test_load_chk4.cpp)
target_link_libraries(test_load_chk4 NeuronAnnotatorLib)
# add_test(test_load_chk4 test_load_chk4)


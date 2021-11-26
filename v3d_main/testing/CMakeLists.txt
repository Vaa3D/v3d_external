
add_executable(TestPositiveControl testPositiveControl.c)
add_test(TestPositiveControl ${EXECUTABLE_OUTPUT_PATH}/TestPositiveControl)

add_executable(TestNegativeControl testNegativeControl.c)
add_test(TestNegativeControl ${EXECUTABLE_OUTPUT_PATH}/TestNegativeControl)
set_tests_properties(TestNegativeControl PROPERTIES WILL_FAIL TRUE)

get_target_property(V3D_EXE_DIR v3d RUNTIME_OUTPUT_DIRECTORY)
if(NOT V3D_EXE_DIR)
    set(V3D_EXE_DIR ${EXECUTABLE_OUTPUT_PATH})
endif()
get_target_property(V3D_EXE_NAME v3d OUTPUT_NAME)
if(NOT V3D_EXE_NAME)
    set(V3D_EXE_NAME v3d)
endif()
# message("${V3D_EXE_DIR} ${V3D_EXE_NAME}")
set(V3D_EXECUTABLE "${V3D_EXE_DIR}/${V3D_EXE_NAME}")
add_test(TestV3DHelp ${V3D_EXECUTABLE} -h)
set_tests_properties(TestV3DHelp PROPERTIES PASS_REGULAR_EXPRESSION "analysis platform")

# add_test(read001 ${V3D_EXECUTABLE})


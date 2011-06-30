

set(V3D_EXECUTABLE ${PROJECT_BINARY_DIR}/bin/v3d)

add_test(read001 ${V3D_EXECUTABLE})

add_executable(TestPositiveControl testPositiveControl.c)
add_test(TestPositiveControl ${EXECUTABLE_OUTPUT_PATH}/TestPositiveControl)

add_executable(TestNegativeControl testNegativeControl.c)
add_test(TestNegativeControl ${EXECUTABLE_OUTPUT_PATH}/TestNegativeControl)
set_tests_properties(TestNegativeControl PROPERTIES WILL_FAIL TRUE)


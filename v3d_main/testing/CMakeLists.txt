

set(V3D_EXECUTABLE ${PROJECT_BINARY_DIR}/bin/v3d)

add_test(read001 ${V3D_EXECUTABLE})

add_executable(TestPositiveControl testPositiveControl.c)
add_test(TestPostiveControl ${EXECUTABLE_OUTPUT_PATH}/TestPositiveControl)


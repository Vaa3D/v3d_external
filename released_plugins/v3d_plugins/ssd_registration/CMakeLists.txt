set(PLUGIN_HEADERS  # Q_OBJECT macro containing files only
    plugin_image_registration.h
    q_paradialog_rigidaffine.h
)
set(PLUGIN_DIRECTORY_NAME image_registration/SSD_registration)
set(PLUGIN_SOURCES
    common/q_imresize.cpp
    common/q_convolve.cpp
    histogram_matching/q_histogram_matching.cpp
    q_paradialog_rigidaffine.cpp
    q_registration_common.cpp
    q_rigidaffine_registration.cpp
    plugin_image_registration.cpp
)
include_directories(
    "${TIFF_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/v3d_main/jba/newmat11"
)
configure_v3d_plugin(plugin_PQ_imagereg)
target_link_libraries(plugin_PQ_imagereg
    ${TIFF_LIBRARY}
    newmat11
)

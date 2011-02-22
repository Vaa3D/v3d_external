# Document the V3D plug-in API using doxygen tool
#
set(V3D_DOXYGEN_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
file(MAKE_DIRECTORY "${V3D_DOXYGEN_OUTPUT_PATH}")

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(V3D_UPDATE_API_DOCS FALSE CACHE BOOL "Whether to update the automated doxygen API documentation")
    if(V3D_UPDATE_API_DOCS)
        SET(DOXY_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
        if(DOXYGEN_DOT_EXECUTABLE)
            set(DOXYGEN_HAVE_DOT YES)
        else()
            set(DOXYGEN_HAVE_DOT NO)
        endif()
        CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in 
          ${DOXY_CONFIG}
          @ONLY )
        ADD_CUSTOM_TARGET(doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG}
            WORKING_DIRECTORY "${V3D_DOXYGEN_OUTPUT_PATH}"
            COMMENT "Generating API documentation with doxygen"
            SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
        ) 
    endif(V3D_UPDATE_API_DOCS)
endif(DOXYGEN_FOUND)

install(DIRECTORY "${V3D_DOXYGEN_OUTPUT_PATH}/html/"
    DESTINATION "doc/api/html"
    COMPONENT Documentation
    PATTERN ".svn" EXCLUDE
)


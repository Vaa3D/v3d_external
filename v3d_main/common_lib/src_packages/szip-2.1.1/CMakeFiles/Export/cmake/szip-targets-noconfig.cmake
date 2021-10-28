#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "szip-static" for configuration ""
set_property(TARGET szip-static APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(szip-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "C"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libszip-static.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS szip-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_szip-static "${_IMPORT_PREFIX}/lib/libszip-static.a" )

# Import target "szip-shared" for configuration ""
set_property(TARGET szip-shared APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(szip-shared PROPERTIES
  IMPORTED_IMPLIB_NOCONFIG "${_IMPORT_PREFIX}/lib/szip-shared.lib"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/szip-shared.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS szip-shared )
list(APPEND _IMPORT_CHECK_FILES_FOR_szip-shared "${_IMPORT_PREFIX}/lib/szip-shared.lib" "${_IMPORT_PREFIX}/bin/szip-shared.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

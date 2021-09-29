########################################################
#  Include file for user options
########################################################

# To use this option, copy both the macro and option code
# into the root UserMacros.cmake file.

#-----------------------------------------------------------------------------
# Option to Build with Static CRT libraries on Windows
#-------------------------------------------------------------------------------
MACRO (TARGET_STATIC_CRT_FLAGS)
  if (MSVC AND NOT BUILD_SHARED_LIBS)
    foreach (flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if (${flag_var} MATCHES "/MD")
        STRING (REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif ()
    endforeach ()
    foreach (flag_var
        CMAKE_Fortran_FLAGS CMAKE_Fortran_FLAGS_DEBUG CMAKE_Fortran_FLAGS_RELEASE
        CMAKE_Fortran_FLAGS_MINSIZEREL CMAKE_Fortran_FLAGS_RELWITHDEBINFO)
      if (${flag_var} MATCHES "/libs:dll")
        STRING (REGEX REPLACE "/libs:dll" "/libs:static" ${flag_var} "${${flag_var}}")
      endif ()
    endforeach ()
    set (WIN_COMPILE_FLAGS "")
    set (WIN_LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
  endif ()
ENDMACRO ()

#-----------------------------------------------------------------------------
option (BUILD_STATIC_CRT_LIBS "Build With Static CRT Libraries" OFF)
if (BUILD_STATIC_CRT_LIBS)
  TARGET_STATIC_CRT_FLAGS ()
endif ()

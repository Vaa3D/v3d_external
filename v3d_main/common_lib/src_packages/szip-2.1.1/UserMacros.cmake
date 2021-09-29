########################################################
#  Include file for user options
########################################################

#-----------------------------------------------------------------------------
# Option to Build with User Defined Values
#-----------------------------------------------------------------------------
macro (MACRO_USER_DEFINED_LIBS)
  set (USER_DEFINED_VALUE "FALSE")
endmacro ()

#-------------------------------------------------------------------------------
option (BUILD_USER_DEFINED_LIBS "Build With User Defined Values" OFF)
if (BUILD_USER_DEFINED_LIBS)
  MACRO_USER_DEFINED_LIBS ()
endif ()
 

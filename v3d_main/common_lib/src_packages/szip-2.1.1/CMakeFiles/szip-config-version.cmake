#-----------------------------------------------------------------------------
# SZIP Version file for install directory
#-----------------------------------------------------------------------------

set (PACKAGE_VERSION 2.1)

if ("${PACKAGE_FIND_VERSION_MAJOR}" EQUAL 2.1)

  # exact match for version 2.1.1
  if ("${PACKAGE_FIND_VERSION_MINOR}" EQUAL 1)

    # compatible with any version 2.1.1.x
    set (PACKAGE_VERSION_COMPATIBLE 1)

    if ("${PACKAGE_FIND_VERSION_PATCH}" EQUAL )
      set (PACKAGE_VERSION_EXACT 1)

      if ("${PACKAGE_FIND_VERSION_TWEAK}" EQUAL )
        # not using this yet
      endif ()

    endif ()

  endif ()
endif ()



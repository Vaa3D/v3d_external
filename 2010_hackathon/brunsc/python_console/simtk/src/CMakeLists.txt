message("simtk common")
file(GLOB SimTKcommon_SRCS "*.cpp")
add_library(SimTKcommon STATIC ${SimTKcommon_SRCS})


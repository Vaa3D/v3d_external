# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.18

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = F:\Cmake\bin\cmake.exe

# The command to remove a file.
RM = F:\Cmake\bin\cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src

# Include any dependencies generated for this target.
include Testing/air/CMakeFiles/test_miscAir.dir/depend.make

# Include the progress variables for this target.
include Testing/air/CMakeFiles/test_miscAir.dir/progress.make

# Include the compile flags for this target's objects.
include Testing/air/CMakeFiles/test_miscAir.dir/flags.make

Testing/air/CMakeFiles/test_miscAir.dir/miscAir.obj: Testing/air/CMakeFiles/test_miscAir.dir/flags.make
Testing/air/CMakeFiles/test_miscAir.dir/miscAir.obj: Testing/air/CMakeFiles/test_miscAir.dir/includes_C.rsp
Testing/air/CMakeFiles/test_miscAir.dir/miscAir.obj: Testing/air/miscAir.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object Testing/air/CMakeFiles/test_miscAir.dir/miscAir.obj"
	cd /d F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air && C:\PROGRA~1\MINGW-~1\X86_64~1.0-W\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\test_miscAir.dir\miscAir.obj -c F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air\miscAir.c

Testing/air/CMakeFiles/test_miscAir.dir/miscAir.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_miscAir.dir/miscAir.i"
	cd /d F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air && C:\PROGRA~1\MINGW-~1\X86_64~1.0-W\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air\miscAir.c > CMakeFiles\test_miscAir.dir\miscAir.i

Testing/air/CMakeFiles/test_miscAir.dir/miscAir.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_miscAir.dir/miscAir.s"
	cd /d F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air && C:\PROGRA~1\MINGW-~1\X86_64~1.0-W\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air\miscAir.c -o CMakeFiles\test_miscAir.dir\miscAir.s

# Object files for target test_miscAir
test_miscAir_OBJECTS = \
"CMakeFiles/test_miscAir.dir/miscAir.obj"

# External object files for target test_miscAir
test_miscAir_EXTERNAL_OBJECTS =

bin/test_miscAir.exe: Testing/air/CMakeFiles/test_miscAir.dir/miscAir.obj
bin/test_miscAir.exe: Testing/air/CMakeFiles/test_miscAir.dir/build.make
bin/test_miscAir.exe: bin/libteem.a
bin/test_miscAir.exe: F:/idea/Library/lib/libbz2.lib
bin/test_miscAir.exe: F:/idea/Library/lib/z.lib
bin/test_miscAir.exe: F:/idea/Library/lib/libpng.lib
bin/test_miscAir.exe: F:/idea/Library/lib/z.lib
bin/test_miscAir.exe: F:/idea/Library/lib/libpng.lib
bin/test_miscAir.exe: Testing/air/CMakeFiles/test_miscAir.dir/linklibs.rsp
bin/test_miscAir.exe: Testing/air/CMakeFiles/test_miscAir.dir/objects1.rsp
bin/test_miscAir.exe: Testing/air/CMakeFiles/test_miscAir.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ..\..\bin\test_miscAir.exe"
	cd /d F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\test_miscAir.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Testing/air/CMakeFiles/test_miscAir.dir/build: bin/test_miscAir.exe

.PHONY : Testing/air/CMakeFiles/test_miscAir.dir/build

Testing/air/CMakeFiles/test_miscAir.dir/clean:
	cd /d F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air && $(CMAKE_COMMAND) -P CMakeFiles\test_miscAir.dir\cmake_clean.cmake
.PHONY : Testing/air/CMakeFiles/test_miscAir.dir/clean

Testing/air/CMakeFiles/test_miscAir.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air F:\v3d_mingw1\v3d_external\v3d_main\common_lib\src_packages\teem-1.11.0-src\Testing\air\CMakeFiles\test_miscAir.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : Testing/air/CMakeFiles/test_miscAir.dir/depend

# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = "/cygdrive/c/Users/Maayan Amid/.CLion2019.3/system/cygwin_cmake/bin/cmake.exe"

# The command to remove a file.
RM = "/cygdrive/c/Users/Maayan Amid/.CLion2019.3/system/cygwin_cmake/bin/cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2"

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/cmake-build-debug"

# Include any dependencies generated for this target.
include CMakeFiles/OS2.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/OS2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/OS2.dir/flags.make

CMakeFiles/OS2.dir/uthreads.cpp.o: CMakeFiles/OS2.dir/flags.make
CMakeFiles/OS2.dir/uthreads.cpp.o: ../uthreads.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/OS2.dir/uthreads.cpp.o"
	/usr/bin/c++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OS2.dir/uthreads.cpp.o -c "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/uthreads.cpp"

CMakeFiles/OS2.dir/uthreads.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OS2.dir/uthreads.cpp.i"
	/usr/bin/c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/uthreads.cpp" > CMakeFiles/OS2.dir/uthreads.cpp.i

CMakeFiles/OS2.dir/uthreads.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OS2.dir/uthreads.cpp.s"
	/usr/bin/c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/uthreads.cpp" -o CMakeFiles/OS2.dir/uthreads.cpp.s

CMakeFiles/OS2.dir/main.cpp.o: CMakeFiles/OS2.dir/flags.make
CMakeFiles/OS2.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir="/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/OS2.dir/main.cpp.o"
	/usr/bin/c++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OS2.dir/main.cpp.o -c "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/main.cpp"

CMakeFiles/OS2.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OS2.dir/main.cpp.i"
	/usr/bin/c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/main.cpp" > CMakeFiles/OS2.dir/main.cpp.i

CMakeFiles/OS2.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OS2.dir/main.cpp.s"
	/usr/bin/c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/main.cpp" -o CMakeFiles/OS2.dir/main.cpp.s

# Object files for target OS2
OS2_OBJECTS = \
"CMakeFiles/OS2.dir/uthreads.cpp.o" \
"CMakeFiles/OS2.dir/main.cpp.o"

# External object files for target OS2
OS2_EXTERNAL_OBJECTS =

OS2.exe: CMakeFiles/OS2.dir/uthreads.cpp.o
OS2.exe: CMakeFiles/OS2.dir/main.cpp.o
OS2.exe: CMakeFiles/OS2.dir/build.make
OS2.exe: CMakeFiles/OS2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir="/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/cmake-build-debug/CMakeFiles" --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable OS2.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/OS2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/OS2.dir/build: OS2.exe

.PHONY : CMakeFiles/OS2.dir/build

CMakeFiles/OS2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/OS2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/OS2.dir/clean

CMakeFiles/OS2.dir/depend:
	cd "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/cmake-build-debug" && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2" "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2" "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/cmake-build-debug" "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/cmake-build-debug" "/cygdrive/c/Users/Maayan Amid/CLionProjects/OS2/cmake-build-debug/CMakeFiles/OS2.dir/DependInfo.cmake" --color=$(COLOR)
.PHONY : CMakeFiles/OS2.dir/depend


# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
CMAKE_COMMAND = /home/kamil/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/202.7319.72/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/kamil/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/202.7319.72/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kamil/CLionProjects/pwsl/szokety

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kamil/CLionProjects/pwsl/szokety/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/szokety.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/szokety.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/szokety.dir/flags.make

CMakeFiles/szokety.dir/main.c.o: CMakeFiles/szokety.dir/flags.make
CMakeFiles/szokety.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kamil/CLionProjects/pwsl/szokety/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/szokety.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/szokety.dir/main.c.o   -c /home/kamil/CLionProjects/pwsl/szokety/main.c

CMakeFiles/szokety.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/szokety.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/kamil/CLionProjects/pwsl/szokety/main.c > CMakeFiles/szokety.dir/main.c.i

CMakeFiles/szokety.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/szokety.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/kamil/CLionProjects/pwsl/szokety/main.c -o CMakeFiles/szokety.dir/main.c.s

CMakeFiles/szokety.dir/CircularBuffer.c.o: CMakeFiles/szokety.dir/flags.make
CMakeFiles/szokety.dir/CircularBuffer.c.o: ../CircularBuffer.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kamil/CLionProjects/pwsl/szokety/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/szokety.dir/CircularBuffer.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/szokety.dir/CircularBuffer.c.o   -c /home/kamil/CLionProjects/pwsl/szokety/CircularBuffer.c

CMakeFiles/szokety.dir/CircularBuffer.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/szokety.dir/CircularBuffer.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/kamil/CLionProjects/pwsl/szokety/CircularBuffer.c > CMakeFiles/szokety.dir/CircularBuffer.c.i

CMakeFiles/szokety.dir/CircularBuffer.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/szokety.dir/CircularBuffer.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/kamil/CLionProjects/pwsl/szokety/CircularBuffer.c -o CMakeFiles/szokety.dir/CircularBuffer.c.s

# Object files for target szokety
szokety_OBJECTS = \
"CMakeFiles/szokety.dir/main.c.o" \
"CMakeFiles/szokety.dir/CircularBuffer.c.o"

# External object files for target szokety
szokety_EXTERNAL_OBJECTS =

szokety: CMakeFiles/szokety.dir/main.c.o
szokety: CMakeFiles/szokety.dir/CircularBuffer.c.o
szokety: CMakeFiles/szokety.dir/build.make
szokety: CMakeFiles/szokety.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kamil/CLionProjects/pwsl/szokety/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable szokety"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/szokety.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/szokety.dir/build: szokety

.PHONY : CMakeFiles/szokety.dir/build

CMakeFiles/szokety.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/szokety.dir/cmake_clean.cmake
.PHONY : CMakeFiles/szokety.dir/clean

CMakeFiles/szokety.dir/depend:
	cd /home/kamil/CLionProjects/pwsl/szokety/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kamil/CLionProjects/pwsl/szokety /home/kamil/CLionProjects/pwsl/szokety /home/kamil/CLionProjects/pwsl/szokety/cmake-build-debug /home/kamil/CLionProjects/pwsl/szokety/cmake-build-debug /home/kamil/CLionProjects/pwsl/szokety/cmake-build-debug/CMakeFiles/szokety.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/szokety.dir/depend

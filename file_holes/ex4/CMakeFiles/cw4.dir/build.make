# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/cmake/619/bin/cmake

# The command to remove a file.
RM = /snap/cmake/619/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kamil/CLionProjects/pwsl/zajecia4/cw4

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kamil/CLionProjects/pwsl/zajecia4/cw4

# Include any dependencies generated for this target.
include CMakeFiles/cw4.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/cw4.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cw4.dir/flags.make

CMakeFiles/cw4.dir/main.c.o: CMakeFiles/cw4.dir/flags.make
CMakeFiles/cw4.dir/main.c.o: main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kamil/CLionProjects/pwsl/zajecia4/cw4/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/cw4.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/cw4.dir/main.c.o -c /home/kamil/CLionProjects/pwsl/zajecia4/cw4/main.c

CMakeFiles/cw4.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cw4.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/kamil/CLionProjects/pwsl/zajecia4/cw4/main.c > CMakeFiles/cw4.dir/main.c.i

CMakeFiles/cw4.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cw4.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/kamil/CLionProjects/pwsl/zajecia4/cw4/main.c -o CMakeFiles/cw4.dir/main.c.s

# Object files for target cw4
cw4_OBJECTS = \
"CMakeFiles/cw4.dir/main.c.o"

# External object files for target cw4
cw4_EXTERNAL_OBJECTS =

cw4: CMakeFiles/cw4.dir/main.c.o
cw4: CMakeFiles/cw4.dir/build.make
cw4: CMakeFiles/cw4.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kamil/CLionProjects/pwsl/zajecia4/cw4/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable cw4"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cw4.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/cw4.dir/build: cw4

.PHONY : CMakeFiles/cw4.dir/build

CMakeFiles/cw4.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cw4.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cw4.dir/clean

CMakeFiles/cw4.dir/depend:
	cd /home/kamil/CLionProjects/pwsl/zajecia4/cw4 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kamil/CLionProjects/pwsl/zajecia4/cw4 /home/kamil/CLionProjects/pwsl/zajecia4/cw4 /home/kamil/CLionProjects/pwsl/zajecia4/cw4 /home/kamil/CLionProjects/pwsl/zajecia4/cw4 /home/kamil/CLionProjects/pwsl/zajecia4/cw4/CMakeFiles/cw4.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cw4.dir/depend


# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /media/skywa04885/Main/Projects/C/fsmtp-server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /media/skywa04885/Main/Projects/C/fsmtp-server/build

# Include any dependencies generated for this target.
include CMakeFiles/db.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/db.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/db.dir/flags.make

CMakeFiles/db.dir/src/db/cassandra.src.cpp.o: CMakeFiles/db.dir/flags.make
CMakeFiles/db.dir/src/db/cassandra.src.cpp.o: ../src/db/cassandra.src.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/skywa04885/Main/Projects/C/fsmtp-server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/db.dir/src/db/cassandra.src.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/db.dir/src/db/cassandra.src.cpp.o -c /media/skywa04885/Main/Projects/C/fsmtp-server/src/db/cassandra.src.cpp

CMakeFiles/db.dir/src/db/cassandra.src.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/db.dir/src/db/cassandra.src.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/skywa04885/Main/Projects/C/fsmtp-server/src/db/cassandra.src.cpp > CMakeFiles/db.dir/src/db/cassandra.src.cpp.i

CMakeFiles/db.dir/src/db/cassandra.src.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/db.dir/src/db/cassandra.src.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/skywa04885/Main/Projects/C/fsmtp-server/src/db/cassandra.src.cpp -o CMakeFiles/db.dir/src/db/cassandra.src.cpp.s

# Object files for target db
db_OBJECTS = \
"CMakeFiles/db.dir/src/db/cassandra.src.cpp.o"

# External object files for target db
db_EXTERNAL_OBJECTS =

libdb.a: CMakeFiles/db.dir/src/db/cassandra.src.cpp.o
libdb.a: CMakeFiles/db.dir/build.make
libdb.a: CMakeFiles/db.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/media/skywa04885/Main/Projects/C/fsmtp-server/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libdb.a"
	$(CMAKE_COMMAND) -P CMakeFiles/db.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/db.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/db.dir/build: libdb.a

.PHONY : CMakeFiles/db.dir/build

CMakeFiles/db.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/db.dir/cmake_clean.cmake
.PHONY : CMakeFiles/db.dir/clean

CMakeFiles/db.dir/depend:
	cd /media/skywa04885/Main/Projects/C/fsmtp-server/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /media/skywa04885/Main/Projects/C/fsmtp-server /media/skywa04885/Main/Projects/C/fsmtp-server /media/skywa04885/Main/Projects/C/fsmtp-server/build /media/skywa04885/Main/Projects/C/fsmtp-server/build /media/skywa04885/Main/Projects/C/fsmtp-server/build/CMakeFiles/db.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/db.dir/depend


# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/blacknand/projects/ShardKV

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/blacknand/projects/ShardKV/build

# Include any dependencies generated for this target.
include CMakeFiles/shardkv.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/shardkv.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/shardkv.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/shardkv.dir/flags.make

CMakeFiles/shardkv.dir/src/main.cpp.o: CMakeFiles/shardkv.dir/flags.make
CMakeFiles/shardkv.dir/src/main.cpp.o: /home/blacknand/projects/ShardKV/src/main.cpp
CMakeFiles/shardkv.dir/src/main.cpp.o: CMakeFiles/shardkv.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/blacknand/projects/ShardKV/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/shardkv.dir/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/shardkv.dir/src/main.cpp.o -MF CMakeFiles/shardkv.dir/src/main.cpp.o.d -o CMakeFiles/shardkv.dir/src/main.cpp.o -c /home/blacknand/projects/ShardKV/src/main.cpp

CMakeFiles/shardkv.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/shardkv.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/blacknand/projects/ShardKV/src/main.cpp > CMakeFiles/shardkv.dir/src/main.cpp.i

CMakeFiles/shardkv.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/shardkv.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/blacknand/projects/ShardKV/src/main.cpp -o CMakeFiles/shardkv.dir/src/main.cpp.s

CMakeFiles/shardkv.dir/src/server.cpp.o: CMakeFiles/shardkv.dir/flags.make
CMakeFiles/shardkv.dir/src/server.cpp.o: /home/blacknand/projects/ShardKV/src/server.cpp
CMakeFiles/shardkv.dir/src/server.cpp.o: CMakeFiles/shardkv.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/blacknand/projects/ShardKV/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/shardkv.dir/src/server.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/shardkv.dir/src/server.cpp.o -MF CMakeFiles/shardkv.dir/src/server.cpp.o.d -o CMakeFiles/shardkv.dir/src/server.cpp.o -c /home/blacknand/projects/ShardKV/src/server.cpp

CMakeFiles/shardkv.dir/src/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/shardkv.dir/src/server.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/blacknand/projects/ShardKV/src/server.cpp > CMakeFiles/shardkv.dir/src/server.cpp.i

CMakeFiles/shardkv.dir/src/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/shardkv.dir/src/server.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/blacknand/projects/ShardKV/src/server.cpp -o CMakeFiles/shardkv.dir/src/server.cpp.s

CMakeFiles/shardkv.dir/src/kv_store.cpp.o: CMakeFiles/shardkv.dir/flags.make
CMakeFiles/shardkv.dir/src/kv_store.cpp.o: /home/blacknand/projects/ShardKV/src/kv_store.cpp
CMakeFiles/shardkv.dir/src/kv_store.cpp.o: CMakeFiles/shardkv.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/blacknand/projects/ShardKV/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/shardkv.dir/src/kv_store.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/shardkv.dir/src/kv_store.cpp.o -MF CMakeFiles/shardkv.dir/src/kv_store.cpp.o.d -o CMakeFiles/shardkv.dir/src/kv_store.cpp.o -c /home/blacknand/projects/ShardKV/src/kv_store.cpp

CMakeFiles/shardkv.dir/src/kv_store.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/shardkv.dir/src/kv_store.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/blacknand/projects/ShardKV/src/kv_store.cpp > CMakeFiles/shardkv.dir/src/kv_store.cpp.i

CMakeFiles/shardkv.dir/src/kv_store.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/shardkv.dir/src/kv_store.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/blacknand/projects/ShardKV/src/kv_store.cpp -o CMakeFiles/shardkv.dir/src/kv_store.cpp.s

CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o: CMakeFiles/shardkv.dir/flags.make
CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o: /home/blacknand/projects/ShardKV/src/consistent_hash.cpp
CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o: CMakeFiles/shardkv.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/blacknand/projects/ShardKV/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o -MF CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o.d -o CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o -c /home/blacknand/projects/ShardKV/src/consistent_hash.cpp

CMakeFiles/shardkv.dir/src/consistent_hash.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/shardkv.dir/src/consistent_hash.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/blacknand/projects/ShardKV/src/consistent_hash.cpp > CMakeFiles/shardkv.dir/src/consistent_hash.cpp.i

CMakeFiles/shardkv.dir/src/consistent_hash.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/shardkv.dir/src/consistent_hash.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/blacknand/projects/ShardKV/src/consistent_hash.cpp -o CMakeFiles/shardkv.dir/src/consistent_hash.cpp.s

CMakeFiles/shardkv.dir/src/raft.cpp.o: CMakeFiles/shardkv.dir/flags.make
CMakeFiles/shardkv.dir/src/raft.cpp.o: /home/blacknand/projects/ShardKV/src/raft.cpp
CMakeFiles/shardkv.dir/src/raft.cpp.o: CMakeFiles/shardkv.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/blacknand/projects/ShardKV/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/shardkv.dir/src/raft.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/shardkv.dir/src/raft.cpp.o -MF CMakeFiles/shardkv.dir/src/raft.cpp.o.d -o CMakeFiles/shardkv.dir/src/raft.cpp.o -c /home/blacknand/projects/ShardKV/src/raft.cpp

CMakeFiles/shardkv.dir/src/raft.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/shardkv.dir/src/raft.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/blacknand/projects/ShardKV/src/raft.cpp > CMakeFiles/shardkv.dir/src/raft.cpp.i

CMakeFiles/shardkv.dir/src/raft.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/shardkv.dir/src/raft.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/blacknand/projects/ShardKV/src/raft.cpp -o CMakeFiles/shardkv.dir/src/raft.cpp.s

# Object files for target shardkv
shardkv_OBJECTS = \
"CMakeFiles/shardkv.dir/src/main.cpp.o" \
"CMakeFiles/shardkv.dir/src/server.cpp.o" \
"CMakeFiles/shardkv.dir/src/kv_store.cpp.o" \
"CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o" \
"CMakeFiles/shardkv.dir/src/raft.cpp.o"

# External object files for target shardkv
shardkv_EXTERNAL_OBJECTS =

shardkv: CMakeFiles/shardkv.dir/src/main.cpp.o
shardkv: CMakeFiles/shardkv.dir/src/server.cpp.o
shardkv: CMakeFiles/shardkv.dir/src/kv_store.cpp.o
shardkv: CMakeFiles/shardkv.dir/src/consistent_hash.cpp.o
shardkv: CMakeFiles/shardkv.dir/src/raft.cpp.o
shardkv: CMakeFiles/shardkv.dir/build.make
shardkv: CMakeFiles/shardkv.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/blacknand/projects/ShardKV/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable shardkv"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/shardkv.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/shardkv.dir/build: shardkv
.PHONY : CMakeFiles/shardkv.dir/build

CMakeFiles/shardkv.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/shardkv.dir/cmake_clean.cmake
.PHONY : CMakeFiles/shardkv.dir/clean

CMakeFiles/shardkv.dir/depend:
	cd /home/blacknand/projects/ShardKV/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/blacknand/projects/ShardKV /home/blacknand/projects/ShardKV /home/blacknand/projects/ShardKV/build /home/blacknand/projects/ShardKV/build /home/blacknand/projects/ShardKV/build/CMakeFiles/shardkv.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/shardkv.dir/depend


# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /home/zbw/SimpleWebServer

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zbw/SimpleWebServer/build

# Include any dependencies generated for this target.
include CMakeFiles/server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/server.dir/flags.make

CMakeFiles/server.dir/src/impl/http.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/src/impl/http.cpp.o: ../src/impl/http.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zbw/SimpleWebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/server.dir/src/impl/http.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/src/impl/http.cpp.o -c /home/zbw/SimpleWebServer/src/impl/http.cpp

CMakeFiles/server.dir/src/impl/http.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/src/impl/http.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zbw/SimpleWebServer/src/impl/http.cpp > CMakeFiles/server.dir/src/impl/http.cpp.i

CMakeFiles/server.dir/src/impl/http.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/src/impl/http.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zbw/SimpleWebServer/src/impl/http.cpp -o CMakeFiles/server.dir/src/impl/http.cpp.s

CMakeFiles/server.dir/src/impl/logger.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/src/impl/logger.cpp.o: ../src/impl/logger.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zbw/SimpleWebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/server.dir/src/impl/logger.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/src/impl/logger.cpp.o -c /home/zbw/SimpleWebServer/src/impl/logger.cpp

CMakeFiles/server.dir/src/impl/logger.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/src/impl/logger.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zbw/SimpleWebServer/src/impl/logger.cpp > CMakeFiles/server.dir/src/impl/logger.cpp.i

CMakeFiles/server.dir/src/impl/logger.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/src/impl/logger.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zbw/SimpleWebServer/src/impl/logger.cpp -o CMakeFiles/server.dir/src/impl/logger.cpp.s

CMakeFiles/server.dir/src/impl/router.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/src/impl/router.cpp.o: ../src/impl/router.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zbw/SimpleWebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/server.dir/src/impl/router.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/src/impl/router.cpp.o -c /home/zbw/SimpleWebServer/src/impl/router.cpp

CMakeFiles/server.dir/src/impl/router.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/src/impl/router.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zbw/SimpleWebServer/src/impl/router.cpp > CMakeFiles/server.dir/src/impl/router.cpp.i

CMakeFiles/server.dir/src/impl/router.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/src/impl/router.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zbw/SimpleWebServer/src/impl/router.cpp -o CMakeFiles/server.dir/src/impl/router.cpp.s

CMakeFiles/server.dir/src/impl/server.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/src/impl/server.cpp.o: ../src/impl/server.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zbw/SimpleWebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/server.dir/src/impl/server.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/src/impl/server.cpp.o -c /home/zbw/SimpleWebServer/src/impl/server.cpp

CMakeFiles/server.dir/src/impl/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/src/impl/server.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zbw/SimpleWebServer/src/impl/server.cpp > CMakeFiles/server.dir/src/impl/server.cpp.i

CMakeFiles/server.dir/src/impl/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/src/impl/server.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zbw/SimpleWebServer/src/impl/server.cpp -o CMakeFiles/server.dir/src/impl/server.cpp.s

CMakeFiles/server.dir/src/impl/socket.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/src/impl/socket.cpp.o: ../src/impl/socket.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zbw/SimpleWebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/server.dir/src/impl/socket.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/src/impl/socket.cpp.o -c /home/zbw/SimpleWebServer/src/impl/socket.cpp

CMakeFiles/server.dir/src/impl/socket.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/src/impl/socket.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zbw/SimpleWebServer/src/impl/socket.cpp > CMakeFiles/server.dir/src/impl/socket.cpp.i

CMakeFiles/server.dir/src/impl/socket.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/src/impl/socket.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zbw/SimpleWebServer/src/impl/socket.cpp -o CMakeFiles/server.dir/src/impl/socket.cpp.s

CMakeFiles/server.dir/src/impl/threadpool.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/src/impl/threadpool.cpp.o: ../src/impl/threadpool.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zbw/SimpleWebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/server.dir/src/impl/threadpool.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/src/impl/threadpool.cpp.o -c /home/zbw/SimpleWebServer/src/impl/threadpool.cpp

CMakeFiles/server.dir/src/impl/threadpool.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/src/impl/threadpool.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zbw/SimpleWebServer/src/impl/threadpool.cpp > CMakeFiles/server.dir/src/impl/threadpool.cpp.i

CMakeFiles/server.dir/src/impl/threadpool.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/src/impl/threadpool.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zbw/SimpleWebServer/src/impl/threadpool.cpp -o CMakeFiles/server.dir/src/impl/threadpool.cpp.s

CMakeFiles/server.dir/src/main.cpp.o: CMakeFiles/server.dir/flags.make
CMakeFiles/server.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zbw/SimpleWebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/server.dir/src/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/server.dir/src/main.cpp.o -c /home/zbw/SimpleWebServer/src/main.cpp

CMakeFiles/server.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/server.dir/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/zbw/SimpleWebServer/src/main.cpp > CMakeFiles/server.dir/src/main.cpp.i

CMakeFiles/server.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/server.dir/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/zbw/SimpleWebServer/src/main.cpp -o CMakeFiles/server.dir/src/main.cpp.s

# Object files for target server
server_OBJECTS = \
"CMakeFiles/server.dir/src/impl/http.cpp.o" \
"CMakeFiles/server.dir/src/impl/logger.cpp.o" \
"CMakeFiles/server.dir/src/impl/router.cpp.o" \
"CMakeFiles/server.dir/src/impl/server.cpp.o" \
"CMakeFiles/server.dir/src/impl/socket.cpp.o" \
"CMakeFiles/server.dir/src/impl/threadpool.cpp.o" \
"CMakeFiles/server.dir/src/main.cpp.o"

# External object files for target server
server_EXTERNAL_OBJECTS =

server: CMakeFiles/server.dir/src/impl/http.cpp.o
server: CMakeFiles/server.dir/src/impl/logger.cpp.o
server: CMakeFiles/server.dir/src/impl/router.cpp.o
server: CMakeFiles/server.dir/src/impl/server.cpp.o
server: CMakeFiles/server.dir/src/impl/socket.cpp.o
server: CMakeFiles/server.dir/src/impl/threadpool.cpp.o
server: CMakeFiles/server.dir/src/main.cpp.o
server: CMakeFiles/server.dir/build.make
server: CMakeFiles/server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zbw/SimpleWebServer/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX executable server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/server.dir/build: server

.PHONY : CMakeFiles/server.dir/build

CMakeFiles/server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/server.dir/clean

CMakeFiles/server.dir/depend:
	cd /home/zbw/SimpleWebServer/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zbw/SimpleWebServer /home/zbw/SimpleWebServer /home/zbw/SimpleWebServer/build /home/zbw/SimpleWebServer/build /home/zbw/SimpleWebServer/build/CMakeFiles/server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/server.dir/depend


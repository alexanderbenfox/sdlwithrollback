#OBJS specifies which files to compile as part of the project
OBJS = src/*.cpp src/Components/*.cpp src/AssetManagement/*.cpp

#CC specifies which compiler we're using
CC = g++-9 -std=c++17

#INCLUDE_PATHS specifies the additional include paths we'll need
INCLUDE_PATHS = -Iinclude -I/usr/local/include

#LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -L/usr/local/lib

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -g adds debugging symbols
COMPILER_FLAGS = -w -g

#FRAMEWORK_PATHS
FRAMEWORK_PATHS = -Fbuild/external/sdl2_mac_libs -Wl,-rpath,$(PWD)/build/external/sdl2_mac_libs

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -framework SDL2 -framework SDL2_image -framework SDL2_ttf

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = game

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(FRAMEWORK_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
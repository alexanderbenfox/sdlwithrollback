# Compiler
CC = clang++ -std=c++17

# Collect all source files recursively
SRC_FILES = $(shell find src -name '*.cpp')
IMGUI_FILES = imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp \
              imgui/impl/imgui_impl_sdl.cpp imgui/impl/imgui_impl_opengl2.cpp
OBJS = $(SRC_FILES) $(IMGUI_FILES)

# Include paths: project source root, GGPO headers, and dependencies via pkg-config
INCLUDE_PATHS = -Isrc \
                -Ibuild/external/ggpo/include \
                $(shell pkg-config --cflags sdl2 sdl2_image SDL2_ttf jsoncpp)

# Library paths and linker flags via pkg-config
LIBRARY_FLAGS = $(shell pkg-config --libs sdl2 sdl2_image SDL2_ttf jsoncpp)

# Compiler flags
# -g adds debugging symbols
# -DGL_SILENCE_DEPRECATION suppresses macOS OpenGL deprecation warnings
COMPILER_FLAGS = -g -DGL_SILENCE_DEPRECATION

# macOS frameworks for OpenGL
FRAMEWORK_FLAGS = -framework OpenGL

# Output binary name
OBJ_NAME = game

# Build target
all: $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(COMPILER_FLAGS) $(LIBRARY_FLAGS) $(FRAMEWORK_FLAGS) -o $(OBJ_NAME)

clean:
	rm -f $(OBJ_NAME)

CXX = g++
CXXFLAGS = -std=c++17 -O2 -I. -Ibackends

SRC = $(wildcard *.cpp)
SRC += $(wildcard backends/*.cpp)
SHADERS = backends/vulkan/glsl_shader.frag.u32 backends/vulkan/glsl_shader.vert.u32

OBJ = $(SRC:.cpp=.o)

all: libimgui.a $(SHADERS)

libimgui.a: $(OBJ)
	ar rcs $@ $^

backends/vulkan/%.u32: backends/vulkan/%
	glslangValidator -V -x -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) libimgui.a $(SHADERS)

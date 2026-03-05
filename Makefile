CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
INCFLAGS = -Iinclude
LDFLAGS = -lvulkan -lglfw -Limgui -limgui

SHADERS = $(patsubst shaders/%.vert, build/shaders/%.vert.spv, $(wildcard shaders/*.vert))
SHADERS += $(patsubst shaders/%.frag, build/shaders/%.frag.spv, $(wildcard shaders/*.frag))
SOURCES = $(wildcard src/*)
OBJECTS = $(patsubst src/%.cpp, build/%.o, $(SOURCES))
HEADERS = $(wildcard include/*)

all: app $(SHADERS)

app: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(INCFLAGS)

build/%.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(INCFLAGS)

build/shaders/%.spv: shaders/%
	glslc $< -o $@

clean:
	rm -f app $(SHADERS) $(OBJECTS)

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g
INCFLAGS = -Iinclude -Iimgui
LDFLAGS = -lvulkan -lglfw
IMGUI_LIB = imgui/libimgui.a

SHADERS = $(patsubst shaders/%.vert, build/shaders/%.vert.spv, $(wildcard shaders/*.vert))
SHADERS += $(patsubst shaders/%.frag, build/shaders/%.frag.spv, $(wildcard shaders/*.frag))
SOURCES = $(wildcard src/*)
OBJECTS = $(patsubst src/%.cpp, build/%.o, $(SOURCES))
HEADERS = $(wildcard include/*)

JOBS := $(shell nproc)
ifeq ($(MAKEFLAGS),)
MAKEFLAGS += -j$(JOBS)
endif

all: app $(SHADERS)

app: $(OBJECTS) $(IMGUI_LIB)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(INCFLAGS)

build/%.o: src/%.cpp $(HEADERS)
	$(CXX) -c $(CXXFLAGS) $< -o $@ $(INCFLAGS)

build/shaders/%.spv: shaders/%
	glslc $< -o $@

$(IMGUI_LIB):
	$(MAKE) -C imgui $(MAKEFLAGS)

clean:
	rm -f app $(SHADERS) $(OBJECTS)
	$(MAKE) -C imgui clean

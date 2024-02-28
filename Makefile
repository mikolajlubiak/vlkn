CXXFLAGS := -Wall -Wextra -pedantic
LDFLAGS = -lglfw -lvulkan

DEBUGFLAGS := --debug
RELEASEFLAGS := -O3 -march=native -mtune=native -DNDEBUG

all: shaders/*.spv debug

debug: *.cpp *.hpp
	clang++ $(CXXFLAGS) $(DEBUGFLAGS) $(LDFLAGS) -o debug *.cpp

release: *.cpp *.hpp
	clang++ $(CXXFLAGS) $(RELEASEFLAGS) $(LDFLAGS) -o release *.cpp

shaders/*.spv: shaders/shader.*
	glslc -O shaders/shader.vert -o shaders/vert.spv
	glslc -O shaders/shader.frag -o shaders/frag.spv

clean:
	rm -f release debug shaders/*.spv


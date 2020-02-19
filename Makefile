# Compilation Options
# CPP =  ~/Desktop/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-g++
CPP = clang++
CPPFLAGS = -Iinclude -std=c++17 -g -lcairo
SRCS = Mesh.cpp Slicer.cpp

# Compilation Option Processing
OBJS = $(SRCS:%.cpp=obj/%.o)
$(shell mkdir -p obj)
$(shell mkdir -p build)

all: build/slicer build/test

build/test: $(wildcard test/*.cpp) $(OBJS)
	$(CPP) $(CPPFLAGS) $^ -o build/test

build/slicer: $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@ $^ src/main.cpp 

obj/%.o: src/%.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -r obj
	rm -r build
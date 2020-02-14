# Compilation Options
# CPP =  ~/Desktop/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-g++
CPP = clang++
CPPFLAGS = -Iinclude -std=c++17
SRCS = main.cpp Mesh.cpp Slicer.cpp

# Compilation Option Processing
OBJS = $(SRCS:%.cpp=obj/%.o)

all: bin/slicer

bin/slicer: $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@ $^

obj/%.o: src/%.cpp
	$(CPP) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f obj/*
	rm -f bin/*
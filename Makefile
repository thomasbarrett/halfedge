# Compilation Options
CPP = clang++
CPPFLAGS = -Iinclude -std=c++17 -g
SRCS = main.cpp Mesh.cpp

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
.phony: clean

all: bin/main
debug: bin/main-debug

FILES = $(wildcard src/**/*.cpp src/main.cpp)

bin/main: $(FILES)
	@echo "Compiling..."
	@g++ $^ -O3 --std=c++20 -o $@

# FILES = $(wildcard src/**/*.cpp)


# OBJECTS = $(widlcard src/Utils/*.cpp) $(wildcard src/DMX/*.cpp) $(wildcard src/Effects/*.cpp)
# OBJECTS = src/Utils/Utils.o \
# 			src/DMX/DMX.o \ 
# 			src/Effects/Effects.o
# OBJS_REL := $(OBJECTS:%.o=%_rel.o)
# %_rel.o: %.cpp
# 	g++ -c $< -O3 --std=c++17 -o $@
	



# bin/main: $(OBJS_REL) | bin
# 	@g++ $^ src/main.cpp -O3 --std=c++17 -o $@

run: bin/main
	@./$^

bin:
	@echo "Making bin directory..."
	@mkdir -p $@

clean:
	@echo "Cleaning..."
	@rm -f bin/*

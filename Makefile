.phony: clean

all: bin/main
debug: bin/main-debug

# FILES = $(wildcard src/**/*.cpp src/main.cpp)

bin/Utils.o: src/Utils/Utils.cpp | bin
	@echo "Compiling Utils..."
	@g++ $< -c -O3 --std=c++20 -o $@

bin/Light.o: src/DMX/Light.cpp | bin
	@echo "Compiling DMX..."
	@g++ -c $< -O3 --std=c++20 -o $@

bin/Universe.o: src/DMX/Universe.cpp | bin
	@echo "Compiling DMX Universe..."
	@g++ -c $< -O3 --std=c++20 -o $@

bin/Effects.o: src/Effects/LightGroup.cpp | bin
	@echo "Compiling Effects..."
	@g++ $< -c -O3 --std=c++20 -o $@

bin/Engine.o : src/Engine/Engine.cpp | bin
	@echo "Compiling Engine..."
	@g++ $< -c -O3 --std=c++20 -o $@

bin/libDMX.a: bin/Utils.o bin/Light.o bin/Universe.o bin/Effects.o bin/Engine.o | bin
	@echo "Creating DMX Library..."
	@ar rcs $@ $^


bin/main: bin/libDMX.a src/main.cpp | bin
	@echo "Compiling..."
	@g++  -O3 -I./src/ -L./bin/ -lDMX --std=c++20 src/main.cpp -o $@

# FILES = $(wildcard src/**/*.cpp)


# OBJECTS = $(widlcard src/Utils/*.cpp) $(wildcard src/DMX/*.cpp) $(wildcard src/Effects/*.cpp)
# OBJECTS = src/Utils/Utils.o \
# 			src/DMX/DMX.o \ 
# 			src/Effects/Effects.o
# OBJS_REL := $(OBJECTS:%.o=%_rel.o)
# %_rel.o: %.cpp
# 	g++ -c $< -O3 --std=c++17 -o $@
	
# ENGINE_FILES = $(wildcard src/**/*.cpp)
# bin/Engine.o: $(ENGINE_FILES) | bin
# 	@echo "Compiling Engine..."
# 	@g++ $^ -c -O3 --std=c++17 -o $@

# bin/main: src/main.cpp bin/Engine.o | bin
# 	@g++ $^ src/main.cpp -O3 --std=c++17 -o $@

run: bin/main
	@./$^

bin:
	@echo "Making bin directory..."
	@mkdir -p $@

clean:
	@echo "Cleaning..."
	@rm -f bin/*

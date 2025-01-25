.phony: clean

all: bin/main
debug: bin/main-debug

bin/Utils.o: src/Utils/Utils.cpp | bin
	g++ $< -c -O3 --std=c++20 -o $@

bin/Light.o: src/DMX/Light.cpp | bin
	g++ $< -c -O3 --std=c++20 -o $@

bin/Universe.o: src/DMX/Universe.cpp | bin
	g++ $< -c -O3 --std=c++20 -o $@

bin/Effects.o: src/Effects/LightGroup.cpp | bin
	g++ $< -c -O3 --std=c++20 -o $@

bin/Engine.o : src/Engine/Engine.cpp | bin
	g++ $< -c -O3 --std=c++20 -o $@

bin/libDMX.a: bin/Utils.o bin/Light.o bin/Universe.o bin/Effects.o bin/Engine.o | bin
	@echo "Building DMX Library to bin/libDMX.a"
	@ar rcs $@ $^


# OBJECTS = src/Utils/Utils.o src/DMX/Light.o src/DMX/Universe.o src/Effects/LightGroup.o src/Engine/Engine.o
# OBJS_REL := $(OBJECTS:%.o=%_rel.o)
# %_rel.o: %.cpp | bin
# 	g++ -c $< -O3 --std=c++17 -o $@

# bin/libDMX.a: $(OBJS_REL)
# 	@echo "Creating DMX Library..."
# 	@ar rcs $@ $^
	
lib: bin/libDMX.a
	@echo "Preparing library include files..."
	@rm -rf include/
	@cp -r src/ include/
	@rm -rf include/**.cpp

bin/main: src/main.cpp bin/libDMX.a | bin
	@echo "Compiling..."
	g++ $< -o $@ -O3 -L./bin/ -lDMX --std=c++20

run: bin/main
	@./$^

bin:
	@echo "Making bin directory..."
	@mkdir -p $@

clean:
	@echo "Cleaning..."
	@rm -f bin/*
	@rm -rf include/


SOURCE_FOLDER = src
SOURCE_FILES = $(wildcard $(SOURCE_FOLDER)/*.cpp)

TARGET_FOLDER = bin
TARGET_NAME = main
TARGET = $(TARGET_FOLDER)/$(TARGET_NAME)

.phony: clean

all: $(TARGET)

$(TARGET): $(SOURCE_FILES)
	@g++ $^ --std=c++17 -o $@

run: $(TARGET)
	@./$^

clean:
	rm -f $(TARGET)
	
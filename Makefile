TARGET = tabletennisscoreboard

SRC_PATH = src
BUILD_PATH = build
BIN_PATH = bin

SOURCES = $(shell find $(SRC_PATH) -name '*.cpp' | sort -k 1nr | cut -f2-)
OBJECTS = $(SOURCES:$(SRC_PATH)/%.cpp=$(BUILD_PATH)/%.o)

CC = g++
CFLAGS = -g -Wall -Wextra -std=c++11
CFLAGSGTK = $(shell pkg-config gtk+-3.0 --cflags)
LDFLAGS = $(shell pkg-config gtk+-3.0 --libs)

ifneq ($(filter arm%,$(shell uname -m)),)
LDFLAGS += -lpigpio
endif

default: build

build: dirs $(BIN_PATH)/$(TARGET)
	@echo Build Complete.

dirs:
	@mkdir -p $(BIN_PATH)
	@mkdir -p $(BUILD_PATH)

clean:
	@echo Cleaning...
	@rm -rf $(BIN_PATH)
	@rm -rf $(BUILD_PATH)

$(BIN_PATH)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_PATH)/main.o: src/main.cpp
	$(CC) $(CFLAGS) $(CFLAGSGTK) -c $< -o $@

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

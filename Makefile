TARGET := tabletennisscoreboard
GIT_VERSION := $(shell git describe --always --tags --dirty | tr a-z A-Z)

SRC_PATH := src
BUILD_PATH := build
BIN_PATH := bin

SOURCES := $(shell find $(SRC_PATH) -name '*.cpp' | sort -k 1nr | cut -f2-)
OBJECTS := $(SOURCES:$(SRC_PATH)/%.cpp=$(BUILD_PATH)/%.o)

CC = g++

CFLAGS = -Wall -Wextra -std=c++11
ifeq ($(BUILD),debug)
CFLAGS += -O0 -g
GIT_VERSION := $(GIT_VERSION)-DEBUG
else
CFLAGS += -O2 -s -DNDEBUG
endif
CFLAGS +=-DGIT_VERSION=\"$(GIT_VERSION)\"

CFLAGSGTK := $(shell pkg-config gtkmm-3.0 --cflags)

LDFLAGS = $(shell pkg-config gtkmm-3.0 --libs)
ifneq ($(filter arm%,$(shell uname -m)),)
LDFLAGS += -lpigpio
endif

DEPS := $(OBJECTS:.o=.d)

default: build

build: dirs $(BIN_PATH)/$(TARGET)
	@echo Build Complete.

debug:
	make "BUILD=debug"
	$(BIN_PATH)/$(TARGET)

dirs:
	@mkdir -p $(BIN_PATH)
	@mkdir -p $(BUILD_PATH)

clean:
	@echo Cleaning...
	@rm -rf $(BIN_PATH)
	@rm -rf $(BUILD_PATH)

install:
	@mkdir -p "${HOME}/.local/share/applications"
	@cp -f "tabletennisscoreboard.desktop" "${HOME}/.local/share/applications/tabletennisscoreboard.desktop"
	@sed -i "s~%PATH%~$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/bin~g" "${HOME}/.local/share/applications/tabletennisscoreboard.desktop"
	@echo Desktop icon installed.

uninstall:
	@rm "${HOME}/.local/share/applications/tabletennisscoreboard.desktop"
	@echo Desktop file removed.

$(BIN_PATH)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_PATH)/game.o: src/game.cpp src/game.h
	$(CC) -c $(CFLAGS) -MP -MMD -o $@ $<

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CC) -c $(CFLAGS) $(CFLAGSGTK) -MP -MMD -o $@ $<

-include $(DEPS)

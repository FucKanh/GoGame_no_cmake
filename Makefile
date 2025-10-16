# Simple Makefile for GoGame (no CMake).
CXX      ?= g++
CXXFLAGS ?= -std=gnu++17 -O2 -Iinclude -pthread
PKG      := sfml-graphics sfml-window sfml-system sfml-audio
CXXFLAGS += $(shell pkg-config --cflags $(PKG))
LDFLAGS  := $(shell pkg-config --libs $(PKG)) -pthread

SOURCES := \
    src/main.cpp \
    src/Game.cpp \
    src/Board.cpp \
    src/AI.cpp \
    src/UI.cpp \
    src/Audio.cpp \
    src/Serializer.cpp

OBJECTS := $(SOURCES:.cpp=.o)
TARGET  := GoGame

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(OBJECTS) $(TARGET)

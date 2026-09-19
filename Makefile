APP        := build/handmadehero
SRC        := src/sdl_handmade.cpp
DEPS       := $(wildcard src/*.cpp src/*.h)
CXX        := g++ -std=c++17
CXXFLAGS   := -g -O0 -Wall -Wno-unused-function -Wno-unused-variable
SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS   := $(shell pkg-config --libs sdl2)

.PHONY: all run clean compdb

all: $(APP)

$(APP): $(DEPS) Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(SRC) -o $@ $(SDL_LIBS)

run: $(APP)
	./$(APP) $(ARGS)

clean:
	rm -rf build compile_commands.json

compdb:
	@printf '[{"directory":"%s","file":"%s","command":"%s"}]\n' \
		"$(CURDIR)" "$(SRC)" "$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $(SRC)" > compile_commands.json

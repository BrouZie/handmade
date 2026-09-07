APP = build/handmadehero
BDIR = build
SRC = src/sdl_handmade.cpp
DEPS = $(wildcard src/*.cpp src/*.h)
TOOLCHAIN = g++
CXXFLAGS = -g -O0 -Wall -Wno-unused-function -Wno-unused-variable
SDL_CFLAGS := $(shell pkg-config --cflags sdl2)
SDL_LIBS := $(shell pkg-config --libs sdl2)
ARGS ?=

.PHONY: compile run clean debug

compile: $(APP)

$(APP): $(DEPS) | $(BDIR)
	$(TOOLCHAIN) $(CXXFLAGS) $(SDL_CFLAGS) $(SRC) -o $(APP) $(SDL_LIBS)

$(BDIR):
	mkdir -p $(BDIR)

run: build
	./$(APP)

debug: $(APP)
	gf2 -ex "break main" --args ./$(APP) $(ARGS)

clean:
	rm -rf $(BDIR)

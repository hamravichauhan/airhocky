CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(OS), Windows_NT)
    # --- Windows (MinGW / MSYS / Git Bash) ---
    BIN     = airhockey.exe
    LIBS    = -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32
else ifeq ($(UNAME_S), Darwin)
    # --- macOS ---
    BIN     = airhockey
    ifeq ($(UNAME_M), arm64)
        CFLAGS += -I/opt/homebrew/include
        LIBS    = -L/opt/homebrew/lib -lraylib \
                  -framework OpenGL -framework Cocoa -framework IOKit
    else
        CFLAGS += -I/usr/local/include
        LIBS    = -L/usr/local/lib -lraylib \
                  -framework OpenGL -framework Cocoa -framework IOKit
    endif
else
    # --- Linux (Fedora / Ubuntu / Debian) ---
    BIN     = airhockey
    LIBS    = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

SRC = src/main.c src/game.c src/render.c src/input.c src/net.c src/solomode.c \
      src/audio.c src/particles.c src/powerup.c src/stats.c \
      src/heatmap.c src/themes.c src/trajectory.c \
      src/dynaudio.c \
      src/tournament.c src/replay.c \
      src/ui.c src/ui_transition.c

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LIBS)

run: $(BIN)
	./$(BIN)

clean:
	rm -f airhockey airhockey.exe

.PHONY: all run clean
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S), Darwin)
    # --- macOS ---
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
    # --- Linux ---
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

SRC = src/main.c src/game.c src/render.c src/input.c src/net.c
BIN = airhockey

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LIBS)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all run clean
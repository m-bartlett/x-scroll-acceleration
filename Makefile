# Compile each .c file into its own synonymous executable
BIN=./bin
CC = gcc
LIBS = xtst x11 xi
INCLUDES =
LDFLAGS = $(shell pkg-config --libs $(LIBS)) -lm
CFLAGS = $(shell pkg-config --cflags $(LIBS))
TARGET = $(patsubst %.c, %, $(addprefix $(BIN)/, $(wildcard *.c)))

all: $(TARGET)

$(BIN)/%: %.c
	mkdir -p $(BIN)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

.PHONY:
	all

clean:
	rm -rf $(BIN)
# Compile each .c file into its own synonymous executable
BIN=bin
CC = gcc
LIBS := xtst x11 xi
LINK_LIBS := m	# math lib not in pkgconfig

LDFLAGS := $(shell pkg-config --libs $(LIBS)) $(addprefix -l, $(LINK_LIBS))
CFLAGS := $(shell pkg-config --cflags $(LIBS))

SOURCES = $(wildcard *.c)
TARGETS = $(SOURCES:.c=)

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin

INSTALL_TARGETS := $(addprefix $(BINPREFIX)/,$(TARGETS))


all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

install: $(TARGETS)
	install -m 755 -D --target-directory "$(BINPREFIX)" $<

uninstall:
	rm $(INSTALL_TARGETS)

.PHONY:
	all

clean:
	rm $(TARGETS)
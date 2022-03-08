TARGET   := $(notdir $(patsubst %/,%,$(CURDIR)))
MAKEFILE := $(firstword $(MAKEFILE_LIST))

CC = gcc
LIBS := xtst x11 xi
LINK_LIBS := m	# math lib not in pkgconfig

LDFLAGS := $(shell pkg-config --libs $(LIBS)) $(addprefix -l, $(LINK_LIBS))
CFLAGS := $(shell pkg-config --cflags $(LIBS))

SOURCES = $(wildcard *.c)

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin

INSTALL_TARGET := $(addprefix $(BINPREFIX)/,$(TARGET))

all: $(TARGET)

debug: CFLAGS += -D DEBUG
debug: clean $(TARGET)


$(TARGET): $(SOURCES) $(MAKEFILE)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $@

install: $(TARGET)
	install -m 755 -D --target-directory "$(BINPREFIX)" $<

uninstall:
	rm $(INSTALL_TARGET)

clean:
	rm $(TARGET)

.PHONY: debug clean all
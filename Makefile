# Makefile for atg, a tiny gui for SDL
PREFIX := /usr/local
CC := gcc
CFLAGS := -Wall -Wextra -Werror -pedantic --std=gnu99 -g -DPREFIX=\"$(PREFIX)\"
SDL := `sdl-config --libs` -lSDL_ttf
SDLFLAGS := `sdl-config --cflags`

all: atg.o test widget

test: test.c atg.h atg.o
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) test.c $(LDFLAGS) -o test atg.o $(SDL)

widget: widget.c atg.h atg.o atg_internals.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) widget.c $(LDFLAGS) -o widget atg.o $(SDL)

atg.o: atg.c atg.h atg_internals.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) -o $@ -c $<

%.o: %.c %.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<


# Makefile for atg, a tiny gui for SDL
PREFIX := /usr/local
CC := gcc
CFLAGS := -Wall -Wextra -Werror -pedantic --std=gnu99 -g -DPREFIX=\"$(PREFIX)\"
SDL := `sdl-config --libs` -lSDL_ttf
SDLFLAGS := `sdl-config --cflags`
OBJS := atg.o
LOBJS := $(OBJS:.o=.lo)
INCLUDES := $(OBJS:.o=.h)
LVERSION := 0:0:0 # rules: http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info

all: libatg.la test widget

install: libatg.la
	libtool --mode=install install -D -m0755 libatg.la $(PREFIX)/lib/libatg.la

uninstall:
	libtool --mode=uninstall /bin/rm -f $(PREFIX)/lib/libatg.la

clean:
	rm -f libatg.la $(LOBJS) test widget

test: test.c $(INCLUDES) libatg.la
	libtool --mode=link $(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) test.c $(LDFLAGS) -o test -latg $(SDL)

widget: widget.c $(INCLUDES) libatg.la atg_internals.h
	libtool --mode=link $(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) widget.c $(LDFLAGS) -o widget -latg $(SDL)

libatg.la: $(LOBJS)
	libtool --mode=link $(CC) -o $@ $(LOBJS) -rpath $(PREFIX)/lib -version-info $(LVERSION)

atg.lo: atg.c $(INCLUDES) atg_internals.h
	libtool --mode=compile $(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) -c $<

%.lo: %.c %.h
	libtool --mode=compile $(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<


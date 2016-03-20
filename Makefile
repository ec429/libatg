# Makefile for atg, a tiny gui for SDL
PREFIX ?= /usr/local
FONTSPATH ?= /usr/share/fonts
MONOFONTPATH != find ${FONTSPATH} -name LiberationMono-Regular.ttf -print -quit
CC ?= gcc
CFLAGS += -Wall -Wextra -Werror -pedantic --std=gnu99 -g -DMONOFONTPATH=\"$(MONOFONTPATH)\"
SDL := `sdl-config --libs` -lSDL_ttf
SDLFLAGS := `sdl-config --cflags`
OBJS := atg.o plumbing.o w_box.o w_label.o w_image.o w_button.o w_spinner.o w_toggle.o w_filepicker.o
LOBJS := $(OBJS:.o=.lo)
INCLUDES := atg.h atg_internals.h
LVERSION := 3:0:0 # rules: http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info

ifeq ($(MONOFONTPATH),)
$(error Liberation Mono Regular font not found: please install it or provide correct FONTSPATH)
endif

all: libatg.la test widget

install: libatg.la
	libtool --mode=install install -D -m0644 atg.h $(PREFIX)/include/atg.h
	libtool --mode=install install -D -m0644 atg_internals.h $(PREFIX)/include/atg_internals.h
	libtool --mode=install install -D -m0644 libatg.la $(PREFIX)/lib/libatg.la
	-ldconfig

uninstall:
	libtool --mode=uninstall /bin/rm -f $(PREFIX)/lib/libatg.la $(PREFIX)/include/atg.h $(PREFIX)/include/atg_internals.h

clean:
	rm -f libatg.la $(LOBJS) test widget

test: test.c atg.h libatg.la
	libtool --mode=link $(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) test.c $(LDFLAGS) -o test -latg $(SDL)

widget: widget.c libatg.la atg.h atg_internals.h
	libtool --mode=link $(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) widget.c $(LDFLAGS) -o widget -latg $(SDL)

libatg.la: $(LOBJS)
	libtool --mode=link $(CC) -o $@ $(LOBJS) -rpath $(PREFIX)/lib -version-info $(LVERSION)

atg.lo: atg.c $(INCLUDES)
	libtool --mode=compile $(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) -c $<

plumbing.lo: plumbing.c $(INCLUDES)
	libtool --mode=compile $(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) -c $<

w_%.lo: w_%.c $(INCLUDES)
	libtool --mode=compile $(CC) $(CFLAGS) $(CPPFLAGS) $(SDLFLAGS) -c $<

%.lo: %.c %.h
	libtool --mode=compile $(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

w32:
	rm -rf libatg-w32
	mkdir libatg-w32
	-for p in *.c *.h readme; do cp $$p libatg-w32/$$p; done
	-for p in $$(ls wbits); do cp wbits/$$p libatg-w32/$$p; done
	make -C libatg-w32 -fMakefile.w32

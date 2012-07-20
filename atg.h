#pragma once
/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	This library is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdbool.h>
#include <SDL.h>

#define ATG_BOX_PACK_HORIZONTAL	0
#define ATG_BOX_PACK_VERTICAL	1

#define ATG_SPINNER_RIGHTCLICK_STEP10	0
#define ATG_SPINNER_RIGHTCLICK_TIMES2	1

#define ATG_ALPHA_TRANSPARENT	SDL_ALPHA_TRANSPARENT
#define ATG_ALPHA_OPAQUE		SDL_ALPHA_OPAQUE

typedef enum
{
	ATG_BOX,
	ATG_LABEL,
	ATG_IMAGE,
	ATG_BUTTON,
	ATG_SPINNER,
	
	ATG_CUSTOM,
}
atg_type;

typedef struct
{
	int x;
	int y;
}
atg_pos;

typedef struct
{
	Uint8 r,g,b,a;
}
atg_colour;

typedef struct
{
	Uint8 flags; // for ATG_BOX_PACK_HORIZONTAL and ATG_BOX_PACK_VERTICAL
	unsigned int nelems;
	struct atg_element **elems;
	atg_colour bgcolour;
}
atg_box;

typedef struct
{
	char *text;
	unsigned int fontsize;
	atg_colour colour;
}
atg_label;

typedef struct
{
	SDL_Surface *data;
}
atg_image;

typedef struct
{
	atg_colour fgcolour;
	atg_box *content;
}
atg_button;

typedef struct
{
	int minval, maxval, step;
	Uint8 flags; // for ATG_SPINNER_RIGHTCLICK_STEP10 and ATG_SPINNER_RIGHTCLICK_TIMES2
	int value;
	atg_box *content;
}
atg_spinner;

typedef struct
{
	SDL_Surface *surface;
	atg_box *box;
}
atg_canvas;

struct atg_event_list
{
	struct atg__event_list *list, *last;
};

typedef struct atg_element
{
	unsigned int w, h; // width and height (0 for either means "shrink around contents")
	SDL_Rect display; // co-ordinates within containing box
	SDL_Surface *(*render_callback)(const struct atg_element *e);
	void (*match_click_callback)(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff);
	void (*free_callback)(struct atg_element *e);
	atg_type type;
	union {
		atg_box *box;
		atg_label *label;
		atg_image *image;
		atg_button *button;
		atg_spinner *spinner;
	} elem;
	bool clickable;
	bool hidden;
	void *userdata; // normally NULL; is not freed by atg_free_element()
}
atg_element;

typedef enum
{
	ATG_EV_RAW, // raw SDL event
	ATG_EV_CLICK, // click in a clickable area
	ATG_EV_TRIGGER, // click on a button
	ATG_EV_TOGGLE, // set/clear a togglebutton, checkbox, or other toggleable
	ATG_EV_VALUE, // change of value (eg. of a spinner)
}
atg_event_type;

typedef enum
{
	ATG_MB_LEFT=SDL_BUTTON_LEFT,
	ATG_MB_RIGHT=SDL_BUTTON_RIGHT,
	ATG_MB_MIDDLE=SDL_BUTTON_MIDDLE,
	ATG_MB_SCROLLUP=SDL_BUTTON_WHEELUP,
	ATG_MB_SCROLLDN=SDL_BUTTON_WHEELDOWN,
}
atg_mousebutton;

typedef struct
{
	atg_element *e; // the element which caught the click
	atg_pos pos; // position of click relative to element top-left corner
	atg_mousebutton button;
}
atg_ev_click;

typedef struct
{
	atg_element *e; // the element which was triggered
	atg_mousebutton button;
}
atg_ev_trigger;

typedef struct
{
	atg_element *e; // the element which was toggled
	atg_mousebutton button;
	bool active; // the new state of the toggleable
}
atg_ev_toggle;

typedef struct
{
	atg_element *e; // the element whose value was changed
	int value; // the new value of the element
}
atg_ev_value;

typedef struct
{
	atg_event_type type;
	union {
		SDL_Event raw;
		atg_ev_click click;
		atg_ev_trigger trigger;
		atg_ev_toggle toggle;
		atg_ev_value value;
	} event;
}
atg_event;

void atg_flip(atg_canvas *canvas);

int atg_poll_event(atg_event *event, atg_canvas *canvas);

atg_canvas *atg_create_canvas(unsigned int w, unsigned int h, atg_colour bgcolour);
int atg_resize_canvas(atg_canvas *canvas, unsigned int w, unsigned int h);
atg_box *atg_create_box(Uint8 flags, atg_colour bgcolour);
atg_label *atg_create_label(const char *text, unsigned int fontsize, atg_colour colour);
atg_image *atg_create_image(SDL_Surface *img);
atg_button *atg_create_button(const char *label, atg_colour fgcolour, atg_colour bgcolour);
atg_spinner *atg_create_spinner(Uint8 flags, int minval, int maxval, int step, int initvalue);

atg_element *atg_create_element_box(Uint8 flags, atg_colour bgcolour);
atg_element *atg_create_element_label(const char *text, unsigned int fontsize, atg_colour colour);
atg_element *atg_create_element_image(SDL_Surface *img);
atg_element *atg_create_element_button(const char *label, atg_colour fgcolour, atg_colour bgcolour);
atg_element *atg_create_element_spinner(Uint8 flags, int minval, int maxval, int step, int initvalue);

int atg_pack_element(atg_box *box, atg_element *elem);
atg_element *atg_copy_element(const atg_element *e);

void atg_free_canvas(atg_canvas *canvas);
void atg_free_box(atg_element *e);
void atg_free_box_box(atg_box *b);
void atg_free_label(atg_element *e);
void atg_free_image(atg_element *e);
void atg_free_button(atg_element *e);
void atg_free_spinner(atg_element *e);

void atg_free_element(atg_element *element);

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
	size_t l; // size of 'd' allocation
	size_t i; // length used
	char *d;
}
atg_string;

typedef struct
{
	atg_string text;
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
	atg_box content;
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

typedef struct atg_element
{
	unsigned int w, h; // width and height (0 for either means "shrink around contents")
	SDL_Rect display;
	atg_type type;
	union {
		atg_box *box;
		atg_label *label;
		atg_image *image;
		atg_button *button;
		atg_spinner *spinner;
	} elem;
	bool clickable;
	void *userdata; // normally NULL; is not freed by atg_free_element()
}
atg_element;

typedef enum
{
	ATG_EV_RAW, // raw SDL event
	ATG_EV_CLICK, // click in a clickable area
	/*ATG_EV_TRIGGER, // click on a button
	ATG_EV_TOGGLE, // set/clear a togglebutton, checkbox, or other toggleable
	ATG_EV_VALUE, // change of value (eg. of a spinner)*/
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
	atg_pos pos;
	atg_mousebutton button;
}
atg_ev_click;

typedef struct
{
	atg_event_type type;
	union {
		SDL_Event *raw;
		atg_ev_click *click;
		/*atg_ev_trigger *trigger;
		atg_ev_toggle *toggle;
		atg_ev_value *value;
		*/
	} event;
}
atg_event;

void atg_flip(atg_canvas *canvas);

atg_canvas *atg_create_canvas(unsigned int w, unsigned int h, atg_colour bgcolour);
atg_box *atg_create_box(Uint8 flags, atg_colour bgcolour);
atg_label *atg_create_label(const atg_string text, unsigned int fontsize, atg_colour colour);

atg_element *atg_create_element_label(const atg_string text, unsigned int fontsize, atg_colour colour);

int atg_pack_element(atg_box *box, atg_element *elem);
atg_element *atg_copy_element(const atg_element *e);

void atg_free_canvas(atg_canvas *canvas);
void atg_free_box(atg_box *box);
void atg_free_label(atg_label *label);

void atg_free_element(atg_element *element);

atg_string init_string(void); // initialises a string buffer in heap
atg_string null_string(void); // returns a null string (no allocation)
atg_string make_string(const char *str); // initialises a string buffer in heap, with initial contents copied from str
void append_char(atg_string *s, char c); // adds a character to a string buffer in heap (and realloc()s if needed)
void append_str(atg_string *s, const char *str); // adds a cstring to a string buffer in heap (and realloc()s if needed)
void append_string(atg_string *s, const atg_string t); // adds a string to a string buffer in heap (and realloc()s if needed)
atg_string atg_strdup(const atg_string string);
atg_string atg_const_string(char *str);
void atg_free_string(atg_string string);

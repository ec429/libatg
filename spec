atg - a tiny gui

Specification

atg is a small, simple GUI library/toolkit for SDL, providing things like buttons and clickables to allow you to concentrate on your program logic.

atg is loosely based on Spiffy's GUI, though genericised.

atg Event Queue:
	atg will poll SDL for events, and if these events trigger any atg elements, these elements' events will be placed on the queue.  In any case all SDL events will be placed on the atg queue, in case the application wishes to further process them.  Note that if the application calls SDL_PollEvent() or SDL_WaitEvent(), these events will not be 'seen' by atg.  Thus, the application should instead call atg_poll_event() or atg_wait_event().  Alternatively, after calling SDL_PollEvent() or SDL_WaitEvent(), the application can pass the returned event to atg with atg_do_event().

API:Types:
	atg_canvas
		struct {
			SDL_Surface *surface;
			atg_box *box;
		}
	
	atg_colour
		struct {
			Uint8 r,g,b,a;
		}
	
	atg_pos
		struct {
			int x;
			int y;
		}
	
	atg_box
		struct {
			Uint8 flags; // for ATG_BOX_PACK_HORIZONTAL and ATG_BOX_PACK_VERTICAL
			unsigned int nelems;
			atg_element **elems;
			atg_colour bgcolour;
		}
	
	atg_element
		struct {
			unsigned int w, h; // width and height (0 for either means "shrink around contents")
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
	
	atg_label
		struct {
			atg_string text;
			unsigned int fontsize;
			atg_colour colour;
		}
	
	atg_string
		struct {
			size_t l; // size of 'd' allocation
			size_t i; // length used
			char *d;
		}
	
	atg_image
		struct {
			SDL_Surface *data;
		}
	
	atg_button
		struct {
			atg_box content;
		}
	
	atg_spinner
		struct {
			int minval, maxval, step;
			Uint8 flags; // for ATG_SPINNER_RIGHTCLICK_STEP10 and ATG_SPINNER_RIGHTCLICK_TIMES2
			int value;
			atg_box *content;
		}

#pragma once
/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	atg_internals.h: definitions for making custom widgets
*/

SDL_Surface *atg_resize_surface(SDL_Surface *src, const atg_element *e);

/* Standard constructors */
atg_label *atg_create_label(const char *text, unsigned int fontsize, atg_colour colour);
atg_button *atg_create_button(const char *label, atg_colour fgcolour, atg_colour bgcolour);

/* Standard renderers */
SDL_Surface *atg_render_element(atg_element *e);
SDL_Surface *atg_render_box(const atg_element *e); /* Most custom widgets will be based on a box */
SDL_Surface *atg_render_button(const atg_element *e);

/* Standard copiers */
atg_element *atg_copy_element(const atg_element *e);
atg_box *atg_copy_box_box(const atg_box *b);

int atg_empty_box(atg_box *b);

/* Event lists */
typedef struct atg__event_list
{
	atg_event event;
	struct atg__event_list *next;
}
atg__event_list;

int atg__push_event(struct atg_event_list *list, atg_event event);

/* Standard recursive click-handling callback */
void atg__match_click_recursive(struct atg_event_list *list, atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff);

void atg__match_click(struct atg_event_list *list, atg_canvas *canvas, SDL_MouseButtonEvent button);

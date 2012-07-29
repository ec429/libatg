#pragma once
/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	atg_internals.h: definitions for making custom widgets
*/

SDL_Surface *atg_resize_surface(SDL_Surface *src, const atg_element *e);

/* Standard renderers */
SDL_Surface *atg_render_element(atg_element *e);
SDL_Surface *atg_render_box(const atg_element *e); /* Most custom widgets will be based on a box */
SDL_Surface *atg_render_label(const atg_element *e);
SDL_Surface *atg_render_image(const atg_element *e);
SDL_Surface *atg_render_button(const atg_element *e);
SDL_Surface *atg_render_spinner(const atg_element *e);
SDL_Surface *atg_render_toggle(const atg_element *e);

/* Standard copiers */
atg_element *atg_copy_element(const atg_element *e);
atg_box *atg_copy_box(const atg_box *b);
atg_label *atg_copy_label(const atg_label *);
atg_image *atg_copy_image(const atg_image *);
atg_button *atg_copy_button(const atg_button *);
atg_spinner *atg_copy_spinner(const atg_spinner *);
atg_toggle *atg_copy_toggle(const atg_toggle *);

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

/* Standard click handlers */
void atg_click_box(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff);
void atg_click_button(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff);
void atg_click_spinner(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff);
void atg_click_toggle(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff);

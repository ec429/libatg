#pragma once
/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	atg_internals.h: definitions for making custom widgets
*/

SDL_Surface *atg_render_element(atg_element *e);

/* Standard renderers */
SDL_Surface *atg_render_box(const atg_element *e); /* Most custom widgets will be based on a box */
SDL_Surface *atg_render_label(const atg_element *e);
SDL_Surface *atg_render_image(const atg_element *e);
SDL_Surface *atg_render_button(const atg_element *e);
SDL_Surface *atg_render_spinner(const atg_element *e);
SDL_Surface *atg_render_toggle(const atg_element *e);

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

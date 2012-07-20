#pragma once
/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	atg_internals: definitions for making custom widgets
*/

SDL_Surface *atg_render_element(const atg_element *e);

SDL_Surface *atg_render_box(const atg_element *e);
SDL_Surface *atg_render_label(const atg_element *e);
SDL_Surface *atg_render_image(const atg_element *e);
SDL_Surface *atg_render_button(const atg_element *e);

typedef struct atg__event_list
{
	atg_event event;
	struct atg__event_list *next;
}
atg__event_list;

int atg__push_event(struct atg_event_list *list, atg_event event);

void atg__match_click_recursive(struct atg_event_list *list, atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff);

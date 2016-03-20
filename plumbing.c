/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	plumbing.c: implements generic-to-specific plumbing
*/
#include "atg.h"
#include "atg_internals.h"

SDL_Surface *atg_render_element(atg_element *e)
{
	if(!e) return(NULL);
	if(e->hidden) return(NULL);
	if(e->cache&&e->cached)
	{
		e->cached->refcount++;
		return(e->cached);
	}
	SDL_Surface *rv=NULL;
	if(e->render_callback)
		rv=e->render_callback(e);
	if(e->cache)
	{
		e->cached=rv;
		if(rv) rv->refcount++;
	}
	return(rv);
}

int atg__push_event(struct atg_event_list *list, atg_event event)
{
	if(!list) return(1);
	if(list->last)
	{
		if(!(list->last->next=malloc(sizeof(atg__event_list))))
			return(1);
		*(list->last=list->last->next)=(atg__event_list){.event=event, .next=NULL};
		return(0);
	}
	else if(list->list)
	{
		list->last=list->list;
		while(list->last->next) list->last=list->last->next;
		if(!(list->last->next=malloc(sizeof(atg__event_list))))
			return(1);
		*(list->last=list->last->next)=(atg__event_list){.event=event, .next=NULL};
		return(0);
	}
	else
	{
		if(!(list->last=list->list=malloc(sizeof(atg__event_list))))
			return(1);
		*list->last=(atg__event_list){.event=event, .next=NULL};
		return(0);
	}
}

void atg__match_click_recursive(struct atg_event_list *list, atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff)
{
	if(!element) return;
	if(element->hidden) return;
	if(
		(button.x>=element->display.x+xoff)
		&&(button.x<element->display.x+xoff+element->display.w)
		&&(button.y>=element->display.y+yoff)
		&&(button.y<element->display.y+yoff+element->display.h)
	)
	{
		if(element->cached) // Catching clicks clears cache.  It's only prudent
		{
			SDL_FreeSurface(element->cached);
			element->cached=NULL;
		}
		if(element->clickable)
		{
			atg_ev_click click;
			click.e=element;
			click.pos=(atg_pos){.x=button.x-element->display.x-xoff, .y=button.y-element->display.y-yoff};
			click.button=button.button;
			atg__push_event(list, (atg_event){.type=ATG_EV_CLICK, .event.click=click});
		}
		if(element->match_click_callback)
			element->match_click_callback(list, element, button, xoff, yoff);
	}
}

void atg__match_click(struct atg_event_list *list, atg_canvas *canvas, SDL_MouseButtonEvent button)
{
	if(!canvas) return;
	atg__match_click_recursive(list, canvas->content, button, 0, 0);
}

atg_element *atg_copy_element(const atg_element *e)
{
	if(!e) return(NULL);
	if(e->copy_callback)
		return(e->copy_callback(e));
	return(NULL);
}

int atg_value_event(atg_element *e, struct atg_event *ev)
{
	struct atg_event event;
	if(!e) return(1);
	if(!e->value_callback) return(1);
	event=e->value_callback(e);
	if(ev)
		*ev=event;
	return(0);
}

void atg_free_element(atg_element *element)
{
	if(element)
	{
		if(element->free_callback)
			element->free_callback(element);
		else /* Bad things */
			fprintf(stderr, "Don't know how to free element of type %s at %p, very bad things!\n", element->type, (void *)element);
		SDL_FreeSurface(element->cached);
	}
	free(element);
}

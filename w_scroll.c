/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2024 Edward Cree
	
	See atg.h for license information
	w_scroll.c: implements the SCROLL widget
*/
#include "atg.h"
#include "atg_internals.h"

#define min(x, y)	((x)<(y)?(x):(y))
#define max(x, y)	((x)>(y)?(x):(y))
#define min_t(t, x, y)	((t)(x)<(t)(y)?(t)(x):(t)(y))
#define SCROLL_BAR_WIDTH	6

SDL_Surface *atg_render_scroll(const atg_element *e)
{
	if(!e) return(NULL);
	atg_scroll *s=e->elemdata;
	if(!s) return(NULL);
	SDL_Surface *content=atg_render_element(s->content);
	if(!content) return(NULL);
	unsigned int w=min_t(unsigned int, e->w, content->w), h=min_t(unsigned int, e->h, content->h);
	if(!e->w) w=content->w;
	SDL_Surface *rv=SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, content->format->BitsPerPixel, content->format->Rmask, content->format->Gmask, content->format->Bmask, content->format->Amask);
	if(!rv)
	{
		SDL_FreeSurface(content);
		return(NULL);
	}
	SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, s->bgcolour.r, s->bgcolour.g, s->bgcolour.b, s->bgcolour.a));
	SDL_BlitSurface(content, &(SDL_Rect){.x=0, .y=s->y, .w=rv->w, .h=rv->h}, rv, &(SDL_Rect){.x=0, .y=0});
	s->content->display=(SDL_Rect){.x=0, .y=-s->y, .w=rv->w, .h=rv->h};
	s->ch=content->h;
	SDL_FreeSurface(content);
	if(e->h<s->ch)
	{
		unsigned int sh=(e->h*e->h)/max(s->ch, 1U), sg=e->h-sh;
		unsigned int y0=(sg*s->y)/max(s->ch-e->h, 1U);
		// TODO cache this surface inside the atg_scroll
		SDL_Surface *bar=SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA, SCROLL_BAR_WIDTH, h, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
		if(bar)
		{
			SDL_FillRect(bar, &(SDL_Rect){.x=0, .y=0, .w=bar->w, .h=bar->h}, ATG_ALPHA_TRANSPARENT&0xff);
			SDL_FillRect(bar, &(SDL_Rect){.x=0, .y=y0, .w=bar->w, .h=sh}, SDL_MapRGBA(bar->format, s->fgcolour.r, s->fgcolour.g, s->fgcolour.b, s->fgcolour.a));
			SDL_FillRect(bar, &(SDL_Rect){.x=1, .y=bar->h/4, .w=bar->w-2, .h=1}, ATG_ALPHA_TRANSPARENT&0xff);
			SDL_FillRect(bar, &(SDL_Rect){.x=1, .y=bar->h*3/4, .w=bar->w-2, .h=1}, ATG_ALPHA_TRANSPARENT&0xff);
			SDL_BlitSurface(bar, NULL, rv, &(SDL_Rect){.x=rv->w-bar->w, .y=0});
		}
		SDL_FreeSurface(bar);
	}
	return(rv);
}

void atg_click_scroll(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff)
{
	atg_scroll *s=element->elemdata;
	atg_element *content=s->content;
	// match the scroll bar
	if(s->ch>element->display.h&&button.x>=element->display.x+xoff+element->display.w-SCROLL_BAR_WIDTH)
	{
		unsigned int eh=element->display.h;
		unsigned int cy=button.y-element->display.y-yoff;
		cy=max(cy*2, eh/2)-eh/2;
		cy=min(cy, eh);
		s->y=((s->ch-eh)*cy)/max(eh, 1U);
	}
	else
	{
		// pass the event through
		struct atg_event_list sub_list={.list=NULL, .last=NULL};
		atg__match_click_recursive(&sub_list, content, button, xoff+element->display.x, yoff+element->display.y);
		while(sub_list.list)
		{
			atg_event event=sub_list.list->event;
			atg__push_event(list, event);
			atg__event_list *next=sub_list.list->next;
			free(sub_list.list);
			sub_list.list=next;
		}
	}
}

atg_scroll *atg_create_scroll(atg_element *content, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_scroll *rv=malloc(sizeof(atg_scroll));
	if(rv)
	{
		rv->content=content;
		rv->fgcolour=fgcolour;
		rv->bgcolour=bgcolour;
		rv->y=0;
		rv->ch=0;
	}
	return(rv);
}

atg_scroll *atg_copy_scroll_scroll(const atg_scroll *s)
{
	atg_scroll *s2=malloc(sizeof(atg_scroll));
	if(!s2)
		return(NULL);
	*s2=*s;
	s2->content=atg_copy_element(s->content);
	return(s2);
}

atg_element *atg_copy_scroll(const atg_element *e)
{
	if(!e) return(NULL);
	atg_scroll *s=e->elemdata;
	if(!s) return(NULL);
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	*rv=*e;
	if(!(rv->elemdata=atg_copy_scroll_scroll(s)))
	{
		free(rv);
		return(NULL);
	}
	return(rv);
}

void atg_free_scroll(atg_element *e)
{
	if(!e) return;
	atg_scroll *s=e->elemdata;
	atg_free_element(s->content);
}

atg_element *atg_create_element_scroll(atg_element *content, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_scroll *s=atg_create_scroll(content, fgcolour, bgcolour);
	if(!s)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type="__builtin_scroll";
	rv->elemdata=s;
	rv->clickable=false;
	rv->hidden=false;
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_scroll;
	rv->match_click_callback=atg_click_scroll;
	rv->pack_callback=NULL;
	rv->copy_callback=atg_copy_scroll;
	rv->free_callback=atg_free_scroll;
	return(rv);
}

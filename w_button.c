/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	w_button.c: implements the BUTTON widget
*/
#include "atg.h"
#include "atg_internals.h"

SDL_Surface *atg_render_button(const atg_element *e)
{
	if(!e) return(NULL);
	atg_button *b=e->elemdata;
	if(!b) return(NULL);
	SDL_Surface *content=atg_render_box(&(atg_element){.w=(e->w>4)?e->w-4:0, .h=(e->h>4)?e->h-4:0, .elemdata=b->content, .clickable=false, .userdata=NULL});
	if(!content) return(NULL);
	bool borders=(content->w>8)&&(content->h>4);
	SDL_Surface *rv=SDL_CreateRGBSurface(SDL_HWSURFACE, content->w+(borders?4:0), content->h+(borders?4:0), content->format->BitsPerPixel, content->format->Rmask, content->format->Gmask, content->format->Bmask, content->format->Amask);
	if(!rv)
	{
		SDL_FreeSurface(content);
		return(NULL);
	}
	SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, b->content->bgcolour.r, b->content->bgcolour.g, b->content->bgcolour.b, b->content->bgcolour.a));
	if(borders)
	{
		SDL_BlitSurface(content, NULL, rv, &(SDL_Rect){.x=2, .y=2});
		SDL_FillRect(rv, &(SDL_Rect){.x=2, .y=1, .w=rv->w-4, .h=1}, SDL_MapRGBA(rv->format, b->fgcolour.r, b->fgcolour.g, b->fgcolour.b, b->fgcolour.a));
		SDL_FillRect(rv, &(SDL_Rect){.x=2, .y=rv->h-2, .w=rv->w-4, .h=1}, SDL_MapRGBA(rv->format, b->fgcolour.r, b->fgcolour.g, b->fgcolour.b, b->fgcolour.a));
		SDL_FillRect(rv, &(SDL_Rect){.x=1, .y=2, .w=1, .h=rv->h-4}, SDL_MapRGBA(rv->format, b->fgcolour.r, b->fgcolour.g, b->fgcolour.b, b->fgcolour.a));
		SDL_FillRect(rv, &(SDL_Rect){.x=rv->w-2, .y=2, .w=1, .h=rv->h-4}, SDL_MapRGBA(rv->format, b->fgcolour.r, b->fgcolour.g, b->fgcolour.b, b->fgcolour.a));
	}
	else
		SDL_BlitSurface(content, NULL, rv, &(SDL_Rect){.x=0, .y=0});
	SDL_FreeSurface(content);
	return(rv);
}

void atg_click_button(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, __attribute__((unused)) unsigned int xoff, __attribute__((unused)) unsigned int yoff)
{
	atg_ev_trigger trigger;
	trigger.e=element;
	trigger.button=button.button;
	atg__push_event(list, (atg_event){.type=ATG_EV_TRIGGER, .event.trigger=trigger});
}

atg_button *atg_create_button(const char *label, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_button *rv=malloc(sizeof(atg_button));
	if(rv)
	{
		rv->fgcolour=fgcolour;
		rv->content=atg_create_box(ATG_BOX_PACK_HORIZONTAL, bgcolour);
		if(rv->content)
		{
			atg_element *l=atg_create_element_label(label, 12, fgcolour);
			if(l)
			{
				if(atg_pack_element(rv->content, l))
				{
					atg_free_element(l);
					atg_free_box_box(rv->content);
					free(rv);
					return(NULL);
				}
			}
			else
			{
				atg_free_box_box(rv->content);
				free(rv);
				return(NULL);
			}
		}
		else
		{
			free(rv);
			return(NULL);
		}
	}
	return(rv);
}

atg_button *atg_create_button_empty(atg_colour fgcolour, atg_colour bgcolour)
{
	atg_button *rv=malloc(sizeof(atg_button));
	if(rv)
	{
		rv->fgcolour=fgcolour;
		rv->content=atg_create_box(ATG_BOX_PACK_HORIZONTAL, bgcolour);
		if(!rv->content)
		{
			free(rv);
			return(NULL);
		}
	}
	return(rv);
}

int atg_pack_button(atg_element *ebox, atg_element *elem)
{
	atg_button *button=ebox->elemdata;
	if(!button)
		return(1);
	atg_box *b=button->content;
		return(atg_pack_element(b, elem));
}

atg_element *atg_copy_button(const atg_element *e)
{
	if(!e) return(NULL);
	atg_button *button=e->elemdata;
	if(!button) return(NULL);
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	*rv=*e;
	atg_button *button2=rv->elemdata=malloc(sizeof(atg_button));
	if(!button2)
	{
		free(rv);
		return(NULL);
	}
	*button2=*button;
	button2->content=button->content?atg_copy_box_box(button->content):NULL;
	return(rv);
}

void atg_free_button(atg_element *e)
{
	if(!e) return;
	atg_button *button=e->elemdata;
	if(button)
	{
		atg_free_box_box(button->content);
	}
	free(button);
}

atg_element *atg_create_element_button(const char *label, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_button *b=atg_create_button(label, fgcolour, bgcolour);
	if(!b)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type="__builtin_button";
	rv->elemdata=b;
	rv->clickable=false; /* because it generates ATG_EV_TRIGGER events instead */
	rv->hidden=false;
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_button;
	rv->match_click_callback=atg_click_button;
	rv->pack_callback=atg_pack_button;
	rv->copy_callback=atg_copy_button;
	rv->free_callback=atg_free_button;
	return(rv);
}

atg_element *atg_create_element_button_empty(atg_colour fgcolour, atg_colour bgcolour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_button *b=atg_create_button_empty(fgcolour, bgcolour);
	if(!b)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type="__builtin_button";
	rv->elemdata=b;
	rv->clickable=false; /* because it generates ATG_EV_TRIGGER events instead */
	rv->hidden=false;
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_button;
	rv->match_click_callback=atg_click_button;
	rv->pack_callback=atg_pack_button;
	rv->copy_callback=atg_copy_button;
	rv->free_callback=atg_free_button;
	return(rv);
}

atg_element *atg_create_element_button_image(SDL_Surface *img, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_element *rv=atg_create_element_button_empty(fgcolour, bgcolour);
	if(!rv)
		return(NULL);
	atg_element *i=atg_create_element_image(img);
	if(!i)
	{
		atg_free_element(rv);
		return(NULL);
	}
	if(atg_ebox_pack(rv, i))
	{
		atg_free_element(i);
		atg_free_element(rv);
		return(NULL);
	}
	return(rv);
}

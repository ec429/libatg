/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	w_spinner.c: implements the SPINNER widget
*/
#include "atg.h"
#include "atg_internals.h"
#include <string.h>

#define VALUE_LEN	32

SDL_Surface *atg_render_spinner(const atg_element *e)
{
	if(!e) return(NULL);
	atg_spinner *s=e->elemdata;
	if(!s) return(NULL);
	atg_box *b=s->content;
	if(!b) return(NULL);
	if(b->nelems&&b->elems[0]&&!strcmp(b->elems[0]->type, "__builtin_label"))
	{
		atg_label *l=b->elems[0]->elemdata;
		if(l->text)
			snprintf(l->text, VALUE_LEN, s->fmt, s->value);
	}
	SDL_Surface *content=atg_render_box(&(atg_element){.w=e->w, .h=e->h, .elemdata=s->content, .clickable=false, .userdata=NULL});
	if(!content) return(NULL);
	SDL_Surface *rv=SDL_CreateRGBSurface(SDL_HWSURFACE, content->w, content->h, content->format->BitsPerPixel, content->format->Rmask, content->format->Gmask, content->format->Bmask, content->format->Amask);
	if(!rv)
	{
		SDL_FreeSurface(content);
		return(NULL);
	}
	SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, s->content->bgcolour.r, s->content->bgcolour.g, s->content->bgcolour.b, s->content->bgcolour.a));
	SDL_BlitSurface(content, NULL, rv, &(SDL_Rect){.x=0, .y=0});
	SDL_FreeSurface(content);
	return(rv);
}

void atg_click_spinner(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff)
{
	atg_spinner *s=element->elemdata;
	if(!s) return;
	atg_box *b=s->content;
	if(!b) return;
	if(!b->elems) return;
	int oldval=s->value;
	if(button.button==ATG_MB_SCROLLDN)
	{
		s->value-=s->step;
	}
	else if(button.button==ATG_MB_SCROLLUP)
	{
		s->value+=s->step;
	}
	else
	{
		struct atg_event_list sub_list={.list=NULL, .last=NULL};
		for(unsigned int i=0;i<b->nelems;i++)
			atg__match_click_recursive(&sub_list, b->elems[i], button, xoff+element->display.x, yoff+element->display.y);
		while(sub_list.list)
		{
			atg_event event=sub_list.list->event;
			if(event.type==ATG_EV_TRIGGER)
			{
				if((event.event.trigger.button==ATG_MB_LEFT)||(event.event.trigger.button==ATG_MB_RIGHT))
				{
					atg_element *e=event.event.trigger.e;
					if(e&&!strcmp(e->type, "__builtin_button"))
					{
						atg_button *b=e->elemdata;
						if(b)
						{
							atg_box *box=b->content;
							if(box&&box->nelems&&box->elems)
							{
								atg_element *e=box->elems[0];
								if(e&&!strcmp(e->type, "__builtin_label"))
								{
									atg_label *l=e->elemdata;
									if(l&&l->text)
									{
										if(strcmp(l->text, "+")==0)
										{
											if(event.event.trigger.button==ATG_MB_RIGHT)
											{
												if(s->flags&ATG_SPINNER_RIGHTCLICK_TIMES2)
													s->value*=2;
												else
													s->value+=10*s->step;
											}
											else
												s->value+=s->step;
										}
										else if(strcmp(l->text, "-")==0)
										{
											if(event.event.trigger.button==ATG_MB_RIGHT)
											{
												if(s->flags&ATG_SPINNER_RIGHTCLICK_TIMES2)
													s->value/=2;
												else
													s->value-=10*s->step;
											}
											else
												s->value-=s->step;
										}
									}
								}
							}
						}
					}
				}
			}
			atg__event_list *next=sub_list.list->next;
			free(sub_list.list);
			sub_list.list=next;
		}
	}
	if(s->value>s->maxval) s->value=s->maxval;
	if(s->value<s->minval) s->value=s->minval;
	if(s->value!=oldval)
		atg__push_event(list, (atg_event){.type=ATG_EV_VALUE, .event.value=(atg_ev_value){.e=element, .value=s->value}});
}

atg_spinner *atg_create_spinner(Uint8 flags, int minval, int maxval, int step, int initvalue, const char *fmt, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_spinner *rv=malloc(sizeof(atg_spinner));
	if(rv)
	{
		rv->minval=minval;
		rv->maxval=maxval;
		rv->step=step;
		rv->flags=flags;
		rv->value=initvalue;
		rv->fmt=strdup(fmt?fmt:"%d");
		rv->content=atg_create_box(ATG_BOX_PACK_HORIZONTAL, bgcolour);
		if(!rv->content)
		{
			free(rv);
			return(NULL);
		}
		char *val=malloc(VALUE_LEN);
		if(!val)
		{
			free(rv);
			return(NULL);
		}
		snprintf(val, VALUE_LEN, rv->fmt, rv->value);
		atg_element *lbl=atg_create_element_label(NULL, 15, fgcolour);
		if(!lbl)
		{
			free(val);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_label *l=lbl->elemdata;
		l->text=val;
		if(atg_pack_element(rv->content, lbl))
		{
			atg_free_element(lbl);
			free(val);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_element *vbox=atg_create_element_box(ATG_BOX_PACK_VERTICAL, bgcolour);
		if(!vbox)
		{
			free(val);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		if(atg_pack_element(rv->content, vbox))
		{
			atg_free_element(vbox);
			free(val);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_element *ubtn=atg_create_element_button_empty(fgcolour, bgcolour);
		if(!ubtn)
		{
			free(val);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_element *ulbl=atg_create_element_label("+", 7, fgcolour);
		if(!ulbl)
		{
			atg_free_element(ubtn);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		if(atg_ebox_pack(vbox, ubtn))
		{
			atg_free_element(ubtn);
			free(val);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_element *dbtn=atg_create_element_button_empty(fgcolour, bgcolour);
		if(!dbtn)
		{
			free(val);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_element *dlbl=atg_create_element_label("-", 7, fgcolour);
		if(!dlbl)
		{
			atg_free_element(dbtn);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		if(atg_ebox_pack(vbox, dbtn))
		{
			atg_free_element(dbtn);
			free(val);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
	}
	return(rv);
}

atg_element *atg_copy_spinner(const atg_element *e)
{
	if(!e) return(NULL);
	atg_spinner *s=e->elemdata;
	if(!s) return(NULL);
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	*rv=*e;
	atg_spinner *s2=rv->elemdata=malloc(sizeof(atg_spinner));
	if(!s2)
	{
		free(rv);
		return(NULL);
	}
	*s2=*s;
	s2->content=s->content?atg_copy_box_box(s->content):NULL;
	s2->fmt=s->fmt?strdup(s->fmt):NULL;
	return(rv);
}

void atg_free_spinner(atg_element *e)
{
	if(!e) return;
	atg_spinner *spinner=e->elemdata;
	if(spinner)
	{
		free(spinner->fmt);
		atg_free_box_box(spinner->content);
	}
	free(spinner);
}

atg_element *atg_create_element_spinner(Uint8 flags, int minval, int maxval, int step, int initvalue, const char *fmt, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_spinner *s=atg_create_spinner(flags, minval, maxval, step, initvalue, fmt, fgcolour, bgcolour);
	if(!s)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type="__builtin_spinner";
	rv->elemdata=s;
	rv->clickable=false; /* because it generates ATG_EV_VALUE events instead */
	rv->hidden=false;
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_spinner;
	rv->match_click_callback=atg_click_spinner;
	rv->pack_callback=NULL;
	rv->copy_callback=atg_copy_spinner;
	rv->free_callback=atg_free_spinner;
	return(rv);
}

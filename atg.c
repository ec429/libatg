/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	atg.c: provides all the actual functions
	
	TODO: this really ought to be split up into separate files.
		Perhaps one file for 'plumbing' and then one for each widget.
*/

#include "atg.h"
#include "atg_internals.h"
#include <string.h>
#include <SDL_ttf.h>

#define MAXFONTSIZE	24
bool ttfinit=false;
const char *monofont="/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf";
TTF_Font *monottf[MAXFONTSIZE];

SDL_Surface *gf_init(unsigned int w, unsigned int h)
{
	SDL_Surface *screen;
	if(SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return(NULL);
	}
	atexit(SDL_Quit);
	if(!(screen=SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE)))
	{
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		return(NULL);
	}
	return(screen);
}

void initttf(void)
{
	if(TTF_Init()) return;
	for(unsigned int i=0;i<MAXFONTSIZE;i++)
		monottf[i]=NULL;
	ttfinit=true;
}

SDL_Surface *atg_resize_surface(SDL_Surface *src, const atg_element *e)
{
	SDL_Surface *rv=SDL_CreateRGBSurface(SDL_HWSURFACE, e->w?(int)e->w:src->w, e->h?(int)e->h:src->h, src->format->BitsPerPixel, src->format->Rmask, src->format->Gmask, src->format->Bmask, src->format->Amask);
	if(!rv) return(NULL);
	SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT));
	SDL_SetAlpha(src, 0, SDL_ALPHA_OPAQUE);
	SDL_BlitSurface(src, NULL, rv, NULL);
	return(rv);
}

SDL_Surface *atg_render_box(const atg_element *e)
{
	SDL_Surface *screen=SDL_GetVideoSurface();
	if(!screen) return(NULL); /* can't find out display format */
	if(!e) return(NULL);
	if(!((e->type==ATG_BOX)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_box *b=e->elem.box;
	if(!b) return(NULL);
	SDL_Surface **els=malloc(b->nelems*sizeof(SDL_Surface *)), *rv=NULL;
	if(!els) return(NULL);
	for(unsigned int i=0;i<b->nelems;i++)
		els[i]=atg_render_element(b->elems[i]);
	/* Box packing.
		This is all a horrid mess; there must be a cleaner way to do this */
	/* Packing rules: for VERTICAL [HORIZONTAL]
		If we have a height [width], we fill up each column [row] to that size;
		once we get something that doesn't fit, we give up on that column [row]
		and never go back to it.
		If we also have a width [height], we ignore anything that's left once
		it's been exceeded.
		If we only have a width [height] and no height [width], we just treat
		it as a minimum, and stack everything into a single column [row].
		If we have neither, we stack everything into a single column [row] that
		is as wide [tall] as the widest [tallest] element. */
	if(b->flags&ATG_BOX_PACK_VERTICAL)
	{
		if(e->h)
		{
			if(e->w)
			{
				rv=SDL_CreateRGBSurface(SDL_HWSURFACE, e->w, e->h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
				SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, b->bgcolour.r, b->bgcolour.g, b->bgcolour.b, b->bgcolour.a));
				unsigned int y=0, x=0, xmax=0;
				for(unsigned int i=0;i<b->nelems;i++)
				{
					if(x>=e->w) break;
					if(!els[i]) continue;
					if(y+els[i]->h>e->h)
					{
						y=0;
						x=xmax;
					}
					SDL_BlitSurface(els[i], NULL, rv, &(SDL_Rect){.x=x, .y=y});
					b->elems[i]->display=(SDL_Rect){.x=x, .y=y, .w=els[i]->w, .h=els[i]->h};
					y+=els[i]->h;
					if(x+els[i]->w>xmax)
						xmax=x+els[i]->w;
				}
			}
			else
			{
				unsigned int x=0, y=0, xmax=0;
				for(unsigned int i=0;i<b->nelems;i++)
				{
					if(!els[i]) continue;
					if(y+els[i]->h>e->h)
					{
						y=0;
						x=xmax;
					}
					b->elems[i]->display=(SDL_Rect){.x=x, .y=y, .w=els[i]->w, .h=els[i]->h};
					y+=els[i]->w;
					if(x+els[i]->w>xmax)
						xmax=x+els[i]->w;
				}
				rv=SDL_CreateRGBSurface(SDL_HWSURFACE, xmax, e->h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
				SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, b->bgcolour.r, b->bgcolour.g, b->bgcolour.b, b->bgcolour.a));
				for(unsigned int i=0;i<b->nelems;i++)
					if(els[i])
						SDL_BlitSurface(els[i], NULL, rv, &b->elems[i]->display);
			}
		}
		else
		{
			unsigned int x=e->w, y=0;
			for(unsigned int i=0;i<b->nelems;i++)
				if(els[i])
				{
					y+=els[i]->h;
					if((unsigned int)els[i]->w>x) x=els[i]->w;
				}
			rv=SDL_CreateRGBSurface(SDL_HWSURFACE, x, y, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
			SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, b->bgcolour.r, b->bgcolour.g, b->bgcolour.b, b->bgcolour.a));
			y=0;
			for(unsigned int i=0;i<b->nelems;i++)
				if(els[i])
				{
					SDL_BlitSurface(els[i], NULL, rv, &(SDL_Rect){.x=0, .y=y});
					b->elems[i]->display=(SDL_Rect){.x=0, .y=y, .w=els[i]->w, .h=els[i]->h};
					y+=els[i]->h;
				}
		}
	}
	else
	{
		if(e->w)
		{
			if(e->h)
			{
				rv=SDL_CreateRGBSurface(SDL_HWSURFACE, e->w, e->h, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
				SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, b->bgcolour.r, b->bgcolour.g, b->bgcolour.b, b->bgcolour.a));
				unsigned int y=0, x=0, ymax=0;
				for(unsigned int i=0;i<b->nelems;i++)
				{
					if(y>=e->h) break;
					if(!els[i]) continue;
					if(x+els[i]->w>e->w)
					{
						x=0;
						y=ymax;
					}
					SDL_BlitSurface(els[i], NULL, rv, &(SDL_Rect){.x=x, .y=y});
					b->elems[i]->display=(SDL_Rect){.x=x, .y=y, .w=els[i]->w, .h=els[i]->h};
					x+=els[i]->w;
					if(y+els[i]->h>ymax)
						ymax=y+els[i]->h;
				}
			}
			else
			{
				unsigned int x=0, y=0, ymax=0;
				for(unsigned int i=0;i<b->nelems;i++)
				{
					if(!els[i]) continue;
					if(x+els[i]->w>e->w)
					{
						x=0;
						y=ymax;
					}
					b->elems[i]->display=(SDL_Rect){.x=x, .y=y, .w=els[i]->w, .h=els[i]->h};
					x+=els[i]->w;
					if(y+els[i]->h>ymax)
						ymax=y+els[i]->h;
				}
				rv=SDL_CreateRGBSurface(SDL_HWSURFACE, e->w, ymax, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
				SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, b->bgcolour.r, b->bgcolour.g, b->bgcolour.b, b->bgcolour.a));
				for(unsigned int i=0;i<b->nelems;i++)
					if(els[i])
						SDL_BlitSurface(els[i], NULL, rv, &b->elems[i]->display);
			}
		}
		else
		{
			unsigned int x=0, y=e->h;
			for(unsigned int i=0;i<b->nelems;i++)
				if(els[i])
				{
					x+=els[i]->w;
					if((unsigned int)els[i]->h>y) y=els[i]->h;
				}
			rv=SDL_CreateRGBSurface(SDL_HWSURFACE, x, y, screen->format->BitsPerPixel, screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
			SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, b->bgcolour.r, b->bgcolour.g, b->bgcolour.b, b->bgcolour.a));
			x=0;
			for(unsigned int i=0;i<b->nelems;i++)
				if(els[i])
				{
					SDL_BlitSurface(els[i], NULL, rv, &(SDL_Rect){.x=x, .y=0});
					b->elems[i]->display=(SDL_Rect){.x=x, .y=0, .w=els[i]->w, .h=els[i]->h};
					x+=els[i]->w;
				}
		}
	}
	for(unsigned int i=0;i<b->nelems;i++)
		SDL_FreeSurface(els[i]);
	free(els);
	return(rv);
}

SDL_Surface *atg_render_label(const atg_element *e)
{
	if(!ttfinit)
		initttf();
	if(!ttfinit) return(NULL);
	if(!e) return(NULL);
	if(!((e->type==ATG_LABEL)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_label *l=e->elem.label;
	if(!l) return(NULL);
	if((l->fontsize>MAXFONTSIZE)||!l->fontsize) return(NULL);
	if(!monottf[l->fontsize-1])
	{
		if(!(monottf[l->fontsize-1]=TTF_OpenFont(monofont, l->fontsize)))
			return(NULL);
	}
	SDL_Surface *text=TTF_RenderUTF8_Blended(monottf[l->fontsize-1], l->text, (SDL_Color){.r=l->colour.r, .g=l->colour.g, .b=l->colour.b, .unused=l->colour.a});
	if(e->w||e->h)
	{
		SDL_Surface *rv=atg_resize_surface(text, e);
		SDL_FreeSurface(text);
		return(rv);
	}
	return(text);
}

SDL_Surface *atg_render_image(const atg_element *e)
{
	if(!e) return(NULL);
	if(!((e->type==ATG_IMAGE)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_image *i=e->elem.image;
	if(!i) return(NULL);
	if(e->w||e->h)
	{
		return(atg_resize_surface(i->data, e));
	}
	SDL_Surface *rv=i->data;
	rv->refcount++;
	return(rv);
}

SDL_Surface *atg_render_button(const atg_element *e)
{
	if(!e) return(NULL);
	if(!((e->type==ATG_BUTTON)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_button *b=e->elem.button;
	if(!b) return(NULL);
	SDL_Surface *content=atg_render_box(&(atg_element){.w=(e->w>4)?e->w-4:0, .h=(e->h>4)?e->h-4:0, .type=ATG_BOX, .elem.box=b->content, .clickable=false, .userdata=NULL});
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

SDL_Surface *atg_render_spinner(const atg_element *e)
{
	if(!e) return(NULL);
	if(!((e->type==ATG_SPINNER)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_spinner *s=e->elem.spinner;
	if(!s) return(NULL);
	SDL_Surface *content=atg_render_box(&(atg_element){.w=e->w, .h=e->h, .type=ATG_BOX, .elem.box=s->content, .clickable=false, .userdata=NULL});
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

SDL_Surface *atg_render_element(const atg_element *e)
{
	if(!e) return(NULL);
	if(e->hidden) return(NULL);
	switch(e->type)
	{
		case ATG_BOX:
			return(atg_render_box(e));
		case ATG_LABEL:
			return(atg_render_label(e));
		case ATG_IMAGE:
			return(atg_render_image(e));
		case ATG_BUTTON:
			return(atg_render_button(e));
		case ATG_SPINNER:
			return(atg_render_spinner(e));
		case ATG_CUSTOM:
			if(e->render_callback)
				return(e->render_callback(e));
			/* fallthrough */
		default:
			return(NULL);
	}
}

void atg_flip(atg_canvas *canvas)
{
	if(!canvas) return;
	if(!canvas->surface) return;
	if(!canvas->box) return;
	SDL_Surface *box=atg_render_box(&(atg_element){.w=canvas->surface->w, .h=canvas->surface->h, .type=ATG_BOX, .elem.box=canvas->box, .clickable=true, .userdata=NULL});
	SDL_BlitSurface(box, NULL, canvas->surface, NULL);
	SDL_FreeSurface(box);
	SDL_Flip(canvas->surface);
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

static struct atg_event_list atg__ev_list={.list=NULL, .last=NULL};

void atg__match_click_recursive(struct atg_event_list *list, atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff)
{
	if(!element) return;
	if(
		(button.x>=element->display.x+xoff)
		&&(button.x<element->display.x+xoff+element->display.w)
		&&(button.y>=element->display.y+yoff)
		&&(button.y<element->display.y+yoff+element->display.h)
	)
	{
		if(element->clickable)
		{
			atg_ev_click click;
			click.e=element;
			click.pos=(atg_pos){.x=button.x-element->display.x, .y=button.y-element->display.y};
			click.button=button.button;
			atg__push_event(list, (atg_event){.type=ATG_EV_CLICK, .event.click=click});
		}
		switch(element->type)
		{
			case ATG_BOX:
			{
				atg_box *b=element->elem.box;
				if(!b->elems) return;
				for(unsigned int i=0;i<b->nelems;i++)
					atg__match_click_recursive(list, b->elems[i], button, xoff+element->display.x, yoff+element->display.y);
			}
			break;
			case ATG_BUTTON:
			{
				atg_ev_trigger trigger;
				trigger.e=element;
				trigger.button=button.button;
				atg__push_event(list, (atg_event){.type=ATG_EV_TRIGGER, .event.trigger=trigger});
			}
			break;
			case ATG_SPINNER:
			{
				atg_spinner *s=element->elem.spinner;
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
								if(e&&e->type==ATG_BUTTON)
								{
									atg_button *b=e->elem.button;
									if(b)
									{
										atg_box *box=b->content;
										if(box&&box->nelems&&box->elems)
										{
											atg_element *e=box->elems[0];
											if(e&&e->type==ATG_LABEL)
											{
												atg_label *l=e->elem.label;
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
				if(b->elems[0]&&b->elems[0]->type==ATG_LABEL)
				{
					atg_label *l=b->elems[0]->elem.label;
					free(l->text);
					l->text=malloc(32);
					if(l->text)
						snprintf(l->text, 32, s->fmt, s->value);
				}
				if(s->value!=oldval)
					atg__push_event(list, (atg_event){.type=ATG_EV_VALUE, .event.value=(atg_ev_value){.e=element, .value=s->value}});
			}
			break;
			case ATG_CUSTOM:
				if(element->match_click_callback)
					element->match_click_callback(list, element, button, xoff, yoff);
				/* fallthrough */
			default:
				/* ignore */
			break;
		}
	}
}

void atg__match_click(struct atg_event_list *list, atg_canvas *canvas, SDL_MouseButtonEvent button)
{
	if(!canvas) return;
	if(!canvas->box) return;
	atg_box *b=canvas->box;
	if(!b->elems) return;
	for(unsigned int i=0;i<b->nelems;i++)
		atg__match_click_recursive(list, b->elems[i], button, 0, 0);
}

int atg_poll_event(atg_event *event, atg_canvas *canvas)
{
	if(!event) return(atg__ev_list.list?1:SDL_PollEvent(NULL));
	if(!canvas) return(0);
	SDL_Event s;
	while(SDL_PollEvent(&s))
	{
		atg__push_event(&atg__ev_list, (atg_event){.type=ATG_EV_RAW, .event.raw=s});
		if(s.type==SDL_MOUSEBUTTONDOWN)
		{
			atg__match_click(&atg__ev_list, canvas, s.button);
		}
	}
	if(atg__ev_list.list)
	{
		*event=atg__ev_list.list->event;
		atg__event_list *next=atg__ev_list.list->next;
		free(atg__ev_list.list);
		if(atg__ev_list.last==atg__ev_list.list) atg__ev_list.last=NULL;
		atg__ev_list.list=next;
		return(1);
	}
	return(0);
}

atg_canvas *atg_create_canvas(unsigned int w, unsigned int h, atg_colour bgcolour)
{
	SDL_Surface *screen=gf_init(w, h);
	if(!screen) return(NULL);
	atg_canvas *rv=malloc(sizeof(atg_canvas));
	if(rv)
	{
		rv->surface=screen;
		rv->box=atg_create_box(ATG_BOX_PACK_VERTICAL, bgcolour);
	}
	return(rv);
}

int atg_resize_canvas(atg_canvas *canvas, unsigned int w, unsigned int h)
{
	if(!canvas) return(1);
	SDL_Surface *screen=SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE);
	if(!screen)
	{
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		return(2);
	}
	canvas->surface=screen;
	return(0);
}

atg_box *atg_create_box(Uint8 flags, atg_colour bgcolour)
{
	atg_box *rv=malloc(sizeof(atg_box));
	if(rv)
	{
		rv->flags=flags;
		rv->nelems=0;
		rv->elems=NULL;
		rv->bgcolour=bgcolour;
	}
	return(rv);
}

atg_label *atg_create_label(const char *text, unsigned int fontsize, atg_colour colour)
{
	atg_label *rv=malloc(sizeof(atg_label));
	if(rv)
	{
		rv->text=strdup(text);
		rv->fontsize=fontsize;
		rv->colour=colour;
	}
	return(rv);
}

atg_image *atg_create_image(SDL_Surface *img)
{
	atg_image *rv=malloc(sizeof(atg_image));
	if(rv)
	{
		(rv->data=img)->refcount++;
	}
	return(rv);
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
		char val[32];
		snprintf(val, sizeof(val), rv->fmt, rv->value);
		atg_element *lbl=atg_create_element_label(val, 15, fgcolour);
		if(!lbl)
		{
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		if(atg_pack_element(rv->content, lbl))
		{
			atg_free_element(lbl);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_element *vbox=atg_create_element_box(ATG_BOX_PACK_VERTICAL, bgcolour);
		if(!vbox)
		{
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		if(atg_pack_element(rv->content, vbox))
		{
			atg_free_element(vbox);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_box *v=vbox->elem.box;
		if(!v)
		{
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_element *ubtn=atg_create_element_button("+", fgcolour, bgcolour);
		if(!ubtn)
		{
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_button *ub=ubtn->elem.button;
		if(ub)
		{
			atg_box *b=ub->content;
			if(b&&b->nelems)
			{
				atg_element *e=b->elems[0];
				if(e&&e->type==ATG_LABEL)
				{
					atg_label *l=e->elem.label;
					if(l)
					{
						l->fontsize=7;
					}
				}
			}
		}
		if(atg_pack_element(v, ubtn))
		{
			atg_free_element(ubtn);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_element *dbtn=atg_create_element_button("-", fgcolour, bgcolour);
		if(!dbtn)
		{
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
		atg_button *db=dbtn->elem.button;
		if(db)
		{
			atg_box *b=db->content;
			if(b&&b->nelems)
			{
				atg_element *e=b->elems[0];
				if(e&&e->type==ATG_LABEL)
				{
					atg_label *l=e->elem.label;
					if(l)
					{
						l->fontsize=7;
					}
				}
			}
		}
		if(atg_pack_element(v, dbtn))
		{
			atg_free_element(dbtn);
			atg_free_box_box(rv->content);
			free(rv);
			return(NULL);
		}
	}
	return(rv);
}

atg_element *atg_create_element_box(Uint8 flags, atg_colour bgcolour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_box *b=atg_create_box(flags, bgcolour);
	if(!b)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type=ATG_BOX;
	rv->elem.box=b;
	rv->clickable=false;
	rv->hidden=false;
	rv->userdata=NULL;
	rv->render_callback=atg_render_box;
	rv->match_click_callback=NULL;
	rv->free_callback=atg_free_box;
	return(rv);
}

atg_element *atg_create_element_label(const char *text, unsigned int fontsize, atg_colour colour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_label *l=atg_create_label(text, fontsize, colour);
	if(!l)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type=ATG_LABEL;
	rv->elem.label=l;
	rv->clickable=false;
	rv->hidden=false;
	rv->userdata=NULL;
	rv->render_callback=atg_render_label;
	rv->match_click_callback=NULL;
	rv->free_callback=atg_free_label;
	return(rv);
}

atg_element *atg_create_element_image(SDL_Surface *img)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_image *i=atg_create_image(img);
	if(!i)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type=ATG_IMAGE;
	rv->elem.image=i;
	rv->clickable=false;
	rv->hidden=false;
	rv->userdata=NULL;
	rv->render_callback=atg_render_image;
	rv->match_click_callback=NULL;
	rv->free_callback=atg_free_image;
	return(rv);
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
	rv->type=ATG_BUTTON;
	rv->elem.button=b;
	rv->clickable=false; /* because it generates ATG_EV_TRIGGER events instead */
	rv->hidden=false;
	rv->userdata=NULL;
	rv->render_callback=atg_render_button;
	rv->match_click_callback=NULL;
	rv->free_callback=atg_free_button;
	return(rv);
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
	rv->type=ATG_SPINNER;
	rv->elem.spinner=s;
	rv->clickable=false; /* because it generates ATG_EV_VALUE events instead */
	rv->hidden=false;
	rv->userdata=NULL;
	rv->render_callback=atg_render_spinner;
	rv->match_click_callback=NULL;
	rv->free_callback=atg_free_spinner;
	return(rv);
}

int atg_pack_element(atg_box *box, atg_element *elem)
{
	unsigned int n=box->nelems++;
	atg_element **new=realloc(box->elems, box->nelems*sizeof(atg_element *));
	if(new)
	{
		(box->elems=new)[n]=elem;
		return(0);
	}
	box->nelems=n;
	return(1);
}

atg_element *atg_copy_element(const atg_element *e);

atg_box *atg_copy_box(const atg_box *b)
{
	if(!b) return(NULL);
	atg_box *rv=malloc(sizeof(atg_box));
	if(!rv) return(NULL);
	*rv=*b;
	for(unsigned int i=0;i<rv->nelems;i++)
		rv->elems[i]=atg_copy_element(b->elems[i]);
	return(rv);
}

atg_label *atg_copy_label(const atg_label *l)
{
	if(!l) return(NULL);
	atg_label *rv=malloc(sizeof(atg_label));
	if(!rv) return(NULL);
	*rv=*l;
	rv->text=l->text?strdup(l->text):NULL;
	return(rv);
}

atg_image *atg_copy_image(const atg_image *i)
{
	if(!i) return(NULL);
	atg_image *rv=malloc(sizeof(atg_image));
	if(!rv) return(NULL);
	*rv=*i;
	(rv->data=i->data)->refcount++;
	return(rv);
}

atg_button *atg_copy_button(const atg_button *b)
{
	if(!b) return(NULL);
	atg_button *rv=malloc(sizeof(atg_button));
	if(!rv) return(NULL);
	*rv=*b;
	rv->content=b->content?atg_copy_box(b->content):NULL;
	return(rv);
}

atg_element *atg_copy_element(const atg_element *e)
{
	if(!e) return(NULL);
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	*rv=*e;
	switch(rv->type)
	{
		case ATG_BOX:
			rv->elem.box=atg_copy_box(e->elem.box);
			return(rv);
		case ATG_LABEL:
			rv->elem.label=atg_copy_label(e->elem.label);
			return(rv);
		case ATG_IMAGE:
			rv->elem.image=atg_copy_image(e->elem.image);
			return(rv);
		case ATG_BUTTON:
			rv->elem.button=atg_copy_button(e->elem.button);
			return(rv);
		default:
			free(rv);
			return(NULL);
	}
}

void atg_free_canvas(atg_canvas *canvas)
{
	if(canvas)
	{
		SDL_FreeSurface(canvas->surface);
		atg_free_box_box(canvas->box);
	}
	free(canvas);
}

void atg_free_box(atg_element *e)
{
	if(!e) return;
	atg_box *box=e->elem.box;
	atg_free_box_box(box);
}

void atg_free_box_box(atg_box *box)
{
	if(box)
	{
		for(unsigned int e=0;e<box->nelems;e++)
			atg_free_element(box->elems[e]);
		free(box->elems);
	}
	free(box);
}

void atg_free_label(atg_element *e)
{
	if(!e) return;
	atg_label *label=e->elem.label;
	if(label)
	{
		free(label->text);
	}
	free(label);
}

void atg_free_image(atg_element *e)
{
	if(!e) return;
	atg_image *image=e->elem.image;
	if(image)
	{
		SDL_FreeSurface(image->data);
	}
	free(image);
}

void atg_free_button(atg_element *e)
{
	if(!e) return;
	atg_button *button=e->elem.button;
	if(button)
	{
		atg_free_box_box(button->content);
	}
	free(button);
}

void atg_free_spinner(atg_element *e)
{
	if(!e) return;
	atg_spinner *spinner=e->elem.spinner;
	if(spinner)
	{
		atg_free_box_box(spinner->content);
	}
	free(spinner);
}

void atg_free_element(atg_element *element)
{
	if(element)
	{
		switch(element->type)
		{
			case ATG_BOX:
				atg_free_box(element);
			break;
			case ATG_LABEL:
				atg_free_label(element);
			break;
			case ATG_IMAGE:
				atg_free_image(element);
			break;
			case ATG_BUTTON:
				atg_free_button(element);
			break;
			case ATG_SPINNER:
				atg_free_spinner(element);
			break;
			case ATG_CUSTOM:
				if(element->free_callback)
				{
					element->free_callback(element);
					break;
				}
				/* fallthrough */
			default:
				/* Bad things */
				fprintf(stderr, "Don't know how to free element of type %d, very bad things!\n", element->type);
			break;
		}
	}
	free(element);
}

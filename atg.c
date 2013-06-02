/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	atg.c: provides top-level abstract entry points
*/

#include "atg.h"
#include "atg_internals.h"

static int have_screen=0;

SDL_Surface *gf_init(unsigned int w, unsigned int h, Uint32 flags)
{
	if(have_screen)
	{
		fprintf(stderr, "gf_init: only one canvas at a time!\n");
		return(NULL);
	}
	SDL_Surface *screen;
	if(SDL_Init(SDL_INIT_VIDEO)<0)
	{
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return(NULL);
	}
	atexit(SDL_Quit);
	if(!(screen=SDL_SetVideoMode(w, h, 32, flags)))
	{
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		return(NULL);
	}
	have_screen++;
	return(screen);
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

int atg_poll_event(atg_event *event, atg_canvas *canvas)
{
	static struct atg_event_list atg__ev_list={.list=NULL, .last=NULL};
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
		else if(s.type==SDL_VIDEORESIZE)
		{
			SDL_ResizeEvent r=s.resize;
			atg_resize_canvas(canvas, r.w, r.h);
			atg_flip(canvas);
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
	return(atg_create_canvas_with_opts(w, h, bgcolour, SDL_HWSURFACE));
}

atg_canvas *atg_create_canvas_with_opts(unsigned int w, unsigned int h, atg_colour bgcolour, Uint32 flags)
{
	SDL_Surface *screen=gf_init(w, h, flags);
	if(!screen) return(NULL);
	atg_canvas *rv=malloc(sizeof(atg_canvas));
	if(rv)
	{
		rv->surface=screen;
		rv->box=atg_create_box(ATG_BOX_PACK_VERTICAL, bgcolour);
		rv->flags=flags;
	}
	else
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		have_screen--;
	}
	return(rv);
}

int atg_resize_canvas(atg_canvas *canvas, unsigned int w, unsigned int h)
{
	if(!canvas) return(1);
	SDL_Surface *screen=SDL_SetVideoMode(w, h, 32, canvas->flags);
	if(!screen)
	{
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		return(2);
	}
	canvas->surface=screen;
	return(0);
}

int atg_setopts_canvas(atg_canvas *canvas, Uint32 flags)
{
	if(!canvas) return(1);
	SDL_Surface *screen=SDL_SetVideoMode(canvas->surface->w, canvas->surface->h, 32, flags);
	if(!screen)
	{
		fprintf(stderr, "SDL_SetVideoMode: %s\n", SDL_GetError());
		return(2);
	}
	canvas->flags=flags;
	canvas->surface=screen;
	return(0);
}


int atg_pack_element(atg_box *box, atg_element *elem)
{
	if(!box) return(1);
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

int atg_ebox_pack(atg_element *ebox, atg_element *elem)
{
	if(!ebox) return(1);
	atg_box *b;
	switch(ebox->type)
	{
		case ATG_BOX:
			b=ebox->elem.box;
		break;
		case ATG_BUTTON:
		{
			atg_button *btn=ebox->elem.button;
			if(btn)
				b=btn->content;
			else
				return(1);
		}
		break;
		default:
			return(1);
	}
	return(atg_pack_element(b, elem));
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

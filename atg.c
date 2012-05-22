#include "atg.h"
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
	if((screen=SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE))==0)
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

SDL_Surface *atg_render_element(const atg_element *e);

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
	if(!screen) return(NULL); // can't find out display format
	if(!e) return(NULL);
	if(e->type!=ATG_BOX) return(NULL);
	atg_box *b=e->elem.box;
	if(!b) return(NULL);
	SDL_Surface **els=malloc(b->nelems*sizeof(SDL_Surface *)), *rv=NULL;
	if(!els) return(NULL);
	for(unsigned int i=0;i<b->nelems;i++)
		els[i]=atg_render_element(b->elems[i]);
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
				fprintf(stderr, "ABPV+h, no w\n");
			}
		}
		else
		{
			fprintf(stderr, "ABPV no h\n");
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
				fprintf(stderr, "ABPH+w, no h\n");
			}
		}
		else
		{
			unsigned int x=0, y=0;
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
	if(e->type!=ATG_LABEL) return(NULL);
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

SDL_Surface *atg_render_button(const atg_element *e)
{
	if(!e) return(NULL);
	if(e->type!=ATG_BUTTON) return(NULL);
	atg_button *b=e->elem.button;
	if(!b) return(NULL);
	SDL_Surface *content=atg_render_box(&(atg_element){.w=e->w, .h=e->h, .type=ATG_BOX, .elem.box=b->content, .clickable=false, .userdata=NULL});
	return(content);
}

SDL_Surface *atg_render_element(const atg_element *e)
{
	if(!e) return(NULL);
	switch(e->type)
	{
		case ATG_BOX:
			return(atg_render_box(e));
		case ATG_LABEL:
			return(atg_render_label(e));
		case ATG_BUTTON:
			return(atg_render_button(e));
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

typedef struct atg__event_list
{
	atg_event event;
	struct atg__event_list *next;
}
atg__event_list;

static atg__event_list *list=NULL, *last=NULL;

int atg__push_event(atg_event event)
{
	if(last)
	{
		if(!(last->next=malloc(sizeof(atg__event_list))))
			return(1);
		*(last=last->next)=(atg__event_list){.event=event, .next=NULL};
		return(0);
	}
	else if(list)
	{
		last=list;
		while(last->next) last=last->next;
		if(!(last->next=malloc(sizeof(atg__event_list))))
			return(1);
		*(last=last->next)=(atg__event_list){.event=event, .next=NULL};
		return(0);
	}
	else
	{
		if(!(last=list=malloc(sizeof(atg__event_list))))
			return(1);
		*last=(atg__event_list){.event=event, .next=NULL};
		return(0);
	}
}

void atg__match_click_recursive(atg_element *element, SDL_MouseButtonEvent button)
{
	if(!element) return;
	if(
		(button.x>=element->display.x)
		&&(button.x<element->display.x+element->display.w)
		&&(button.y>=element->display.y)
		&&(button.y<element->display.y+element->display.h)
	)
	{
		if(element->clickable)
		{
			atg_ev_click *click=malloc(sizeof(atg_ev_click));
			if(click)
			{
				click->e=element;
				click->pos=(atg_pos){.x=button.x-element->display.x, .y=button.y-element->display.y};
				click->button=button.button;
				if(atg__push_event((atg_event){.type=ATG_EV_CLICK, .event.click=click}))
					free(click);
			}
		}
		switch(element->type)
		{
			case ATG_BOX:;
				atg_box *b=element->elem.box;
				if(!b->elems) return;
				for(unsigned int i=0;i<b->nelems;i++)
					atg__match_click_recursive(b->elems[i], button);
			break;
			default:
				// ignore
			break;
		}
	}
}

void atg__match_click(atg_canvas *canvas, SDL_MouseButtonEvent button)
{
	if(!canvas) return;
	if(!canvas->box) return;
	atg_box *b=canvas->box;
	if(!b->elems) return;
	for(unsigned int i=0;i<b->nelems;i++)
		atg__match_click_recursive(b->elems[i], button);
}

int atg_poll_event(atg_event *event, atg_canvas *canvas)
{
	if(!event) return(list?1:SDL_PollEvent(NULL));
	if(!canvas) return(0);
	SDL_Event s;
	while(SDL_PollEvent(&s))
	{
		SDL_Event *sc=malloc(sizeof(SDL_Event));
		if(sc)
		{
			*sc=s;
			if(atg__push_event((atg_event){.type=ATG_EV_RAW, .event.raw=sc}))
				free(sc);
		}
		if(s.type==SDL_MOUSEBUTTONDOWN)
		{
			atg__match_click(canvas, s.button);
		}
	}
	if(list)
	{
		*event=list->event;
		atg__event_list *next=list->next;
		free(list);
		if(last==list) last=NULL;
		list=next;
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

atg_button *atg_create_button(const char *label, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_button *rv=malloc(sizeof(atg_button));
	if(rv)
	{
		rv->content=atg_create_box(ATG_BOX_PACK_HORIZONTAL, bgcolour);
		if(rv->content)
		{
			atg_element *l=atg_create_element_label(label, 12, fgcolour);
			if(l)
			{
				if(atg_pack_element(rv->content, l))
				{
					atg_free_element(l);
					atg_free_box(rv->content);
					free(rv);
					return(NULL);
				}
			}
			else
			{
				atg_free_box(rv->content);
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
	rv->userdata=NULL;
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
	rv->clickable=true;
	rv->userdata=NULL;
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
		atg_free_box(canvas->box);
	}
	free(canvas);
}

void atg_free_box(atg_box *box)
{
	if(box)
	{
		for(unsigned int e=0;e<box->nelems;e++)
			atg_free_element(box->elems[e]);
		free(box->elems);
	}
	free(box);
}

void atg_free_label(atg_label *label)
{
	if(label)
	{
		free(label->text);
	}
	free(label);
}

void atg_free_button(atg_button *button)
{
	if(button)
	{
		atg_free_box(button->content);
	}
	free(button);
}

void atg_free_element(atg_element *element)
{
	if(element)
	{
		switch(element->type)
		{
			case ATG_BOX:
				atg_free_box(element->elem.box);
			break;
			case ATG_LABEL:
				atg_free_label(element->elem.label);
			break;
			case ATG_BUTTON:
				atg_free_button(element->elem.button);
			break;
			default:
				/* Bad things */
			break;
		}
	}
	free(element);
}

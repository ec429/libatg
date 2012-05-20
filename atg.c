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
	/*if(b->nelems==1) // special case for testing, until I write the box packing code
	{
		SDL_Surface *el=atg_render_element(b->elems[0]);
		if(e->w||e->h)
		{
			SDL_Surface *rv=atg_resize_surface(el, e);
			SDL_FreeSurface(el);
			el=rv;
		}
		SDL_Surface *rv=SDL_CreateRGBSurface(SDL_HWSURFACE, el->w, el->h, el->format->BitsPerPixel, el->format->Rmask, el->format->Gmask, el->format->Bmask, el->format->Amask);
		SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, b->bgcolour.r, b->bgcolour.g, b->bgcolour.b, b->bgcolour.a));
		SDL_BlitSurface(el, NULL, rv, NULL);
		return(rv);
	}*/
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
				unsigned int y=0, x=0, xmax=0;
				for(unsigned int i=0;i<b->nelems;i++)
				{
					if(!els[i]) continue;
					if(y+els[i]->h>e->h)
					{
						y=0;
						x=xmax;
					}
					SDL_BlitSurface(els[i], NULL, rv, &(SDL_Rect){.x=x, .y=y});
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
		fprintf(stderr, "ABPH\n");
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
	SDL_Surface *text=TTF_RenderUTF8_Blended(monottf[l->fontsize-1], l->text.d, (SDL_Color){.r=l->colour.r, .g=l->colour.g, .b=l->colour.b, .unused=l->colour.a});
	if(e->w||e->h)
	{
		SDL_Surface *rv=atg_resize_surface(text, e);
		SDL_FreeSurface(text);
		return(rv);
	}
	return(text);
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

atg_label *atg_create_label(const atg_string text, unsigned int fontsize, atg_colour colour)
{
	atg_label *rv=malloc(sizeof(atg_label));
	if(rv)
	{
		rv->text=atg_strdup(text);
		rv->fontsize=fontsize;
		rv->colour=colour;
	}
	return(rv);
}

atg_element *atg_create_element_label(const atg_string text, unsigned int fontsize, atg_colour colour)
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

void append_char(atg_string *s, char c)
{
	if(s->l&&s->d)
	{
		s->d[s->i++]=c;
	}
	else
	{
		*s=init_string();
		append_char(s, c);
	}
	char *nbuf=s->d;
	if(s->i>=s->l)
	{
		if(s->i)
			s->l=s->i*2;
		else
			s->l=80;
		nbuf=(char *)realloc(s->d, s->l);
	}
	if(nbuf)
	{
		s->d=nbuf;
		s->d[s->i]=0;
	}
	else
	{
		free(s->d);
		*s=init_string();
	}
}

void append_str(atg_string *s, const char *str)
{
	while(str && *str) // not the most tremendously efficient implementation, but conceptually simple at least
	{
		append_char(s, *str++);
	}
}

void append_string(atg_string *s, const atg_string t)
{
	if(!s) return;
	size_t i=s->i+t.i;
	if(i>=s->l)
	{
		size_t l=i+1;
		char *nbuf=(char *)realloc(s->d, l);
		if(!nbuf) return;
		s->l=l;
		s->d=nbuf;
	}
	memcpy(s->d+s->i, t.d, t.i);
	s->d[i]=0;
	s->i=i;
}

atg_string init_string(void)
{
	atg_string s;
	s.d=(char *)malloc(s.l=80);
	if(s.d)
		s.d[0]=0;
	else
		s.l=0;
	s.i=0;
	return(s);
}

atg_string null_string(void)
{
	return((atg_string){.d=NULL, .l=0, .i=0});
}

atg_string make_string(const char *str)
{
	atg_string s=init_string();
	append_str(&s, str);
	return(s);
}

atg_string atg_strdup(const atg_string string)
{
	atg_string rv;
	if(!(rv.d=malloc(string.i+1)))
	{
		rv.l=rv.i=0;
		return(rv);
	}
	rv.l=string.i+1;
	rv.i=string.i;
	memcpy(rv.d, string.d, string.i);
	rv.d[rv.i]=0;
	return(rv);
}

atg_string atg_const_string(char *str)
{
	return((atg_string){.d=str, .l=0, .i=strlen(str)});
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
	rv->text=atg_strdup(l->text);
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
		atg_free_string(label->text);
	}
	free(label);
}

void atg_free_string(atg_string string)
{
	if(string.l)
		free(string.d);
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
			default:
				/* Bad things */
			break;
		}
	}
	free(element);
}

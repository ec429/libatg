/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	w_label.c: implements the LABEL widget
*/
#include "atg.h"
#include "atg_internals.h"
#include <SDL_ttf.h>

#define MAXFONTSIZE	24
bool ttfinit=false;
#ifdef WINDOWS
const char *monofont="./LiberationMono-Regular.ttf";
#else // !WINDOWS
#ifdef MONOFONTPATH
const char *monofont=MONOFONTPATH;
#else // !MONOFONT
const char *monofont="/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf";
#endif // MONOFONT
#endif // WINDOWS
TTF_Font *monottf[MAXFONTSIZE];

void initttf(void)
{
	if(TTF_Init()) return;
	for(unsigned int i=0;i<MAXFONTSIZE;i++)
		monottf[i]=NULL;
	ttfinit=true;
}

SDL_Surface *atg_render_label(const atg_element *e)
{
	if(!ttfinit)
		initttf();
	if(!ttfinit) return(NULL);
	if(!e) return(NULL);
	atg_label *l=e->elemdata;
	if(!l) return(NULL);
	if(!l->text) return(NULL);
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

atg_label *atg_create_label_nocopy(char *text, unsigned int fontsize, atg_colour colour)
{
	atg_label *rv=malloc(sizeof(atg_label));
	if(rv)
	{
		rv->text=text;
		rv->fontsize=fontsize;
		rv->colour=colour;
	}
	return(rv);
}

atg_label *atg_create_label(const char *text, unsigned int fontsize, atg_colour colour)
{
	if(text)
	{
		atg_label *rv=NULL;
		char *dtext=strdup(text);
		if(dtext)
			rv=atg_create_label_nocopy(dtext, fontsize, colour);
		return(rv);
	}
	else
	{
		return(atg_create_label_nocopy(NULL, fontsize, colour));
	}
}

atg_element *atg_copy_label(const atg_element *e)
{
	if(!e) return(NULL);
	atg_label *l=e->elemdata;
	if(!l) return(NULL);
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	*rv=*e;
	atg_label *l2=rv->elemdata=malloc(sizeof(atg_label));
	if(!l2)
	{
		free(rv);
		return(NULL);
	}
	*l2=*l;
	l2->text=l->text?strdup(l->text):NULL;
	return(rv);
}

void atg_free_label(atg_element *e)
{
	if(!e) return;
	atg_label *label=e->elemdata;
	if(label)
		free(label->text);
	free(label);
}

atg_element *atg_create_element_label_nocopy(char *text, unsigned int fontsize, atg_colour colour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_label *l=atg_create_label_nocopy(text, fontsize, colour);
	if(!l)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type="__builtin_label";
	rv->elemdata=l;
	rv->clickable=false;
	rv->hidden=false;
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_label;
	rv->match_click_callback=NULL;
	rv->pack_callback=NULL;
	rv->copy_callback=atg_copy_label;
	rv->free_callback=atg_free_label;
	return(rv);
}

atg_element *atg_create_element_label(const char *text, unsigned int fontsize, atg_colour colour)
{
	if(text)
	{
		atg_element *rv=NULL;
		char *dtext=strdup(text);
		if(dtext)
			rv=atg_create_element_label_nocopy(dtext, fontsize, colour);
		return(rv);
	}
	else
	{
		return(atg_create_element_label_nocopy(NULL, fontsize, colour));
	}
}

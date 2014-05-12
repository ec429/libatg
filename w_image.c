/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	w_image.c: implements the IMAGE widget
*/
#include "atg.h"
#include "atg_internals.h"

SDL_Surface *atg_render_image(const atg_element *e)
{
	if(!e) return(NULL);
	atg_image *i=e->elemdata;
	if(!i) return(NULL);
	if(e->w||e->h)
	{
		return(atg_resize_surface(i->data, e));
	}
	SDL_Surface *rv=i->data;
	rv->refcount++;
	return(rv);
}

atg_image *atg_create_image(SDL_Surface *img)
{
	atg_image *rv=malloc(sizeof(atg_image));
	if(rv)
	{
		if((rv->data=img))
			img->refcount++;
	}
	return(rv);
}

atg_element *atg_copy_image(const atg_element *e)
{
	if(!e) return(NULL);
	atg_image *i=e->elemdata;
	if(!i) return(NULL);
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	*rv=*e;
	atg_image *i2=rv->elemdata=malloc(sizeof(atg_image));
	if(!i2)
	{
		free(rv);
		return(NULL);
	}
	*i2=*i;
	if(i2->data)
		i2->data->refcount++;
	return(rv);
}

void atg_free_image(atg_element *e)
{
	if(!e) return;
	atg_image *image=e->elemdata;
	if(image)
		SDL_FreeSurface(image->data);
	free(image);
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
	rv->elemdata=i;
	rv->clickable=false;
	rv->hidden=false;
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_image;
	rv->match_click_callback=NULL;
	rv->pack_callback=NULL;
	rv->copy_callback=atg_copy_image;
	rv->free_callback=atg_free_image;
	return(rv);
}

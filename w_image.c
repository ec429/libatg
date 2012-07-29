#include "atg.h"
#include "atg_internals.h"

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
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_image;
	rv->match_click_callback=NULL;
	rv->free_callback=atg_free_image;
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
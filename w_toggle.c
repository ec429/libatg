/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	w_toggle.c: implements the TOGGLE widget
*/
#include "atg.h"
#include "atg_internals.h"

SDL_Surface *atg_render_toggle(const atg_element *e)
{
	if(!e) return(NULL);
	if(!((e->type==ATG_TOGGLE)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_toggle *t=e->elem.toggle;
	if(!t) return(NULL);
	atg_box *b=t->content;
	if(b)
	{
		b->bgcolour=t->state?t->fgcolour:t->bgcolour;
		atg_element *le=b->nelems?b->elems[0]:NULL;
		if(le&&le->type==ATG_LABEL)
		{
			atg_label *l=le->elem.label;
			if(l)
			{
				l->colour=t->state?t->bgcolour:t->fgcolour;
			}
		}
	}
	return(atg_render_button(&(atg_element){.type=ATG_BUTTON, .w=e->w, .h=e->h, .elem.button=&(atg_button){.fgcolour=t->state?t->bgcolour:t->fgcolour, .content=t->content}}));
}

void atg_click_toggle(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, __attribute__((unused)) unsigned int xoff, __attribute__((unused)) unsigned int yoff)
{
	atg_ev_toggle toggle;
	toggle.e=element;
	toggle.button=button.button;
	atg_toggle *t=element?element->elem.toggle:NULL;
	if(t)
		toggle.state=(t->state=!t->state);
	else
		toggle.state=false;
	atg__push_event(list, (atg_event){.type=ATG_EV_TOGGLE, .event.toggle=toggle});
}

atg_toggle *atg_create_toggle(const char *label, bool state, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_toggle *rv=malloc(sizeof(atg_button));
	if(rv)
	{
		rv->fgcolour=fgcolour;
		rv->bgcolour=bgcolour;
		rv->content=atg_create_box(ATG_BOX_PACK_HORIZONTAL, state?fgcolour:bgcolour);
		if(rv->content)
		{
			atg_element *l=atg_create_element_label(label, 12, state?bgcolour:fgcolour);
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
	rv->state=state;
	return(rv);
}

atg_element *atg_create_element_toggle(const char *label, bool state, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_toggle *t=atg_create_toggle(label, state, fgcolour, bgcolour);
	if(!t)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type=ATG_TOGGLE;
	rv->elem.toggle=t;
	rv->clickable=false; /* because it generates ATG_EV_TOGGLE events instead */
	rv->hidden=false;
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_toggle;
	rv->match_click_callback=atg_click_toggle;
	rv->copy_callback=atg_copy_toggle;
	rv->free_callback=atg_free_toggle;
	return(rv);
}

atg_element *atg_copy_toggle(const atg_element *e)
{
	if(!e) return(NULL);
	if(!((e->type==ATG_TOGGLE)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_toggle *t=e->elem.toggle;
	if(!t) return(NULL);
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	*rv=*e;
	atg_toggle *t2=rv->elem.toggle=malloc(sizeof(atg_toggle));
	if(!t2)
	{
		free(rv);
		return(NULL);
	}
	*t2=*t;
	t2->content=t->content?atg_copy_box_box(t->content):NULL;
	return(rv);
}

void atg_free_toggle(atg_element *e)
{
	if(!e) return;
	atg_toggle *t=e->elem.toggle;
	if(t)
	{
		atg_free_box_box(t->content);
	}
	free(t);
}

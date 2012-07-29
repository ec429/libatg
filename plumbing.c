#include "atg.h"
#include "atg_internals.h"

SDL_Surface *atg_render_element(atg_element *e)
{
	if(!e) return(NULL);
	if(e->hidden) return(NULL);
	if(e->cache&&e->cached)
	{
		e->cached->refcount++;
		return(e->cached);
	}
	SDL_Surface *rv=NULL;
	switch(e->type)
	{
		case ATG_BOX:
			rv=atg_render_box(e);
		break;
		case ATG_LABEL:
			rv=atg_render_label(e);
		break;
		case ATG_IMAGE:
			rv=atg_render_image(e);
		break;
		case ATG_BUTTON:
			rv=atg_render_button(e);
		break;
		case ATG_SPINNER:
			rv=atg_render_spinner(e);
		break;
		case ATG_TOGGLE:
			rv=atg_render_toggle(e);
		break;
		case ATG_CUSTOM:
			if(e->render_callback)
				rv=e->render_callback(e);
		break;
		default:
		break;
	}
	if(e->cache)
	{
		e->cached=rv;
		if(rv) rv->refcount++;
	}
	return(rv);
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
				atg_click_box(list, element, button, xoff, yoff);
			break;
			case ATG_BUTTON:
				atg_click_button(list, element, button, xoff, yoff);
			break;
			case ATG_SPINNER:
				atg_click_spinner(list, element, button, xoff, yoff);
			break;
			case ATG_TOGGLE:
				atg_click_toggle(list, element, button, xoff, yoff);
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

atg_element *atg_copy_element(const atg_element *e)
{
	if(!e) return(NULL);
	switch(e->type)
	{
		case ATG_BOX:
			return(atg_copy_box(e));
		case ATG_LABEL:
			return(atg_copy_label(e));
		case ATG_IMAGE:
			return(atg_copy_image(e));
		case ATG_BUTTON:
			return(atg_copy_button(e));
		case ATG_SPINNER:
			return(atg_copy_spinner(e));
		case ATG_TOGGLE:
			return(atg_copy_toggle(e));
		case ATG_CUSTOM:
			if(e->copy_callback)
				return(e->copy_callback(e));
		default:
			return(NULL);
	}
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
			case ATG_TOGGLE:
				atg_free_toggle(element);
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
	SDL_FreeSurface(element->cached);
	free(element);
}

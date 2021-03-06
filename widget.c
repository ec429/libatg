/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	widget: an example of a custom widget
*/

#include <stdio.h>
#include <SDL.h>
#include "atg.h"
#include "atg_internals.h"

/* Prototype for the constructor of the selector widget */
atg_element *create_selector(unsigned int *sel);

/* Selector widget and button to print current selected value to stdout */
int main(void)
{
	/* Set up the atg canvas */
	atg_canvas *canvas=atg_create_canvas(116, 36, (atg_colour){15, 15, 15, ATG_ALPHA_OPAQUE});
	if(!canvas)
	{
		fprintf(stderr, "atg_create_canvas failed\n");
		return(1);
	}
	atg_element *mainbox=canvas->content;
	if(!mainbox)
	{
		fprintf(stderr, "mainbox is NULL!\n");
		return(1);
	}
	else
	{
		atg_box *mb=mainbox->elemdata;
		if(!mb)
		{
			fprintf(stderr, "mainbox->elemdata is NULL!\n");
			return(1);
		}
		mb->flags=ATG_BOX_PACK_VERTICAL; /* Pack the elements vertically */
	}
	unsigned int sel=0; /* 'selector' value */
	/* Create the selector widget */
	atg_element *sel_widget=create_selector(&sel);
	if(!sel_widget)
	{
		fprintf(stderr, "create_selector failed\n");
		return(1);
	}
	/* Pack it into the mainbox */
	if(atg_ebox_pack(mainbox, sel_widget))
		perror("atg_pack_element");
	/* Create the button */
	atg_element *btn=atg_create_element_button("Print sel value", (atg_colour){255, 255, 255, ATG_ALPHA_OPAQUE}, (atg_colour){0, 0, 0, ATG_ALPHA_OPAQUE});
	if(!btn)
	{
		fprintf(stderr, "atg_create_element_button failed\n");
		return(1);
	}
	/* Pack it into the mainbox */
	if(atg_ebox_pack(mainbox, btn))
		perror("atg_pack_element");
	/* Main event loop */
	atg_event e;
	int errupt=0;
	while(!errupt)
	{
		atg_flip(canvas);
		while(atg_poll_event(&e, canvas))
		{
			switch(e.type)
			{
				case ATG_EV_RAW:; /* For quit events */
					SDL_Event s=e.event.raw;
					switch(s.type)
					{
						case SDL_QUIT: /* Quit the program */
							errupt++;
						break;
					}
				break;
				case ATG_EV_TRIGGER:; /* Button presses */
					atg_ev_trigger trigger=e.event.trigger;
					/* If it's the "Print..." button, then print */
					if(trigger.e==btn)
						printf("sel = %u\n", sel);
				break;
				case ATG_EV_VALUE:; /* Value change event */
					atg_ev_value value=e.event.value;
					/* If it's the selector widget, write the value to stderr */
					if(value.e==sel_widget)
						fprintf(stderr, "value %u\n", value.value);
				break;
				default:
					/* ignore the event */
				break;
			}
		}
	}
	/* Clean up and exit */
	atg_free_canvas(canvas);
	return(0);
}

/* Data for the selector */
const char *sel_labels[4]={"NONE","LOW","MED","HIGH"};
atg_colour sel_colours[4]={{31, 31, 95, 0}, {95, 31, 31, 0}, {95, 95, 15, 0}, {31, 159, 31, 0}};

/* Prototype for the selector renderer */
SDL_Surface *selector_render_callback(const struct atg_element *e);

/* Prototype for the click handler */
void selector_match_click_callback(struct atg_event_list *list, atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff);

/* Function to create a custom 'selector' widget, which behaves like a radiobutton list */
atg_element *create_selector(unsigned int *sel)
{
	atg_element *rv=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, (atg_colour){63, 63, 63, ATG_ALPHA_OPAQUE}); /* Start with an atg_box */
	if(!rv) return(NULL);
	rv->render_callback=selector_render_callback; /* Connect up our renderer callback */
	rv->match_click_callback=selector_match_click_callback; /* Connect up our click-handling callback */
	for(unsigned int i=0;i<4;i++) /* The widget is a row of four buttons */
	{
		atg_colour fg=sel_colours[i];
		/* Create the button */
		atg_element *btn=atg_create_element_button(sel_labels[i], fg, (atg_colour){63, 63, 63, ATG_ALPHA_OPAQUE});
		if(!btn)
		{
			atg_free_element(rv);
			return(NULL);
		}
		/* Pack it into the box */
		if(atg_ebox_pack(rv, btn))
		{
			atg_free_element(btn);
			atg_free_element(rv);
			return(NULL);
		}
	}
	rv->userdata=sel; /* sel stores the currently selected value */
	return(rv);
}

/* Function to render the 'selector' widget */
SDL_Surface *selector_render_callback(const struct atg_element *e)
{
	if(!e) return(NULL);
	atg_box *b=e->elemdata;
	if(!b) return(NULL);
	if(!b->elems) return(NULL);
	/* Set the background colours */
	for(unsigned int i=0;i<b->nelems;i++)
	{
		if(b->elems[i]&&!strcmp(b->elems[i]->type, "__builtin_button"))
		{
			atg_button *btn=b->elems[i]->elemdata;
			if(e->userdata)
			{
				if(*(unsigned int *)e->userdata==i)
					btn->content->bgcolour=(atg_colour){159, 159, 159, ATG_ALPHA_OPAQUE};
				else
					btn->content->bgcolour=(atg_colour){31, 31, 31, ATG_ALPHA_OPAQUE};
			}
			else
			{
				btn->content->bgcolour=(atg_colour){63, 63, 63, ATG_ALPHA_OPAQUE};
			}
		}
	}
	/* Hand off the actual rendering to atg_render_box */
	return(atg_render_box(e));
}

/* Function to handle clicks within the 'selector' widget */
void selector_match_click_callback(struct atg_event_list *list, atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff)
{
	atg_box *b=element->elemdata;
	if(!b->elems) return;
	struct atg_event_list sub_list={.list=NULL, .last=NULL}; /* Sub-list to catch all the events generated by our child elements */
	for(unsigned int i=0;i<b->nelems;i++) /* For each child element... */
		atg__match_click_recursive(&sub_list, b->elems[i], button, xoff+element->display.x, yoff+element->display.y); /* ...pass it the click and let it generate trigger events into our sub-list */
	unsigned int oldsel=0;
	if(element->userdata) oldsel=*(unsigned int *)element->userdata;
	while(sub_list.list) /* Iterate over the sub-list */
	{
		atg_event event=sub_list.list->event;
		if(event.type==ATG_EV_TRIGGER)
		/* We're only interested in trigger events */
		{
			if(event.event.trigger.button==ATG_MB_LEFT)
			/* Left-click on a button selects that value */
			{
				for(unsigned int i=0;i<b->nelems;i++)
				{
					if(event.event.trigger.e==b->elems[i])
					{
						if(element->userdata) *(unsigned int *)element->userdata=i;
					}
				}
			}
			else if(event.event.trigger.button==ATG_MB_SCROLLDN)
			/* Scrolling over the selector cycles through the values */
			{
				if(element->userdata) *(unsigned int *)element->userdata=(1+*(unsigned int *)element->userdata)%b->nelems;
			}
			else if(event.event.trigger.button==ATG_MB_SCROLLUP)
			/* Cycle in the opposite direction */
			{
				if(element->userdata) *(unsigned int *)element->userdata=(b->nelems-1+*(unsigned int *)element->userdata)%b->nelems;
			}
		}
		/* Get the next element from the sub-list */
		atg__event_list *next=sub_list.list->next;
		free(sub_list.list);
		sub_list.list=next;
	}
	if(element->userdata&&(*(unsigned int *)element->userdata!=oldsel))
	{
		atg__push_event(list, (atg_event){.type=ATG_EV_VALUE, .event.value=(atg_ev_value){.e=element, .value=*(unsigned int *)element->userdata}});
	}
}

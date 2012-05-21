#include <stdio.h>
#include <SDL.h>
#include "atg.h"

int main(void)
{
	atg_canvas *canvas=atg_create_canvas(320, 240, (atg_colour){15, 15, 15, ATG_ALPHA_OPAQUE});
	if(!canvas)
	{
		fprintf(stderr, "atg_create_canvas failed\n");
		return(1);
	}
	atg_box *mainbox=canvas->box;
	atg_element *hello=atg_create_element_label("Hello World!", 12, (atg_colour){255, 255, 0, ATG_ALPHA_OPAQUE});
	if(hello)
	{
		if(atg_pack_element(mainbox, hello))
			perror("atg_pack_element");
		for(unsigned int i=0;i<20;i++)
			if(atg_pack_element(mainbox, atg_copy_element(hello)))
				perror("atg_pack_element");
		hello->clickable=true;
	}
	else
		fprintf(stderr, "atg_create_element_label failed\n");
	atg_event e;
	int errupt=0;
	while(!errupt)
	{
		atg_flip(canvas);
		while(atg_poll_event(&e, canvas))
		{
			switch(e.type)
			{
				case ATG_EV_RAW:;
					SDL_Event *s=e.event.raw;
					if(s)
					{
						switch(s->type)
						{
							case SDL_QUIT:
								errupt++;
							break;
						}
					}
					else
					{
						fprintf(stderr, "e.event.raw==NULL!\n");
					}
				break;
				case ATG_EV_CLICK:;
					atg_ev_click *click=e.event.click;
					if(click)
					{
						if(click->e)
						{
							switch(click->e->type)
							{
								case ATG_LABEL:;
									atg_label *l=click->e->elem.label;
									if(l)
									{
										l->colour=(atg_colour){255, 0, 0, ATG_ALPHA_OPAQUE};
									}
								break;
								default:
									fprintf(stderr, "Clicked on an element.type==%u\n", click->e->type);
								break;
							}
						}
						else
						{
							fprintf(stderr, "click->e==NULL!\n");
						}
					}
				break;
				default:
					fprintf(stderr, "Unrecognised event type %u\n", e.type);
				break;
			}
		}
	}
	atg_free_canvas(canvas);
	return(0);
}

/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	test: A test program for the basic functionality
*/

#include <stdio.h>
#include <SDL.h>
#include "atg.h"
#include <sys/stat.h>

int main(void)
{
	atg_canvas *canvas=atg_create_canvas(640, 480, (atg_colour){15, 15, 15, ATG_ALPHA_OPAQUE});
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
		for(unsigned int i=0;i<21;i++)
			if(atg_pack_element(mainbox, atg_copy_element(hello)))
				perror("atg_pack_element");
		hello->clickable=true;
	}
	else
		fprintf(stderr, "atg_create_element_label failed\n");
	atg_element *spin=atg_create_element_spinner(ATG_SPINNER_RIGHTCLICK_STEP10, 0, 100, 1, 10, "%03d", (atg_colour){255, 255, 255, ATG_ALPHA_OPAQUE}, (atg_colour){47, 47, 47, ATG_ALPHA_OPAQUE});
	if(spin)
	{
		if(atg_pack_element(mainbox, spin))
			perror("atg_pack_element");
	}
	else
		fprintf(stderr, "atg_create_element_spinner failed\n");
	atg_element *fp=atg_create_element_filepicker("Stat file", NULL, (atg_colour){255, 255, 255, ATG_ALPHA_OPAQUE}, (atg_colour){47, 47, 47, ATG_ALPHA_OPAQUE});
	if(fp)
	{
		fp->h=360;
		if(atg_pack_element(mainbox, fp))
			perror("atg_pack_element");
	}
	else
		fprintf(stderr, "atg_create_element_filepicker failed\n");
	atg_element *go=atg_create_element_button("Go!", (atg_colour){255, 255, 255, ATG_ALPHA_OPAQUE}, (atg_colour){47, 47, 47, ATG_ALPHA_OPAQUE});
	if(go)
	{
		if(atg_pack_element(mainbox, go))
			perror("atg_pack_element");
	}
	else
		fprintf(stderr, "atg_create_element_button failed\n");
	atg_event e;
	int errupt=0;
	bool going=false;
	atg_element *stop=NULL;
	unsigned int colcycle=0;
	while(!errupt)
	{
		if(going&&mainbox&&mainbox->elems)
		{
			for(unsigned int i=0;i<mainbox->nelems;i++)
			{
				atg_element *e=mainbox->elems[i];
				if(e&&(e->type==ATG_LABEL))
				{
					atg_label *l=e->elem.label;
					if(l)
					{
						switch(colcycle)
						{
							case 0:
								l->colour=(atg_colour){255, 255, 127, ATG_ALPHA_OPAQUE};
							break;
							case 1:
								l->colour=(atg_colour){127, 255, 255, ATG_ALPHA_OPAQUE};
							break;
							case 2:
								l->colour=(atg_colour){255, 127, 255, ATG_ALPHA_OPAQUE};
							break;
						}
						colcycle=(colcycle+1)%3;
					}
				}
			}
		}
		atg_flip(canvas);
		while(atg_poll_event(&e, canvas))
		{
			switch(e.type)
			{
				case ATG_EV_RAW:;
					SDL_Event s=e.event.raw;
					switch(s.type)
					{
						case SDL_QUIT:
							errupt++;
						break;
					}
				break;
				case ATG_EV_CLICK:;
					atg_ev_click click=e.event.click;
					if(click.e)
					{
						switch(click.e->type)
						{
							case ATG_LABEL:;
								atg_label *l=click.e->elem.label;
								if(l)
								{
									switch(click.button)
									{
										case ATG_MB_LEFT:
											l->colour=(atg_colour){0, 255, 0, ATG_ALPHA_OPAQUE};
										break;
										case ATG_MB_RIGHT:
											l->colour=(atg_colour){255, 0, 0, ATG_ALPHA_OPAQUE};
										break;
										default:
											l->colour=(atg_colour){255, 255, 0, ATG_ALPHA_OPAQUE};
										break;
									}
								}
							break;
							default:
								fprintf(stderr, "Clicked on an element.type==%u\n", click.e->type);
							break;
						}
					}
					else
					{
						fprintf(stderr, "click.e==NULL!\n");
					}
				break;
				case ATG_EV_TRIGGER:;
					atg_ev_trigger trigger=e.event.trigger;
					if(trigger.e)
					{
						if(trigger.e==go)
						{
							if(!stop)
							{
								stop=atg_create_element_button("Stop!", (atg_colour){255, 63, 0, ATG_ALPHA_OPAQUE}, (atg_colour){0, 15, 47, ATG_ALPHA_OPAQUE});
								if(stop)
								{
									if(atg_pack_element(mainbox, stop))
										perror("atg_pack_element");
									else
										going=true;
								}
								else
									fprintf(stderr, "atg_create_element_button failed\n");
							}
							else
								going=true;
						}
						else if(trigger.e==stop)
						{
							going=false;
							atg_resize_canvas(canvas, 240, 160);
						}
						else if(trigger.e==fp)
						{
							atg_filepicker *f=fp->elem.filepicker;
							if(f&&f->curdir&&f->value)
							{
								size_t n=strlen(f->curdir), m=strlen(f->value);
								char file[n+m+1];
								snprintf(file, n+m+1, "%s%s", f->curdir, f->value);
								struct stat st;
								if(stat(file, &st))
								{
									fprintf(stderr, "Failed to stat %s:\n", file);
									perror("\tstat");
								}
								else
								{
									printf("sizeof %s = %zuB\n", file, (size_t)st.st_size);
								}
							}
						}
						else
						{
							fprintf(stderr, "trigger.e not recognised!\n");
						}
					}
					else
					{
						fprintf(stderr, "trigger.e==NULL!\n");
					}
				break;
				case ATG_EV_VALUE:;
					atg_ev_value value=e.event.value;
					if(value.e)
					{
						if(value.e==spin)
						{
							printf("Spinner value set to %d\n", value.value);
						}
						else
						{
							fprintf(stderr, "value.e not recognised!\n");
						}
					}
					else
					{
						fprintf(stderr, "value.e==NULL!\n");
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

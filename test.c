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
	atg_element *hello=atg_create_element_label(atg_const_string("Hello World!"), 12, (atg_colour){255, 255, 0, ATG_ALPHA_OPAQUE});
	if(hello)
	{
		if(atg_pack_element(mainbox, hello))
			perror("atg_pack_element");
		for(unsigned int i=0;i<20;i++)
		if(atg_pack_element(mainbox, atg_copy_element(hello)))
			perror("atg_pack_element");
	}
	else
		fprintf(stderr, "atg_create_element_label failed\n");
	atg_flip(canvas);
	getchar();
	atg_free_canvas(canvas);
	return(0);
}

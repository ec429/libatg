/*
	atg - a tiny GUI toolkit for SDL
	Copyright (C) 2012 Edward Cree
	
	See atg.h for license information
	w_filepicker.c: implements the FILEPICKER widget
*/
#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif /* ifndef GNU_SOURCE */
#include "atg.h"
#include "atg_internals.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

// TODO: sort entries in atg_render_filepicker

#define CWD_BUF_SIZE	4096

static const char *filters_dir;

static int filter_dirs(const struct dirent *d)
{
	if(d->d_name[0]=='.' && (strcmp(d->d_name, "..")||!strcmp(filters_dir, "/"))) return(0);
	size_t n=strlen(d->d_name)+3, m=strlen(filters_dir);
	struct stat st;
	char fullpath[m+n];
	snprintf(fullpath, m+n, "%s%s", filters_dir, d->d_name);
	if(stat(fullpath, &st)) return(0);
	if(st.st_mode&S_IFDIR) return(1);
	return(0);
}

static int filter_files(const struct dirent *d)
{
	if(d->d_name[0]=='.') return(0);
	size_t n=strlen(d->d_name)+3, m=strlen(filters_dir);
	struct stat st;
	char fullpath[m+n];
	snprintf(fullpath, m+n, "%s%s", filters_dir, d->d_name);
	if(stat(fullpath, &st)) return(0);
	if(st.st_mode&S_IFDIR) return(0);
	return(1);
}

static int filter_stats(const struct dirent *d)
{
	if(d->d_name[0]=='.') return(0);
	size_t n=strlen(d->d_name)+3, m=strlen(filters_dir);
	struct stat st;
	char fullpath[m+n];
	snprintf(fullpath, m+n, "%s%s", filters_dir, d->d_name);
	if(stat(fullpath, &st)) return(1);
	return(0);
}

SDL_Surface *atg_render_filepicker(const atg_element *e)
{
	if(!e) return(NULL);
	if(!((e->type==ATG_FILEPICKER)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_filepicker *f=e->elem.filepicker;
	if(!f) return(NULL);
	if(!f->curdir) return(NULL);
	atg_box *b=f->content;
	if(!b) return(NULL);
	if(b->nelems>1&&b->elems[1]&&b->elems[1]->type==ATG_LABEL)
	{
		atg_label *l=b->elems[1]->elem.label;
		free(l->text);
		l->text=strdup(f->curdir);
	}
	if(b->nelems>2&&b->elems[2]&&b->elems[2]->type==ATG_BOX)
	{
		b->elems[2]->h=(e->h>28)?e->h-28:0;
		atg_free_box_box(b->elems[2]->elem.box);
		atg_box *b2=b->elems[2]->elem.box=atg_create_box(ATG_BOX_PACK_VERTICAL, f->bgcolour);
		if(!b2) return(NULL);
		struct dirent **dirs, **files, **stats;
		filters_dir=f->curdir;
		int ndirs=scandir(f->curdir, &dirs, filter_dirs, versionsort);
		if(ndirs==-1) return(NULL);
		int nfiles=scandir(f->curdir, &files, filter_files, versionsort);
		if(nfiles==-1)
		{
			while(ndirs--)
				free(dirs[ndirs]);
			free(dirs);
			return(NULL);
		}
		int nstats=scandir(f->curdir, &stats, filter_stats, versionsort);
		if(nstats==-1)
		{
			while(files--)
				free(files[nfiles]);
			free(files);
			while(ndirs--)
				free(dirs[ndirs]);
			free(dirs);
			return(NULL);
		}
		for(int n=0;n<ndirs;n++)
		{
			size_t l=strlen(dirs[n]->d_name);
			char lbl[l+2];
			snprintf(lbl, l+2, "%s/", dirs[n]->d_name);
			atg_element *btn=atg_create_element_button(lbl, f->fgcolour, f->bgcolour);
			if(btn)
				atg_pack_element(b2, btn);
			free(dirs[n]);
		}
		free(dirs);
		for(int n=0;n<nfiles;n++)
		{
			bool sel=f->value&&!strcmp(files[n]->d_name, f->value);
			atg_element *btn=atg_create_element_button(files[n]->d_name, sel?f->bgcolour:f->fgcolour, sel?f->fgcolour:f->bgcolour);
			if(btn)
				atg_pack_element(b2, btn);
			free(files[n]);
		}
		free(files);
		for(int n=0;n<nstats;n++)
		{
			size_t l=strlen(stats[n]->d_name);
			char lbl[l+3];
			snprintf(lbl, l+3, "! %s", stats[n]->d_name);
			atg_element *btn=atg_create_element_label(lbl, 14, f->fgcolour);
			if(btn)
				atg_pack_element(b2, btn);
			free(stats[n]);
		}
		free(stats);
	}
	SDL_Surface *content=atg_render_box(&(atg_element){.w=e->w, .h=e->h, .type=ATG_BOX, .elem.box=f->content, .clickable=false, .userdata=NULL});
	if(!content) return(NULL);
	SDL_Surface *rv=SDL_CreateRGBSurface(SDL_HWSURFACE, content->w, content->h, content->format->BitsPerPixel, content->format->Rmask, content->format->Gmask, content->format->Bmask, content->format->Amask);
	if(!rv)
	{
		SDL_FreeSurface(content);
		return(NULL);
	}
	SDL_FillRect(rv, &(SDL_Rect){.x=0, .y=0, .w=rv->w, .h=rv->h}, SDL_MapRGBA(rv->format, f->content->bgcolour.r, f->content->bgcolour.g, f->content->bgcolour.b, f->content->bgcolour.a));
	SDL_BlitSurface(content, NULL, rv, &(SDL_Rect){.x=0, .y=0});
	SDL_FreeSurface(content);
	return(rv);
}

void atg_click_filepicker(struct atg_event_list *list, struct atg_element *element, SDL_MouseButtonEvent button, unsigned int xoff, unsigned int yoff)
{
	atg_filepicker *f=element->elem.filepicker;
	if(!f) return;
	atg_box *b=f->content;
	if(!b) return;
	if(!b->elems) return;
	struct atg_event_list sub_list={.list=NULL, .last=NULL};
	for(unsigned int i=0;i<b->nelems;i++)
		atg__match_click_recursive(&sub_list, b->elems[i], button, xoff+element->display.x, yoff+element->display.y);
	while(sub_list.list)
	{
		atg_event event=sub_list.list->event;
		if(event.type==ATG_EV_TRIGGER)
		{
			atg_element *e=event.event.trigger.e;
			if(e&&e->type==ATG_BUTTON)
			{
				atg_button *btn=e->elem.button;
				if(btn)
				{
					atg_box *box=btn->content;
					if(box&&box->nelems&&box->elems)
					{
						atg_element *e=box->elems[0];
						if(e&&e->type==ATG_LABEL)
						{
							atg_label *l=e->elem.label;
							if(l&&l->text)
							{
								size_t n=strlen(l->text);
								if(l->text[n-1]=='/')
								{
									if(strcmp(l->text, "../")==0)
									{
										if(f->curdir)
										{
											size_t m=strlen(f->curdir);
											f->curdir[m-1]=0;
											char *s=strrchr(f->curdir, '/');
											if(s) s[1]=0;
										}
									}
									else
									{
										char *newdir=malloc(strlen(f->curdir)+strlen(l->text)+1);
										sprintf(newdir, "%s%s", f->curdir, l->text);
										free(f->curdir);
										f->curdir=newdir;
									}
									free(f->value);
									f->value=NULL;
								}
								else
								{
									f->value=strdup(l->text);
								}
							}
						}
					}
				}
				atg__push_event(list, (atg_event){.type=ATG_EV_VALUE, .event.value=(atg_ev_value){.e=element, .value=0}});
			}
		}
		atg__event_list *next=sub_list.list->next;
		free(sub_list.list);
		sub_list.list=next;
	}
}

atg_filepicker *atg_create_filepicker(const char *title, const char *dir, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_filepicker *rv=malloc(sizeof(atg_filepicker));
	if(rv)
	{
		rv->title=strdup(title?title:NULL);
		if(dir)
			rv->curdir=strdup(dir);
		else
		{
			rv->curdir=malloc(CWD_BUF_SIZE);
			if(!(rv->curdir&&getcwd(rv->curdir, CWD_BUF_SIZE)))
			{
				free(rv->title);
				free(rv->curdir);
				free(rv);
				return(NULL);
			}
		}
		size_t n=strlen(rv->curdir);
		while(n&&rv->curdir[n-1]=='/')
			rv->curdir[--n]=0;
		char *new=realloc(rv->curdir, n+1);
		if(!new)
		{
			free(rv->title);
			free(rv->curdir);
			free(rv);
			return(NULL);
		}
		(rv->curdir=new)[n++]='/';
		rv->curdir[n]=0;
		rv->value=NULL;
		rv->fgcolour=fgcolour;
		rv->bgcolour=bgcolour;
		rv->content=atg_create_box(ATG_BOX_PACK_VERTICAL, bgcolour);
		if(!rv->content)
		{
			free(rv->title);
			free(rv->curdir);
			free(rv);
			return(NULL);
		}
		atg_element *l_title=atg_create_element_label(title, 12, fgcolour);
		if(!l_title)
		{
			atg_free_box_box(rv->content);
			free(rv->title);
			free(rv->curdir);
			free(rv);
			return(NULL);
		}
		if(atg_pack_element(rv->content, l_title))
		{
			atg_free_element(l_title);
			atg_free_box_box(rv->content);
			free(rv->title);
			free(rv->curdir);
			free(rv);
			return(NULL);
		}
		atg_element *l_dir=atg_create_element_label(rv->curdir, 12, fgcolour);
		if(!l_dir)
		{
			atg_free_box_box(rv->content);
			free(rv->title);
			free(rv->curdir);
			free(rv);
			return(NULL);
		}
		if(atg_pack_element(rv->content, l_dir))
		{
			atg_free_element(l_dir);
			atg_free_box_box(rv->content);
			free(rv->title);
			free(rv->curdir);
			free(rv);
			return(NULL);
		}
		atg_element *vbox=atg_create_element_box(ATG_BOX_PACK_VERTICAL, bgcolour);
		if(!vbox)
		{
			atg_free_box_box(rv->content);
			free(rv->title);
			free(rv->curdir);
			free(rv);
			return(NULL);
		}
		if(atg_pack_element(rv->content, vbox))
		{
			atg_free_element(vbox);
			atg_free_box_box(rv->content);
			free(rv->title);
			free(rv->curdir);
			free(rv);
			return(NULL);
		}
	}
	return(rv);
}

atg_element *atg_create_element_filepicker(const char *title, const char *dir, atg_colour fgcolour, atg_colour bgcolour)
{
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	atg_filepicker *f=atg_create_filepicker(title, dir, fgcolour, bgcolour);
	if(!f)
	{
		free(rv);
		return(NULL);
	}
	rv->w=rv->h=0;
	rv->type=ATG_FILEPICKER;
	rv->elem.filepicker=f;
	rv->clickable=false;
	rv->hidden=false;
	rv->cache=false;
	rv->cached=NULL;
	rv->userdata=NULL;
	rv->render_callback=atg_render_filepicker;
	rv->match_click_callback=atg_click_filepicker;
	rv->copy_callback=atg_copy_filepicker;
	rv->free_callback=atg_free_filepicker;
	return(rv);
}

atg_element *atg_copy_filepicker(const atg_element *e)
{
	if(!e) return(NULL);
	if(!((e->type==ATG_FILEPICKER)||(e->type==ATG_CUSTOM))) return(NULL);
	atg_filepicker *f=e->elem.filepicker;
	if(!f) return(NULL);
	atg_element *rv=malloc(sizeof(atg_element));
	if(!rv) return(NULL);
	*rv=*e;
	atg_filepicker *f2=rv->elem.filepicker=malloc(sizeof(atg_filepicker));
	if(!f2)
	{
		free(rv);
		return(NULL);
	}
	*f2=*f;
	f2->content=f->content?atg_copy_box_box(f->content):NULL;
	f2->title=f->title?strdup(f->title):NULL;
	f2->curdir=f->curdir?strdup(f->curdir):NULL;
	f2->value=f->value?strdup(f->value):NULL;
	return(rv);
}

void atg_free_filepicker(atg_element *e)
{
	if(!e) return;
	atg_filepicker *f=e->elem.filepicker;
	if(f)
	{
		atg_free_box_box(f->content);
		free(f->title);
		free(f->curdir);
		free(f->value);
	}
	free(f);
}

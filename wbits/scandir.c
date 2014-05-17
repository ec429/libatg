/* scandir implementation based on Cygwin's scandir.cc
   Copyright 2014 Edward Cree, licensed under GNU GPLv3
*/

/* scandir.cc

   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.

   Written by Corinna Vinschen <corinna.vinschen@cityweb.de>
*/

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int scandir(const char *dir, struct dirent ***namelist, int (*select)(const struct dirent *), int (*compar)(const struct dirent **, const struct dirent **))
{
	DIR *dirp;
	struct dirent *ent, *etmp, **nl=NULL, **ntmp;
	int count=0;
	int allocated=0;

	if(!(dirp=opendir(dir)))
		return(-1);

	errno=0;
	while((ent=readdir(dirp)))
	{
		if(!select||select(ent))
		{
			if(count==allocated)
			{
				if(!allocated)
					allocated=10;
				else
					allocated*=2;

				ntmp=realloc(nl, allocated*sizeof(*nl));
				if(!ntmp)
					break;
				nl=ntmp;
			}

			if(!(etmp=malloc(sizeof(*ent))))
				break;
			*etmp=*ent;
			nl[count++]=etmp;
		}
	}

	closedir(dirp);

	if(errno)
	{
		int e=errno;
		if(nl)
		{
			while(count>0)
				free(nl[--count]);
			free(nl);
		}
		errno=e;
		return(-1);
	}

	qsort(nl, count, sizeof(*nl), (int (*)(const void *, const void *))compar);
	if(namelist)
		*namelist=nl;
	return count;
}

int alphasort(const struct dirent **a, const struct dirent **b)
{
	return strcoll((*a)->d_name, (*b)->d_name);
}

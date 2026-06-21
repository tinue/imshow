/*
 *  im : A General Purpose Image (File) Format
 *
 *  Version 0.93(beta)    12/12/1989
 *
 *  Authors: Dieter Schaufelberger, Urs Meyer
 *
 *  Copyright July 1989
 *
 *  University of Zurich, Dept. of Computer Science, Multi-Media Lab,
 *  Winterthurerstrasse 190, CH-8057 Zurich
 */

#include "imlib.h"

/* functions for setting the values of the image structure */

int imsettype(img, type)
	IMAGE *img;
	imushort type;
{
	img->type = type;
	return(1);
}  /* imsettype */


int imsetxsize(img, xsize)
	IMAGE *img;
	imushort xsize;
{
	img->xsize = xsize;
	return(1);
}  /* imsetxsize */


int imsetysize(img, ysize)
	IMAGE *img;
	imushort ysize;
{
	img->ysize = ysize;
	return(1);
}  /* imsetysize */


int imsetstor(img, stor)
	IMAGE *img;
	imushort stor;
{
	img->storage = stor;
	return(1);
}  /* imsetstor */


/*
 * int imsetdepth(img, depth)
 */
 
int imsetdepth(img, depth)
	IMAGE *img;
	imushort depth;
{
	img->depth = depth;
	return(1);
}  /* imsetdepth */


/*
 * int imsetsamples(img, samples)
 */
 
int imsetsamples(img, samples)
	IMAGE *img;
	imushort samples;
{
	img->samples = samples;
	return(1);
}  /* imsetsamples */


int imsetcolors(img, colors)
	IMAGE *img;
	int colors;
{
	img->colors = colors - 1;
	return(1);
}  /* imsetcolors */


void imsetcmap(img, cmap)
	IMAGE *img;
	IMPIXEL *cmap;
{
	img->colormap = cmap;
}  /* imsetcmap */


imushort imgettype(img)
	IMAGE *img;
{
	return(img->type);
}  /* imgettype */


imushort imgetxsize(img)
	IMAGE *img;
{
	return(img->xsize);
}  /* imgetxsize */


imushort imgetysize(img)
	IMAGE *img;
{
	return(img->ysize);
}  /* imgetysize */


imushort imgetstor(img)
	IMAGE *img;
{
	return(img->storage);
}  /* imgetstor */


imushort imgetdepth(img)
	IMAGE *img;
{
	return(img->depth);
}  /* imgetdepth */


imushort imgetsamples(img)
	IMAGE *img;
{
	return(img->samples);
}  /* imgetsamples */


int imgetcolors(img)
	IMAGE *img;
{
	return(img->colors + 1);
}  /* imgetcolors */


IMPIXEL *imgetcmap(img)
	IMAGE *img;
{
	return(img->colormap);
}  /* imgetcmap */


/* long imgetrowsize(width, samples, depth)
 * calculates the size of a row in bytes for a row of 'width' pixels 
 * with the depth 'depth' and 'samples' samples per pixel
 */
 
long imgetrowsize(width, samples, depth)
	imushort width, samples, depth;
{
	int pixperbyte;       /* # of pixels that can be stored in one byte */
	long size;            
	
	if ((pixperbyte = 8 / depth) == 0)
        size = (depth / 8) * width * samples;
	else
		size = ((width + pixperbyte - 1) / pixperbyte) * samples;
	
	return(size);
}  /* imgetrowsize */
       

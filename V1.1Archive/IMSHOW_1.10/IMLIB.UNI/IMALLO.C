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

static void fillzero( /* IMBYTE *p, long l */ );
static IMBYTE *imalloc( /* long size */ );

/* functions for storage allocation */   

static void fillzero(p, l)
	IMBYTE *p;
	long l;
{
    while (l--)
		*p++ = 0;
}  /* fillzero */


/* 
 * IMBYTE *imalloc(size)
 * allocates 'size' bytes and calls a function to zero the bytes 
 */
 
static IMBYTE *imalloc(size)
	long size;
{
	IMBYTE *newmem;
	
	if ((newmem = ((IMBYTE *) malloc(size))) == NULL)
		imerror(NoMemerr);
	else
		fillzero(newmem, size);  
	return (newmem);
}  /* imalloc */

  
/*
 * IMBYTE *imrowalloc(width, samples, depth)
 * calls imgetrowsize(), which calculates the size of the row in  
 * bytes and calls then imalloc() to allocate memory for the row
 */

IMBYTE *imrowalloc(width, samples, depth)
	imushort width, samples, depth;
{
	IMBYTE *row;
	long size;
	
	size = imgetrowsize(width, samples, depth);
	row = imalloc(size);
	return(row);
}  /* imrowalloc */


/* 
 * IMBYTE **impicalloc(width, height, samples, depth)
 * calls imgetrowsize(), which calculates the size of the rows in  
 * bytes and calls then imalloc to allocate memory for the number 
 * of rows in the image.
 */

IMBYTE **impicalloc(width, height, samples, depth)
	imushort width, height, samples, depth;
{
	IMBYTE **row, **image;
	long size;
	
	size = imgetrowsize(width, samples, depth);   
	row = image = (IMBYTE **) imalloc((long) (height * sizeof(IMBYTE *)));
	while (height--)
		*row++ = imalloc(size);
	return(image);
}  /* impicalloc */


/*
 * IMPIXEL *immapalloc(colors)
 * allocates memory for a colormap
 */

IMPIXEL *immapalloc(colors)
	int colors;
{
	IMPIXEL *map;
	
	map = (IMPIXEL *) imalloc((long) (colors * sizeof(IMPIXEL)));
	return(map);
}  /* immapalloc */
  

/*
 * char *imfltalloc(samples, depth)
 * allocates memory for a float pixel
 */

char *imfltalloc(samples, depth)
	imushort samples, depth;
{
	char *f;
	
	f = (char *) imalloc((long) ((depth / 8) * samples));
	return(f);
}  /* imfltalloc */
  


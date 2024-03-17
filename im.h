/*
 *  im : A General Purpose Image (File) Format
 *
 *  Version 1.0        9/01/1990
 *
 *  Authors: Dieter Schaufelberger, Urs Meyer
 *
 *  Copyright July 1989
 *
 *  University of Zurich, Dept. of Computer Science, Multi-Media Lab,
 *  Winterthurerstrasse 190, CH-8057 Zurich
 */

#include <stdio.h>
#include "imgen.h"
 
/* definitions for error handling */

extern int imerrnum;       /* contains the error number */

char imerrors [20][40] = 
	{
/*      " maximal length error message string    "   */
		"no error",
		"out of memory",
		"wrong image type",
		"wrong file mode",
		"file open returned NULL",
		"file is not an im file",
		"color is not in colormap",
		"colormap index out of range",
		"different major version numbers",
		"different minor version numbers",
		"not enough data for scanline",
		"output file is larger then input file",
		"image type should be IMMAP",
		"wrong storage type",
		"xsize of the image is out of range",
		"ysize of the image is out of range",
		"depth of the image is out of range",
		"# of samples is out of range",
		"# of colors is out of range",
		"image depth is to small for # of colors"
	};


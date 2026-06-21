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

#include <stdio.h>
#include "imgen.h"

#define MSC

#define MAXSAMPLE   8
#define MAXDEPTH    64

#define MAJLIBVN    0
#define MINLIBVN    93
#define IM_MAGIC    1301
#define IM_HDRSIZE  512

#ifdef MSC
#define IM_RMODE  "rb"
#define IM_WMODE  "wb"
#else
#define IM_RMODE  "r"
#define IM_WMODE  "w"
#endif

/* definitions for error handling */

extern int imerrnum;       /* contains the error number */

/* set the error number  */
#define imerror(en)  (imerrnum = en)

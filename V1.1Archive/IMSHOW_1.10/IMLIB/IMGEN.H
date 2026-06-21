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


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define imushort  unsigned short

/* image type values */

#define IMGRY   0x02             /* greyscale image */
#define IMMAP   0x04             /* color-mapped image */
#define IMRGB   0x08             /* RGB true color image */
#define IMFLT   0x10             /* floating point, not supported */

/* storage type values */

#define IMRAW   0x01             /* store uncompressed */
#define IMLZW   0x02             /* LZW compression */
#define IMRLE   0x04             /* RLE compression */

#define IMTYP   0x0F             /* mask for first four bits */

typedef unsigned char IMBYTE;    /* used for GRY and MAP */
typedef long          IMPIXEL;   /* used for 24-bit RGB */

/* image definition structure */

typedef struct
    {
     /* contents of file header */

     imushort  magic;              /* file verification */
     imushort  majfilvn;           /* major file version number */
     imushort  minfilvn;           /* minor file version number */
     imushort  type;               /* bitmap type */
     imushort  storage;            /* storage method used */
     imushort  xsize;              /* width of image */
     imushort  ysize;              /* height of image */
     imushort  samples;            /* samples per pixel */
     imushort  depth;              /* bits per sample */
     imushort  colors;             /* # of colors in colormap */

     /* used in memory only */

     imushort psize;           /* bytes per pixel */
     IMPIXEL *colormap;        /* pointer to colormap */
     FILE    *fp;              /* pointer to image file */
     char    *filemode;        /* file mode (read or write) */
     char    *pack_state;      /* pointer to private pack/unpack structure */
     IMBYTE **fltbytes;        /* pointer to private representation of a float pixel */
    }
     IMAGE;


/* definitions for error handling */

#define NoError                 0               /* no error */
#define NoMemerr                1               /* out of memory */
#define ImageTyperr             2               /* wrong image type */
#define FileModerr              3               /* wrong file mode */
#define FileOpenerr             4               /* file open returned NULL */
#define ImFilerr                5               /* file is not an im file */
#define ColMaperr               6               /* color is not in colormap */
#define CmapInderr              7               /* colormap index out of range */
#define MajorVNerr              8               /* different major version numbers */
#define MinorVNerr              9               /* different minor version numbers */
#define LZWDecoderr             10              /* not enough data for scanline */
#define LZWRatioerr             11              /* output file is larger then input file */
#define MapTyperr               12              /* image type should be IMMAP */
#define StorTyperr              13              /* wrong storage type */
#define XSizerr                 14              /* xsize of the image is out of range */
#define YSizerr                 15              /* ysize of the image is out of range */
#define Deptherr                16              /* depth of the image is out of range */
#define Samplerr                17              /* # of samples is out of range */
#define Colorserr               18              /* # of colors is out of range */
#define MapDeptherr             19              /* image depth is to small for # of colors */


/* external functions */
   /* functions for setting the image structure contents */

extern int cdecl imsettype( IMAGE *img, imushort type );
extern int cdecl imsetxsize( IMAGE *img, imushort xsize );
extern int cdecl imsetysize( IMAGE *img, imushort ysize );
extern int cdecl imsetstor( IMAGE *img, imushort storage );
extern int cdecl imsetdepth( IMAGE *img, imushort depth );
extern int cdecl imsetsamples( IMAGE *img, imushort samples );
extern int cdecl imsetcolors( IMAGE *img, int colors );
extern void cdecl imsetcmap( IMAGE *img, IMPIXEL *cmap );

   /* functions for getting the image structure contents */

extern imushort cdecl imgettype( IMAGE *img );
extern imushort cdecl imgetxsize( IMAGE *img );
extern imushort cdecl imgetysize( IMAGE *img );
extern imushort cdecl imgetstor( IMAGE *img );
extern imushort cdecl imgetdepth( IMAGE *img );
extern imushort cdecl imgetsamples( IMAGE *img );
extern int cdecl imgetcolors( IMAGE *img );
extern IMPIXEL * cdecl imgetcmap( IMAGE *img );
extern long cdecl imgetrowsize( imushort width, imushort samples, imushort depth );

   /* functions for storage allocation */

extern IMBYTE * cdecl imrowalloc( imushort width, imushort samples, imushort depth );
extern IMBYTE ** cdecl impicalloc( imushort width, imushort height, imushort samples, imushort depth );
extern IMPIXEL * cdecl immapalloc( int colors );
extern char * cdecl imfltalloc( imushort samples, imushort depth );

   /* functions for open and close */

extern IMAGE * cdecl imopen( char *filename, char *mode );
extern void cdecl imclose( IMAGE *img );
extern void cdecl imflush( IMAGE *img, IMBYTE *rowbuf );

   /* functions for reading and writing */

extern int cdecl imrhdr( IMAGE *img );
extern int cdecl imwhdr( IMAGE *img );

extern int cdecl imrcmap( IMAGE *img );
extern int cdecl imwcmap( IMAGE *img );

extern long cdecl imrrow( IMAGE *img, IMBYTE *rowbuf );
extern long cdecl imwrow( IMAGE *img, IMBYTE *rowbuf );

extern long cdecl imrpic( IMAGE *img, IMBYTE **impic );
extern long cdecl imwpic( IMAGE *img, IMBYTE **impic );

   /* functions for ideal pixels */

extern void cdecl imsplitrgb( IMPIXEL rgbval,IMBYTE *red,IMBYTE *green,IMBYTE *blue );
extern IMPIXEL cdecl immergergb( IMBYTE red, IMBYTE green, IMBYTE blue );
extern IMBYTE  cdecl imrgb2grey( IMPIXEL rgbval, imushort depth );
extern IMBYTE  cdecl imnormalize( IMBYTE val,int srcbits,int dstbits );

extern IMPIXEL cdecl imrpix( IMAGE *img, IMBYTE *rowbuf, int xpos );
extern IMBYTE  cdecl imrgry( IMAGE *img, IMBYTE *rowbuf, int xpos );
extern void    cdecl imrflt( IMAGE *img, IMBYTE *rowbuf, char *pixarr, int xpos );

extern void cdecl imwpix( IMAGE *img, IMBYTE *rowbuf, IMPIXEL rgbval, int xpos );
extern void cdecl imwgry( IMAGE *img, IMBYTE *rowbuf, IMBYTE greyval, int xpos );
extern void cdecl imwflt( IMAGE *img, IMBYTE *rowbuf, char *pixarr, int xpos );

extern int cdecl imridx( IMAGE *img, IMBYTE *rowbuf, int xpos );
extern int cdecl imwidx( IMAGE *img, IMBYTE *rowbuf, int idxval, int xpos );

extern IMPIXEL cdecl imrcolor( IMAGE *img, int xpos );
extern int     cdecl imwcolor( IMAGE *img, IMPIXEL color, int pos );

   /* transformation functions */

extern void cdecl impx2byte( IMAGE *img, IMPIXEL *lonrow, IMBYTE *rowbuf );
extern void cdecl imbyte2px( IMAGE *img, IMBYTE *rowbuf, IMPIXEL *lonrow );

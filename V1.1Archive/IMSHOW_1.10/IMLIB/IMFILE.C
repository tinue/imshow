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
#include "impack.h"
#include <string.h>
#include <malloc.h>

#define getshort(s,fp) (s = 256*fgetc(fp), s += fgetc(fp))

/* functions for image opening and closing */

/*
 * IMAGE *imopen(filename, type)
 * opens a file with the name 'filename' for reading ('type' == "r")
 * or writing ('type' == "w").
 * if no filename is specified or the filename is "-", standard input
 * (or standard output) is opened
 */

IMAGE *imopen(filename, type)
        char   *filename, *type;
{
        IMAGE  *img;
        FILE   *fp;
        int createit;

        imerror(NoError);

        if (strcmp(type, "r") == 0)                             /* determine type */
        {
                createit = FALSE;
                type = IM_RMODE;
        }
        else if (strcmp(type, "w") == 0)
                {
                        createit = TRUE;
                        type = IM_WMODE;
                }
                else
                {
                        imerror(FileModerr);
                        return(NULL);
                }

        if (filename == NULL || strlen(filename) == 0 || (strcmp(filename, "-") == 0))
                if (createit)
                        fp = stdout;
                else
                        fp = stdin;
        else                                                    /* regular filename */
                fp = fopen(filename, type);

        if (fp == NULL)
        {
                imerror(FileOpenerr);
                return(NULL);
        }                                       /* error in fopen */

        if ( (img = (IMAGE *) malloc(sizeof(IMAGE))) == NULL )
        {
                imerror(NoMemerr);
                return(NULL);           /* not enough memory */
        }

        img->fp = fp;
        img->filemode = type;
        img->pack_state = NULL;
        img->fltbytes = NULL;

        if (createit)
        {
                img->magic = IM_MAGIC;
                img->majfilvn = MAJLIBVN;
                img->minfilvn = MINLIBVN;
                img->colors = 0;
                img->colormap = NULL;
        }
        else
        {
                getshort(img->magic, img->fp);
                if (img->magic != IM_MAGIC)
                {
                        imerror(ImFilerr);
                        return(NULL);
                }
        }

        return(img);
}   /* imopen */


/*
 * closes the file of the image 'img'
 */

void imclose(img)
        IMAGE *img;
{
        LZWState *lzs;

        if (img->storage == IMLZW)
        {
                lzs = (LZWState *) img->pack_state;
                if (strcmp(img->filemode, IM_WMODE) == 0)
                        LZWPostEncode(lzs);
                LZWDone(lzs);
        }

        fclose(img->fp);

        if ((img->type & IMTYP) == IMMAP)
                free(img->colormap);

        free(img);
}  /* imclose */


/*
 * writes the contents of rowbuf to the file and then calls fflush
 * to write the file buffer to the file
 */

void imflush(img, rowbuf)
        IMAGE  *img;
        IMBYTE *rowbuf;
{
        imwrow(img, rowbuf);
        fflush(img->fp);
}  /* imflush */


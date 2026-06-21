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

static IMBYTE packgreypx( IMBYTE byo, IMBYTE val, int xpos, int depth );
static IMBYTE unpackgreypx( IMBYTE byo, int xpos, int depth );
static void flt2byte( IMAGE *img, char *pixarr, imushort size );
static void byte2flt(IMAGE *img, imushort size );

/* ideal pixel functions */


/*
 * static IMBYTE packgreypx(byo, val, xpos, depth)
 * Packs the bits of the grey pixel stored in 'val' into the byte 'byo'.
 * 'xpos' is the position of the pixel in the image row, 'depth'
 * is the number of significant bits that represent the pixel. Those
 * significant bits are the 'depth' lowest bits in val. All the other
 * bits have to be zero.
 * Example: byo = 10111011, val = 00000011, depth = 2, xpos = 6
 */

static IMBYTE packgreypx(byo, val, xpos, depth)
        IMBYTE byo;
        IMBYTE val;
        int xpos;
        int depth;
{
        int d;
        IMBYTE by1, by2;

        by1 = by2 = byo;

        d = ((8 / depth) - (xpos % (8 / depth)) - 1) * depth;

/* The expression ((8 / depth) - (xpos % (8 / depth)) - 1) calculates
 * at which pixel position within the byte the new pixel has to be.
 * The pixel position 0 denotes the pixel that occupies the lowest
 * bits of the byte. In our example the value of the expression is
 * 1, i.e. the pixel is the second from the right side. */

        by1 <<= (8 - d);
        by1 >>= (8 - d);

/* by1 now contains the bits that are on the right side of the bits of
 * the new pixel: by1 = 00000011 */

        by2 >>= (depth + d);
        by2 <<= (depth + d);

/* by2 now contains the bits that are on the left side of the bits of
 * the new pixel: by2 = 10110000 */

        by1 |= by2;

/* by1 now contains the bits that are on the left and on the right
 * side of the bits of the new pixel. The bits where the new pixel
 * will be stored are zero: by1 = 10110011 */

        val <<= d;

/* The bits in val are now positioned: val = 00001100 */

        by1 |= val;

/* The bits of val are now packed in by1: by1 = 10111111 */

        return(by1);
}  /* packgreypx */


/*
 * static IMBYTE unpackgreypx(byo, xpos, depth)
 * Unpacks the bits of the pixel at position 'xpos' in the image row.
 * 'byo' is byte that has to be unpacked, 'depth' is the number of bits
 * per pixel.
 * Unpacking is done by shifting to the left until the most significant
 * bit of the pixel is the most significant bit of the byte and then
 * shift to right until the least significant bit of the pixel is the
 * least significant bit of the byte.
 * Example: byo = 10111111, xpos = 6, depth = 2
 */

static IMBYTE unpackgreypx(byo, xpos, depth)
        IMBYTE byo;
        int xpos;
        int depth;
{
        int d;
        int i = 0;
        IMBYTE by1;

        by1 = byo;
        if (depth == 3)
                i = 2;
        else
                if (depth > 4)
                        i = (8 - depth);

/* i is the number of bits which are not used to represent pixels
 * e.g. if the depth of a pixel is 3, only 2 pixels (each 3 bits)
 * can be packed into one byte, the two leftmost bits remain unused */

        d = ((xpos % (8 / depth)) * depth) + i;

/* d is the number of bits for the left shift: d = 4 */

        by1 <<= d;            /* by1 looks now like this: by1 = 11110000 */
        by1 >>= (8 - depth);  /* by1 looks now like this: by1 = 00000011 */

        return(by1);
}  /* unpackgreypx */


/*
 * void imwpix(img, rowbuf, pixval, xpos)
 * Writes the pixel 'pixval' (which is of the type IMRGB) to the position
 * 'xpos' in the buffer 'rowbuf'. This is an Ideal Pixel function, since
 * the type of the image 'img' can be any of the types IMGRY, IMMAP or IMRGB.
 * 'pixval' is first converted to the image type of 'img' and then written
 * to 'rowbuf'.
 * The FOR-loop at the end of the function does the writing of the byte(s).
 */

void imwpix(img, rowbuf, pixval, xpos)
    IMAGE *img;
    IMBYTE *rowbuf;
    IMPIXEL pixval;
    int xpos;
{
        IMPIXEL v, *cm;
        int i, xrpos;                   /* xrpos is the position of the byte in rowbuf */

        v = pixval;

        switch(img->type & IMTYP)
        {
                case IMGRY:
                        v = ((IMPIXEL) imrgb2grey(v, img->depth));
                        xrpos = (xpos + (8 / img->depth)) / (8 / img->depth) - 1;
                        v = ((IMPIXEL) packgreypx(rowbuf[xrpos], (IMBYTE) (v & 0xFF), xpos, (int) img->depth));
                        break;
                case IMMAP:
                        xrpos = xpos;
                        cm = imgetcmap(img);
                        i = imgetcolors(img);
                        while(i--)
                                if (cm[i] == v)
                                {
                                        v = (IMPIXEL) i;
                                        break;
                                }
                        if (i < 0)
                                imerror(ColMaperr);
                        break;
                case IMRGB:
                        xrpos = xpos;
                        /* picture already is rgb */
                        break;
        }

        for (i = 0; i <= img->psize - 1; i++)
        {
                rowbuf[xrpos + (img->xsize * i)] = ((IMBYTE) v & 0xFF);
                v >>= 8;
        }
}  /* imwpix */


/*
 * IMPIXEL imrpix(img, rowbuf, xpos)
 * Returns the value (of the type IMRGB) of the pixel on the position
 * 'xpos' in the buffer 'rowbuf'. This is an Ideal Pixel function, since
 * the type of the image 'img' can be any of the types IMGRY, IMMAP or IMRGB.
 * The WHILE-loop at the beginning of the function does the reading of the
 * byte(s).
 * Then (if necessary) the pixel is converted to the image type IMRGB.
 */

IMPIXEL imrpix(img, rowbuf, xpos)
    IMAGE *img;
    IMBYTE *rowbuf;
    int xpos;
{
        IMPIXEL v = 0L;
        int i, xrpos;                   /* xrpos is the position of the byte in rowbuf */
        IMBYTE vb;

        if (img->depth > 4)
                xrpos = xpos;
        else
                xrpos = (xpos + (8 / img->depth)) / (8 / img->depth) - 1;
        i = img->psize;
        while (i--)
        {
                v <<= 8;
                v |= (rowbuf[xrpos + (img->xsize * i)]);
        }

        switch(img->type & IMTYP)
        {
                case IMGRY:
                        vb = unpackgreypx((IMBYTE) (v & 0xFF), xpos, (int) img->depth);
                        vb = imnormalize(vb, img->depth, 8);
                        v = ((IMPIXEL) vb);
                        v |= (v << 16) | (v << 8);
                        break;
                case IMMAP:
                        i = ((int) (v & 0xFFFF));
                        v = img->colormap[i];
                        break;
                case IMRGB:
                        /* picture already is rgb */
                        break;
        }
        return(v);
}  /* imrpix */


/*
 * void imwflt(img, rowbuf, pixarr, xpos)
 * Writes the float pixel pointed to by 'pixarr' to
 * the position 'xpos' in the buffer 'rowbuf'.
 */

void imwflt(img, rowbuf, pixarr, xpos)
    IMAGE  *img;
    IMBYTE *rowbuf;
    char   *pixarr;
    int     xpos;
{
        int      samplesize, s, d, x0, x1;
        imushort   samples, depth;
        IMBYTE **bytearr;

        samples = img->samples;
        depth = img->depth / 8;             /* depth of a sample in bytes */
        bytearr = img->fltbytes;
        x0 = xpos * depth;

        if (img->depth < 64)
                /* assumption: depth < 64 is a single precision float */
                flt2byte(img, pixarr, samples);
/*      else  */
                /* assumption: depth >= 64 is a double precision float */
/*              dbl2byte(img, pixarr, samples);  */

        samplesize = img->xsize * depth;

        /*
         * write the bytes of a float pixel to rowbuf.
         */
        for (s = 0; s < samples; s++)
        {
                x1 = x0 + (samplesize * s);
                for (d = 0; d < depth; d++)
                        rowbuf[x1 + d] = bytearr[s][d];
        }
}  /* imwflt */


/*
 * char imrflt(img, rowbuf, xpos)
 * Returns a pointer to a float pixel. A float pixel is a list of float
 * values.
 */

void imrflt(img, rowbuf, pixarr, xpos)
    IMAGE  *img;
    IMBYTE *rowbuf;
    char   *pixarr;
    int     xpos;
{
        int      samplesize, s, d, x0, x1;
        imushort   samples, depth;
        IMBYTE **bytearr;

        IMBYTE   tb;

        samples = img->samples;
        depth = img->depth / 8;             /* depth of a sample in bytes */
        bytearr = img->fltbytes;
        x0 = xpos * depth;

        samplesize = img->xsize * depth;

        /*
         * read the bytes of a float pixel from rowbuf.
         */
        for (s = 0; s < samples; s++)
        {
                x1 = x0 + (samplesize * s);
                for (d = 0; d < depth; d++)
                {
                        bytearr[s][d] = rowbuf[x1 + d];
                        tb = bytearr[s][d];
                }
        }

        if (img->depth < 64)
                /* assumption: depth < 64 is a single precision float */
                byte2flt(img, pixarr, samples);
/*      else  */
                /* assumption: depth >= 64 is a double precision float */
/*              byte2dbl(img, pixarr, samples);  */
}  /* imrflt */


/*
 * void imwgry(img, rowbuf, gryval, xpos)
 * Writes the pixel 'gryval' (which is of the type IMGRY) to the position
 * 'xpos' in the buffer 'rowbuf'. This is an Ideal Pixel function, since
 * the type of the image 'img' can be any of the types IMGRY, IMMAP or IMRGB.
 * 'gryval' is first converted to the image type of 'img' and then written
 * to 'rowbuf'.
 * The FOR-loop at the end of the function does the writing of the byte(s).
 */

void imwgry(img, rowbuf, gryval, xpos)
    IMAGE *img;
    IMBYTE *rowbuf;
    IMBYTE gryval;
    int xpos;
{
        IMBYTE vb;
        IMPIXEL v, *cm;
        int i, xrpos;                   /* xrpos is the position of the byte in rowbuf */

        vb = gryval;

        switch(img->type & IMTYP)
        {
                case IMGRY:
                        xrpos = (xpos + (8 / img->depth)) / (8 / img->depth) - 1;
                        v = ((IMPIXEL) packgreypx(rowbuf[xrpos], vb, xpos, (int) img->depth));
                        break;
                case IMMAP:
                        xrpos = xpos;
                        v = ((IMPIXEL) vb);
                        v |= (v << 16) | (v << 8);
                        cm = imgetcmap(img);
                        i = imgetcolors(img);
                        while(i--)
                                if (cm[i] == v)
                                {
                                        v = (IMPIXEL) i;
                                        break;
                                }
                        if (i < 0)
                                imerror(ColMaperr);
                        break;
                case IMRGB:
                        xrpos = xpos;
                        v = ((IMPIXEL) vb);
                        v |= (v << 16) | (v << 8);
                        break;
        }

        for (i = 0; i <= img->psize - 1; i++)
        {
                rowbuf[xrpos + (img->xsize * i)] = ((IMBYTE) v & 0xFF);
                v >>= 8;
        }
}  /* imwgry */


/*
 * IMBYTE imrgry(img, rowbuf, xpos)
 * Returns the value (of the type IMGRY) of the pixel on the position
 * 'xpos' in the buffer 'rowbuf'. This is an Ideal Pixel function, since
 * the type of the image 'img' can be any of the types IMGRY, IMMAP or IMRGB.
 * The WHILE-loop at the beginning of the function does the reading of the
 * byte(s).
 * Then (if necessary) the pixel is converted to the image type IMGRY.
 */

IMBYTE imrgry(img, rowbuf, xpos)
    IMAGE *img;
    IMBYTE *rowbuf;
    int xpos;
{
        IMPIXEL v = 0L;
        int i, xrpos;                   /* xrpos is the position of the byte in rowbuf */
        IMBYTE vb;

        if (img->depth > 4)
                xrpos = xpos;
        else
                xrpos = (xpos + (8 / img->depth)) / (8 / img->depth) - 1;
        i = img->psize;
        while (i--)
        {
                v <<= 8;
                v |= (rowbuf[xrpos + (img->xsize * i)]);
        }

        switch(img->type & IMTYP)
        {
                case IMGRY:
                        vb = unpackgreypx((IMBYTE) (v & 0xFF), xpos, (int) img->depth);
                        break;
                case IMMAP:
                        i = ((int) (v & 0xFFFF));
                        v = img->colormap[i];
                        vb = (imrgb2grey(v, img->depth));
                        break;
                case IMRGB:
                        vb = (imrgb2grey(v, img->depth));
                        break;
        }
        return(vb);
}  /* imrgry */


/*
 * int imwidx(img, rowbuf, idxval, xpos)
 * Writes the colormap index 'idxval' of the pixel at position 'xpos' in
 * the buffer 'rowbuf'.
 * This function only works for the image type IMMAP.
 */

int imwidx(img, rowbuf, idxval, xpos)
    IMAGE *img;
    IMBYTE *rowbuf;
    int idxval;
    int xpos;
{
        int v = idxval, i, xrpos;
        IMBYTE vb;
        imerror(NoError);

        if ((img->type & IMTYP) == IMMAP)
        {
                if (v < imgetcolors(img))
                {
                        if (img->depth > 4)
                                for (i = 0; i <= img->psize - 1; i++)
                                {
                                        rowbuf[xpos + (img->xsize * i)] = ((IMBYTE) v & 0xFF);
                                        v >>= 8;
                                }
                        else
                        {
                                /* xrpos is the position of the byte in rowbuf into
                                 * which the bits of the pixel will be packed
                                 */
                                xrpos = (xpos + (8 / img->depth)) / (8 / img->depth) - 1;
                                vb = packgreypx(rowbuf[xrpos], (IMBYTE) (v & 0xFF), xpos, (int) img->depth);
                                rowbuf[xrpos] = vb;
                        }
                }
                else
                        imerror(CmapInderr);
        }
        else
                imerror(MapTyperr);

        return(imerrnum);
}  /* imwidx */


/*
 * int imridx(img, rowbuf, xpos)
 * Returns the colormap index of the pixel at position 'xpos' in the buffer
 * 'rowbuf'.
 * This function only works for the image type IMMAP.
 */

int imridx(img, rowbuf, xpos)
    IMAGE *img;
    IMBYTE *rowbuf;
    int xpos;
{
        int v = 0;
        int i, xrpos;

        imerror(NoError);

        if ((img->type & IMTYP) == IMMAP)
        {
                if (img->depth > 4)
                {
                        i = img->psize;
                        while (i--)
                        {
                                v <<= 8;
                                v |= (rowbuf[xpos + (img->xsize * i)]);
                        }
                }
                else
                {
                        /* bits are packed in rowbuf , xrpos is the position of the
                         * byte in rowbuf that contains the bits of the pixel
                         */
                        xrpos = (xpos + (8 / img->depth)) / (8 / img->depth) - 1;
                        v = (int) unpackgreypx(rowbuf[xrpos], xpos, (int) img->depth);
                }
        }
        else
                imerror(MapTyperr);
        return(v);
}  /* imridx */


/*
 * int imwcolor(img, colval, pos)
 * Writes the color value 'colval' (of the type IMRGB) to the
 * position 'pos' in the colormap of the image 'img'.
 * This function only works for the image type IMMAP.
 */

int imwcolor(img, colval, pos)
        IMAGE *img;
        IMPIXEL colval;
        int pos;
{
        IMPIXEL *cm;

        imerror(NoError);

        if ((img->type & IMTYP) == IMMAP)
        {
                cm = imgetcmap(img);
                cm[pos] = colval;
        }
        else
                imerror(MapTyperr);

        return(imerrnum);
}  /* imwcolor */


/*
 * IMPIXEL imrcolor(img, pos)
 * Returns the value (of the type IMRGB) of the color at position 'pos'
 * in the colormap of the image 'img'.
 * This function only works for the image type IMMAP.
 */

IMPIXEL imrcolor(img, pos)
    IMAGE *img;
        int pos;
{
        IMPIXEL v, *cm;

        imerror(NoError);

        if ((img->type & IMTYP) == IMMAP)
        {
                cm = imgetcmap(img);
                v = cm[pos];
        }
        else
                imerror(MapTyperr);
        return(v);
}  /* imrcolor */


/*
 * IMBYTE imrgb2grey(rgbval, depth)
 * Returns the grey value of 'rgbval' (which is of the type IMRGB).
 * The returned grey value has a depth of 'depth' bits.
 * The formula used is:
 * Luminence = 29.9% red + 58.7% green + 11.4% blue
 */

IMBYTE imrgb2grey(rgbval, depth)
        IMPIXEL rgbval;
        imushort depth;
{
        IMBYTE greyval;
        IMBYTE red, green, blue;
        IMBYTE grey = 0;

        imsplitrgb(rgbval, &red, &green, &blue);

        grey = (((77 * red) + (150 * green) + (29 * blue)) / 256);
        greyval = imnormalize(grey,8,depth);
        return(greyval);
}  /* imrgb2grey */


/*
 * void imsplitrgb(rgbval, red, green, blue)
 * Splits 'rgbval' to its red, green and blue components.
 */

void imsplitrgb(rgbval, red, green, blue)
        IMPIXEL rgbval;
        IMBYTE *red, *green, *blue;
{
    IMPIXEL v;

    v = rgbval;
        *red = ((IMBYTE) v & 0xFF);
        v >>= 8;
        *green = ((IMBYTE) v & 0xFF);
        v >>= 8;
        *blue = ((IMBYTE) v & 0xFF);
}  /* imsplitrgb */


/*
 * IMPIXEL immergergb(red, green, blue)
 * Returns the RGB value with the red, green and blue components
 * 'red', 'green' and 'blue'.
 */

IMPIXEL immergergb(red, green, blue)
        IMBYTE red, green, blue;
{
        IMPIXEL rgbval = 0L;

        rgbval = rgbval | blue;
        rgbval <<= 8;
        rgbval = rgbval | green;
        rgbval <<= 8;
        rgbval = rgbval | red;
        return(rgbval);
}  /* immergergb */


/*
 * void impx2byte(img, longrow, rowbuf)
 * Takes the pixels (of type IMPIXEL) of the buffer 'longrow' and
 * and stores them in 'rowbuf' (of type IMBYTE) in the scanline format.
 * (See the documentation for a description of the scanline format)
 * The # of pixels in the buffers 'longrow' and 'rowbuf' is equal to
 * 'img->xsize'.
 */

void impx2byte(img, longrow, rowbuf)
    IMAGE *img;
    IMPIXEL *longrow;
    IMBYTE *rowbuf;
{
        IMPIXEL v;
        int i, xpos;

        for (xpos = 0; xpos < img->xsize; xpos++)
        {
                v = *longrow++;
                for (i = 0; i <= img->psize - 1; i++)
                {
                        rowbuf[xpos + (img->xsize * i)] = ((IMBYTE) v & 0xFF);
                        v >>= 8;
                }
        }
}  /* impx2byte */


/*
 * void imbyte2px(img, rowbuf, longrow)
 * Takes the pixels stored in 'rowbuf' and converts them to pixels of
 * type IMPIXEL and stores them in 'longrow'.
 * The # of pixels in the buffers 'longrow' and 'rowbuf' is equal to
 * 'img->xsize'.
 */

void imbyte2px(img, rowbuf, longrow)
    IMAGE *img;
    IMBYTE *rowbuf;
    IMPIXEL *longrow;
{
        IMPIXEL v;
        int i, xpos;

        for (xpos = 0; xpos < img->xsize; xpos++)
        {
                v = 0L;
                i = img->psize;

                while (i--)
                {
                        v <<= 8;
                        v = v | (rowbuf[xpos + (img->xsize * i)]);
                }
                *longrow++ = v;
        }
}  /* imbyte2px */


/*
 * IMBYTE imnormalize(value, srcbits, dstbits)
 * Normalizes the byte 'value' from 'srcbits' to 'dstbits' and returns
 * the new value.
 */

IMBYTE imnormalize(value, srcbits, dstbits)
        IMBYTE value;
        int    srcbits;
        int    dstbits;
{
        long n;

        if (srcbits != dstbits)
        {
                if (srcbits > dstbits)
                        value >>= srcbits - dstbits;
                else
                {
                        n = ((long) value);
                        while (srcbits < dstbits)
                        {
                                n |= n << srcbits;
                                srcbits <<= 1;
                        }
                        n >>= (srcbits - dstbits);
                        value = ((IMBYTE) n);
                }
        }
        return(value);
}  /* imnormalize */


static void flt2byte(img, pixarr, size)
        IMAGE *img;
        char  *pixarr;
        imushort size;
{
        float   *f;
        IMBYTE **b;
        long    *l;
        int      i, j;

        b = img->fltbytes;
        f = (float *) pixarr;

        for (i = 0; i < size; i++)
        {
                l = (long *) f++;
                b[i][0] = (IMBYTE) (*l & 0xFF);
                b[i][1] = (IMBYTE) ((*l >> 8) & 0xFF);
                b[i][2] = (IMBYTE) ((*l >> 16) & 0xFF);
                b[i][3] = (IMBYTE) ((*l >> 24) & 0xFF);
        }
}


static void byte2flt(img, pixarr, size)
        IMAGE *img;
        char  *pixarr;
        imushort size;
{
        float   *f, *f1;
        IMBYTE **b;
        int      i;
        long    *l;
        IMBYTE        testb;

        f = (float *) pixarr;
        b = img->fltbytes;

        for (i = 0; i < size; i++)
        {
                *l = 0L;
                testb = b[i][3] & 0xFF;
                *l = (long) b[i][3] & 0xFF;
                *l = (long) ((*l << 8) | b[i][2]);
                *l = (long) ((*l << 8) | b[i][1]);
                *l = (long) ((*l << 8) | b[i][0]);
                f1 = (float *) l;
                *f++ = *f1;
        }
}


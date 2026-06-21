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

static long rrow( /* IMAGE *img, IMBYTE *rowbuf, long size */ );
static long wrow( /* IMAGE *img, IMBYTE *rowbuf, long size */ );
void FlushData( /* LZWState  *lzs */ );
static int checkhdr( /* IMAGE *img */ );

/* functions for writing and reading */

/* a) functions for writing */


/*
 * int imwhdr(img)
 * writes the header of the image specified in 'img' and writes then
 * zeroes to fill the header to the size of IM_HDRSIZE, so that the size
 * of the header is one block (512 bytes)
 */
 
int imwhdr(img)
	IMAGE  *img;
{
int i, err;

	err = checkhdr(img);      /* check if data in the header is consistent */
	fputc(img->magic/256, img->fp);
	fputc(img->magic%256, img->fp);
	fputc(img->majfilvn/256, img->fp);
	fputc(img->majfilvn%256, img->fp);
	fputc(img->minfilvn/256, img->fp);
	fputc(img->minfilvn%256, img->fp);

	fputc(img->type/256, img->fp);
	fputc(img->type%256, img->fp);
	fputc(img->storage/256, img->fp);
	fputc(img->storage%256, img->fp);

	fputc(img->xsize/256, img->fp);
	fputc(img->xsize%256, img->fp);
	fputc(img->ysize/256, img->fp);
	fputc(img->ysize%256, img->fp);

	fputc(img->samples/256, img->fp);
	fputc(img->samples%256, img->fp);
	fputc(img->depth/256, img->fp);
	fputc(img->depth%256, img->fp);
	fputc(img->colors/256, img->fp);
	fputc(img->colors%256, img->fp);

	for (i = IM_HDRSIZE-20; i; i--)		/* fill up */
		fputc(0, img->fp);
		
	return(err);
		
}  /* imwhdr */


/* 
 * long imwrow(img, rowbuf)
 * calls wrow() to write the contents of rowbuf (which is one row of 
 * the image 'img') to the file
 */
  
long imwrow(img, rowbuf)
	IMAGE  *img;
	IMBYTE *rowbuf;
{
	long nw, size;
	
	size = imgetrowsize(img->xsize, img->samples, img->depth);
	if (((nw = wrow(img, rowbuf, size)) == 0) && (imerrnum != 0))
		return(0);
	return(nw);
}  /* imwrow */
	

/*
 * long imwpic(img, impic)
 * calls wrow() 'ysize' times to write all the rows of the image 'img'
 * to the file
 */
 	
long imwpic(img, impic)
	IMAGE  *img;
	IMBYTE **impic;
{
	int y;
	long nw, size, re;
	IMBYTE **pic;
	
	nw = 0;
	pic = impic;
	y = img->ysize;
	size = imgetrowsize(img->xsize, img->samples, img->depth);
	while (y--)
	{
		if (((re = wrow(img, *pic, size)) == 0) && (imerrnum != 0))
			return(0);
			
		nw += re;
		pic++;
	}
	return(nw);
}  /* imwpic */
	

/*
 * long wrow(img, rowbuf, size)
 * writes the contents of rowbuf as binary to the file
 * (if compression is implemented, the contents of rowbuf will
 *  get compressed according to img->storage before written to
 *  to the file)
 */
 
static long wrow(img, rowbuf, size)
	IMAGE    *img;
	IMBYTE   *rowbuf;
	long      size;
{
	long      nw, pb_size;
	IMBYTE   *packed_buf;
	LZWState *lzs;
	
	
	switch(img->storage  & IMTYP)
	{
		case IMRAW:
			nw = fwrite(rowbuf, sizeof(IMBYTE), size, img->fp);
			break;
		case IMRLE:
			if (!(img->storage & IMRAW))
				if ((packed_buf = ((IMBYTE *) malloc(2 * size))) == NULL)
				{
					imerror(NoMemerr);
					return(0);
				}
			pb_size = PackRow(rowbuf, packed_buf, size);
			nw = fwrite(packed_buf, sizeof(IMBYTE), pb_size, img->fp);
			free(packed_buf);
			break;
		case IMLZW:
			if ((lzs = (LZWState *) img->pack_state) == NULL)
			{
				if ((img->pack_state = (char *) malloc(sizeof(LZWState))) == NULL)
				{
					imerror(NoMemerr);
					return(0);
				}
				lzs = (LZWState *) img->pack_state;
				LZWPreEncode(lzs, (int) (2 * size), img->fp);
			}
			
			LZWEncode(lzs, rowbuf, (int) size, FlushData);

			nw = 1;
			break;
	}
	
	return(nw);
}  /* wrow */
 
/*
 * int imwcmap(img)
 * writes the colormap of img to the file.
 * Each color is written as four successive bytes. The first byte
 * contains the red, the second the green and the third the blue component.
 * The value of the fourth byte is not defined.
 * If all colors are written, the file is filled with zeroes to 
 * the next block boundary.
 */
 
int imwcmap(img)
	IMAGE *img;
{
	int i,j;
	IMPIXEL v;
	IMPIXEL *cm;
	IMBYTE c;

	imerror(NoError);
	if ((img->type  & IMTYP) == IMMAP)
	{
		cm = imgetcmap(img);
		for (i = 0; i < imgetcolors(img); i++)
		{
			v = cm[i];
			for (j = 3; j >= 0; j--)
			{
				c = v & 0xFF;
				fputc(c, img->fp);
				v >>= 8;
			}
		}
		
		i = 511 - ((sizeof(IMPIXEL) * imgetcolors(img) - 1) % 512);
	
		while (i--)		/* fill up */
			fputc(0, img->fp);	
	}
	else
		imerror(MapTyperr);	
	
	return(imerrnum);	
}  /* imwcmap */


/* b) functions for reading */

/*
 * int imrhdr(img)
 * reads the header of the file specified in 'img' and stores the
 * contents in the structure 'img'.
 * Then it checks the version numbers to see whether the image is
 * compatible with the loading program.
 */
 
int imrhdr(img)
	IMAGE  *img;
{
	int i;

#define getshort(s,fp) (s = 256*fgetc(fp), s += fgetc(fp))

	imerror(NoError);
	getshort(img->majfilvn, img->fp);
	getshort(img->minfilvn, img->fp);

	getshort(img->type, img->fp);
	getshort(img->storage, img->fp);
	
	getshort(img->xsize, img->fp);
	getshort(img->ysize, img->fp);

	getshort(img->samples, img->fp);
	getshort(img->depth, img->fp);
	getshort(img->colors, img->fp);
	
	img->psize = ((img->samples * img->depth) + 7) / 8;

	for (i = IM_HDRSIZE - 20; i; i--)		/* overread those bytes */
		fgetc(img->fp);

	if (img->majfilvn != MAJLIBVN)
		imerror(MajorVNerr);
	else
		if (img->minfilvn > MINLIBVN)
			imerror(MinorVNerr);
	
	if (img->type == IMFLT)
		/* misuse impicalloc() to allocate a two dimensional
		 * array of IMBYTE
		 */
		img->fltbytes = impicalloc(img->depth / 8, img->samples, 1, 8);

	return(imerrnum);
}   /* imrhdr */


/* 
 * long imrrow(img, rowbuf)
 * calls rrow() to read one row of the image 'img' from the file and
 * stores that row in rowbuf
 */
  
long imrrow(img, rowbuf)
	IMAGE  *img;
	IMBYTE *rowbuf;
{
	long nw, size;
	
	size = imgetrowsize(img->xsize, img->samples, img->depth);
	if (((nw = rrow(img, rowbuf, size)) == 0) && (imerrnum != 0))
		return(0);
		
	return(nw);
}  /* imrrow */
	
	
/*
 * long imrpic(img, impic)
 * calls rrow() 'ysize' times to read all the rows of the image 'img'
 * from the file and store them in 'impic'
 */
 	
long imrpic(img, impic)
	IMAGE  *img;
	IMBYTE **impic;
{
	int y;
	long nw, re, size;
	IMBYTE **pic;
	
	nw = 0L;
	pic = impic;
	y = img->ysize;
	size = imgetrowsize(img->xsize, img->samples, img->depth);
	while (y--)
	{
		if (((re = rrow(img, *pic, size)) == 0) && (imerrnum != 0))
			return(0);
		nw += re;
		pic++;
	}
	return(nw);
}  /* imrpic */
	
	
/*
 * long rrow(img, rowbuf, size)
 * reads 'size' bytes as binary from the file and stores them in rowbuf
 * (if compression is implemented, the contents of rowbuf will
 *  get decompressed according to img->storage after being read from
 *  the file)
 */
 
static long rrow(img, rowbuf, size)
	IMAGE  *img;
	IMBYTE *rowbuf;
	long size;
{
	int        rb, i, rawstart, upb = 0;
	IMBYTE    *read_buf;
	RLE_STATE *rls;
	LZWState  *lzs;

	switch(img->storage  & IMTYP)
	{
		case IMRAW:
			rb = fread(rowbuf, sizeof(IMBYTE), size, img->fp);
			break;
		case IMRLE:
			if ((rls = (RLE_STATE *) img->pack_state) == NULL)
			{
				if ((img->pack_state = (char *) malloc(sizeof(RLE_STATE))) == NULL)
				{
					imerror(NoMemerr);
					return(0);
				}
				
				rls = (RLE_STATE *) img->pack_state;
				InitRLEState(rls, (int) (2 * size));
			}
			
			if ((read_buf = ((IMBYTE *) malloc(rls->rd_size))) == NULL)
			{
				imerror(NoMemerr);
				return(0);
			}
				
			rb = fread(read_buf, sizeof(IMBYTE), rls->rd_eb, img->fp);
			
			for (i = rls->rd_es; i < (rls->rd_es + rb); i++)
				rls->rawdata[i] = read_buf[i - rls->rd_es];
			
			free (read_buf);
				
			UnPackRow(rls->rawdata, rowbuf, (long) rls->rd_size, (imushort) size, &upb);

			for (i = upb; i < rls->rd_size; i++)
				rls->rawdata[i - upb] = rls->rawdata[i];
			
			rls->rd_es = (int) ((rls->rd_size) - upb);
			rls->rd_eb = upb;
			break;
		case IMLZW:
			if ((lzs = (LZWState *) img->pack_state) == NULL)  
			{
			    /* initialize for decoding */
				if ((img->pack_state = (char *) malloc(sizeof(LZWState))) == NULL)
				{
					imerror(NoMemerr);
					return(0);
				}
				lzs = (LZWState *) img->pack_state;
				LZWPreDecode(lzs, (int) (size * 2));
				lzs->lzw_processed = lzs->lzw_rbufsize;
			}
			if ((read_buf = ((IMBYTE *) malloc(lzs->lzw_rbufsize))) == NULL)
			{
				imerror(NoMemerr);
				return(0);
			}
				
			rb = fread(read_buf, sizeof(IMBYTE), lzs->lzw_processed, img->fp);
			if (rb < lzs->lzw_processed)
				lzs->dec_eofdata = TRUE;
				
			rawstart = lzs->lzw_rbufsize - lzs->lzw_processed;
			/* fill up lzs->lzw_rawbuf with the data read with fread() */
			for (i = 0; i < rb; i++)   
				lzs->lzw_rawbuf[i + rawstart] = read_buf[i];
			
			LZWDecode(lzs, rowbuf, (int) size);
	
			free (read_buf);
			break;
	}
	
	return(rb);
}  /* rrow */
  

/*
 * void imrcmap(img)
 * reads the colormap of img from the file.
 * Each color is read as four successive bytes. The first byte
 * contains the red, the second the green and the third the blue component.
 * The value of the fourth byte is set to zero.
 * If all colors are read, the file is read to the next block boundary
 */
 
int imrcmap(img)
	IMAGE *img;
{
	int i,j;
	IMPIXEL v = 0L;
	IMPIXEL *cm;
	IMBYTE lb[4];

	if ((img->type  & IMTYP) == IMMAP)
	{
		cm = imgetcmap(img);
		for (i = 0; i < imgetcolors(img); i++)
		{
			for (j = 0; j <= 3; j++)
				lb[j] = fgetc(img->fp);
			
			v = lb[3];
			for (j = 2; j >= 0; j--)
			{
				v <<= 8;
				v |= lb[j];
			}
			cm[i] = v;
		}
	
		imsetcmap(img,cm);
	
		i = 511 - ((sizeof(IMPIXEL) * imgetcolors(img) - 1) % 512);
	
		while (i--)		/* fill up */
			fgetc(img->fp);
	}
	
	return(imerrnum);
}  /* imrcmap */


void FlushData(lzs)
	LZWState  *lzs;
{
	fwrite(lzs->lzw_rawbuf, sizeof(IMBYTE), lzs->lzw_processed, lzs->lzw_fp);
}


static int checkhdr(img)
	IMAGE *img;
{
	switch (img->type)
	{
		case IMGRY: if ((img->depth < 1) || (img->depth > 8))
						imerror(Deptherr); 
					if (img->samples != 1)
						imerror(Samplerr); 
					break;
		case IMRGB: if (img->depth != 8) 
						imerror(Deptherr); 
					if ((img->samples < 1) || (img->samples > 3))
						imerror(Samplerr); 
					break;
		case IMMAP: if ((img->depth < 1) || (img->depth > 16)) 
						imerror(Deptherr); 
					if (img->samples != 1)
						imerror(Samplerr); 
					if (img->colors > (1 << img->depth))
						imerror(MapDeptherr); 
					break;
		case IMFLT: if ((img->depth != 32) && (img->depth !=64)) 
						imerror(Deptherr); 
					if ((img->samples < 1) || (img->samples > 8))
						imerror(Samplerr); 
					img->fltbytes = impicalloc(img->depth / 8, img->samples, 1, 8);
					break;
		default: imerror(ImageTyperr);
	}
	
	img->psize = ((img->samples * img->depth) + 7) / 8;
		
	if ((img->storage != IMRAW) && (img->storage != IMRLE) && (img->storage != IMLZW))
		imerror(StorTyperr);

	if ((img->xsize < 0) || (img->xsize > 65535))
		imerror(XSizerr);
		
	if ((img->ysize < 0) || (img->ysize > 65535))
		imerror(YSizerr);
		
	if ((img->colors < 0) || (img->colors > 65535))
		imerror(Colorserr);
		
	return(imerrnum);
}  /* checkhdr */

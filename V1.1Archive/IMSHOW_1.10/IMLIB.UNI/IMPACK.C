/*
 *  im : A General Purpose Image (File) Format
 *
 *  Version 0.93(beta)    12/12/1989
 *
 *  Author: Dieter Schaufelberger
 *
 *  Copyright October 1989
 *
 *  University of Zurich, Dept. of Computer Science, Multi-Media Lab,
 *  Winterthurerstrasse 190, CH-8057 Zurich
 */

#include "imlib.h"
#include "impack.h"

static int GetNextCode( /* LZWState *lzs */ );
static void PutNextCode( /* LZWState *lzs, int c */ );
static void cl_block( /* LZWState *lzs */);
static void cl_hash( /* LZWState *lzs */ );
static void makezero( /* lzwchar *p, long l */);

static IMBYTE *PutDump();  
static IMBYTE *PutRun();   

/*----------------------------------------------------------------------*
 * packer.c Convert data to "cmpByteRun1" run compression.     11/15/85
 *
 * By Jerry Morrison and Steve Shaw, Electronic Arts.
 * This software is in the public domain.
 *
 * Modified by D. Schaufelberger, October 1989
 *
 *	control bytes:
 *	 [0..127]   : followed by n+1 bytes of data.
 *	 [-1..-127] : followed by byte to be repeated (-n)+1 times.
 *	 -128       : NOOP.
 *
 *----------------------------------------------------------------------*/

long putSize;
char buf[256];	/* [TBD] should be 128?  on stack?*/

static IMBYTE *PutDump(dest, nn)  
	IMBYTE *dest;  
	int nn; 
{
	int i;

	PutByte(nn-1);
	for(i = 0;  i < nn;  i++)   
		PutByte(buf[i]);
	return(dest);
}  /* PutDump */


static IMBYTE *PutRun(dest, nn, cc)   
	IMBYTE *dest;  
	int nn;
	int cc; 
{
	PutByte(-(nn-1));
	PutByte(cc);
	return(dest);
}  /* PutRun */


/*----------- PackRow --------------------------------------------------*/
/* Packs one row, updating the source and
   destination pointers.  RETURNs count of packed bytes.*/
	
long PackRow(pSource, pDest, rowSize)
	IMBYTE  *pSource;
	IMBYTE  *pDest;   
	long     rowSize; 
{
    IMBYTE *source, *dest;
    char    c, lastc = '\0';
    int     mode = DUMP;
    short   nbuf = 0;          /* number of chars in buffer */
    short   rstart = 0;        /* buffer index current run starts */

    source = pSource;
    dest = pDest;
    putSize = 0;
    buf[0] = lastc = c = GetByte();     /* so have valid lastc */
    nbuf = 1;   rowSize--;              /* since one byte eaten.*/


	for (;  rowSize;  --rowSize) 
	{
		buf[nbuf++] = c = GetByte();
		switch (mode) 
		{
			case DUMP: 
				/* If the buffer is full, write the length byte,
			 	  then the data */
				if (nbuf>MaxDat) 
				{
					OutDump(nbuf-1);  
					buf[0] = c; 
					nbuf = 1;   rstart = 0; 
					break;
				}

				if (c == lastc) 
				{
			    	if (nbuf-rstart >= MinRun) 
					{
						if (rstart > 0) OutDump(rstart);
							mode = RUN;
					}
			    	else 
						if (rstart == 0)
					mode = RUN;	/* no dump in progress, so can't lose by making these 2 a run.*/
			    }
				else  
					rstart = nbuf-1;		/* first of run */ 
				break;

			case RUN: 
				if ( (c != lastc)|| ( nbuf-rstart > MaxRun)) 
				{                          /* output run */
	 		  		OutRun(nbuf-1-rstart,lastc);
	    			buf[0] = c;
	    			nbuf = 1; rstart = 0;
	    			mode = DUMP;
	    		}
				break;
		}

	lastc = c;
	}

	switch (mode) 
	{
		case DUMP: OutDump(nbuf); break;
		case RUN: OutRun(nbuf-rstart,lastc); break;
	}
	pSource = source;
	pDest = dest;
	return(putSize);
}  /* PackRow */


/* Given POINTERS to POINTER variables, unpacks one row, updating the source
 * and destination pointers until it produces dstBytes bytes. */
 
int UnPackRow(pSource, pDest, srcBytes0, dstBytes0, unpacked_bytes)
	IMBYTE *pSource;
	IMBYTE *pDest;  
	long    srcBytes0;
	lzwshort  dstBytes0; 
	unsigned int *unpacked_bytes;  
{
	IMBYTE *source;
	IMBYTE *dest;
	char    ch;
	IMBYTE  c;
	int     n;
	long srcBytes = srcBytes0;
	short dstBytes = dstBytes0;
	int error = TRUE;	/* assume error until we make it through the loop */
	char minus128 = -128;  /* get the compiler to generate a CMP.W */

	source = pSource;
	dest = pDest;
	*unpacked_bytes = 0;    

	while( dstBytes > 0 )  
	{
		if ( (srcBytes -= 1) < 0 ) 
			goto ErrorExit;
		ch = GetByte();

    	if (ch >= 0)
		{
			n = (int) ch + 1;
			*unpacked_bytes += (n + 1);
			if ( (srcBytes -= n) < 0 )  
				goto ErrorExit;
			if ( (dstBytes -= n) < 0 )  
				goto ErrorExit;
			
			do 
			{  
				UPutByte(GetByte());  
			} while (--n > 0);
	    }
		else 
			if (ch != minus128) 
			{
				n = ((int) -ch) + 1;
				if ( (srcBytes -= 1) < 0 )  
					goto ErrorExit;
				if ( (dstBytes -= n) < 0 )  
					goto ErrorExit;
				c = GetByte();
				
				do 
				{  
					UPutByte(c);
				} while (--n > 0);
				*unpacked_bytes += 2;
	    	}
	}
	
	error = FALSE;	/* success! */
	ErrorExit:
		return(error);
}  /* UnPackRow */


int InitRLEState(rs, size)
	RLE_STATE  *rs;
	int         size;
{
	if ((rs->rawdata = ((IMBYTE *) malloc(size))) == NULL)
		imerror(NoMemerr);
	rs->rd_size = rs->rd_eb = size;
	rs->rd_es = 0;
	return(imerrnum);
}  /* InitRLEState */




/**********************************************************
  LZW Compression
  ---------------
  
  The code for the LZW compression is taken from the TIFF Library

  Adapted to the im library by Dieter Schaufelberger. 
  
  13/11/1989

  TIFF Library.
  Rev 5.0 Lempel-Ziv & Welch Compression Support
  This code is derived from the compress program.

 **********************************************************/

/*
 * LZW Decoder.
 */

/*
 * Setup state for decoding a strip.
 */

int LZWPreDecode(lzs, srcsize)
	LZWState *lzs;
	int       srcsize;
{
	int code;
	
	imerror(NoError);
	lzs->lzw_rawbuf = (lzwchar *) malloc(srcsize);
	if (lzs->lzw_rawbuf == NULL)
		imerror(NoMemerr);	

	lzs->dec_eofdata = FALSE;
	lzs->lzw_processed = 0;
	lzs->lzw_rbufsize = srcsize;
	lzs->lzw_nbits = BITS_MIN;
	lzs->lzw_maxcode = MAXCODE(BITS_MIN);
	/*
	 * Pre-load the table.
	 */
	for (code = 255; code >= 0; code--)
		lzs->dec_suffix[code] = (lzwchar ) code;
	lzs->lzw_free_ent = CODE_FIRST;
	lzs->lzw_bitoff = 0;
	/* calculate data size in bits */
	lzs->lzw_bitsize = srcsize;
	lzs->lzw_bitsize = (lzs->lzw_bitsize << 3) - (BITS_MAX-1);
	lzs->dec_stackp = lzs->dec_stack;
	lzs->lzw_oldcode = -1;
	
	return(imerrnum);
}  /* LZWPreDecode */

/*
 * Decode the next scanline.
 */

int LZWDecode(lzs, dstbuf, dstby)
	LZWState *lzs;
	lzwchar    *dstbuf;
	int       dstby;
{
	int      code, occ, i;
	lzwchar   *op;
	lzwchar   *stackp;
	int      firstchar, oldcode, incode;

	imerror(NoError);
	stackp = lzs->dec_stackp;
	op = dstbuf; occ = dstby;
	lzs->lzw_processed = 0;      /* start of a new scanline */
	/*
	 * Restart interrupted unstacking operations.
	 */
	if (lzs->lzw_flags & FLAG_RESTART) 
	{
		do 
		{
			if (--occ < 0)    	/* end of scanline */
			{
				lzs->dec_stackp = stackp;
				return (1);
			}
			*op++ = *--stackp;
		} while (stackp > lzs->dec_stack);
		lzs->lzw_flags &= ~FLAG_RESTART;
	}
	oldcode = lzs->lzw_oldcode;
	firstchar = lzs->dec_suffix[oldcode];
	while (occ > 0 && (code = GetNextCode(lzs)) != CODE_EOI) 
	{
		if (code == CODE_CLEAR) 
		{
			makezero(lzs->dec_prefix, (long) sizeof (lzs->dec_prefix));
			lzs->lzw_flags |= FLAG_CODEDELTA;
			lzs->lzw_free_ent = CODE_FIRST;
			if ((code = GetNextCode(lzs)) == CODE_EOI)
				break;
			*op++ = code, occ--;
			oldcode = firstchar = code;
			continue;
		}
		incode = code;
		/*
		 * When a code is not in the table we use (as spec'd):
		 *    StringFromCode(oldcode) +
		 *        FirstChar(StringFromCode(oldcode))
		 */
		if (code >= lzs->lzw_free_ent)     	/* code not in table */
		{
			*stackp++ = firstchar;
			code = oldcode;
		}

		/*
		 * Generate output string (first in reverse).
		 */
		for (; code >= 256; code = lzs->dec_prefix[code])
			*stackp++ = lzs->dec_suffix[code];
			
		*stackp++ = firstchar = lzs->dec_suffix[code];
		do 
		{
			if (--occ < 0)                 /* end of scanline */
			{
				lzs->lzw_flags |= FLAG_RESTART;
				break;
			}
			*op++ = *--stackp;
		} while (stackp > lzs->dec_stack);

		/*
		 * Add the new entry to the code table.
		 */
		if ((code = lzs->lzw_free_ent) < CODE_MAX) 
		{
			lzs->dec_prefix[code] = (lzwshort)oldcode;
			lzs->dec_suffix[code] = firstchar;
			lzs->lzw_free_ent++;
		} 
		oldcode = incode;
	}
	if (occ > 0)
		imerror(LZWDecoderr);
	lzs->dec_stackp = stackp;
	lzs->lzw_oldcode = oldcode;

	if (! lzs->dec_eofdata)
	{
/*		lzs->lzw_processed = lzs->lzw_bitoff >> 3;   */
		lzs->lzw_bitoff = (lzs->lzw_bitoff &= 7);	
		/*
		 * Move the not unpacked bytes to the begining of rawbuf 
		 */
		for (i = lzs->lzw_processed; i < lzs->lzw_rbufsize; i++)   
			lzs->lzw_rawbuf[i - lzs->lzw_processed] = lzs->lzw_rawbuf[i];
	}
	return (imerrnum);
}  /* LZWDecode */

/*
 * Get the next code from the raw data buffer.
 */

static int GetNextCode(lzs)
	LZWState *lzs;
{
	int     code, r_off, bits;
	lzwchar  *bp;

	/*
	 * This check shouldn't be necessary because each
	 * strip is suppose to be terminated with CODE_EOI.
	 * At worst it's a substitute for the CODE_EOI that's
	 * supposed to be there (see calculation of lzw_bitsize
	 * in LZWPreDecode()).
	 */
	if (lzs->lzw_bitoff > lzs->lzw_bitsize)
		return (CODE_EOI);
	/*
	 * If the next entry is too big for the
	 * current code size, then increase the
	 * size up to the maximum possible.
	 */
	if (lzs->lzw_free_ent > lzs->lzw_maxcode) 
	{
		lzs->lzw_nbits++;
		if (lzs->lzw_nbits > BITS_MAX)
			lzs->lzw_nbits = BITS_MAX;
		lzs->lzw_maxcode = MAXCODE(lzs->lzw_nbits);
	}
	
	if (lzs->lzw_flags & FLAG_CODEDELTA) 
	{
		lzs->lzw_maxcode = MAXCODE(lzs->lzw_nbits = BITS_MIN);
		lzs->lzw_flags &= ~FLAG_CODEDELTA;
	}
	r_off = lzs->lzw_bitoff;
	bits = lzs->lzw_nbits;
	/*
	 * Get to the first byte.
	 */
	bp = (lzwchar  *) lzs->lzw_rawbuf + (r_off >> 3);
	r_off &= 7;
	/* Get first part (low order bits) */
	code = (*bp++ >> r_off);
	bits -= (8 - r_off);
	r_off = 8 - r_off;		/* now, offset into code word */
	/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
	if (bits >= 8) 
	{
		code |= *bp++ << r_off;
		r_off += 8;
		bits -= 8;
	}
	/* high order bits. */
	code |= (*bp & rmask[bits]) << r_off;
	lzs->lzw_bitoff += lzs->lzw_nbits;
	if (! lzs->dec_eofdata)
		lzs->lzw_processed = lzs->lzw_bitoff >> 3;
	
	return (code);
}  /* GetNextCode */

/*
 * LZW Encoding.
 */

/*
 * Encode a scanline of pixels.
 *
 * Uses an open addressing double hashing (no chaining) on the 
 * prefix code/next character combination.  We do a variant of
 * Knuth's algorithm D (vol. 3, sec. 6.4) along with G. Knott's
 * relatively-prime secondary probe.  Here, the modular division
 * first probe is gives way to a faster exclusive-or manipulation. 
 * Also do block compression with an adaptive reset, whereby the
 * code table is cleared when the compression ratio decreases,
 * but after the table fills.  The variable-length output codes
 * are re-sized at this point, and a CODE_CLEAR is generated
 * for the decoder. 
 */
void LZWEncode(lzs, srcbuf, srcby, FlushData)
	LZWState *lzs;
	lzwchar    *srcbuf;
	int       srcby;
	int     (*FlushData) ();
{
	long     fcode;
	int      h, c, ent, disp, cc;
	lzwchar   *sb;
	
	sb = srcbuf;
	cc = srcby;
	ent = lzs->lzw_oldcode;
	
	if (ent == -1 && cc > 0) 
	{
		PutNextCode(lzs, CODE_CLEAR);
		ent = *sb++; cc--; lzs->enc_incount++;
	}
	
	while (cc > 0) 
	{
		c = *sb++; cc--; lzs->enc_incount++;
		fcode = ((long)c << BITS_MAX) + ent;
		h = (c << HSHIFT) ^ ent;	/* xor hashing */
		if (lzs->enc_htab[h] == fcode) 
		{
			ent = lzs->enc_codetab[h];
			continue;
		}
		
		if (lzs->enc_htab[h] >= 0) 
		{
			/*
			 * Primary hash failed, check secondary hash.
			 */
			disp = HSIZE - h;
			if (h == 0)
				disp = 1;
			do 
			{
				if ((h -= disp) < 0)
					h += HSIZE;
				if (lzs->enc_htab[h] == fcode) 
				{
					ent = lzs->enc_codetab[h];
					goto hit;
				}
			} while (lzs->enc_htab[h] >= 0);
		}

		/*
		 * New entry, add to table.
		 */
		PutNextCode(lzs, ent);
		ent = c;
		if (lzs->lzw_free_ent < CODE_MAX) 
		{
			lzs->enc_codetab[h] = lzs->lzw_free_ent++;
			lzs->enc_htab[h] = fcode;
		} 
		else 
			if (lzs->enc_incount >= lzs->enc_checkpoint)
				cl_block(lzs);
	hit:
		;
	}
	lzs->lzw_oldcode = ent;
}  /* LZWEncode */

/*
 * Finish off an encoded strip by flushing the last
 * string and tacking on an End Of Information code.
 * Then reset state for the next strip to be encoded.
 */
int LZWPostEncode(lzs)
	LZWState *lzs;
{
	imerror(NoError);
	if (lzs->lzw_oldcode != -1)
		PutNextCode(lzs, lzs->lzw_oldcode);
	PutNextCode(lzs, CODE_EOI);
	FlushData(lzs);
	lzs->enc_ratio = ((lzs->enc_outcount >> 3) * 100) / lzs->enc_incount;
	if (lzs->enc_ratio > 100)
		imerror(LZWRatioerr);
	return(imerrnum);
}

int LZWPreEncode(lzs, dstsize, fp)
	LZWState *lzs;
	int       dstsize;
	FILE     *fp;
{
	imerror(NoError);
	lzs->lzw_rawbuf = (lzwchar *) malloc(dstsize);
	if (lzs->lzw_rawbuf == NULL)
		imerror(NoMemerr);
	lzs->lzw_rbufsize = dstsize;
	lzs->lzw_fp = fp;
	lzs->lzw_processed = 0;
	lzs->lzw_flags = 0;
	lzs->enc_ratio = 0;
	lzs->enc_checkpoint = CHECK_GAP;
	lzs->lzw_maxcode = MAXCODE(lzs->lzw_nbits = BITS_MIN);
	lzs->lzw_free_ent = CODE_FIRST;
	lzs->lzw_bitoff = 0;
	lzs->lzw_bitsize = (dstsize << 3) - (BITS_MAX-1);
	cl_hash(lzs);           /* clear hash table */
	lzs->lzw_oldcode = -1;  /* generates CODE_CLEAR in LZWEncode */
	return(imerrnum);
}  /* LZWPostEncode */


static void PutNextCode(lzs, c)
	LZWState *lzs;
	int       c;
{
	int     r_off, bits, code = c;
	lzwchar  *bp;

	r_off = lzs->lzw_bitoff;
	bits = lzs->lzw_nbits;
		
	/*
	 * Flush buffer if code doesn't fit.
	 */
	if (r_off + bits > lzs->lzw_bitsize) 
	{
		/*
		 * Calculate the number of full bytes that can be
		 * written and save anything else for the next write.
		 */
		if (r_off & 7) 
		{
			lzs->lzw_processed = r_off >> 3;
			bp = lzs->lzw_rawbuf + lzs->lzw_processed;
			FlushData(lzs);        
			lzs->lzw_rawbuf[0] = *bp;
		} 
		else 
			/*
			 * Otherwise, on a byte boundary (in
			 * which tiff_rawcc is already correct).
			 */
			FlushData(lzs);
						
		bp = lzs->lzw_rawbuf;
		lzs->lzw_bitoff = (r_off &= 7);
	}
	else
	{
		/*
		 * Get to the first byte.
		 */
		bp = lzs->lzw_rawbuf + (r_off >> 3);
		r_off &= 7;
	}

	/*
	 * Since code is always >= 8 bits, only
	 * need to mask the first hunk on the left.
	 */
	*bp = (*bp & rmask[r_off]) | (code << r_off) & lmask[r_off];
	bp++;
	bits -= 8 - r_off;
	code >>= 8 - r_off;

	/*
	 * Get any 8 bit parts in the
	 * middle (<=1 for up to 16 bits).
	 */
	if (bits >= 8) 
	{
		*bp++ = code;
		code >>= 8;
		bits -= 8;
	}
	
	if (bits)			/* last bits */
		*bp = code;

	/*
	 * enc_outcount is used by the compression analysis machinery
	 * which resets the compression tables when the compression
	 * ratio goes up.  lzw_bitoff is used here (in PutNextCode) for
	 * inserting codes into the output buffer.  tif_rawcc must
	 * be updated for the mainline write code in TIFFWriteScanline()
	 * so that data is flushed when the end of a strip is reached.
	 * Note that the latter is rounded up to ensure that a non-zero
	 * byte count is present. 
	 */
	lzs->enc_outcount += lzs->lzw_nbits;
	lzs->lzw_bitoff += lzs->lzw_nbits;
	lzs->lzw_processed = (lzs->lzw_bitoff + 7) >> 3;

	/*
	 * If the next entry is going to be too big for
	 * the code size, then increase it, if possible.
	 */
	if (lzs->lzw_flags & FLAG_CODEDELTA) 
	{
		lzs->lzw_maxcode = MAXCODE(lzs->lzw_nbits = BITS_MIN);
		lzs->lzw_flags &= ~FLAG_CODEDELTA;
	} 
	else 
		if (lzs->lzw_free_ent > lzs->lzw_maxcode) 
		{
			lzs->lzw_nbits++;
			if (lzs->lzw_nbits > BITS_MAX)
				lzs->lzw_nbits = BITS_MAX;
			lzs->lzw_maxcode = MAXCODE(lzs->lzw_nbits);
		}
}  /* PutNextCode */


/*
 * deallocate memory used by compression 
 */
void LZWDone(lzs)
	LZWState *lzs;
{

	if (lzs) 
	{
		if (lzs->lzw_rawbuf)
		{
			free(lzs->lzw_rawbuf);
			lzs->lzw_rawbuf = NULL;
		}
		free(lzs);
		lzs = NULL;
	}
}  /* LZWDone */


/*
 * Check compression ratio and, if things seem to
 * be slipping, clear the hash table and reset state.
 */
static void cl_block(lzs)
	LZWState *lzs;
{
	long rat;

	lzs->enc_checkpoint = lzs->enc_incount + CHECK_GAP;
	if (lzs->enc_incount > 0x007fffff) 
	{	/* shift will overflow */
		rat = lzs->enc_outcount >> 8;
		rat = (rat == 0 ? 0x7fffffff : lzs->enc_incount / rat);
	} 
	else
		rat = (lzs->enc_incount << 8) / lzs->enc_outcount; /* 8 fract bits */
	if (rat <= lzs->enc_ratio) 
	{
		lzs->enc_ratio = 0;
		cl_hash(lzs);
		lzs->lzw_free_ent = CODE_FIRST;
		lzs->lzw_flags |= FLAG_CODEDELTA;
		PutNextCode(lzs, CODE_CLEAR);
	} 
	else
		lzs->enc_ratio = rat;
}  /* cl_block */


/*
 * Reset code table.
 */
static void cl_hash(lzs)
	LZWState *lzs;
{
	int *htab_p = lzs->enc_htab+HSIZE;
	long i, m1 = -1;

	i = HSIZE - 16;
 	do {
		*(htab_p-16) = m1;
		*(htab_p-15) = m1;
		*(htab_p-14) = m1;
		*(htab_p-13) = m1;
		*(htab_p-12) = m1;
		*(htab_p-11) = m1;
		*(htab_p-10) = m1;
		*(htab_p-9) = m1;
		*(htab_p-8) = m1;
		*(htab_p-7) = m1;
		*(htab_p-6) = m1;
		*(htab_p-5) = m1;
		*(htab_p-4) = m1;
		*(htab_p-3) = m1;
		*(htab_p-2) = m1;
		*(htab_p-1) = m1;
		htab_p -= 16;
	} while ((i -= 16) >= 0);
    	for (i += 16; i > 0; i--)
		*--htab_p = m1;
}  /* cl_hash */


/*
 *  set elements of p to zero
 */
static void makezero(p, l)
	lzwchar *p;
	long   l;
{
    while (l--)
                *p++ = 0;
}  /* makezero */


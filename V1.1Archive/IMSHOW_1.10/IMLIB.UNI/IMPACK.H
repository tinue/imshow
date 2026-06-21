/*
 *  im : A General Purpose Image (File) Format
 *
 *  Version 0.93(beta)    12/12/1989
 *
 *  Author: Dieter Schaufelberger
 *
 *  Copyright July 1989
 *
 *  University of Zurich, Dept. of Computer Science, Multi-Media Lab,
 *  Winterthurerstrasse 190, CH-8057 Zurich
 */


/*
 * TIFF Library.
 * Rev 5.0 Lempel-Ziv & Welch Compression Support
 * This code is derived from the compress program.
 */

#define MAXCODE(n)	((1 << (n)) - 1)
/*
 * Encoded bit strings range from 9 to 12 bits.  The upper
 * bound is defined by the TIFF spec.  Full color RGB pictures
 * often need about 14 bits for reasonable compression.
 */
#define	BITS_MIN	9		/* start with 9 bits */

#define	BITS_MAX	12		/* max of 12 bit strings */
#define	HSIZE		5003		/* 80% occupancy */
#define	HSHIFT		(8-(16-12))

/* predefined codes */
#define	CODE_CLEAR	256		/* code to clear string table */
#define	CODE_EOI	257		/* end-of-information code */
#define CODE_FIRST	258		/* first free code entry */
#define	CODE_MAX	MAXCODE(BITS_MAX)
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define lzwchar unsigned char

#define lzwshort unsigned short

/*
 * Decoding-specific state.
 */
struct decode {
	short	endofdata;          /* TRUE if all remaining data is in lzw_rawbuf */
	short	prefixtab[HSIZE];	/* prefix(code) */
	lzwchar 	suffixtab[CODE_MAX+1];	/* suffix(code) */
	lzwchar 	stack[HSIZE-(CODE_MAX+1)];  
	lzwchar 	*stackp;		/* stack pointer */
};

/*
 * Encoding-specific state.
 */
struct encode {
	int   checkpoint;          /* point at which to clear table */
#define CHECK_GAP  10000       /* enc_ratio check interval */
	long  ratio;               /* current compression ratio */
	int   incount;             /* (input) data bytes encoded */
	int   outcount;            /* encoded (output) bytes */
	int   htab[HSIZE];         /* hash table */
	short codetab[HSIZE];      /* code table */
};

/*
 * State block for each open file using LZW compression.
 */
typedef	struct 
{
	FILE  *lzw_fp;              /* pointer to outout file */
	int    lzw_processed;       /* the # of processed bytes in rawbuf */
	int    lzw_oldcode;	        /* last code encountered */
	int    lzw_rbufsize;        /* size of the buffer for the compressed data */
	lzwchar *lzw_rawbuf;          /* buffer for the compressed data */
	lzwshort lzw_flags;
#define	FLAG_CODEDELTA  0x1		/* increase code string size */
#define	FLAG_RESTART    0x2     /* restart interrupted decode */
	lzwshort lzw_nbits;           /* number of bits/code */
	int    lzw_maxcode;         /* maximum code for lzw_nbits */
	int    lzw_bitoff;          /* bit offset into data */
	int    lzw_bitsize;         /* size of strip in bits */
	int    lzw_free_ent;		/* next free entry in hash table */
	union 
	{
		struct	decode dec;
		struct	encode enc;
	} u;
} LZWState;

#define	dec_eofdata	u.dec.endofdata
#define	dec_prefix	u.dec.prefixtab
#define	dec_suffix	u.dec.suffixtab
#define	dec_stack	u.dec.stack
#define	dec_stackp	u.dec.stackp

#define	enc_checkpoint  u.enc.checkpoint
#define	enc_ratio       u.enc.ratio
#define	enc_incount     u.enc.incount
#define	enc_outcount    u.enc.outcount
#define	enc_htab        u.enc.htab
#define	enc_codetab     u.enc.codetab

/* masks for extracting/inserting variable length bit codes */
static	lzwchar  lmask[9] =
    { 0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00 };
static	lzwchar  rmask[9] =
    { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };

extern int LZWPreDecode( /* LZWState *lzs, int srcsize */ );
extern int LZWDecode( /* LZWState *lzs, lzwchar *dstbuf, int dstby */ );
extern int LZWPreEncode( /* LZWState *lzs, int dstsize, FILE *fp */ );
extern void LZWEncode( /* LZWState *lzs, lzwchar *srcbuf, int srcby, int (*FlushData)() */ );
extern int LZWPostEncode( /* LZWState *lzs */ );
extern void LZWDone( /* LZWState *lzs */ );


/*  definitions for RLE compression */

#define DUMP	0
#define RUN	1

#define MinRun 3	
#define MaxRun 128
#define MaxDat 128

/* RowBytes computes the number of bytes in a row, from the width in pixels.*/
#define RowBytes(w)   (((w) + 15) >> 4 << 1)

#define GetByte()	  (*source++)
#define PutByte(c)	 { *dest++ = (c);   ++putSize; }
#define UPutByte(c)  (*dest++ = (c))

#define OutDump(nn)   dest = PutDump(dest, nn)
#define OutRun(nn,cc) dest = PutRun(dest, nn, cc)

/*  structure for RLE compression  */
typedef struct
{
	lzwchar   *rawdata;        /* buffer for not yet decoded data */
	int      rd_size;        /* size of rawdate buffer */
	int      rd_es;          /* position from which rawdata is empty */
	int      rd_eb;          /* number of empty bytes in rawdata */
} RLE_STATE;

extern  long PackRow( /* IMBYTE *pSource, IMBYTE *pDest, long rowSize */ );
extern  int UnPackRow(/* IMBYTE *pSource, IMBYTE *pDest, long srcBytes0, 
              lzwshort dstBytes0, int *unpacked_bytes */);
extern  int InitRLEState( /* RLE_STATE *rs, int size */ );


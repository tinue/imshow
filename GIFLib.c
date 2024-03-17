/*******************  Module "GIFLIB.C" Source code file  ******************/
/*                                                                         */
/* MODULE: GIFLIB.C (Procedures to manipulate GIF files)                   */
/*                                                                         */
/*                                                                         */
/* DEVELOPED BY:                                                           */
/* -------------                                                           */
/*  Martin Erzberger, 1989/93                                              */
/*                                                                         */
/*                                                                         */
/* VERSION:                                                                */
/* --------                                                                */
/*  - 2.02, 1/93                                                           */
/*                                                                         */
/*                                                                         */
/* PURPOSE OF MODULE:                                                      */
/* ------------------                                                      */
/*  - Procedures to read GIF images.                                       */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


#define INCL_GPIBITMAPS            /* Used for bitmaps                     */
#define INCL_BITMAPFILEFORMAT      /* Used for bitmap files                */

#define MAX_LWZ_BITS 12

#include <os2.h>                   /* OS/2 headerfile                      */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imshow.h"

static ULONG  ulBytesPerLine;      /* Number of bytes per scanline         */
       ULONG  ulBytesRead;         /* Dummy for DosRead                    */

int GetCode (HFILE, int, int);
int LWZReadByte(HFILE, int, int);

/**********************  Start of procedure gifopen  ***********************/
/*                                                                         */
/* Gifopen opens a gif file to read and gives back its handler.            */
/* If the file isn't a GIF or can't be opened at all, it gives back NULL.  */
/*                                                                         */
/***************************************************************************/
HFILE gifopen (CHAR *szFileName, CHAR *szMode)
{
  HFILE   fGif;                                  /* Filehandler            */
  ULONG   ulActionTaken;                         /* For DosRead            */
  CHAR    buf[6];                                /* Buffer                 */

  if (!strcmp (szMode, "rb")) {                   /* Open to read          */
    DosOpen (szFileName, &fGif, &ulActionTaken, 0L, 0,
             OPEN_ACTION_OPEN_IF_EXISTS |
             OPEN_ACTION_FAIL_IF_NEW,
             OPEN_ACCESS_READONLY       |
             OPEN_SHARE_DENYNONE        |
             OPEN_FLAGS_NOINHERIT       |
             OPEN_FLAGS_NO_CACHE        |       
             OPEN_FLAGS_SEQUENTIAL,             
             0L);

    if (fGif == 0)                               /* Couldn't open          */
      return 0;                                  /* Give back NULL         */

    DosRead (fGif, buf, 6, &ulBytesRead);        /* Header  */
    if (strncmp (buf, "GIF87a", 6) != 0)
      return 0;
    else return fGif;
  } else
    return 0;
}
/***********************  End of procedure gifopen  ************************/



/**********************  Start of procedure gifclose  **********************/
/*                                                                         */
/* Gifclose closes an opened GIF file. The returnvalue is undefined.       */
/*                                                                         */
/***************************************************************************/
VOID gifclose (HFILE fGif)                       /* Filehandler            */
{
  if (fGif != 0)
    DosClose (fGif);
}
/************************  End of procedure gifclose  **********************/



/**********************  Start of procedure gifrhdr  ***********************/
/*                                                                         */
/* Gifrhdr reads in the GIF header. Returnvalue is 0 if successful.        */
/*                                                                         */
/***************************************************************************/
LONG gifrhdr (HFILE hImage, PBITMAPINFO2 pbmiBitmap)
{
  USHORT            i;                              /* Loop counter        */
  CHAR              buf[16];

  DosRead (hImage, buf, 7, &ulBytesRead);
  memset (pbmiBitmap, 0, 64);
  pbmiBitmap->cbFix = 64;
  pbmiBitmap->cx = (ULONG) ((buf[1]<<8) | buf[0]);
  pbmiBitmap->cy = (ULONG) ((buf[3]<<8) | buf[2]);
  pbmiBitmap->cPlanes = 1;
  pbmiBitmap->cBitCount = (USHORT)((buf[4]&0x07) + 1);

  if ((buf[4]&0x80) == 0x80)
    for (i=0; i<(1<<pbmiBitmap->cBitCount);i++) {
      DosRead (hImage, buf, 3, &ulBytesRead);
      pbmiBitmap->argbColor[i].bRed   = buf[0];
      pbmiBitmap->argbColor[i].bGreen = buf[1];
      pbmiBitmap->argbColor[i].bBlue  = buf[2];
    }

    DosRead (hImage, buf, 1, &ulBytesRead);
    if (ulBytesRead == 0)
      return (-IDS_GIF_NOIMG);
    if (buf[0] == ';')
      return (-IDS_GIF_BOGUS);
    if (buf[0] == '!')
      return (-IDS_GIF_BOGUS);
    if (buf[0] != ',')
      return (-IDS_GIF_BOGUS);

    DosRead (hImage, buf, 9, &ulBytesRead);
    if ((buf[8] & 0x40) == 0x40)              /* Interlaced */
      return (-IDS_GIF_INTL);
    pbmiBitmap->cx = (ULONG) ((buf[5]<<8) | buf[4]);
    pbmiBitmap->cy = (ULONG) ((buf[7]<<8) | buf[6]);
    if ((buf[8] & 0x80) == 0x80) {
      pbmiBitmap->cBitCount = (USHORT)((buf[8] & 0x07) + 1);
      for (i=0; i<(1<<pbmiBitmap->cBitCount);i++) {
        DosRead (hImage, buf, 3, &ulBytesRead);
        pbmiBitmap->argbColor[i].bRed   = buf[0];
        pbmiBitmap->argbColor[i].bGreen = buf[1];
        pbmiBitmap->argbColor[i].bBlue  = buf[2];
      }
    }

  ulBytesPerLine = ((pbmiBitmap->cBitCount * pbmiBitmap->cx + 31) / 32)
                   * pbmiBitmap->cPlanes * 4;

  if (DosRead(hImage, buf, 1, &ulBytesRead))
    return (-IDS_GIF_NOIMG);
  if (LWZReadByte (hImage, TRUE, buf[0])<0)
    return (-IDS_GIF_BOGUS);

  return(0);
}
/************************  End of procedure gifrhdr ************************/



/***********************  Start of procedure gifrrows  *********************/
/*                                                                         */
/* Gifrrows reads the given number of rows. There is no return value.      */
/*                                                                         */
/***************************************************************************/
VOID gifrrows (HFILE hImage, BYTE *imbRowBuff, ULONG ulRowLength, ULONG ulNumRows)
{
  register int x, y;
  BYTE     *imbTemp;

  for (y=(int)ulNumRows-1; y>=0; y--) {                    /* Row loop     */
    imbTemp = imbRowBuff + y*ulBytesPerLine;
    for (x=0; x<ulRowLength; x++)                          /* Column loop  */
      imbTemp[x] = (BYTE)LWZReadByte (hImage, FALSE, 1024);
  }
}
/***********************  End of procedure gifrrows  ***********************/



/***************************************************************************/
/*                                                                         */
/* I found this last part (the actual GIF decoder) lying around on our     */
/* Unix server. It had the following disclaimer:                           */
/*                                                                         */
/* +------------------------------------------------------------------+    */
/* | Copyright 1989, David Koblas.                                    |    */
/* |   You may copy this file in whole or in part as long as you      |    */
/* |   don't try to make money off it, or pretend that you wrote it.  |    */
/* +------------------------------------------------------------------+    */
/*                                                                         */
/* Which I don't...                                                        */
/*                                                                         */
/***************************************************************************/
int GetCode(HFILE fd, int code_size, int flag)
{
  static unsigned char   buf[280];
  static int             curbit,lastbit,done,last_byte;
  register int           i, j;
  int                    ret;
  unsigned char          count;

  if (flag) {
    curbit = 0;
    lastbit = 0;
    done = FALSE;
    return 0;
  }

  if ( (curbit+code_size) >= lastbit) {
    if (done) {
      if (curbit>=lastbit)
      exit(1);
    }
    buf[0] = buf[last_byte-2];
    buf[1] = buf[last_byte-1];
    DosRead(fd, &count, 1, &ulBytesRead);
    if (count == 0)
      done = TRUE;
    else
      DosRead(fd, &buf[2], count, &ulBytesRead);
    last_byte = 2 + count;
    curbit = (curbit - lastbit) + 16;
    lastbit = (2+count)<<3 ;
  }

  ret = 0;
  for( i = curbit, j = 0; j < code_size; i++, j++ )
    ret |= ((buf[i >> 3] & (1 << (i % 8))) != 0) << j;

  curbit += code_size;

  return ret;
}

int LWZReadByte(HFILE fd, int flag, int input_code_size)
{
  static int      fresh=FALSE;
  register int    i;
  register int    code,incode;
  static int      code_size,set_code_size;
  static int      max_code,max_code_size;
  static int      firstcode,oldcode;
  static int      clear_code,end_code;
  static int      table[2][(1<< MAX_LWZ_BITS)];
  static int      stack[(1<<(MAX_LWZ_BITS))*2],*sp;

  if (flag) {
    set_code_size = input_code_size;
    code_size = set_code_size+1;
    clear_code = 1 << set_code_size ;
    end_code = clear_code + 1;
    max_code_size = 2*clear_code;
    max_code = clear_code+2;

    GetCode(fd,0,TRUE);

    fresh=TRUE;

    for (i=0;i<clear_code;i++) {
      table[0][i] = 0;
      table[1][i] = i;
    }
    for (;i<(1<<MAX_LWZ_BITS);i++)
      table[0][i] = table[1][0] = 0;

    sp=stack;

    return 0;
  } else if (fresh) {
    fresh = FALSE;
    do {
      firstcode=oldcode=
        GetCode(fd, code_size, FALSE);
    } while (firstcode == clear_code);
    return firstcode;
  }

  if (sp > stack)
  return *--sp;

  while ((code=GetCode(fd,code_size,FALSE))>=0) {
    if (code == clear_code) {
    for (i=0;i<clear_code;i++) {
      table[0][i] = 0;
      table[1][i] = i;
    }
    for (;i<(1<<MAX_LWZ_BITS);i++)
      table[0][i] = table[1][i] = 0;
      code_size = set_code_size+1;
      max_code_size = 2*clear_code;
      max_code = clear_code+2;
      sp=stack;
      firstcode=oldcode=GetCode(fd,code_size,FALSE);
      return firstcode;
    } else if (code == end_code) {
      unsigned char  count;
      unsigned char  junk;

      while ((DosRead(fd,&count,1,&ulBytesRead)==0) && (count!=0))
        while (count-->0 && (DosRead(fd,&junk,1,&ulBytesRead)==0));
      if (count!=0)
      exit(1);
    }

    incode = code;

    if (code >= max_code) {
      *sp++ = firstcode;
      code = oldcode;
    }

    while (code >= clear_code) {
      *sp++ = table[1][code];
      if (code == table[0][code])
        exit(1);
      code = table[0][code];
    }

    *sp++ = firstcode = table[1][code];

    if ((code=max_code)<(1<<MAX_LWZ_BITS)) {
      table[0][code] = oldcode;
      table[1][code] = firstcode;
      max_code++;
      if ((max_code >= max_code_size) &&
        (max_code_size < (1<<MAX_LWZ_BITS))) {
        max_code_size *= 2;
        code_size++;
      }
    }

    oldcode = incode;

    if (sp > stack)
      return *--sp;
  }
  return code;
} /* LWZReadByte */

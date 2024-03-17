/******************  Module "BMLIB.C" Source Code File  ********************/
/*                                                                         */
/* MODULE: BMLib.c (Routines to manage bitmap files)                       */
/*                                                                         */
/*                                                                         */
/* DEVELOPED BY:                                                           */
/* -------------                                                           */
/*  Martin Erzberger, 1989/93                                              */
/*                                                                         */
/*                                                                         */
/* VERSION:                                                                */
/* --------                                                                */
/*  - 2.02,  1/93                                                          */
/*                                                                         */
/*                                                                         */
/* PURPOSE OF MODULE:                                                      */
/* ------------------                                                      */
/*  - Some Functions to manipulate OS/2 bitmap files.                      */
/*                                                                         */
/***************************************************************************/


#define INCL_GPIBITMAPS            /* Bitmap headers etc.                  */
#define INCL_BITMAPFILEFORMAT      /* Bitmapfile format                    */

#include <os2.h>                   /* Main OS/2 header file                */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static ULONG  ulBytesPerLine;      /* Bytes per scanline                   */
static ULONG  ulOffBits;           /* Start of image data in file          */
static LONG   lMoreOffset;         /* Used with bitmap arrays              */


/**************************  Start of bmopen  ******************************/
/*                                                                         */
/* BMOPEN tries to open a OS/2 bitmap file and gives bacj the handler      */
/* to it. If the file can't be opened (either because it is not a bitmap   */
/* or because it can't be opened at all), NULL will be given back.         */
/*                                                                         */
/***************************************************************************/
HFILE bmopen (CHAR *szFileName, CHAR *szMode)
{
  HFILE   fBitmap;                               /* Filehandler            */
  ULONG   ulActionTaken;                         /* Used with DosRead      */
  ULONG   ulBytesRead;                           /* Used eith DosRead      */
  USHORT  usType;                                /* Filetype               */

  if (!strcmp (szMode, "rb")) {                   /* Opened to read?       */
    DosOpen (szFileName, &fBitmap, &ulActionTaken, 0L, 0,
             OPEN_ACTION_OPEN_IF_EXISTS |
             OPEN_ACTION_FAIL_IF_NEW,
             OPEN_ACCESS_READONLY       |
             OPEN_SHARE_DENYNONE        |
             OPEN_FLAGS_NOINHERIT       |
             OPEN_FLAGS_NO_CACHE        |        /* No need to cache it    */
             OPEN_FLAGS_SEQUENTIAL,              /* Mostly read sequential */
             0L);

    if (fBitmap == 0)                            /* Couldn't open it       */
      return 0;                                  /* Give back NULL         */

    DosRead (fBitmap, &usType, sizeof (USHORT), &ulBytesRead);  /* Header  */
    if (usType == BFT_BMAP) {                    /* Is it bitmap?          */
      lMoreOffset = 0;                            /* See below             */
      return fBitmap;                             /* Give back handler     */
    } else if (usType == BFT_BITMAPARRAY) {      /* Bitmap array?          */
      lMoreOffset = 14;                           /* Remove array header   */
      return fBitmap;                             /* Give back handler     */
    } else {
      DosClose (fBitmap);                        /* No bitmap              */
      return 0;                                  /* Give back NULL         */
    }
  } else {                                      /* Open to write           */
    DosOpen (szFileName, &fBitmap, &ulActionTaken, 0L, 0,
             OPEN_ACTION_OPEN_IF_EXISTS |
             OPEN_ACTION_CREATE_IF_NEW,
             OPEN_ACCESS_WRITEONLY      |
             OPEN_SHARE_DENYWRITE       |
             OPEN_FLAGS_NOINHERIT       |
             OPEN_FLAGS_NO_CACHE        |   
             OPEN_FLAGS_SEQUENTIAL,         
             0L);

    if (fBitmap == 0)                            /* Couldn't open          */
      return 0;                                  /* Give back NULL         */
    else return fBitmap;                         /* Or handler             */
  }
}
/***************************  Ende of bmopen  ******************************/



/**************************  Start of  bmclose  ****************************/
/*                                                                         */
/* bmclose closes a opened bitmap file.                                    */
/* There is no return value.                                               */
/*                                                                         */
/***************************************************************************/
VOID bmclose (HFILE fBitmap)                     /* Filehandler            */
{
  if (fBitmap != 0)
    DosClose (fBitmap);
}
/*****************************  End of bmclose  ****************************/



/**********************  Start of procedure bmrhdr  ************************/
/*                                                                         */
/* Bmrhdr reads in the BITMAPINFO2 structure of an OS/2 bitmap file.       */
/* In the BITMAPINFO2 structure is all the information about the file,     */
/* such as size, compression etc. Also in the header is the colormap       */
/* (if there is one).                                                      */
/* The return value of bmrhdr is undefined.                                */
/*                                                                         */
/***************************************************************************/
VOID  bmrhdr (HFILE hImage, PBITMAPINFO2 pbmiBitmap)
{
  ULONG             ulNewPointer;                   /* For DosChgFilePtr   */
  ULONG             ulBytesRead;                    /* For DosRead         */
  ULONG             cbFix;                          /* Header size         */
  BITMAPINFOHEADER  bmpTemp;                        /* Temp. header        */
  RGB               rgbTemp[256];                   /* Temp. colortable    */
  USHORT            i;                              /* Loop counter        */

 /*************************************************************************/
 /* The ULONG on file offset 10 states, where the actual image data       */
 /* starts. This is necessary because the colormap isn't always the same  */
 /* size.                                                                 */
 /*************************************************************************/
  DosSetFilePtr (hImage, 10+lMoreOffset, FILE_BEGIN, &ulNewPointer);
  DosRead (hImage, &ulOffBits, sizeof (ULONG), &ulBytesRead);  /* ULONG   */

 /*************************************************************************/
 /* At file offset 14 starts the BITMAPINFOHEADER2. The first try will be */
 /* the OS/2 2.0 version of the header. With lookint at "cbFix" one can   */
 /* determine if it is an OS/2 1.1 bitmap or a newer one.                 */
 /*************************************************************************/
  DosSetFilePtr (hImage, 14+lMoreOffset, FILE_BEGIN, &ulNewPointer);
  DosRead (hImage, pbmiBitmap, sizeof (BITMAPINFOHEADER2),
            &ulBytesRead);

 /*************************************************************************/
 /* If it is an OS/2 1.1 bitmap ist, the header will be read in again,    */
 /* this time into a BITMAPINFO. The values will then be copied field     */
 /* by field into BITMAPINFO2. The colormap must also be converted        */
 /* (RGB to RGB2).                                                        */
 /* If it is a newer bitmap, it will also be read in again, but this      */
 /* time only the first cbFix values.                                     */
 /*************************************************************************/

  DosSetFilePtr (hImage, 14+lMoreOffset, FILE_BEGIN, &ulNewPointer);

  if (pbmiBitmap->cbFix == 12) {                        /* OS/2 1.1 BMP    */
    memset (pbmiBitmap, 0, sizeof (BITMAPINFO2));       /* Clear header    */
    DosRead (hImage, &bmpTemp, sizeof (BITMAPINFOHEADER), /* Get header    */
             &ulBytesRead);
    pbmiBitmap->cbFix     = bmpTemp.cbFix;          /* Copy it             */
    pbmiBitmap->cx        = (ULONG)bmpTemp.cx;
    pbmiBitmap->cy        = (ULONG)bmpTemp.cy;
    pbmiBitmap->cPlanes   = bmpTemp.cPlanes;
    pbmiBitmap->cBitCount = bmpTemp.cBitCount;
    DosRead (hImage, rgbTemp, ulOffBits-26,         /* Get colormap        */
              &ulBytesRead);                     
    ulBytesRead = (ulBytesRead / 3);
    for (i=0; i<ulBytesRead; i++) {                 /* RGB2 = 4 cytes/color*/
      pbmiBitmap->argbColor[i].bBlue = rgbTemp[i].bBlue;   /* Copy it     */
      pbmiBitmap->argbColor[i].bGreen = rgbTemp[i].bGreen; /* RGB =       */
      pbmiBitmap->argbColor[i].bRed = rgbTemp[i].bRed;    /* 3 bytes/color*/
    }
  } else {                                          /* OS/2 2.0 BMP        */
    cbFix = pbmiBitmap->cbFix;
    memset (pbmiBitmap, 0, sizeof (BITMAPINFO2));       /* Clear header    */
    DosRead (hImage, pbmiBitmap, cbFix, &ulBytesRead);  /* Get header      */
    DosRead (hImage, pbmiBitmap->argbColor,           /* Get colormap      */
             ulOffBits-cbFix-14-lMoreOffset, &ulBytesRead);
  } /* endif */

 /*************************************************************************/
 /* We need this static global variable later:                            */
 /*************************************************************************/
  ulBytesPerLine = ((pbmiBitmap->cBitCount * pbmiBitmap->cx + 31) / 32)
                   * pbmiBitmap->cPlanes * 4;


 /*************************************************************************/
 /* At the end, set the filepointer to the start of the image data:       */
 /*************************************************************************/
  DosSetFilePtr (hImage, (LONG)ulOffBits, FILE_BEGIN, &ulNewPointer);
}
/***********************  End of procedure bmrhdr  ************************/



/**********************  Start of procedure bmwhdr  ************************/
/*                                                                         */
/* The procedure bmwhdr writes the BITMAPINFO2 into a file.                */
/* The return value is undefined.                                          */
/*                                                                         */
/* Please note: imShow doesn't need this procedure, but I wrote also a     */
/* conversion routine (IM to OS/2 BMP), which needs it.                    */
/*                                                                         */
/***************************************************************************/
VOID  bmwhdr (HFILE hImage, PBITMAPINFOHEADER2 pbmiBitmap, RGB2 *pargbColors)
{
  BITMAPFILEHEADER2 bmpFile;                     /* File header            */
  ULONG ulBytesWritten;                          /* For DosRead            */

  memset (&bmpFile, 0, sizeof(BITMAPFILEHEADER2));
  bmpFile.usType = BFT_BMAP;                     /* Type: bitmap           */
  bmpFile.xHotspot = 0;                          /* no hotspot             */
  bmpFile.yHotspot = 0;

  ulBytesPerLine = ((pbmiBitmap->cBitCount * pbmiBitmap->cx + 31) / 32)
                   * pbmiBitmap->cPlanes * 4;

  bmpFile.bmp2.cbFix     = pbmiBitmap->cbFix;      /* Copy header           */
  bmpFile.bmp2.cx        = pbmiBitmap->cx;
  bmpFile.bmp2.cy        = pbmiBitmap->cy;
  bmpFile.bmp2.cPlanes   = pbmiBitmap->cPlanes;
  bmpFile.bmp2.cBitCount = pbmiBitmap->cBitCount;

  if (pbmiBitmap->cBitCount == 24L)        /* No colormap?                 */
    bmpFile.offBits = 54;                  /* --> Header has size 54 bits  */
  else
                                           /* else 54 bits plus size of    */
                                           /* colormap                     */
    bmpFile.offBits = 54 + (sizeof (RGB2))*(1<<(ULONG)(pbmiBitmap->cBitCount));

  ulOffBits = bmpFile.offBits;             /* Set this static variable     */

 /*************************************************************************/
 /* bmpFile.cbSize is the total filesize.                                 */
 /*************************************************************************/
  bmpFile.cbSize = (ulBytesPerLine * pbmiBitmap->cy)
                  + (ULONG)bmpFile.offBits;

  DosWrite (hImage, &bmpFile, 54, &ulBytesWritten);  /* Write header      */

  if (pbmiBitmap->cBitCount != 24L) {
    DosWrite (hImage, pargbColors,                   /* Write colormap    */
              (ULONG)((1<<(pbmiBitmap->cBitCount)) * sizeof (RGB2)),
              &ulBytesWritten);
  }
}
/************************  End of procedure bmwhdr  ************************/




/***********************  Start of procedure bmrrows  **********************/
/*                                                                         */
/* Bmrrows reads in rows of an OS/2 bitmap file.                           */
/* The return value is undefined.                                          */
/*                                                                         */
/* The static variable ulBytesPerLine is used, therefore before using      */
/* bmrrows one must either call bmrhdr or bmwhdr.                          */
/*                                                                         */
/***************************************************************************/
VOID bmrrows (HFILE hImage, BYTE *imbRowBuff, ULONG ulLine, ULONG ulNumLines)
{
  ULONG ulNewPointer;                               /* For DosChgFilePtr   */
  ULONG ulBytesRead;                                /* For DosRead         */

  DosSetFilePtr (hImage, (LONG)(ulLine*ulBytesPerLine+ulOffBits),
                 FILE_BEGIN, &ulNewPointer);
  DosRead (hImage, imbRowBuff,
           ulNumLines * ulBytesPerLine, &ulBytesRead);
}
/***********************  End of procedure bmrrows  ************************/



/****************************  Start of bmwrows  ***************************/
/*                                                                         */
/* bmwrows writes rows into an OS/2 bitmap file.                           */
/* The return value is undefined.                                          */
/*                                                                         */
/* The static variable ulBytesPerLine is used, therefore before using      */                                                                                                                                                                                 
/* bmrrows one must either call bmrhdr or bmwhdr.                          */                                                                                                                                                                                 
/*                                                                         */
/***************************************************************************/
VOID bmwrows (HFILE hImage, BYTE *imbRowBuff, ULONG ulLine, ULONG ulNumLines)
{
  ULONG  ulNewPointer;                              /* For DosChgFilePtr   */
  ULONG  ulBytesWritten;                            /* For DosRead         */

  DosSetFilePtr (hImage, (LONG)(ulLine*ulBytesPerLine+ulOffBits),
                 FILE_BEGIN, &ulNewPointer);
  DosWrite (hImage, imbRowBuff,
            ulNumLines * ulBytesPerLine, &ulBytesWritten);
}
/****************************  End of bmwrows  *****************************/

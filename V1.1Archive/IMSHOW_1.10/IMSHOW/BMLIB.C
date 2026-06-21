/*******************  Modul "BMLIB.C" Source Code File  ********************/
/*                                                                         */
/* MODUL: BMLIB.C (Routinen fÅr OS/2-Bitmaps)                              */
/*                                                                         */
/*                                                                         */
/* PROGRAMMIERT VON:                                                       */
/* -----------------                                                       */
/*  Martin Erzberger, 1989/90                                              */
/*                                                                         */
/*                                                                         */
/* VERSION:                                                                */
/* --------                                                                */
/*  - 1.10, 1/90                                                           */
/*                                                                         */
/*                                                                         */
/* ZWECK DES MODULS:                                                       */
/* -----------------                                                       */
/*  - Hilfsroutinen zur Manipulation von OS/2 Bitmaps.                     */
/*                                                                         */
/*                                                                         */
/* WIE WIRD DAS PROGRAMM COMPILIERT:                                       */
/* ---------------------------------                                       */
/*  - Mit dem MAKEFILE von IMSHOW                                          */
/*                                                                         */
/*                                                                         */
/*  BENôTIGTE SOURCEFILES:                                                 */
/*  ----------------------                                                 */
/*   Siehe bei IMSHOW.C                                                    */
/*                                                                         */
/*                                                                         */
/*  BENôTIGTE LIBRARIES:                                                   */
/*  --------------------                                                   */
/*   Siehe bei IMSHOW.C                                                    */
/*                                                                         */
/*                                                                         */
/*  BENôTIGTE SOFTWARE:                                                    */
/*  -------------------                                                    */
/*   Siehe bei IMSHOW.C                                                    */
/*                                                                         */
/***************************************************************************/


#define INCL_GPIBITMAPS            /* Bezeichner fÅr Bitmaps allgemein     */
#define INCL_BITMAPFILEFORMAT      /* Bezeichner fÅr Bitmap-Fileformat     */

#include <os2.h>                   /* OS/2 Headerfile                      */

#include <mt\stdio.h>              /* Multithread Include-Files, da        */
#include <mt\stdlib.h>             /* IMSHOW mit der Multithread-Library   */
#include <mt\string.h>             /* gelinkt werden muss.                 */


static USHORT usBytesPerLine;      /* Anz. Bytes pro Scanline              */
static ULONG  ulOffBits;           /* Beginn der Bilddaten                 */


/*********************  Beginn der Prozedur bmopen  ************************/
/*                                                                         */
/* Die Prozedur bmopen îffnet ein OS/2 Bitmap File und gibt dessen Handler */
/* zurÅck. Wenn das File zum Lesen geîffnet wird und nicht im richtigen    */
/* Format ist, wird NULL zurÅckgegeben, ebenso wenn das File Åberhaupt     */
/* nicht geîffnet werden kann.                                             */
/*                                                                         */
/***************************************************************************/
FILE *bmopen (CHAR *szFileName, CHAR *szMode)    /* szMode: E/A-Modus      */
{
  FILE   *fBitmap;                               /* Filehandler            */
  USHORT usType;                                 /* Filetyp                */

  fBitmap = fopen (szFileName, szMode);

  if (fBitmap == NULL)                           /* Fehler beim ôffnen     */
    return NULL;                                 /* NULL zurÅck            */

  if (!strcmp (szMode, "rb")) {                   /* Zum Lesen geîffnet    */
    fread (&usType, sizeof (USHORT), 1, fBitmap); /* Erste 2 Bytes lesen   */
    if (usType == BFT_BMAP)                       /* Ist es OS/2 Bitmap?   */
      return fBitmap;                             /* ja: Handler zurÅck    */
    else {
      fclose (fBitmap);                           /* Nein: File schliessen */
      return NULL;                                /* NULL zurÅck           */
    }
  } else
    return fBitmap;     /* Im richtigen Format oder zum Schreiben geîffnet */
}
/***********************  Ende der Prozedur bmopen  ************************/



/*********************  Beginn der Prozedur bmclose  ***********************/
/*                                                                         */
/* Die Prozedur bmclose schliesst ein geîffnetes OS/2 Bitmap File.         */
/* Der Returnwert ist undefiniert.                                         */
/*                                                                         */
/***************************************************************************/
VOID bmclose (FILE *fBitmap)                     /* File-Handler           */
{
  if (fBitmap != NULL)
    fclose (fBitmap);
}
/***********************  Ende der Prozedur bmclose  ***********************/



/*********************  Beginn der Prozedur bmrhdr  ************************/
/*                                                                         */
/* Die Prozedur bmrhdr liest den BIMAPINFO eines OS/2 Bitmaps ein.         */
/* Der BITMAPINFO eines OS/2 Bitmaps besteht aus Informationen Åber        */
/* Bildtyp, Bildgrîsse etc. sowie einer Farbtabelle (ausser bei            */
/* RGB-Bildern).                                                           */
/* Der Returnwert ist undefiniert.                                         */
/*                                                                         */
/***************************************************************************/
VOID  bmrhdr (FILE *hImage, PBITMAPINFO pbmiBitmap)
{

 /*************************************************************************/
 /* Der ULONG bei File-Offset 10 gibt an, wo die eigentlichen Bilddaten   */
 /* beginnen. Dies ist nîtig, weil die Farbtabelle unterschiedlich lang   */
 /* sein kann.                                                            */
 /*************************************************************************/
  fseek (hImage, 10L, SEEK_SET);                 /* Pointer auf Offset 10  */
  fread (&ulOffBits, sizeof (ULONG), 1, hImage); /* ULONG einlesen         */

 /*************************************************************************/
 /* Bei Offset 14 beginnt der BITMAPINFO. Dieser Datentyp besteht aus     */
 /* einem fixen Header (Dem BITMAPINFOHEADER) plus der Farbtabelle (ein   */
 /* Array aus RGB's). Obwohl die Farbtabelle unterschiedlich lang sein    */
 /* kann (sie kann bei einem 24-Bit RGB-Bild sogar ganz fehlen) werden    */
 /* hier der Einfachheit halber 256 Werte eingelesen. Dies ist die        */
 /* maximale Grîsse einer Farbtabelle.                                    */
 /*************************************************************************/
  fseek (hImage, 14L, SEEK_SET);
  fread (pbmiBitmap, sizeof (BITMAPINFOHEADER) + 256*sizeof (RGB), 1, hImage);

 /*************************************************************************/
 /* Die folgende Formel findet man auf Seite 7-123 des "Operating System/2*/
 /* 1.1 Technical Reference, Volume 1.                                    */
 /*************************************************************************/
  usBytesPerLine = ((pbmiBitmap->cBitCount * pbmiBitmap->cx + 31) / 32)
                   * pbmiBitmap->cPlanes * 4;

 /*************************************************************************/
 /* Zum Schluss wird der Filepointer auf den Beginn der Bilddaten gesetzt:*/
 /*************************************************************************/
  fseek (hImage, (long int)ulOffBits, SEEK_SET);
}
/***********************  Ende der Prozedur bmrhdr  ************************/



/*********************  Beginn der Prozedur bmwhdr  ************************/
/*                                                                         */
/* Die Prozedur bmwhdr schreibt den BITMAPINFO eines Bildes in ein File.   */
/* Der Returnwert ist undefiniert.                                         */
/*                                                                         */
/***************************************************************************/
VOID  bmwhdr (FILE *hImage, PBITMAPINFOHEADER pbmiBitmap, RGB *pargbColors)
{
  BITMAPFILEHEADER bmpFile;                      /* File-Header            */

  bmpFile.usType = BFT_BMAP;                     /* Typ: Bitmap            */
  bmpFile.xHotspot = 0;                          /* kein Hotspot           */
  bmpFile.yHotspot = 0;

 /*************************************************************************/
 /* Die folgende Formel findet man auf Seite 7-123 des "Operating System/2*/
 /* 1.1 Technical Reference, Volume 1.                                    */
 /*************************************************************************/
  usBytesPerLine = ((pbmiBitmap->cBitCount * pbmiBitmap->cx + 31) / 32)
                   * pbmiBitmap->cPlanes * 4;

  bmpFile.bmp.cbFix     = pbmiBitmap->cbFix;      /* Header umkopieren     */
  bmpFile.bmp.cx        = pbmiBitmap->cx;
  bmpFile.bmp.cy        = pbmiBitmap->cy;
  bmpFile.bmp.cPlanes   = pbmiBitmap->cPlanes;
  bmpFile.bmp.cBitCount = pbmiBitmap->cBitCount;

  if (pbmiBitmap->cBitCount == 24L)        /* keine Farbtabelle            */
    bmpFile.offBits = 26;                  /* --> Header ist 26 Bits gross */
  else
                                           /* sonst 26 Bits + Grîsse der   */
                                           /* Farbtabelle                  */
    bmpFile.offBits = 26 + (sizeof (RGB))*(1<<(pbmiBitmap->cBitCount));

  ulOffBits = bmpFile.offBits;             /* statische Variable setzen    */

 /*************************************************************************/
 /* bmpFile.cbSize gibt die totale Filegrîsse an.                         */
 /*************************************************************************/
  bmpFile.cbSize = (usBytesPerLine * pbmiBitmap->cy) + bmpFile.offBits;

  fwrite (&bmpFile, sizeof bmpFile, 1, hImage); /* Header schreiben        */

  if (pbmiBitmap->cBitCount != 24L) {
    fwrite (pargbColors,                  /* Farbtabelle schreiben         */
            sizeof (RGB),
            1<<(pbmiBitmap->cBitCount),
            hImage);
  }
}
/***********************  Ende der Prozedur bmwhdr  ************************/



/********************  Beginn der Prozedur bmrowalloc  *********************/
/*                                                                         */
/* Die Prozedur bmrowalloc alloziert den Speicherplatz fÅr eine Zeile      */
/* eines OS/2 Bitmap Bildes und gibt einen Pointer auf den allozierten     */
/* Speicherbereich zurÅck. Wenn der Speicher nicht alloziert werden kann,  */
/* wird stattdessen NULL zurÅckgegeben.                                    */
/*                                                                         */
/* Die statische Variable usBytesPerLine wird verwendet. Deshalb muss vor  */
/* bmrowalloc entweder bmrhdr oder bmwhdr aufgerufen werden!               */
/*                                                                         */
/***************************************************************************/
BYTE *bmrowalloc (VOID)
{
  BYTE   *pbBuffer;                            /* Pointer auf Puffer       */



  pbBuffer = (PBYTE) malloc (usBytesPerLine);

  return pbBuffer;   /* Kann auch NULL sein (bei zu wenig Speicher)        */
}
/**********************  Ende der Prozedur bmrowalloc  *********************/



/**********************  Beginn der Prozedur bmrrow  ***********************/
/*                                                                         */
/* Die Prozedur bmrrow liest eine Zeile eines OS/2 Bitmap Bildes ein.      */
/* Der Returnwert ist undefiniert.                                         */
/*                                                                         */
/* Die statische Variable usBytesPerLine wird verwendet. Deshalb muss vor  */
/* bmrrow entweder bmrhdr oder bmwhdr aufgerufen werden!                   */
/*                                                                         */
/***************************************************************************/
VOID bmrrow (FILE *hImage, BYTE *imbRowBuff, USHORT usLine)
{
  fseek (hImage, (LONG)usLine*(LONG)usBytesPerLine+ulOffBits, SEEK_SET);                 /* Pointer auf Offset 10  */
  fread (imbRowBuff, usBytesPerLine, 1, hImage);
}
/**********************  Ende der Prozedur bmrrow  *************************/



/**************************  Beginn von bmwrow  ****************************/
/*                                                                         */
/* bmrrow schreibt eine Zeile eines OS/2-'bitmap'-Bildes.                  */
/* Der Returnwert ist undefiniert.                                         */
/*                                                                         */
/* Die statische Variable usBytesPerLine wird verwendet. Deshalb muss vor  */
/* bmwrow entweder bmrhdr oder bmwhdr aufgerufen werden!                   */
/*                                                                         */
/***************************************************************************/
VOID bmwrow (FILE *hImage, BYTE *imbRowBuff, USHORT usLine)
{
  fseek (hImage, (LONG)usLine*(LONG)usBytesPerLine+ulOffBits, SEEK_SET);                 /* Pointer auf Offset 10  */
  fwrite (imbRowBuff, 1, usBytesPerLine, hImage);
}
/***************************  Ende von bmwrow  *****************************/

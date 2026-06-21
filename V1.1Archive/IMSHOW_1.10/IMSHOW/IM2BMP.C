/*******************  Modul "IM2BMP.C" Source Code File  *******************/
/*                                                                         */
/* MODUL: IM2BMP.C (Datei-Konversion)                                      */
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
/*  - Konvertierung von 'im' zu OS/2-'Bitmap' Format.                      */
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

#define INCL_GPIBITMAPS                /* Header fÅr Bitmaps               */
#define INCL_BITMAPFILEFORMAT          /* Header fÅr Bitmapfiles           */
#include <os2.h>                       /* OS/2 Headerfile                  */

#include <stdio.h>
#include <stdlib.h>

#include "bmlib.h"                     /* Bitmap - Subroutinen             */
#include "im.h"                        /* Header der im-Library            */


 /*************************************************************************/
 /*                                                                       */
 /*   Das SchlÅsselwort 'cdecl' ist eine Option des Microsoft             */
 /*   C-Compilers. Es muss bei OS/2 Programmen stehen, kann aber          */
 /*   bei anderen C-Compilers problemlos weggelassen werden.              */
 /*                                                                       */
 /*************************************************************************/
BOOL cdecl main(int argc, char *argv[])
{

USHORT           x,y;                    /* Schlaufenvariablen             */
USHORT           usTemp;                 /* TemporÑrvariable               */
USHORT           usType;                 /* Bildtyp                        */
USHORT           usDepth;                /* Bittiefe                       */
USHORT           usSamples;              /* Samples / Pixel                */
USHORT           usColors;               /* Anzahl Farben                  */
IMAGE           *imBild;                 /* Pointer auf im-Bild            */
FILE            *bmBild;                 /* Pointer auf Bitmap-Bild        */
BITMAPINFOHEADER bmiBitmap;              /* Header eines Bitmap-Bildes     */
RGB             *pargbColors;            /* Farbtabelle                    */
PBYTE            pbTemp;                 /* TemporÑrvariable               */
PBYTE            pbRowBuff;              /* Pointer auf Zeilenpuffer       */
IMBYTE          *imbRowBuff;             /* Pointer auf Zeilenpuffer       */
IMBYTE          *pimMap;                 /* Farbtabelle                    */
IMPIXEL          impPixel;               /* RGB-Punkt                      */


  if (argc != 3)                         /* MÅssen drei Argumente sein     */
    return FALSE;

  imBild = imopen (argv[1], "r");        /* im-Bild îffnen                 */
  if (imBild == NULL)
    return FALSE;

  bmBild = bmopen (argv[2], "wb");       /* Bitmap-Bild îffnen             */
  if (bmBild == NULL)
    return FALSE;

  imrhdr (imBild);                       /* Header einlesen                */

  bmiBitmap.cbFix   = 12L;                /* Bitmap-Header fÅllen          */
  bmiBitmap.cx      = imgetxsize(imBild);
  bmiBitmap.cy      = imgetysize(imBild);
  bmiBitmap.cPlanes = 1;

  usType    = imgettype (imBild);        /* Bildtyp etc. abfragen          */
  usDepth   = imgetdepth (imBild);
  usSamples = imgetsamples (imBild);
  usColors  = imgetcolors (imBild);

                                         /* Zeilenpuffer allozieren:       */
  imbRowBuff = imrowalloc (bmiBitmap.cx, usSamples, usDepth);
    if (imbRowBuff == NULL) return 1;

  switch (usType) {

    case IMRGB:                          /* RGB-Bild                       */
                                         /* 2. Zeilenpuffer allozieren:    */
      pbRowBuff = malloc (bmiBitmap.cx * sizeof (RGB));
      if (pbRowBuff == NULL)
        return 0;
      bmiBitmap.cBitCount = 24;          /* Rest des Bitmapheaders setzen  */
      bmwhdr (bmBild, &bmiBitmap, NULL); /* Header schreiben               */

      /*******************************************************************/
      /*  OS/2 Bitmap Bilder stehen im Vergleich zu im-Bildern auf dem   */
      /*  Kopf. Deshalb zÑhlt man die Schlaufe rÅckwarts.                */
      /*******************************************************************/
      for (y=bmiBitmap.cy; y>0; y--) {
        imrrow (imBild, imbRowBuff);    /* Zeile einlesen                  */
        for (x=0; x<bmiBitmap.cx; x++) {
          /***************************************************************/
          /* Beim im-Format ist der Zeilenpuffer Farbenweise abgelegt,   */
          /* d.h. RRRGGGBBB. OS/2 braucht die Zeile aber Pel-weise,      */
          /* d.h. RGBRGBRGB. dies wird mit den folgenden Zeilen gemacht: */
          /***************************************************************/
          pbTemp = pbRowBuff+x+x+x;
          *(pbTemp+2) = *(imbRowBuff+x                          );
          *(pbTemp+1) = *(imbRowBuff+x+bmiBitmap.cx             );
          *(pbTemp  ) = *(imbRowBuff+x+bmiBitmap.cx+bmiBitmap.cx);
        } /* Ende for x */
        bmwrow (bmBild, pbRowBuff, y-1); /* umgerechnete Zeile schreiben   */
      } /* Ende for y */
      break;

    case IMMAP:   /* Farbtabellenbilder */

       /******************************************************************/
       /* Im 1. Teil wird der Header mit der Farbtabelle umgesetzt:      */
       /******************************************************************/
      pimMap = (IMBYTE *)immapalloc (usColors); /* Tabelle allozieren      */
      imsetcmap (imBild, (IMPIXEL *)pimMap);    /* Pointer setzen          */
      imrcmap (imBild);                         /* Tabelle lesen           */


      if (usColors > 257) {                     /* mehr als 256 Farben     */
        bmiBitmap.cBitCount = 24;               /* -> RGB abspeichern      */
        bmwhdr (bmBild, &bmiBitmap, NULL);      /* Header schreiben        */

      } else {

        pargbColors = malloc (usColors*sizeof(RGB)); /* Farbtabelle fÅr    */
                                                     /* Bitmap-Bild        */

        if (usColors > 16) {         /* 32 bis 256 Farben: 8 Bits Tiefe    */
          bmiBitmap.cBitCount = 8;   /*                                    */
        } else if (usColors > 4) {   /*  8 bis  16 Farben: 4 Bits Tiefe    */
          bmiBitmap.cBitCount = 4;   /*                                    */
        } else if (usColors > 2) {   /*          4 Farben: 2 Bits Tiefe    */
          bmiBitmap.cBitCount = 2;   /*                                    */
        } else {                     /*          2 Farben: 1 Bit Tiefe     */
          bmiBitmap.cBitCount = 1;   /*                                    */
        }                            /*                                    */

        for (x = 0; x < usColors; x++) {
          pargbColors[x].bBlue  = *(pimMap + 4*x + 2); /* Farbtabelle      */
          pargbColors[x].bGreen = *(pimMap + 4*x + 1); /* konvertieren     */
          pargbColors[x].bRed   = *(pimMap + 4*x);
        } /* Ende for */
        bmwhdr (bmBild, &bmiBitmap, pargbColors); /* Header schreiben      */
      }

       /*******************************************************************/
       /* Im 2. Teil werden die Bilddaten umgesetzt:                      */
       /*******************************************************************/
      if (usColors < 257) {   /* Bei 256 Farben oder weniger ist das       */
                              /* Zeilenformat kompatibel und die Zeilen    */
                              /* mÅssen lediglich umkopiert werden.        */
                              /* Es wird wieder rÅckwÑrts gezÑhlt          */
        for (y=bmiBitmap.cy; y>0; y--) {
          imrrow (imBild, imbRowBuff);      /* Zeile einlesen...           */
          bmwrow (bmBild, imbRowBuff, y-1); /* ... und schreiben           */
        } /* Ende for */
      } else {                /* Das im-Format kennt Colormap-Bilder mit */
                              /* bis zu 65536 Farben (16 Bit). OS/2      */
                              /* kennt aber hîchstens 8 Bit Bilder mit   */
                              /* Farbtabelle. Deshalb muss ein im-Bild   */
                              /* mit mehr als 256 Farben im ein RGB-Bild */
                              /* (24 Bit) umgewandelt werden.            */
                              /* FÅr die umwandlung braucht man einen    */
                              /* zweiten Zeilenpuffer.                   */
        pbRowBuff = malloc (bmiBitmap.cx * sizeof (RGB));
        for (y=bmiBitmap.cy; y>0; y--) { /* RÅckwÑrts zÑhlen               */
          imrrow (imBild, imbRowBuff);   /* Zeile einlesen                 */
          /***************************************************************/
          /* Jetzt wird Punkt fÅr Punkt mit der 'ideal-pixel' Funktion   */
          /* 'imrpix' in einen 24-Bit umgewandelt und im Zeilenpuffer    */
          /* in der richtigen Reigenfolge abgelegt:                      */
          /***************************************************************/
          for (x=0; x<bmiBitmap.cx; x++) {
            impPixel = imrpix (imBild, imbRowBuff, x); /* Punkt lesen      */
            pbRowBuff[x+x+x+2] = (BYTE)((impPixel & 0x0000FF));
            pbRowBuff[x+x+x+1] = (BYTE)((impPixel & 0x00FF00) >>  8);
            pbRowBuff[x+x+x  ] = (BYTE)((impPixel & 0xFF0000) >> 16);
          }
          bmwrow (bmBild, pbRowBuff, y-1); /* Zeile schreiben              */
        } /* Ende for y */
      } /* Ende if */
    break;

    case IMGRY: /* Graustufenbilder */
     /*********************************************************************/
     /* 1. Teil: Header mit Farbtabelle schreiben. (OS/2 kennt keine      */
     /*          Graustufenbilder. Deshalb muss man ein normales Farb-    */
     /*          tebellenbild erstellen, wobei die einzelnen Farbwerte    */
     /*          lauter Graustufen sind).                                 */
     /*********************************************************************/
      pargbColors = malloc ((1<<usDepth)*sizeof(RGB)); /* Farbtabelle      */
      if (usDepth < 3) {                 /* 1 oder 2 Bit Tiefe             */
        bmiBitmap.cBitCount = usDepth;
      } else if (usDepth < 5) {          /* 3 oder 4 Bit --> 4 Bit         */
        bmiBitmap.cBitCount = 4;
      } else {                           /* 5 bis 8 Bit  --> 8 Bit         */
        bmiBitmap.cBitCount = 8;
      } /* Ende if */

      usTemp = (255 / ((1<<usDepth)-1));  /* Farbtabelle errechnen         */
      for (x = 0; x < (1<<usDepth); x++) { /* usTemp = Farbabstufung       */
        pargbColors[x].bBlue  = (BYTE)(x * usTemp); /* Drei gleiche Werte  */
        pargbColors[x].bGreen = (BYTE)(x * usTemp); /* fÅr R, G, und B     */
        pargbColors[x].bRed   = (BYTE)(x * usTemp); /* --> Grauwert        */
      } /* Ende for */

      bmwhdr (bmBild, &bmiBitmap, pargbColors); /* Header schreiben        */

      for (y=bmiBitmap.cy; y>0; y--) {     /* Das Zeilenformat ist gleich  */
        imrrow (imBild, imbRowBuff);       /* Zeile einlesen...            */
        bmwrow (bmBild, imbRowBuff, y-1);  /* ...und schreiben             */
      } /* Ende for */
    break;

    case IMFLT: /* Floating-Point Bilder */
    break;      /* --> nicht unterstÅtzt */

  } /* Ende switch */

  bmclose (bmBild);  /* Files schliessen */
  imclose (imBild);
}
/***********************  Ende von im2bmp()  *******************************/

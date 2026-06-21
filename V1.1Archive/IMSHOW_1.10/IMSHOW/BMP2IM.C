/*******************  Modul "BMP2IM.C" Source Code File  *******************/
/*                                                                         */
/* MODUL: BMP2IM.C (Datei-Konversion)                                      */
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
/*  - Konvertierung vom OS/2-'Bitmap' ins 'im'-Format.                     */
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
#include "im.h"                        /* Header der 'im'-Library          */

 /*************************************************************************/
 /*                                                                       */
 /*   Das SchlÅsselwort 'cdecl' ist eine Option des Microsoft             */
 /*   C-Compilers. Es muss bei OS/2 Programmen stehen, kann aber          */
 /*   bei anderen C-Compilers problemlos weggelassen werden.              */
 /*                                                                       */
 /*************************************************************************/
BOOL cdecl main(int argc, char *argv[])
{

USHORT           x,y;                     /* Schlaufenvariablen            */
USHORT           usColors;                /* Anzahl Farben                 */
IMAGE           *imBild;                  /* Pointer auf im-Bild           */
FILE            *bmBild;                  /* Pointer auf Bitmap-Bild       */
PBITMAPINFO      pbmiBitmap;              /* Header des Bitmap-Bilder      */
PBYTE            pbTemp;                  /* TemporÑrvariable              */
PBYTE            pbRowBuff;               /* Pointer auf Zeilenpuffer      */
IMBYTE          *imbRowBuff;              /* Pointer auf Zeilenpuffer      */
IMPIXEL         *pimMap;                  /* Pointer auf Farbtabelle       */


  if (argc != 3)                          /* MÅssen drei Argumente sein    */
    return FALSE;

  bmBild = bmopen (argv[1], "rb");        /* Bitmap-Bild îffnen            */
  if (bmBild == NULL)
    return FALSE;

  imBild = imopen (argv[2], "w");         /* im-Bild îffnen                */
  if (imBild == NULL)
    return FALSE;

                                          /* Puffer fÅr Header allozieren  */
  pbmiBitmap = malloc (sizeof (BITMAPINFOHEADER) + 256*sizeof (RGB));
  if (pbmiBitmap == NULL)
    return FALSE;
  bmrhdr (bmBild, pbmiBitmap);            /* Header einlesen               */

  imsetxsize (imBild, pbmiBitmap->cx);    /* Parameter des im-Bildes       */
  imsetysize (imBild, pbmiBitmap->cy);    /*  setzen                       */
  imsetstor  (imBild, IMRAW);             /* Keine Komprimierung           */

  if (pbmiBitmap->cBitCount == 24) {      /* RGB-Bild                      */
    imsettype (imBild, IMRGB);
    imsetdepth (imBild, 8);               /* Bittiefe 8                    */
    imsetsamples (imBild, 3);             /* 3 Samples / Pixel             */

    imwhdr (imBild);                      /* Header schreiben              */

                                          /* Zeilenpuffer allozieren       */
    imbRowBuff = imrowalloc (pbmiBitmap->cx, 3, 8);
    if (imbRowBuff == NULL)
      return FALSE;
                                          /* 2. Zeilenpuffer allozieren    */
    pbRowBuff  = malloc (pbmiBitmap->cx * sizeof (RGB));
    if (pbRowBuff == NULL)
      return FALSE;

      /*******************************************************************/
      /*  OS/2 Bitmap Bilder stehen im Vergleich zu im-Bildern auf dem   */
      /*  Kopf. Deshalb zÑhlt man die Schlaufe rÅckwarts.                */
      /*******************************************************************/
    for (y=pbmiBitmap->cy; y>0; y--) {
      bmrrow (bmBild, pbRowBuff, y-1);    /* Zeile einlesen                */
      for (x=0; x<pbmiBitmap->cx; x++) {
        /***************************************************************/
        /* Beim im-Format ist der Zeilenpuffer Farbenweise abgelegt,   */
        /* d.h. RRRGGGBBB. OS/2 liefert die Zeile aber Pel-weise,      */
        /* d.h. RGBRGBRGB. dies wird mit den folgenden Zeilen um-      */
        /*                 gerechnet:                                  */
        /***************************************************************/
        pbTemp = pbRowBuff+x+x+x;
        *(imbRowBuff+x                              ) = *(pbTemp+2);
        *(imbRowBuff+x+pbmiBitmap->cx               ) = *(pbTemp+1);
        *(imbRowBuff+x+pbmiBitmap->cx+pbmiBitmap->cx) = *(pbTemp  );
        } /* Ende for x */
      imwrow (imBild, imbRowBuff);  /* Umgerechnete Zeile schreiben        */
    } /* Ende for y */

  } else {  /* OS/2 kennt keine Graustufenbilder */

    imsettype (imBild, IMMAP);                  /* Typ Colormap            */
    imsetdepth (imBild, pbmiBitmap->cBitCount);
    imsetsamples (imBild, 1);
    usColors = (1<<(pbmiBitmap->cBitCount));    /* Farben = 2^Bittiefe     */
    imsetcolors (imBild, usColors);

    imwhdr (imBild);                            /* Header schreiben        */

    pimMap = immapalloc (usColors);             /* Colormap allozieren     */
    if (pimMap == NULL)
      return FALSE;
    imsetcmap (imBild, pimMap);                 /* Pointer setzen          */

    for (x = 0; x < usColors; x++) {            /* Colormap umkopieren     */
      *((BYTE *)pimMap + 4*x + 2) = pbmiBitmap->argbColor[x].bBlue;
      *((BYTE *)pimMap + 4*x + 1) = pbmiBitmap->argbColor[x].bGreen;
      *((BYTE *)pimMap + 4*x    ) = pbmiBitmap->argbColor[x].bRed;
    } /* Ende for */

    imwcmap (imBild);                            /* Colormap schreiben     */

                                 /* Zeilenpuffer allozieren:               */
    imbRowBuff = imrowalloc (pbmiBitmap->cx, 1, pbmiBitmap->cBitCount);
    if (imbRowBuff == NULL)
      return FALSE;

    for (y=pbmiBitmap->cy; y>0; y--) {   /* RÅckwÑrts zÑhlen               */
      bmrrow (bmBild, imbRowBuff, y-1);  /* Zeile lesen...                 */
      imwrow (imBild, imbRowBuff);       /* ...und schreiben               */
    } /* Ende for */
  } /* Ende if */

  imclose (imBild);                      /* Files schliessen               */
  bmclose (bmBild);
}
/***********************  Ende von im2bmp()  *******************************/

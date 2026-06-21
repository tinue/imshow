/**************  Semesterarbeit "IMSHOW.C" Source Code File  ***************/
/*                                                                         */
/* PROGRAMM-NAME: IMSHOW (Anzeige eines Bildes im "im"-Format.)            */
/*                                                                         */
/*                                                                         */
/* PROGRAMMIERT VON:                                                       */
/* -----------------                                                       */
/*  Martin Erzberger, 1989/90                                              */
/*                                                                         */
/*                                                                         */
/* ZWECK DES PROGRAMMS:                                                    */
/* --------------------                                                    */
/*  - Anzeige von Bildern im "im" - und OS/2 - "Bitmap" - Format.          */
/*  - Vorlage fÅr weitere, Ñhnliche Programme.                             */
/*  - Liefert Entscheidungsgrundlagen fÅr die Beantwortung der Frage       */
/*    "Ist OS/2 geeignet zur Bildverarbeitung und -darstellung?"           */
/*                                                                         */
/*                                                                         */
/* VERSION: 1.10, 1/90                                                     */
/* -------------------                                                     */
/*                                                                         */
/*                                                                         */
/* PROGRAMMGESCHICHE:                                                      */
/* ------------------                                                      */
/*  - 1.00, 12/89: Erste Ausgabe                                           */
/*  - 1.10,  1/90: Konvertierroutinen hinzugefÅgt,                         */
/*                 UnterstÅtzt neue im-Library (0.93 beta), d.h.           */
/*                  - Verbesserte Fehlerbehandlung,                        */
/*                  - Liest komprimierte Bilder,                           */
/*                  - Effizientere Speichermethode fÅr IMMAP-Bilder,       */
/*                 Userinterface auf Englisch,                             */
/*                 Userinterface SAA-konform.                              */
/*                                                                         */
/*                                                                         */
/* WIE WIRD DAS PROGRAMM COMPILIERT:                                       */
/* ---------------------------------                                       */
/*                                                                         */
/*  "make all"                                                             */
/*                                                                         */
/*  BENôTIGTE SOURCEFILES:                                                 */
/*  ----------------------                                                 */
/*                                                                         */
/*                                                                         */
/*    MAKEFILE       - FÅr make-Utility                                    */
/*    IMSHOW.C       - Source-Code                                         */
/*    IMSHOW.DEF     - Modulbeschreibung fÅr den Linker                    */
/*    IMSHOW.H       - Header File der Applikation                         */
/*    IMSHOW.ICO     - Icon                                                */
/*    IMSHOW.RC      - Resourcen (Menus, Texte)                            */
/*    BMLIB.C        - Prozeduren fÅr Bitmap-Handling                      */
/*    BMLIB.H        - Header dazu                                         */
/*    FILEDLG.C      - Prozeduren fÅr die "File Open"-Dialogbox            */
/*    IM2BMP.C       - Konvertierroutine im --> bmp                        */
/*    IM2BMP.DEF     - Modulbeschreibung                                   */
/*    BMP2IM.C       - Konvertierroutine bmp --> in                        */
/*    BMP2IM.DEF     - Modulbeschreibung                                   */
/*                                                                         */
/*                                                                         */
/*  BENôTIGTE LIBRARIES:                                                   */
/*  --------------------                                                   */
/*                                                                         */
/*    IMLIB.LIB      - Import-Library "im"                                 */
/*    IMLIB.DLL      - "im"-Library (erst zur Laufzeit)                    */
/*                                                                         */
/*  BENôTIGTE SOFTWARE:                                                    */
/*  -------------------                                                    */
/*                                                                         */
/*    'im' Library                                                         */
/*    IBM Operating System/2, Version 1.1, Standard oder Extended Edition  */
/*    IBM C/2 Compiler, Version 1.1                                        */
/*    IBM Operating System/2 1.1 Toolkit                                   */
/*                                                                         */
/*  LITERATUR:                                                             */
/*  ----------                                                             */
/*    IBM Operating System/2 1.1 Technical Reference,                      */
/*     IBM Corporation, 1988,                                              */
/*                                                                         */
/*    Kerninghan, Brian W., Ritchie, Dennis M.:                            */
/*     "The C Programming Language", Second Edition,                       */
/*     Prentice Hall, 1988, 1978, ISBN: 0-13-110362-8                      */
/*                                                                         */
/*    Petzold Charles: "Programming the OS/2 Presentation Manager",        */
/*     Microsoft Press, 1989, ISBN: 1-55615-170-5                          */
/*                                                                         */
/*    Schildt Herbert: "OS/2 Programming: An Introduction",                */
/*     McGraw-Hill, 1988, ISBN: 0-07-881427-8                              */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

#define INCL_WIN                       /* Window-Manager Headerfiles       */
#define INCL_GPIBITMAPS                /* Header fÅr Bitmaps               */
#define INCL_GPILOGCOLORTABLE          /* Header fÅr log. Farbtabelle      */
#define INCL_GPIPRIMITIVES             /* Box, Line etc.                   */
#define INCL_BITMAPFILEFORMAT          /* Header fÅr Bitmapfiles           */
#define INCL_ERRORS                    /* Fehlerdefinitionen               */
#define INCL_DOSSESMGR                 /* Sessionmanager                   */

#include <os2.h>                       /* OS/2 Header-File                 */
#include <mt/stdlib.h>                 /* Multi Thread-Library!            */
#include <mt/stdio.h>
#include <mt/string.h>                 /* Stringverarbeitung               */
#include <mt/process.h>                /* FÅr _CreateThread()              */

#include "imshow.h"                    /* Allgemeine Bezeichner            */
#include "im.h"                        /* 'im' - Library                   */
#include "bmlib.h"                     /* Bitmap - Subroutinen             */

#define STACKSIZE 4096                 /* Stack fÅr asynchronen Thread     */



/***************************************************************************/
/* Variabeln zur Steuerung des Presentation-Managers.                      */
/***************************************************************************/

HAB          hab;                       /* PM anchor block Handler         */
HMQ          hmq;                       /* Message queue Handler           */
HWND         hwndClient;                /* Client area window Handler      */
HWND         hwndFrame;                 /* Frame window Handler            */
HWND         hwndHscroll, hwndVscroll;  /* Scroll-Bar Handlers             */
HWND         hwndDlg;                   /* Dialog-Box Handler              */
HDC          hdc;                       /* Screen-Device-Context Handler   */
HDC          hdcMem;                    /* Memory-Device-Context Handler.  */
HPS          hps;                       /* Presentation Space Handler      */
HPS          hpsMem = NULL;             /* Memory Pres. Space Handler      */
HPS          hpsScaled = NULL;          /* Skalierter Bitmap Handler       */
HBITMAP      hbm = NULL;                /* Bitmap Handler                  */
HBITMAP      hbmCopy = NULL;            /* Bitmap fÅr Clipboard            */
PLONG        plColTable;                /* Pointer auf Farbtabelle         */
PBITMAPINFO  pbmiBitmap;                /* Pointer auf Bitmapinfo          */
ULONG    flCreate = FCF_STANDARD   |    /* Window Kontroll-Flags           */
                    FCF_VERTSCROLL |    /*  --> Standard Window plus       */
                    FCF_HORZSCROLL;     /*      Scrollbars                 */
CHAR     titleBar[80] = "IMSHOW";       /* Title-Bar Text                  */
TRACKINFO    tiStruct;                  /* Info fÅr Mouse-Track            */


/***************************************************************************/
/* Sonstige Steuervariabeln.                                               */
/***************************************************************************/

BOOL   fErrMem = FALSE;                 /* Flag fÅr Memory-Fehler          */
BOOL   fCmdLine = FALSE;                /* Flag fÅr Command-Line Argument  */
BOOL   fFill = TRUE;                    /* Flag fÅr Hintergrund fÅllen     */
BOOL   fScroll = TRUE;                  /* Flag fÅr Scrollen/Komprimieren  */
BOOL   fMaximized = FALSE;              /* Nur IMSHOW sichtbar?            */
BOOL   fRealizePossible = FALSE;        /* Realizing Åberhaupt mîglich?    */
BOOL   fRealized = FALSE;               /* Ist ColorTable realisiert?      */
ULONG  semInputReady;                   /* Ein Filename ist bereit         */
ULONG  semPictureReady;                 /* Bild ist bereit zur Anzeige     */
ULONG  semHalt;                         /* Das Laden soll gestoppt werden  */
TID    tidLoad;                         /* Thread-ID des Lade-Threads      */


/***************************************************************************/
/* Variabeln der Applikation.                                              */
/***************************************************************************/

USHORT   usType;                        /* Bildtyp                         */
USHORT   usStorage;                     /* Speichermethode                 */
USHORT   usSamples;                     /* Anzahl Sampels pro Pixel        */
USHORT   usDepth;                       /* Bit-Tiefe Pro Sample            */
USHORT   usColors;                      /* Anzahl Farben des Bildes        */
CHAR     szFileSource[80];              /* Filename des Bildes             */
CHAR     szFileDest[80];                /* Filename des umgewandelten B.   */
IMAGE   *imBild;                        /* Pointer auf 'im'-Bild           */
FILE    *hImage;                        /* Pointer auf 'Bitmap'-Bild       */
IMBYTE  *imbRowBuff;                    /* Pointer auf Zeilenpuffer        */

USHORT usDisplayColors;                 /* Anzahl Farben des Bildschirms   */
BOOL   bRealizeSupported;               /* Realizing unterstÅtzt?          */

int    iSizeX = 0, iSizeY = 0;          /* Grîsse des Bildes               */
SHORT  cxClient, cyClient;              /* Fenstergrîsse                   */
SHORT  cxMaximized, cyMaximized;        /* Fenstergrîsse wenn maximized    */
SHORT  cxEdge, cyEdge;                  /* Ecke beim Scrollen              */


/***************************************************************************/
/* Funktionen (Prototypen), diese werden im Programmtext kommentiert.      */
/***************************************************************************/

VOID cdecl       main (int argc, char *argv[]);
VOID             CreateThread (VOID);
VOID             Error (HAB);
MRESULT EXPENTRY WinProc (HWND, USHORT, MPARAM, MPARAM);
VOID             Init (HWND);
VOID             Paint (HWND);
VOID             SetScrollBars (VOID);
BOOL             MakeColTable (PBITMAPINFO, PLONG *);
MRESULT EXPENTRY AboutDlgProc (HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY CancelDlgProc (HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY ParamDlgProc (HWND, USHORT, MPARAM, MPARAM);
VOID             CentreDlgBox(HWND, HWND);
VOID             Help (HWND);
BOOL             OpenFile (HAB, HDC, HWND, HBITMAP *);
BOOL             WriteFile (VOID);
VOID       cdecl ReadFile ();
HPS              ScaleBitmap(VOID);
VOID             MouseButtonDown(HWND, MPARAM);
HBITMAP          GetBitmap(VOID);
VOID             DrawRect(VOID);
VOID             RemoveRect (VOID);
VOID             ShowMsg (INT, INT, USHORT);
VOID             ImErrorMsg (int);

extern BOOL cdecl GetFileName (CHAR *, HWND, HWND, BOOL);


/*************************  Beginn von main()  *****************************/
/*                                                                         */
/* Die main() Prozedur hat 3 Aufgaben:                                     */
/*  -Initialisieren des ganzen Systems (Fenster erstellen,                 */
/*                                      Threads starten, etc.).            */
/*  -Weiterleiten der Systemmeldungen zum richtigen Fenster.               */
/*  -Abschliessen und beenden der Applikation.                             */
/***************************************************************************/

VOID cdecl main(int argc, char *argv[]) /* Die main()-Prozedur muss mit    */
                                        /* 'cdecl' deklariert werden.      */
{
  QMSG qmsg;                            /* Message aus der Message-queue   */

 /*************************************************************************/
 /*            Initialisierungsteil der main()-Routine                    */
 /*************************************************************************/
  if (argc == 2) {                      /* Wenn ein Command-Line Argument  */
    strcpy (szFileSource, argv[1]);     /* vorliegt, wird es in szFileSource */
    fCmdLine = TRUE;                    /* kopiert, das Flag wird gesetzt  */
  }

  hab = WinInitialize(NULL);            /* PM initialisieren               */

  hmq = WinCreateMsgQueue(hab, 0);      /* Message-Queue erstellen         */

  WinRegisterClass(                     /* Window-Klasse registrieren      */
                   hab,                 /* Anchor-block Handler            */
                   "IMSHOWWindow",      /* Klassen-Name                    */
                   WinProc,             /* Adresse der Window-Prozedur     */
                   CS_SIZEREDRAW |
                   CS_SYNCPAINT,        /* Stil der Klasse                 */
                   0                    /* Keine speziellen Window-Words   */
                  );

  hwndFrame = WinCreateStdWindow(       /* Erstellen des Windows           */
               HWND_DESKTOP,            /* Parent ist das Desktop-Window   */
               WS_VISIBLE,              /* Das Fenster ist sichtbar        */
               (PULONG)&flCreate,       /* Frame control flags             */
               "IMSHOWWindow",          /* Klassen-Name                    */
               "",                      /* Fenster-Titel (leer lassen!)    */
               0L,                      /* Kein spezieller Stil            */
               NULL,                    /* Alle Ressourcen im EXE-File     */
               ID_RESOURCE,             /* ID der Ressourcen               */
               (PHWND)&hwndClient       /* "Client window" Handler         */
              );

  DosSemSet (&semInputReady);           /* Filename nicht bereit           */
  DosSemSet (&semPictureReady);         /* Bild noch nicht geladen         */

  CreateThread();                       /* Lade-Thread starten             */

 /*************************************************************************/
 /* Hauptaufgabe der main-Prozedur: Meldungen des PM empfangen und        */
 /* weiterleiten an die Window-Prozedur. Dies, bis eine WM_QUIT-Message   */
 /* empfangen wird. Dann wird die Applikation beendet.                    */
 /*************************************************************************/
  while (WinGetMsg(hab, (PQMSG)&qmsg, (HWND)NULL, 0, 0))
    WinDispatchMsg(hab, (PQMSG)&qmsg);

 /*************************************************************************/
 /*        Abschliessen der Applikation, Freigeben der Ressourcen:        */
 /*************************************************************************/
  WinDestroyWindow(hwndFrame);          /* Fenster zerstîren.              */
  WinDestroyMsgQueue(hmq);              /* Message-Queue zerstîren.        */
  WinTerminate(hab);                    /* Applikation beenden.            */
}

/**************************  Ende von main()  ******************************/



/**********************  Beginn von CreateThread()  ************************/
/*                                                                         */
/* CreateThread() startet den Thread fÅr das Laden des Files.              */
/* Dazu wird zuerst Speicher alloziert (fÅr den Stack), anschliessend      */
/* wird die C-Funktion _beginthread aufgerufen, um den Thread zu starten.  */
/*                                                                         */
/* Der Stack kînnte auch mit malloc() alloziert werden. Das                */
/* Folgende ist aber ein Beispiel, wie mit OS/2-Funktionen die meisten     */
/* C/2-Library-Funktionen ersetzt werden kînnen.                           */
/*                                                                         */
/***************************************************************************/

VOID CreateThread(VOID)
{
  SEL   segment;         /* Selector zum allozierten Speichersegment       */
  PBYTE stkBot;          /* Pointer zum Stack                              */


  DosAllocSeg(STACKSIZE, (PSEL)&segment, SEG_NONSHARED);
  SELECTOROF(stkBot) = segment;       /* SELECTOROF kopiert den Selector-  */
                                      /* Teil in einen Pointer.            */
                                      /* Ein Selector entspricht einem     */
                                      /* Segment des 80286 Real-Mode.      */
  OFFSETOF(stkBot) = 0;               /* OFFSETOF kopiert den Offset-Teil. */
  tidLoad = _beginthread(ReadFile,    /* Adresse der Function, die asyn-   */
                                      /* chron gestartet werden soll.      */
                         stkBot,      /* Pointer zum Stack.                */
                         STACKSIZE-2, /* Grîsse des Stacks.                */
                         NULL);       /* Argumente (hier keine).           */
}

/***********************  Ende von CreateThread()  *************************/



/*************************  Start von Error()  *****************************/
/*                                                                         */
/* Die Prozedur Error ist eine reine Debug-Hilfe. Sie zeigt den            */
/* letzten aufgetretenen Fehler an.                                        */
/* Wenn der Fehler wegen zu wenig Memory auftritt, kann die Funktion       */
/* WinGetErrInfo nicht verwendet werden. Deshalb muss noch mit einem       */
/* Flag (fErrMem) gearbeitet werden.                                       */
/* Leider ist es keine ganz befriedigende Lîsung: Wenn sie z.B wÑhrend     */
/* einer WM_PAINT-Message aufgerufen wird, fÅhrt dies zu einem Deadlock.   */
/* Eine bessere Lîsung ist kaum mîglich, man mÅsste mit einem Error-Log    */
/* File auf der Festplatte arbeiten. Die Routine ist aber eine Reine       */
/* Debug-Hilfe und hier nur noch als Beispiel vorhanden.                   */
/*                                                                         */
/***************************************************************************/

VOID Error(HAB hab)
{
  PERRINFO  perriBlk;              /* Variable fÅr die Fehlerinformationen */
  PSZ       pszErrMsg;             /* String fÅr die Fehlermeldung.        */
  USHORT *  TempPtr;

  if (!fErrMem){
      perriBlk = WinGetErrorInfo(hab);
      if (perriBlk == (PERRINFO)NULL){
          return;
     }
     SELECTOROF(pszErrMsg) = SELECTOROF(perriBlk); /* Etwas Pointerarith-  */
     SELECTOROF(TempPtr) = SELECTOROF(perriBlk);   /* metik, um die Meldung*/
     OFFSETOF(TempPtr) = perriBlk->offaoffszMsg;   /* in pszErrMsg zu be-  */
     OFFSETOF(pszErrMsg) =  *TempPtr;              /* kommen.              */
     WinMessageBox(HWND_DESKTOP,                   /* Anzeigen der Meldung.*/
                   hwndFrame,
                   (PSZ)(pszErrMsg),
                   (PSZ)titleBar,
                   0,
                   MB_CUACRITICAL | MB_ENTER
                  );
     WinFreeErrorInfo(perriBlk);
  } else {
     WinMessageBox(HWND_DESKTOP,
                   hwndFrame,
                   (PSZ)"FEHLER - Kein Speicher mehr!",
                   (PSZ)titleBar,
                   0,
                   MB_CUACRITICAL | MB_ENTER
                  );
    }
}

/***************************  Ende von Error()  ****************************/



/*********************  Start der Window-Prozedur  *************************/
/*                                                                         */
/* Die Window-Prozedur bearbeitet die Meldungen, die von OS/2 gesendet     */
/* werden. Dabei werden (mittels einer Reihe von CASE-Statements) nur      */
/* diejenigen Meldungen bearbeitet, an denen die Applikation interessiert  */
/* ist. Alle anderen werden zur Behandlung an OS/2 geschickt.              */
/*                                                                         */
/***************************************************************************/

MRESULT EXPENTRY WinProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{

  switch(msg)                      /* Die Variable msg enthÑlt die Meldung */
  {

    case WM_CREATE:                /* Fenster wurde soeben erstellt.       */
      Init(hwnd);                  /* --> Prozedur Init()                  */
      break;

    case WM_COMMAND:               /* Eine Menuauswahl wurde getroffen.    */
      switch (SHORT1FROMMP(mp1))   /* in mp1 steht, welche Auswahl es ist  */
      {

        case IDM_OPEN:                              /* "File, open"        */
          if (OpenFile(hab, hdcMem, hwnd, &hbm))    /* --> Funktion        */
            WinInvalidateRect( hwnd, NULL, FALSE);  /*  "OpenFile"         */
          break;

        case IDM_WRITE:                             /* "File, Save as.."   */
          WriteFile();
          break;

        case IDM_QUIT:              /* Bei Quit wird die Applikat. beendet.*/
          WinPostMsg(hwnd, WM_CLOSE, 0L, 0L);
          break;

        case IDM_ABOUT:            /* About-Box wird angezeigt:            */
          WinDlgBox(HWND_DESKTOP,  /* Parent-Handler (fÅr Clipping)        */
                    hwnd,          /* Owner-Handle (bekommt Messages)      */
                    AboutDlgProc,  /* Dialog-Procedure                     */
                    NULL,          /* Template im EXE-File                 */
                    IDD_ABOUT,     /* Dialog-Template IDD_ABOUT            */
                    NULL);         /* keine anderen Daten                  */
          break;

        case IDM_COPY:             /* "Edit, Copy"                         */
          hbmCopy = GetBitmap();   /* Bitmap des ausgew. Rechtecks holen   */
          if (hbmCopy != (HBITMAP)NULL) {
            WinOpenClipbrd (hab);           /* Clipboard îffnen und Bitmap */
                                            /* handler hinein stellen.     */
            WinSetClipbrdData (hab, (ULONG)hbmCopy, CF_BITMAP, 0L);
            WinCloseClipbrd (hab);          /* Clipboard schliessen.       */
            RemoveRect();                   /* Rechteck wieder entfernen   */
          } /* Ende if */
          break;

        case IDM_SCROLL:                    /* "View, Scroll"              */
          WinSendDlgItemMsg (                         /* Message an Menus  */
            hwndFrame, FID_MENU, MM_SETITEMATTR,      /* Set Attribute     */
            MPFROM2SHORT (IDM_SCROLL, TRUE),          /* Menu "IDM_SCROLL" */
            MPFROM2SHORT (MIA_CHECKED, MIA_CHECKED)); /* Checkmark setzen  */

          WinSendDlgItemMsg (                      /* Bei "IDM_COMP" die   */
            hwndFrame, FID_MENU, MM_SETITEMATTR,   /* Checkmark lîschen    */
            MPFROM2SHORT (IDM_COMP, TRUE),
            MPFROM2SHORT (MIA_CHECKED, 0));

          fScroll = TRUE;                         /* Flag setzen           */

          SetScrollBars();                        /* Init. Scroll-Bars     */

          hpsScaled = ScaleBitmap();              /* Bitmap "entskalieren" */

          fFill = TRUE;                           /* Hintergrund lîschen   */
          WinInvalidateRect( hwnd, NULL, FALSE);  /* Neu zeichnen          */

          break;

        case IDM_COMP:                            /* "View, Fit"           */
          WinSendDlgItemMsg (                        /* Bei IDM_SCROLL die */
            hwndFrame, FID_MENU, MM_SETITEMATTR,     /* Checkmark lîschen..*/
            MPFROM2SHORT (IDM_SCROLL, TRUE),
            MPFROM2SHORT (MIA_CHECKED, 0));

          WinSendDlgItemMsg (
            hwndFrame, FID_MENU, MM_SETITEMATTR,     /* und bei IDM_COMP   */
            MPFROM2SHORT (IDM_COMP, TRUE),           /* setzen.            */
            MPFROM2SHORT (MIA_CHECKED, MIA_CHECKED));

          fScroll = FALSE;                           /* Flag lîschen       */
          SetScrollBars();                           /* Scrollbars initial.*/
          hpsScaled = ScaleBitmap();                 /* Bitmap skalieren   */
          WinInvalidateRect (hwnd, NULL, FALSE);     /* Fenster zeichnen   */
          break;

        case IDM_PARAM:            /* "Parameter"-Dialog-Box anzeigen:     */
          WinDlgBox(HWND_DESKTOP,  /* Parent-Handler (fÅr Clipping)        */
                    hwnd,          /* Owner-Handle (erhÑlt Messages)       */
                    ParamDlgProc,  /* Dialog-Procedure                     */
                    NULL,          /* Template im EXE-File                 */
                    IDD_PARAM,     /* Dialog-Template IDD_PARAM            */
                    NULL);         /* keine anderen Daten                  */
          break;

        case IDM_HELPHELP:         /* "Help, Help for Help"                */
          Help(hwnd);
          break;

        default:
          break;

      } /* Ende der "Menu-switches" */
      break;


    case WM_ERASEBACKGROUND:       /* Der Hintergrund muss gelîscht werden.*/
      fFill = TRUE;                /* Flag setzen                          */
      return (MRESULT)(FALSE);     /* --> Wird von Applikation erledigt.   */

    case WM_PAINT:                 /* Das Fenster muss neu gezeichnet      */
      Paint(hwnd);                 /*  werden. --> Prozedur Paint()        */
      break;

    case WM_SIZE:                  /* Die Fenstergrîsse wurde geÑndert     */
      cxClient = SHORT1FROMMP(mp2); /* Neue Grîsse des Fensters auslesen   */
      cyClient = SHORT2FROMMP(mp2);

                                    /* Wurde das Fenster "maximized"?      */
      fMaximized = ((cxClient == cxMaximized) && (cyClient == cyMaximized));

      SetScrollBars();              /* Scroll Bars neu skalieren           */

      hpsScaled = ScaleBitmap();    /* Bitmap Skalieren                    */

      WinInvalidateRect (hwnd, NULL, FALSE);     /* Fenster zeichnen   */

      break;

    case WM_ACTIVATE:              /* Fenster bekommt oder verliert Focus  */
      if (fMaximized && !(CHAR1FROMMP(mp1))) {
        fMaximized = FALSE;        /* Fenster kam in den Hintergrund -->   */
                                   /* das Flag wird gelîscht, um den Bild- */
                                   /* schirm wieder auf die normalen       */
                                   /* Farben einzustellen.                 */

        WinInvalidateRect (hwnd, NULL, FALSE); /* Fenster neu malen        */
      }
      break;

    case WM_VSCROLL:                /* Meldung der rechten Scroll-Bar      */
      switch (SHORT2FROMMP (mp2)) {

        case SB_LINEUP:             /* Oberer Pfeil angeklickt             */
          cyEdge--;                 /* Nullpunkt neu setzen                */
          break;

        case SB_PAGEUP:             /* öber dem Slider wurde geklickt      */
          cyEdge -= (iSizeY / 10);  /* in grîsseren Schritten scrollen     */
          break;

        case SB_PAGEDOWN:           /* Unter dem Slider wurde geklickt     */
          cyEdge += (iSizeY / 10);
          break;

        case SB_LINEDOWN:           /* Unterer Pfeil wurde angeklickt      */
          cyEdge++;
          break;

        case SB_SLIDERTRACK:        /* Der Slider wird bewegt              */
          cyEdge = SHORT1FROMMP (mp2);   /* Position auslesen              */
          WinInvalidateRect (hwnd, NULL, FALSE);
          return 0;                 /* Der Slider selbst wird nicht neu    */
                                    /* gesetzt, deshalb hier: return 0;    */

        case SB_SLIDERPOSITION:     /* Erst hier wird der Slider neu ge-   */
          cyEdge = SHORT1FROMMP (mp2);   /* setzt. Dies ergibt auf der     */
          break;                    /* Scrollbar keine unschînen Neben-    */
                                    /* effekte.                            */

        default: break;

      } /* Ende der switches fÅr die rechte Scroll-Bar                     */
        /* Es folgen generelle Scroll-Bar Routinen, die von allen 'cases'  */
        /* nach dem 'break' erreicht werden. (Ausnahme: siehe oben)        */

      cyEdge = max (0, cyEdge);               /* Wertebereich sicher-      */
      cyEdge = min (iSizeY-cyClient, cyEdge); /* stellen                   */

            /* Position des Sliders neu setzen: */
        WinSendMsg (hwndVscroll, SBM_SETPOS, MPFROMSHORT (cyEdge), NULL);
            /* Fenster neu zeichnen: */
        WinInvalidateRect (hwnd, NULL, FALSE);

      break; /* Der break aus WM_VSCROLL */

    case WM_HSCROLL:                   /* Meldung der unteren Scrollbar    */
      switch (SHORT2FROMMP (mp2)) {

        case SB_LINELEFT:              /* linker Pfeil                     */
          cxEdge--;
          break;

        case SB_PAGELEFT:              /* links vom Slider                 */
          cxEdge -= (iSizeX / 10);
          break;

        case SB_PAGERIGHT:             /* rechts vom Slider                */
          cxEdge += (iSizeX / 10);
          break;

        case SB_LINERIGHT:             /* rechter Pfeil                    */
          cxEdge++;
          break;

        case SB_SLIDERTRACK:           /* Slider wird bewegt               */
          cxEdge = SHORT1FROMMP (mp2);
          WinInvalidateRect (hwnd, NULL, FALSE);
          return 0;                /* --> siehe oben (bei WM_VSCROLL)      */

        case SB_SLIDERPOSITION:
          cxEdge = SHORT1FROMMP (mp2);   /* Position auslesen              */
          break;

      } /* Ende des VM_HSCROLL-switch */

      /* Siehe oben (bei VM_VSCROLL)                                       */

      cxEdge = max (0, cxEdge);               /* Wertebereich sicher-      */
      cxEdge = min (iSizeX-cxClient, cxEdge); /* stellen                   */

        WinSendMsg (hwndHscroll, SBM_SETPOS, MPFROMSHORT (cxEdge), NULL);

        WinInvalidateRect (hwnd, NULL, FALSE);

      break; /* Break aus VM_HSCROLL */

    case WM_BUTTON1DOWN:               /* Rechteck zeichnen                */
      MouseButtonDown(hwnd, mp1);
      break;

    default:
      break;

  } /* Ende des Window-Procedure-'switch' */

  /* Zur Sicherheit werden die Meldungen auch noch an OS/2 weitergegeben   */
  /* Dies MUSS man tun fÅr alle Meldungen, die man nicht selbst bearbeitet */
  /* (hier wird dies erreicht durch das 'default: break;'-Statement).      */

  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}
/**********************  Ende der Window-Prozedur  *************************/



/*************************  Start von Init()  ******************************/
/*                                                                         */
/*  Initialisierungen beim Erîffnen des Fensters.                          */
/*                                                                         */
/*  Die Initialisierungen fÅr das Hauptfenster werden an dieser Stelle     */
/*  vorgenommen, das erst hier das Fenster erstellt ist und so             */
/*  z.B dessen Grîsse abgefragt werden kann.                               */
/*                                                                         */
/***************************************************************************/

VOID Init(HWND hwnd)    /* Der Window-Handler wird als Parameter Åbergeben */
                        /* die restlichen Elemente der WM_CREATE-Message   */
                        /* werden nicht verwendet.                         */
{
  SIZEL sizlTemp;

  /* Als erstes wird ein "Memory Device Context" erîffnet.                 */
  /* Dieser wird spÑter fÅr die Erstellung eines Bitmaps im Speicher       */
  /* verwendet.                                                            */

  hdcMem = DevOpenDC(hab,                     /* Anchor Block Handler      */
                     OD_MEMORY,               /* Memory Device Context     */
                     (PSZ)"*",                /* keine Device-Information  */
                     0L,                      /* keine Datenblîcke         */
                     NULL,                    /* Datenblîcke               */
                     (HDC)NULL                /* Kompatibel zu Screen-DC   */
                    );
  if (hdcMem == (HDC)NULL)
    Error(hab);


  /* Da die Scroll-Bar ID's relativ hÑufig verwendet werden, werden sie    */
  /* gleich zu Beginn festgestellt und gespeichert. Die Variable           */
  /* "hwndFrame" ist zu diesem Zeitpunkt noch nicht definiert, deshalb     */
  /* muss die etwas kompliziertere Methode mit QW_PARENT verwendet werden: */

  hwndHscroll = WinWindowFromID
                 (WinQueryWindow (hwnd, QW_PARENT, FALSE), FID_HORZSCROLL);

  hwndVscroll = WinWindowFromID
                 (WinQueryWindow (hwnd, QW_PARENT, FALSE), FID_VERTSCROLL);

  /* Die Titlebar wird gesetzt:                                            */
  WinSetWindowText (WinQueryWindow (hwnd, QW_PARENT, FALSE), titleBar);

  /* Die Grîsse eines 'maximized' Fensters wird abgefragt:                 */
  cxMaximized = (SHORT)WinQuerySysValue (HWND_DESKTOP, SV_CXFULLSCREEN);
  cyMaximized = (SHORT)WinQuerySysValue (HWND_DESKTOP, SV_CYFULLSCREEN);

  sizlTemp.cx = cxMaximized;
  sizlTemp.cy = cyMaximized;

  hdc = WinOpenWindowDC (hwnd);
  hps = GpiCreatePS (hab, hdc, &sizlTemp, GPIA_ASSOC | PU_PELS);

  /* Test, ob ein File mit dem eingegebenen Namen existiert:               */
  if (fCmdLine) {                         /* Nur, wenn auch ein Argument   */
    if (fopen (szFileSource, "rb") == NULL) /* eingegeben wurde              */
      fCmdLine = FALSE;                   /* kein File --> Flag lîschen    */
    else {
      fcloseall();                        /* File wieder schliessen        */
                                          /* Eine Menuauswahl simulieren:  */
      WinPostMsg (hwndFrame, WM_COMMAND, MPFROMSHORT(IDM_OPEN), NULL);
    } /* Ende if (fopen...) */
  } /* Ende if (FCmdLine) */
} /* Ende Init() */

/*************************  Ende von Init()  *******************************/



/************************  Beginn von Paint()  *****************************/
/*                                                                         */
/*  Bearbeitet die WM_PAINT-Message der Window-Prozedur                    */
/*                                                                         */
/*  Eine WM_PAINT-Message wird immer dann gesendet, wenn das Fenster un-   */
/*  gÅltig geworden ist. Dies ist z.B. dann der Fall, wenn ein das Fenster */
/*  teilweise verdeckt war, und dann wieder frei wird.                     */
/*                                                                         */
/*  Eine WM_PAINT Meldung kann  forciert werden, indem man selber das      */
/*  Fenster als ungÅltig erklÑrt (Mit WinInvalidateRect).                  */
/*                                                                         */
/***************************************************************************/

VOID       Paint(HWND hwnd)  /* Nur der Window-Handler wird Åbergeben,     */
                             /* die anderen Parameter der WM_PAINT-Message */
                             /* sind NULL und somit nicht interessant.     */
{
  RECTL rc;                  /* Update-Region.                             */
  POINTL pt[3];              /* Rechtecke fÅr GpiPlayBitmap                */


  WinQueryUpdateRect (hwnd, &rc);


/***************************************************************************/
/* Die Prozesse mÅssen nicht synchronisiert werden, deshalb wird im        */
/* Prinzip nur der Wert der Semaphore ausgelesen. Dies geschieht, indem    */
/* ein Time-Out-Value von 0 gewÑhlt und danach getestet wird, ob ein       */
/* Time-Out erfolgt ist (=Die Semaphore war besetzt).                      */
/* Semaphoren sollen NIE direkt abgefragt werden.                          */
/* Wenn die Semaphore besetzt war, ist das Bild noch nicht geladen         */
/* und es wird einfach das ganze Fenster schwarz gemalt:                   */
/***************************************************************************/
  if (DosSemWait (&semPictureReady, 0L) == ERROR_SEM_TIMEOUT)
    WinFillRect (hps, &rc, CLR_BLACK);

  else {
    if (fFill) {                        /* fFill zeigt an, ob zuerst das   */
                                        /* ganze Fenster gelîscht werden   */
                                        /* muss. Die ist z.B. der Fall,    */
                                        /* wenn es vergrîssert wurde. Dann */
                                        /* enthÑlt es noch Reste des ur-   */
                                        /* sprÅnglichen Bildes.            */
      WinFillRect (hps, &rc, CLR_BLACK);

    } /* Ende if (fFill) */

    fFill = FALSE;                      /* Flag gleich wieder lîschen      */

     /*********************************************************************/
     /* Alle Elemente, die mit der logischen Farbtabelle zu tun haben,    */
     /* sind noch nicht getestet, da sie erst unter der Version 1.2 von   */
     /* OS/2 laufen.                                                      */
     /*********************************************************************/
    if (fMaximized & fRealizePossible) {  /* soll die Farbtabelle          */
                                          /* realisiert werden?            */
          /* Wenn ja, wird zuerst eine logische Farbtabelle erstellt,      */
          /* welche dann realisiert wird:                                  */

      GpiCreateLogColorTable (hps, LCOL_REALIZABLE, LCOLF_CONSECRGB,
                              0L, (LONG)usColors, plColTable);
      GpiRealizeColorTable (hps);
      fRealized = TRUE;   /* Flag setzen                                   */
    } else {
       /* Hier ist die Farbtabelle zwar realisiert, dÅrfte aber nicht      */
       /* mehr realisiert sein --> Die Realisierung wird rÅckgÑngig gemacht*/
      if (fRealized) {
        GpiUnrealizeColorTable (hps);
      } /* Ende if */
    } /* Ende if */


    pt[0].x = 0;                     /* Dest.-Bitmap, unten links          */
    pt[0].y = 0;                     /*                                    */
    pt[1].x = cxClient;              /* Dest.-Bitmap, Grîsse               */
    pt[1].y = cyClient;              /*                                    */
    pt[2].x = cxEdge;                /* Source Bitmap, unten links         */
    pt[2].y = cyEdge;                /*                                    */


    GpiBitBlt(hps,                   /* Dest. Handler                      */
              hpsScaled,             /* Source-Handler                     */
              3L,                    /* Anzahl Daten (3 oder 4)            */
              pt,                    /* Daten                              */
              ROP_SRCCOPY,           /* Dest. einfach Åbermalen            */
              BBO_IGNORE             /* beim Komprimieren die vorigen      */
                                     /* Zeilen einfach auslassen           */
             );


  }  /* Ende if */

  if (!WinIsRectEmpty (hab, &tiStruct.rclTrack))
    DrawRect();

  WinValidateRect (hwnd, &rc, FALSE); /* Das Fenster als gÅltig erklÑren.  */
}
/*************************  Ende von Paint()  ******************************/



/**********************  Start von SetScrollBars()  ************************/
/*                                                                         */
/* Initialisiert die Scroll-Bars nach einer WM_SIZE Message, wenn ein      */
/* neues File geladen worden ist oder wenn die "Optionen" geÑndert worden  */
/* sind.                                                                   */
/*                                                                         */
/***************************************************************************/

VOID SetScrollBars(VOID)
{
  cxEdge = 0;       /* Das Bild wird immer mit der unteren linken Ecke     */
  cyEdge = 0;       /* sichtbar gezeigt                                    */

  if (fScroll) {                               /* 1. Teil: Scrollen        */
    if (cxClient >= iSizeX) {                  /* Fenster zu gross         */
      WinEnableWindow (hwndHscroll, FALSE);    /* -> Scrollbar ausschalten */
    } else {
      WinEnableWindow (hwndHscroll, TRUE);     /* Scrollbar einschalten    */

      WinSendMsg (hwndHscroll, SBM_SETSCROLLBAR, /* Scrollbar neu skalie-  */
                  MPFROM2SHORT (0, 0),           /* ren und den Slider auf */
                  MPFROM2SHORT (0, iSizeX-cxClient)); /* 0 setzen          */
    }

    if (cyClient >= iSizeY) {                  /* Dasselbe fÅr die rechte  */
      WinEnableWindow (hwndVscroll, FALSE);    /* Scrollbar                */
    } else {
      WinEnableWindow (hwndVscroll, TRUE);
      WinSendMsg (hwndVscroll, SBM_SETSCROLLBAR,
                  MPFROM2SHORT (0, 0),
                  MPFROM2SHORT (0, iSizeY-cyClient));
    }
  } else {                                 /* 2. Teil: Komprimieren.       */
    WinEnableWindow (hwndVscroll, FALSE);  /* Scrollbars ausschalten       */
    WinEnableWindow (hwndHscroll, FALSE);
  }
}

/***********************  Ende von SetScrollBars()  ************************/



/***********************  Start von MakeColTable()  ************************/
/*                                                                         */
/* Erstellt die Logische Farbtabelle.                                      */
/* Dieses Modul ist noch nicht getestet, da es erst unter der Version 1.2  */
/* von OS/2 lÑuft.                                                         */
/*                                                                         */
/***************************************************************************/

BOOL MakeColTable (PBITMAPINFO pbmi, PLONG *plTable)
{
  USHORT i;

  switch (pbmi->cBitCount) {     /* Grîsse feststellen: */

    case 8:
      *plTable = malloc (256 * sizeof (LONG));
      break;

    case 4:
      *plTable = malloc (16 * sizeof (LONG));
      break;

    default:
      return FALSE;
  } /* Ende switch */

  if (*plTable == NULL)
    Error (hab);

  for (i=0; i<usColors; i++) {   /* Tabelle auffÅllen */
    (*plTable)[i] = (pbmi->argbColor[i].bRed   << 16 +
                     pbmi->argbColor[i].bGreen <<  8 +
                     pbmi->argbColor[i].bBlue);
  } /* Ende for */
  return TRUE;
}
/***********************  Ende von MakeColTable()  *************************/



/*******************  Start der About-Box Prozedur  ************************/
/*                                                                         */
/* Behandelt die Messages an die "about"-Box.                              */
/*                                                                         */
/***************************************************************************/

MRESULT EXPENTRY AboutDlgProc(HWND hwndDlg, USHORT msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
  {
    case WM_INITDLG:
      CentreDlgBox (hwndDlg, hwndClient);
      break;

    default: break;
  }
  return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
}
/**********************  Ende der About-Box Prozedur ***********************/



/*********************  Start der Cancel-Box Prozedur  *********************/
/*                                                                         */
/* Behandelt die Messages an die "Cancel"-Box.                             */
/*                                                                         */
/* Diese Box erscheint beim Laden eines Files und gibt einem die Option,   */
/* den Ladevorgang abzubrechen.                                            */
/*                                                                         */
/***************************************************************************/

MRESULT EXPENTRY CancelDlgProc(HWND hwndDlg, USHORT msg,
                               MPARAM mp1, MPARAM mp2)
{
  switch(msg)
  {
    case WM_INITDLG:
      CentreDlgBox (hwndDlg, hwndClient);  /* Die Box wird zentriert       */
      break;

    case WM_COMMAND:
      switch(COMMANDMSG(&msg)->cmd)
      {
        case DID_CANCEL:          /* Abbruch wird gewÅnscht                */
          DosSemSet (&semHalt);   /* --> Lade-Thread informieren           */
          return 0;
      }
      break;

    case WM_FINISHED:             /* Lade-Thread ist fertig und hat eine   */
      WinDismissDlg(hwndDlg, TRUE); /* Message "WM_FINISHED" gesendet      */
      return 0;
  }
  return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
}
/*********************  Ende der Cancel-Box Prozedur  **********************/



/*********************  Start der Param-Box Prozedur  **********************/
/*                                                                         */
/* Behandelt die Messages an die "Param"-Box.                              */
/*                                                                         */
/* Die "Param"-Box zeigt die Fileparameter (Properties) an.                */
/*                                                                         */
/***************************************************************************/

MRESULT EXPENTRY ParamDlgProc(HWND hwndDlg, USHORT msg, MPARAM mp1, MPARAM mp2)
{
  switch(msg)
  {
    case WM_INITDLG:
      CentreDlgBox (hwndDlg, hwndClient); /* Box zentrieren                */
      switch (usType) {                   /* Alle Parameter einfÅllen...   */

        case BFT_BMAP:
          WinSetDlgItemText (hwndDlg, IDD_TYPE, "BITMAP");
          break;

        case IMGRY:
          WinSetDlgItemText (hwndDlg, IDD_TYPE, "IMGRY");
          break;

        case IMRGB:
          WinSetDlgItemText (hwndDlg, IDD_TYPE, "IMRGB");
          break;

        case IMMAP:
          WinSetDlgItemText (hwndDlg, IDD_TYPE, "IMMAP");
          break;

        case IMFLT:
          WinSetDlgItemText (hwndDlg, IDD_TYPE, "IMFLT");
          break;

        default:
          WinSetDlgItemText (hwndDlg, IDD_TYPE, "?");
      }


      switch (usStorage) {
        case IMRAW:
          WinSetDlgItemText (hwndDlg, IDD_STORAGE, "IMRAW");
          break;

        case IMRLE:
          WinSetDlgItemText (hwndDlg, IDD_STORAGE, "IMRLE");
          break;

        case IMLZW:
          WinSetDlgItemText (hwndDlg, IDD_STORAGE, "IMLZW");
          break;

        case BFT_BMAP:
          WinSetDlgItemText (hwndDlg, IDD_STORAGE, "BITMAP");
          break;

        default:
          WinSetDlgItemText (hwndDlg, IDD_STORAGE, "?");
      }

      WinSetDlgItemShort (hwndDlg, IDD_SIZEX,   (USHORT)iSizeX, FALSE);
      WinSetDlgItemShort (hwndDlg, IDD_SIZEY,   (USHORT)iSizeY, FALSE);
      WinSetDlgItemShort (hwndDlg, IDD_SAMPLES, usSamples,      FALSE);
      WinSetDlgItemShort (hwndDlg, IDD_DEPTH,   usDepth,        FALSE);
      WinSetDlgItemShort (hwndDlg, IDD_COLORS,  usColors,       FALSE);

      break;

    default: break;

  }
  return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
}
/********************  Ende der Param-Box Prozedur  ************************/


/**********************  Start von CentreDlgBox()  *************************/
/*                                                                         */
/*  Die Funktion CentreDialogBox zentriert die Dialog-Box in der           */
/*  Mitte des durch hwndParent angegebenen Fensters.                       */
/*                                                                         */
/***************************************************************************/

VOID CentreDlgBox(HWND hwndDlg, HWND hwndParent)
{
  SHORT ix, iy;                     /* Neue Position der Dialog-Box.       */
  SHORT iwidth, idepth;             /* Grîsse des Bildschirms              */
  SWP   swp;                        /* Window-Informationen                */
  RECTL rect;                       /* Grîsse des Fensters                 */

  /*************************************************************************/
  /* Grîsse des Bildschirms abfragem.                                      */
  /*************************************************************************/
  iwidth = (SHORT)WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN );
  idepth = (SHORT)WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN );

  /*************************************************************************/
  /* Grîsse des Fensters abfragen und normieren bezÅglich Bildschirm:      */
  /*************************************************************************/
  WinQueryWindowRect (hwndParent, &rect);
  WinMapWindowPoints (hwndParent, HWND_DESKTOP, (PPOINTL)&rect, 2);

  /*************************************************************************/
  /* Window-Informationen der Dialog-Box fragen.                           */
  /*************************************************************************/
  WinQueryWindowPos (hwndDlg, (PSWP)&swp);

  /*************************************************************************/
  /* Dialog-Box zentrieren.                                                */
  /*************************************************************************/
  ix = (SHORT)(((rect.xRight - rect.xLeft   - swp.cx) / 2) + rect.xLeft);
  iy = (SHORT)(((rect.yTop   - rect.yBottom - swp.cy) / 2) + rect.yBottom);

  /*************************************************************************/
  /* Sicherstellen, dass Die Dialog-Box ganz sichtbar ist:                 */
  /*************************************************************************/
  ix = max (ix, 0);
  ix = min (ix, (iwidth - swp.cx));

  iy = max (iy, 0);
  iy = min (iy, (idepth - swp.cy));

  /*************************************************************************/
  /* Neue Position setzen:                                                 */
  /*************************************************************************/
  WinSetWindowPos( hwndDlg, HWND_TOP, ix, iy, 0, 0, SWP_MOVE );
}
/**********************  Ende von CentreDlgBox()  **************************/



/*************************  Beginn von  Help()  ****************************/
/*                                                                         */
/*  Zeigt eine Hilfemeldung an.                                            */
/*                                                                         */
/*  Der Common-User-Access Teil der System Application Architecture (SAA)  */
/*  von IBM schreibt eigentlich ein indexiertes, kontext-sensitives Hilfe- */
/*  system vor. Dies muss bei dee Version 1.1 von OS/2 selbst programmiert */
/*  werden. Aus ZeitgrÅnden wurde darauf verzichtet.                       */
/*  Mit der Version 1.2 von OS/2 wird ein "Help-Manager" mitgeliefert.     */
/*  In der Dokumentation zur Semesterarbeit ist dieser erwas nÑher         */
/*  beschrieben.                                                           */
/***************************************************************************/
VOID Help(HWND hwnd)
{
  char title[20];
  char help[200];

  WinLoadString(hab,             /* Titel der Help-Box laden               */
                0L,
                IDS_HELPTITLE,
                20,
                title
               );
  WinLoadString(hab,             /* Inhalt der Help-Box laden              */
                0L,
                IDS_HELP,
                200,
                help
               );
  WinMessageBox(HWND_DESKTOP,    /* Help-Box anzeigen                      */
                hwnd,
                help,
                title,
                0L,
                MB_OK | MB_NOICON
               );
}
/**************************  Ende von Help()  ******************************/



/************************  Beginn von OpenFile()  **************************/
/*                                                                         */
/* Die Funktion OpenFile wird als Resultat der File-ôffnen Menuposition    */
/* aufgerufen (bzw. beim Starten der Applikation automatisch).             */
/*                                                                         */
/***************************************************************************/

BOOL OpenFile(HAB hab, HDC hdcMem, HWND hwnd, HBITMAP *hbm)
{
  static CHAR *szMode     = "r";
  static LONG plTempArray[1];
  USHORT      usTemp;
  USHORT      usIndex;
  IMBYTE      *pimMap;

  BITMAPINFOHEADER bmHeader;       /* Header des Bitmaps.                  */
  SIZEL            size;           /* Grîsse der Presentationspaces.       */
  HDC              hdc;


  if (fCmdLine) fCmdLine = FALSE;  /* Command-Line Argument?               */
                                   /* Nein: File-Dialogbox anzeigen:       */
  else if (!GetFileName (szFileSource, HWND_DESKTOP, hwnd, TRUE))
         return 0;

  fFill = TRUE;                    /* Hintergrund lîschen bei WM_PAINT     */

  hImage = bmopen (szFileSource, "rb");          /* 'binary' îffnen!       */

  if (hImage != NULL) {            /* ist ein OS/2 Bitmap-File             */
                                   /* Platz fÅr Header allozieren:         */
    pbmiBitmap = malloc (sizeof(BITMAPINFOHEADER) + 256*sizeof(RGB));
    bmrhdr (hImage, pbmiBitmap);   /* Header lesen                         */

    iSizeX = pbmiBitmap->cx;       /* Parameter auslesen...                */
    iSizeY = pbmiBitmap->cy;

    usType  = BFT_BMAP;
    usStorage = BFT_BMAP;
    usDepth = pbmiBitmap->cBitCount;
    usSamples = pbmiBitmap->cPlanes;
    usColors = (1 << usDepth);

    imbRowBuff = bmrowalloc ();    /* Platz fÅr Zeilenpuffer allozieren    */
    if (imbRowBuff == NULL) return 1;

  } else {                         /* evtl. 'im'-Bild                      */
    imBild = imopen (szFileSource, szMode);
    if (imBild == NULL) {          /* Fehler:                              */

      switch (imerrnum) {          /* File konnte nicht geîffnet werden    */
        case 4:
          ShowMsg (IDS_ERRTITLE, IDS_FILEERR, MB_ICONEXCLAMATION);
          return 0;

        case 5:                    /* File ist nicht 'im'                  */
          ShowMsg (IDS_ERRTITLE, IDS_TYPEERR, MB_ICONEXCLAMATION);
          return 0;
      }

      ImErrorMsg (imerrnum);       /* Anderer Fehler                       */
      return 0;
    }

    if (imrhdr (imBild)) {         /* Header einlesen                      */
      ImErrorMsg (imerrnum);
      return 0;
    }

    iSizeX = imgetxsize(imBild);   /* Bildparameter einlesen               */
    iSizeY = imgetysize(imBild);

    usType  = imgettype (imBild);
    if (usType == IMFLT) {         /* IMFLT nicht unterstÅtzt              */
      ShowMsg (IDS_ERRTITLE, IDS_FLTERR, MB_ICONEXCLAMATION);
      return 0;
    }

    usStorage = imgetstor (imBild);
    usDepth = imgetdepth (imBild);
    usSamples = imgetsamples (imBild);
    usColors = imgetcolors (imBild);

                               /* Platz fÅr Zeilenpuffer allozieren        */
    imbRowBuff = imrowalloc( iSizeX, usSamples, usDepth);
    if (imbRowBuff == NULL) return 1;
  } /* Ende if */

  WinSetWindowText (hwndFrame, szFileSource); /* Fenstertitel setzen       */
  SetScrollBars();                            /* Scrollbars initialisieren */

  hdc = WinOpenWindowDC (hwnd);               /* Device-Context erîffnen   */

  DevQueryCaps (hdc, CAPS_COLORS, 1L, plTempArray); /* Anzahl Farben des   */
  usDisplayColors = (USHORT)plTempArray[0];         /* Displays            */

                                              /* Realize unterstÅtzt?      */
  DevQueryCaps (hdc, CAPS_COLOR_TABLE_SUPPORT, 1L, plTempArray);
  fRealizePossible = (BOOL)(plTempArray[0] & CAPS_COLTABL_REALIZE);

  /* Realizing nur mîglich, wenn das Bild nicht mehr Farben als Display hat*/
  fRealizePossible = fRealizePossible && (usDisplayColors >= usColors);

  DevCloseDC (hdc);            /* Device-Context wieder schliessen         */

  size.cx = iSizeX;                           /* Grîsse der Presentation-  */
  size.cy = iSizeY;                           /* spaces.                   */

  if (*hbm != (HBITMAP) NULL) {            /* Existiert schon ein Bitmap?  */
    GpiSetBitmap (hpsMem, (HBITMAP)NULL);  /* ja: lîschen, Ressourcen      */
    GpiDeleteBitmap (*hbm);                /*     freigeben                */
    GpiAssociate (hpsMem, (HDC)NULL);
    GpiDestroyPS (hpsMem);
  }
  hpsMem = GpiCreatePS(hab,                   /* Presentation Space fÅr    */
                        hdcMem,                /*  Memory Dev. Context     */
                        (PSIZEL)&size,
                        (LONG)PU_PELS | GPIT_NORMAL | GPIA_ASSOC
                       );

  bmHeader.cbFix     = 12L;                   /* 12 Datenbytes             */
  bmHeader.cx        = iSizeX;
  bmHeader.cy        = iSizeY;
  bmHeader.cPlanes   = 1;                     /* Anzahl Planes (immer 1)   */


  switch (usType) {

    case BFT_BMAP:
      bmHeader.cBitCount = usDepth;
      break;

    case IMRGB:
       /* Platz fÅr Header allozieren:                                     */
      pbmiBitmap = malloc (sizeof (BITMAPINFOHEADER));
      bmHeader.cBitCount  = 24;               /* Bits/Punkt (24 Bit RGB).  */
      pbmiBitmap->cBitCount = 24;
      break;

    case IMGRY:
       /* Platz fÅr Header allozieren:                                     */
      pbmiBitmap =
          malloc (sizeof(BITMAPINFOHEADER)+(1<<usDepth)*sizeof(RGB));
      if (usDepth < 3) {                  /* 1 oder 2 Bit Tiefe            */
        bmHeader.cBitCount  = usDepth;
        pbmiBitmap->cBitCount = usDepth;
      } else if (usDepth < 5) {           /* 3 bis 4 Bit Tiefe --> 4 Bit   */
        bmHeader.cBitCount  = 4;
        pbmiBitmap->cBitCount = 4;
      } else {
        bmHeader.cBitCount  = 8;          /* 5 bis 8 Bit Tiefe --> 8 Bit   */
        pbmiBitmap->cBitCount = 8;
      } /* Ende if */

     /*********************************************************************/
     /* OS/2 kennt keine                                                  */
     /* Graustufenbilder. Deshalb muss man ein normales Farb-             */
     /* tebellenbild erstellen, wobei die einzelnen Farbwerte             */
     /* lauter Graustufen sind.                                           */
     /*********************************************************************/
      usTemp = (255 / ((1<<usDepth)-1));
      for (usIndex = 0; usIndex < (1<<usDepth); usIndex++) {
        pbmiBitmap->argbColor[usIndex].bBlue  = (BYTE)(usIndex * usTemp);
        pbmiBitmap->argbColor[usIndex].bGreen = (BYTE)(usIndex * usTemp);
        pbmiBitmap->argbColor[usIndex].bRed   = (BYTE)(usIndex * usTemp);
      } /* Ende for */

      break;

    case IMMAP:
      pimMap = (IMBYTE *)immapalloc (usColors); /* Platz fÅr Header        */
      imsetcmap (imBild, (IMPIXEL *)pimMap);    /* Pointer setzen          */
      if (imrcmap (imBild)) {                   /* Colormap lesen          */
        ImErrorMsg (imerrnum);
        return 0;
      }

      if (usColors > 257) {                          /* Zu viele Farben... */
        pbmiBitmap = malloc (sizeof(BITMAPINFOHEADER));
        bmHeader.cBitCount = 24;                     /* --> RGB Bild       */
        pbmiBitmap->cBitCount = 24;

      } else {                                       /* sonst Colormapbild */
        pbmiBitmap =
            malloc (sizeof(BITMAPINFOHEADER)+(usColors)*sizeof(RGB));

        if (usColors > 16) {         /* 32 bis 256 Farben: 8 Bits          */
          bmHeader.cBitCount = 8;    /*                                    */
          pbmiBitmap->cBitCount = 8; /*                                    */
        } else if (usColors > 4) {   /*  8 bis  16 Farben: 4 Bits          */
          bmHeader.cBitCount = 4;    /*                                    */
          pbmiBitmap->cBitCount = 4; /*                                    */
        } else if (usColors > 2) {   /*          4 Farben: 2 Bits          */
          bmHeader.cBitCount = 2;    /*                                    */
          pbmiBitmap->cBitCount = 2; /*                                    */
        } else {                     /*                                    */
          bmHeader.cBitCount = 1;    /*          2 Farben: 1 Bit           */
          pbmiBitmap->cBitCount = 1; /*                                    */
        }                            /*                                    */
      }

         /* Farbtabelle einlesen:                                          */
        for (usIndex = 0; usIndex < usColors; usIndex++) {
          pbmiBitmap->argbColor[usIndex].bBlue  = *(pimMap + 4*usIndex + 2);
          pbmiBitmap->argbColor[usIndex].bGreen = *(pimMap + 4*usIndex + 1);
          pbmiBitmap->argbColor[usIndex].bRed   = *(pimMap + 4*usIndex);
        } /* Ende for */

      break;

  } /* Ende switch */

  pbmiBitmap->cbFix     = 12L;
  pbmiBitmap->cx        = iSizeX;
  pbmiBitmap->cy        = iSizeY;
  pbmiBitmap->cPlanes   = 1;

  fRealizePossible = fRealizePossible &
                     MakeColTable (pbmiBitmap, &plColTable);

  *hbm = GpiCreateBitmap(hpsMem,              /* Erstellen des Bitmaps.    */
                         &bmHeader,
                         0L,
                         (PBYTE)NULL,
                         (PBITMAPINFO)NULL
                        );
  GpiSetBitmap(hpsMem, *hbm);                /* Bitmap wird angewÑhlt.    */

 /************************************************************************/
 /* Hier kann man leicht einen Fehler machen: UrsprÅnglich war die       */
 /* Reihenfolge: DosSemClear, WinLoadDlg, WinProcessDlg. D.h. der Lade-  */
 /* Thread wurde gestartet, bevor das Dialog-Window kreiert wurde. Dies  */
 /* funktionierte bei grossen Bildern bestens. Beim Laden eines sehr     */
 /* kleinen Bildes war der Lade-Thread schon fertig, bevor               */
 /* der WinLoadDlg fertig war, h.h. die Variable hwndDlg war ungÅltig.   */
 /* Der Lade-Thread sendet aber eine Message an hwndDlg --> Protection   */
 /* Violation. Mit der neuen Reihenfolge funktioniert es jetzt.          */
 /************************************************************************/
  hwndDlg = WinLoadDlg(HWND_DESKTOP,     /* Cancel-Box anzeigen          */
                       hwnd,
                       CancelDlgProc,
                       NULL,
                       IDD_CANCEL,
                       NULL);

  DosSemSet (&semPictureReady);          /* Bild nicht bereit            */
  DosSemClear (&semInputReady);          /* Lade-Thread starten          */

  WinProcessDlg (hwndDlg);               /* Eingaben an Cancel-Box bearb.*/

  WinDestroyWindow (hwndDlg);            /* Cancel-Box lîschen           */

  hpsScaled = ScaleBitmap();             /* Neuer Bitmap skalieren       */

  return 1;
}
/*************************  Ende von OpenFile()  ***************************/



/***********************  Beginn von WriteFile()  **************************/
/*                                                                         */
/* WriteFile wird als Resultat der File-Schreiben Menuposition             */
/* aufgerufen.                                                             */
/*                                                                         */
/***************************************************************************/

BOOL WriteFile(VOID)
{
  STARTDATA stdata;                 /* Informationen fÅr eine nene Session */
  USHORT    usSession;              /* Sessionnummer                       */
  USHORT    usProcess;              /* Prozessnummer                       */
  CHAR     *szArguments;            /* Argumente an neue Session           */

   /* File Dialogbox anzeigem:                                             */
  if (!GetFileName (szFileDest, HWND_DESKTOP, hwndFrame, FALSE))
    return 0;

  szArguments = malloc (250);         /* Platz allozieren fÅr Argumente    */
  strcpy (szArguments, szFileSource); /* Source-Filename = 1. Argument     */
  strcat (szArguments, " ");          /* Space dazwischen                  */
  strcat (szArguments, szFileDest);   /* Dest. Filename = 2. Argument      */

  stdata.Length = 24;                 /* Div. Daten setzen                 */
  stdata.Related = 0;
  stdata.FgBg = 1;
  stdata.TraceOpt = 0;
  stdata.PgmTitle = NULL;
  stdata.PgmInputs = szArguments;
  stdata.TermQ = NULL;
  stdata.Environment = NULL;
  stdata.InheritOpt = 0;
  stdata.SessionType = 0;

  switch (usType) {

    case BFT_BMAP:
      stdata.PgmName = "BMP2IM.EXE";
      break;

    case IMRGB:    /* Da es sich bei allen dreien um 'im'-Formate handelt, */
    case IMMAP:    /* kann immer die Routine 'im2bmp' verwendet werden     */
    case IMGRY:
      stdata.PgmName = "IM2BMP.EXE";
      break;

  } /* Ende switch */

    DosStartSession (&stdata, &usSession, &usProcess); /* Pgm. starten     */
    ShowMsg (IDS_IMTITLE, IDS_CONVERT, MB_NOICON);     /* Meldung anzeigen */
}
/**********************  Ende von WriteFile  *******************************/



/**********************  Beginn des Threads ReadFile  **********************/
/*                                                                         */
/* Der Thread ReadFile liest ein File asynchron zur restlichen Applikation */
/* ein. Er ist als endlosschlaufe implementiert und wird durch Semaphoren  */
/* gesteuert.                                                              */
/*                                                                         */
/***************************************************************************/

VOID cdecl ReadFile ()
{

PBYTE   pbBuffer, pbTemp;                 /* Pointers auf Zeilenpuffer     */
USHORT  x,y;                              /* Schlaufenvariablen            */
IMPIXEL impPixel;                         /* 'im'-Punkt                    */

  while (TRUE) {                          /* Beginn Endlosschlaufe         */
    DosSemWait (&semInputReady, -1L);     /* Warten, bis File parat        */
    DosSemSet (&semInputReady);           /* File noch nicht gelesen       */

    switch (usType) {                     /* je nach Typ verschieden lesen */
      case BFT_BMAP:                      /* OS/2-Bitmap                   */
        for (y=0; y<=iSizeY; y++) {
          /*****************************************************************/
          /* Vor jeder Zeile wird getestet, ob der User einen Abbruch des  */
          /* Lesevorganges wÅnscht. Dies wird mit der Semaphore 'semHalt'  */
          /* signalisiert.                                                 */
          /*****************************************************************/
          if (DosSemWait (&semHalt, 0L) == ERROR_SEM_TIMEOUT)
            goto Halt;
          bmrrow (hImage, imbRowBuff, y);  /* Eine Zeile einlesen           */
          /*****************************************************************/
          /* Die Zeile kann ohne énderungen in den Bitmap kopiert werden.  */
          /* Die passiert mit der Funktion GpiSetBitmapBits.               */
          /*****************************************************************/
          GpiSetBitmapBits (hpsMem, (LONG)y, 1L, imbRowBuff, pbmiBitmap);
        }
        break;

      case IMRGB:                         /* 'im'-Typ IMRGB                */
        /*******************************************************************/
        /* Hier braucht es etwas Arithmetik, um das Zeilenformat eines     */
        /* IMRGB-Bilder in das OS/2-Bitmap Zeilenformat umzuwandeln.       */
        /* Deshalb wird ein zweiter Zeilenpuffer alloziert.                */
        /*******************************************************************/
        pbBuffer = (PBYTE) malloc (iSizeX * 3);
        for (y=iSizeY; y>0; y--) {
          if (DosSemWait (&semHalt, 0L) == ERROR_SEM_TIMEOUT)
            goto Halt;
          if ((!imrrow (imBild, imbRowBuff)) && imerrnum) {
            ImErrorMsg (imerrnum);
            goto Halt;
          }
          for (x=0; x<iSizeX; x++) {      /* jetzt die Rechenarbeit...     */
            /***************************************************************/
            /* Beim im-Format ist der Zeilenpuffer Farbenweise abgelegt,   */
            /* d.h. RRRGGGBBB. OS/2 braucht die Zeile aber Pel-weise,      */
            /* d.h. RGBRGBRGB. dies wird mit den folgenden Zeilen gemacht: */
            /***************************************************************/
            pbTemp = pbBuffer+x+x+x;
            *(pbTemp+2) = *(imbRowBuff+x              );
            *(pbTemp+1) = *(imbRowBuff+x+iSizeX       );
            *(pbTemp  ) = *(imbRowBuff+x+iSizeX+iSizeX);
          } /* Ende for x */
          GpiSetBitmapBits (hpsMem, (LONG)y-1, 1L, pbBuffer, pbmiBitmap);
        } /* Ende for y */
        break;

      case IMGRY:               /* 'im'-Typ IMGRY und                      */
      case IMMAP:               /* 'im'-Typ IMMAP kînnen hier gleich       */
                                /* behandelt werden, da bei beiden mit     */
                                /* einem Colormap gearbeitet werden muss   */
        if (usColors < 257) {   /* ausser, wenn IMMAP mehr als 256 Farben  */
                                /* enthÑlt.                                */
          for (y=iSizeY; y>0; y--) {
            if (DosSemWait (&semHalt, 0L) == ERROR_SEM_TIMEOUT)
              goto Halt;
            imrrow (imBild, imbRowBuff); /* Das Zeilenformat ist gleich    */
            GpiSetBitmapBits (hpsMem, (LONG)y-1, 1L, imbRowBuff, pbmiBitmap);
          } /* Ende for */

        } else {                /* Bei mehr als 256 Farben muss in OS/2    */
                                /* mit einem RGB-Typ gearbeitet werden.    */
                                /* (siehe auch IM2BMP.C)                   */
          pbBuffer = (PBYTE) malloc (iSizeX * 3); /* 2. Zeilenpuffer       */
          for (y=iSizeY; y>0; y--) {
            if (DosSemWait (&semHalt, 0L) == ERROR_SEM_TIMEOUT)
              goto Halt;
            if ((!imrrow (imBild, imbRowBuff)) && imerrnum) {
              ImErrorMsg (imerrnum);
              goto Halt;
            }
            /***************************************************************/
            /* Jetzt wird Punkt fÅr Punkt mit der 'ideal-pixel' Funktion   */
            /* 'imrpix' in einen 24-Bit umgewandelt und im Zeilenpuffer    */
            /* in der richtigen Reigenfolge abgelegt:                      */
            /***************************************************************/
            for (x=0; x<iSizeX; x++) {
              impPixel = imrpix (imBild, imbRowBuff, x);
              pbBuffer[x+x+x+2] = (BYTE)((impPixel & 0x0000FF));
              pbBuffer[x+x+x+1] = (BYTE)((impPixel & 0x00FF00) >>  8);
              pbBuffer[x+x+x  ] = (BYTE)((impPixel & 0xFF0000) >> 16);
            }
            GpiSetBitmapBits (hpsMem, (LONG)y-1, 1L, pbBuffer, pbmiBitmap);
          } /* Ende for y */
        } /* Ende if */
        break;

    } /* Ende switch */

    DosSemClear (&semPictureReady);    /* Das Bild ist eingelesen           */

    Halt:                             /* Hierhin wird bei Abbruch des      */
                                      /* Ladevorganges gesprungen.         */

    DosSemClear (&semHalt);           /* zur Sicherheit lîschen            */

    free (imbRowBuff);
    if (usType == BFT_BMAP)
      bmclose (hImage);               /* Files schliessen                  */
    else
      imclose (imBild);

    WinPostMsg (hwndDlg, WM_FINISHED, 0L, 0L);  /* Melden, dass fertig     */
                                                /* geladen ist.            */
  } /* Ende der Endlosschlaufe */
}
/**********************  Ende des Threads ReadFile  ************************/



/**********************  Beginn von ScaleBitmap()  *************************/
/*                                                                         */
/* Die Funktion ScaleBitmap komprimiert (oder expandiert) den Bitmap       */
/* auf die Grîsse des Windows.                                             */
/*                                                                         */
/***************************************************************************/

HPS ScaleBitmap(VOID)
{
  static HDC              hdcComp;         /* Device-Context Handler       */
  static HPS              hpsComp;         /* Presentation-Space Handler   */
  static HBITMAP          hbmComp;         /* Bitmap Handler               */
  BITMAPINFOHEADER        bmiComp;         /* Header des Bitmaps           */
  SIZEL                   sizel;           /* Grîsse des Bitmaps           */
  POINTL                  pt[4];           /* Rechtecke fÅr GpiPlayBitmap  */



  if (DosSemWait (&semPictureReady, 0L) == ERROR_SEM_TIMEOUT)
    return NULL;
  DosSemSet (&semPictureReady);            /* Bild nicht mehr ready        */
  WinInvalidateRect (hwndFrame, NULL, FALSE);       /* Neu zeichnen        */
  RemoveRect();                            /* Gibt auch WM_PAINT           */

  if (hbmComp != (HBITMAP) NULL) {         /* Existiert der Bitmap schon?  */
    GpiSetBitmap (hpsComp, (HBITMAP)NULL); /* ja: alles lîschen            */
    GpiDeleteBitmap (hbmComp);
    GpiAssociate (hpsComp, (HDC)NULL);
    GpiDestroyPS (hpsComp);
    DevCloseDC (hdcComp);
  }

  if (fScroll) {                           /* Muss komprimiert werden?     */
    DosSemClear (&semPictureReady);        /* Nein: Bitmap unverÑndert     */
    return hpsMem;                         /* zurÅckgeben                  */
  }

  hdcComp = DevOpenDC(hab,                    /* Anchor Block Handler      */
                      OD_MEMORY,              /* Memory Device Context     */
                      (PSZ)"*",               /* keine Device-Information  */
                      0L,                     /* keine Datenblîcke         */
                      NULL,                   /* Datenblîcke               */
                      (HDC)NULL               /* Kompatibel zu Screen-DC   */
                     );

  sizel.cx = cxClient;                        /* Grîsse des PS setzen      */
  sizel.cy = cyClient;

  hpsComp = GpiCreatePS(hab, hdcComp, &sizel, GPIA_ASSOC | PU_PELS);

  bmiComp.cbFix     = 12;                     /* Bitmapinfos setzen        */
  bmiComp.cx        = (USHORT)cxClient;
  bmiComp.cy        = (USHORT)cyClient;
  bmiComp.cPlanes   = 1L;
  bmiComp.cBitCount = 24L;                    /* 24 Bit --> universell     */

  hbmComp = GpiCreateBitmap(hpsComp,          /* Pres. space Handler       */
                            &bmiComp,         /* Bitmapinfos               */
                            0L,               /* keine Options             */
                            (PBYTE)NULL,      /* keine Init. Daten         */
                            (PBITMAPINFO)NULL /* -> auch keine Init-Infos  */
                           );

  GpiSetBitmap(hpsComp, hbmComp);             /* Bitmap in PS wÑhlen       */

  pt[0].x = 0;                       /* Dest.-Bitmap, unten links          */
  pt[0].y = 0;                       /*                                    */
  pt[1].x = cxClient;                /* Dest.-Bitmap, Grîsse               */
  pt[1].y = cyClient;                /*                                    */
  pt[2].x = cxEdge;                  /* Source Bitmap, unten links         */
  pt[2].y = cyEdge;                  /*                                    */
  pt[3].x = iSizeX;                  /* Source-Bitmap, Grîsse              */
  pt[3].y = iSizeY;                  /*                                    */

  /*************************************************************************/
  /* Das Komprimieren kann eine Weile dauern, deshalb wird der             */
  /* 'Uhrglas'-Mauszeiger angezeigt:                                       */
  /*************************************************************************/
  WinSetPointer (HWND_DESKTOP, WinQuerySysPointer (HWND_DESKTOP,
                             SPTR_WAIT, FALSE));

  GpiBitBlt(hpsComp,               /* Dest. Handler                        */
            hpsMem,                /* Source-Handler                       */
            4L,                    /* Anzahl Daten                         */
            pt,                    /* Daten                                */
            ROP_SRCCOPY,           /* Dest. einfach Åbermalen              */
            BBO_IGNORE             /* beim Komprimieren die vorigen        */
                                   /* Zeilen einfach auslassen             */
           );

  /*************************************************************************/
  /* Wieder den normalen Mauszeiger anzeigen:                              */
  /*************************************************************************/
  WinSetPointer (HWND_DESKTOP, WinQuerySysPointer (HWND_DESKTOP,
                         SPTR_ARROW, FALSE));

  DosSemClear (&semPictureReady);    /* Bild ist wieder Ready              */

  return hpsComp;                    /* komprimierten PS zurÅckgeben       */
}
/************************  Ende von ScaleBitmap()  *************************/



/**********************  Beginn von MouseButtonDown  ***********************/
/*                                                                         */
/* Die Funktion MouseButtonDown initialisiert das 'Mouse-Tracking'         */
/*                                                                         */
/***************************************************************************/

VOID MouseButtonDown(HWND hwnd, MPARAM mp1)
{
  /*************************************************************************/
  /* Test, ob Åberhaupt ein Bild bereit ist und ob die Maus innerhalb      */
  /* des Bildes geklickt worden ist:                                       */
  /*************************************************************************/
  if ((((LOUSHORT(mp1) > iSizeX) || (HIUSHORT(mp1) > iSizeY)) && fScroll) ||
      (DosSemWait (&semPictureReady, 0L) == ERROR_SEM_TIMEOUT)) {
    WinAlarm (HWND_DESKTOP, WA_ERROR);             /* Beep                 */
    return;
  }

  /*************************************************************************/
  /* Der Focus wird direkt zum Client-Window gesetzt, da dort das Tracking */
  /* vorgenommen wird.                                                     */
  /*************************************************************************/
  WinSetFocus (HWND_DESKTOP, hwndClient);

  /*************************************************************************/
  /* Zuerst wird ein allfÑlliges Rechteck gelîscht. Dann werden die        */
  /* Parameter gesetzt und der Tracking-Modus gestartet.                   */
  /*************************************************************************/
  if (!WinIsRectEmpty(hab, &tiStruct.rclTrack))
    RemoveRect();

  tiStruct.cxBorder = 1;                       /* Rand ist nur 1 Pel breit */
  tiStruct.cyBorder = 1;
  tiStruct.cxGrid = 0;                         /* Mit der kleinsten Ein-   */
  tiStruct.cyGrid = 0;                         /* heit arbeiten            */

  WinSetRect(hab, &tiStruct.rclTrack,          /* Rechteck zuerst auf die  */
             LOUSHORT(mp1),                    /* Mausposition setzen      */
             HIUSHORT(mp1),
             LOUSHORT(mp1),
             HIUSHORT(mp1));
  if (fScroll)                                 /* Maximale Grîsse setzen   */
    WinSetRect(hab, &tiStruct.rclBoundary,     /* Scrollen --> man darf    */
               0,                              /* nicht in den schwarzen   */
               0,                              /* Bereich kommen           */
               iSizeX - cxEdge,                /* Die Ecke muss abgezogen  */
               iSizeY - cyEdge);               /* werden!                  */
  else
    WinQueryWindowRect (hwnd, &tiStruct.rclBoundary); /* Ganzes Window gut */

  tiStruct.ptlMinTrackSize.x = 1;
  tiStruct.ptlMinTrackSize.y = 1;
  tiStruct.ptlMaxTrackSize.x = ++tiStruct.rclBoundary.xRight;
  tiStruct.ptlMaxTrackSize.y = ++tiStruct.rclBoundary.yTop;
  tiStruct.fs  = TF_STANDARD | TF_ALLINBOUNDARY | TF_RIGHT | TF_BOTTOM;

  WinTrackRect (hwnd, NULL, &tiStruct);

  /*************************************************************************/
  /* Das Fenster wird nur genommen, wenn es mindestens 2 Pel gross ist:    */
  /*************************************************************************/
  if ((tiStruct.rclTrack.xRight - tiStruct.rclTrack.xLeft) > 2 &&
      (tiStruct.rclTrack.yTop - tiStruct.rclTrack.yBottom) > 2)
    DrawRect();
  else
    WinSetRect(hab, &tiStruct.rclTrack, 0, 0, 0, 0);
}
/**********************  Ende von MouseButtonDown()  ***********************/



/************************  Beginn von GetBitmap()  *************************/
/*                                                                         */
/* Die Funktion GetBitmap gibt den Bitmaphandler des ausgewÑhlten          */
/* Rechtecks zurÅck.                                                       */
/*                                                                         */
/***************************************************************************/
HBITMAP GetBitmap(VOID)
{
  HDC              hdc;
  HPS              hps;
  HBITMAP          hbm;
  SIZEL            sizlTemp;
  BITMAPINFOHEADER bmi;
  POINTL           aptlCorners[4];

  if (WinIsRectEmpty (hab, &tiStruct.rclTrack)) {  /* Ist Rechteck defin.? */
    return((HBITMAP)NULL);
  } /* Ende if */

  /*************************************************************************/
  /* Grîsse bestimmen:                                                     */
  /*************************************************************************/
  sizlTemp.cx = tiStruct.rclTrack.xRight - tiStruct.rclTrack.xLeft - 1;
  sizlTemp.cy = tiStruct.rclTrack.yTop - tiStruct.rclTrack.yBottom - 1;

   /* Zuerst wird ein neuer Bitmap kreiert, in den dann das Rechteck       */
   /* kopiert wird.                                                        */
  hdc = DevOpenDC(hab,                        /* Anchor Block Handler      */
                  OD_MEMORY,                  /* Memory Device Context     */
                  (PSZ)"*",                   /* keine Device-Information  */
                  0L,                         /* keine Datenblîcke         */
                  NULL,                       /* Datenblîcke               */
                  (HDC)NULL                   /* Kompatibel zu Screen-DC   */
                 );

  bmi.cbFix     = 12;
  bmi.cx        = (USHORT)sizlTemp.cx;
  bmi.cy        = (USHORT)sizlTemp.cy;
  bmi.cPlanes   = 1L;
  bmi.cBitCount = 24L;

  hps = GpiCreatePS(hab, hdc, (PSIZEL)&sizlTemp, GPIA_ASSOC | PU_PELS);

  hbm = GpiCreateBitmap(hps,
                        (PBITMAPINFOHEADER)&bmi,
                        0L,
                        (PBYTE)NULL,
                        (PBITMAPINFO)NULL
                        );

  GpiSetBitmap(hps, hbm);

  aptlCorners[0].x = 0;          /* Rechteck setzen                        */
  aptlCorners[0].y = 0;
  aptlCorners[1].x = sizlTemp.cx;
  aptlCorners[1].y = sizlTemp.cy;
  aptlCorners[2].x = tiStruct.rclTrack.xLeft + cxEdge;
  aptlCorners[2].y = tiStruct.rclTrack.yBottom + cyEdge;

   /* Rechteck in neuen Bitmap kopieren                                    */
  GpiBitBlt(hps, hpsScaled, 3L, aptlCorners, ROP_SRCCOPY, BBO_IGNORE);

  GpiSetBitmap(hps, (HBITMAP)NULL); /* Alles lîschen ausser Bitmap selbst  */
  GpiAssociate(hps, (HDC)NULL);
  GpiDestroyPS(hps);
  DevCloseDC(hdc);

  return(hbm);                      /* Bitmap zurÅckgeben                  */
}
/*************************  Ende von GetBitmap  ****************************/



/*************************  Beginn von DrawRect  ***************************/
/*                                                                         */
/* Die Funktion DrawRect zeichnet ein Rechteck auf den Bildschirm,         */
/* sie benÅtzt dabei die letzte Position des Tracking-Rectangle.           */
/*                                                                         */
/***************************************************************************/
VOID DrawRect(VOID)
{
  POINTL ptlTemp;

  GpiSetLineType (hps, LINETYPE_DOT);
  GpiSetColor (hps, CLR_RED);

  ptlTemp.x = tiStruct.rclTrack.xLeft;
  ptlTemp.y = tiStruct.rclTrack.yBottom;
  GpiSetCurrentPosition (hps, &ptlTemp);

  ptlTemp.x = tiStruct.rclTrack.xRight;
  ptlTemp.y = tiStruct.rclTrack.yTop;
  GpiBox (hps, DRO_OUTLINE, &ptlTemp, 0L, 0L);

  WinSendDlgItemMsg (                        /* Message an Menus           */
       hwndFrame, FID_MENU, MM_SETITEMATTR,  /* Set Attribute              */
       MPFROM2SHORT (IDM_COPY, TRUE),        /* Menu "IDM_COPY"            */
       MPFROM2SHORT (MIA_DISABLED, 0));      /* Menu enabeln               */
}
/************************  Ende von DrawRect()  ****************************/



/***********************  Beginn von RemoveRect  ***************************/
/*                                                                         */
/* Die Funktion RemoveRect entfernt das Rechteck wieder vom Schirm.        */
/*                                                                         */
/***************************************************************************/
VOID RemoveRect (VOID)
{
  WinSetRect (hab, &tiStruct.rclTrack, 0, 0, 0, 0);  /* Rechteck lîschen   */

  WinSendDlgItemMsg (                              /* Message an Menu      */
       hwndFrame, FID_MENU, MM_SETITEMATTR,        /* Set Attribute        */
       MPFROM2SHORT (IDM_COPY, TRUE),              /* Menu "IDM_COPY"      */
       MPFROM2SHORT (MIA_DISABLED, MIA_DISABLED)); /* Menu disabeln        */

  if (fScroll)
    fFill = TRUE;                                    /* Hintergrund lîsch. */
  WinInvalidateRect (hwndClient, NULL, FALSE);       /* Neu zeichnen       */
}
/************************  Ende von RemoveRect  ****************************/



/************************  Beginn von ShowMsg  *****************************/
/*                                                                         */
/* Die Funktion ShowMsg zeigt eine Meldung an.                             */
/*                                                                         */
/***************************************************************************/

VOID ShowMsg (INT iTitle, INT iMessage, USHORT usIcon)
{
  char cTitle[32];
  char cMessage[255];

   /* Zuerst werden der Titel der Message und der Inhalt aus dem           */
   /* Ressourcenfile geladen:                                              */
  WinLoadString (hab, NULL, iTitle, sizeof cTitle, cTitle);
  WinLoadString (hab, NULL, iMessage, sizeof cMessage, cMessage);

  WinMessageBox (HWND_DESKTOP,   /* Danach wird die Message angezeigt      */
                 hwndFrame,
                 cMessage,
                 cTitle,
                 -1L,
                 MB_OK | usIcon);
}

/*************************  Ende von ShowMsg()  ****************************/



/************************  Beginn von ImErrorMsg  **************************/
/*                                                                         */
/* ImErrorMsg zeigt eine Fehlermeldung der im-Library an.                  */
/*                                                                         */
/***************************************************************************/

VOID ImErrorMsg (int iMessageNum)
{
  char cTitle[32];

   /* Zuerst wird der Titel der Message-Box geladen:                       */
  WinLoadString (hab, NULL, IDS_IMTITLE, sizeof cTitle, cTitle);

  WinMessageBox (HWND_DESKTOP,   /* Dann wird die Message angezeigt        */
                 hwndFrame,
                 imerrors[iMessageNum],
                 cTitle,
                 -1L,
                 MB_OK | MB_ICONEXCLAMATION);
}

/*************************  Ende von ImErrorMsg  ***************************/

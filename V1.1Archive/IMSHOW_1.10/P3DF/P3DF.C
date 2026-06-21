/**************  Semesterarbeit "P3DF.C" Source Code File  *****************/
/*                                                                         */
/* PROGRAMM-NAME: P3DF (Perspektivische Darstellung dreidimensionaler      */
/* -------------        Funktionen)                                        */
/*                                                                         */
/*                                                                         */
/* PROGRAMMIERT VON:                                                       */
/* -----------------                                                       */
/*  Martin Erzberger, 1989                                                 */
/*  Algorithmus aus: M+K Computer, Heft 85-2, S.53                         */
/*                     "Perspektivische Darstellung dreidimensionaler      */
/*                      Funktionen mit HRG", von Gerhard Piran             */
/*                   M+K Computer Verlag AG, Luzern 1985                   */
/*                                                                         */
/*                                                                         */
/* VERSION: 1.00, 8/89                                                     */
/* -------------------                                                     */
/*                                                                         */
/*                                                                         */
/* PROGRAMMGESCHICHTE:                                                     */
/* -------------------                                                     */
/*  - 1.00,  8/89: Erste Ausgabe                                           */
/*                                                                         */
/*                                                                         */
/* ZWECK DES PROGRAMMS:                                                    */
/* --------------------                                                    */
/*  Dieses Programm ist ein Beispiel einer einfacheren Presentation        */
/*  Manager Applikation. Es kann als Vorlage fÅr eigene Entwicklungen      */
/*  dienen. Insbesondere werden gezeigt:                                   */
/*    -Verwendung einer asynchronen Routine fÅr die Berechnung.            */
/*    -Multi-Thread C-Library.                                             */
/*    -Pull-Down Menus.                                                    */
/*    -Dialog-Box.                                                         */
/*    -Einfacher Bit-Map Output.                                           */
/*                                                                         */
/*                                                                         */
/* WIE WIRD DAS PROGRAMM COMPILIERT:                                       */
/* ---------------------------------                                       */
/*  "make all"                                                             */
/*                                                                         */
/*                                                                         */
/*  BENôTIGTE SOURCEFILES:                                                 */
/*  ----------------------                                                 */
/*    MAKEFILE       - FÅr make-Utility                                    */
/*    P3DF.C         - Source-Code                                         */
/*    P3DF.DEF       - Modulbeschreibung fÅr den Linker                    */
/*    P3DF.H         - Header File der Applikation                         */
/*    P3DF.ICO       - Icon                                                */
/*    P3DF.L         - Linker "Automatic Response"-File                    */
/*    P3DF.RC        - Resourcen (Menus, Texte)                            */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/*  BENôTIGTE PROGRAMME:                                                   */
/*  --------------------                                                   */
/*                                                                         */
/*    IBM Operating System/2, Version 1.1 (Standard oder Extended Edition) */
/*    IBM C Compiler, Version 1.1                                          */
/*    IBM Operating System/2 1.1 Toolkit                                   */
/*                                                                         */
/***************************************************************************/

#define INCL_BASE                       /*                                 */
#define INCL_WIN                        /* the PM header file              */
#define INCL_GPI                        /*                                 */

#include <os2.h>                        /* OS/2 Header-File                */
#include <mt\string.h>                  /* C/2 string functions            */
#include <mt\stdlib.h>                  /*                                 */
#include <mt\math.h>                    /* Mathematische Funktionen        */
#include <mt\process.h>                 /* fÅr _createthread               */
#include "p3df.h"                       /* Allgemeine Bezeichner           */

#define STRINGLENGTH 20                 /* StringlÑnge                     */
#define RAD 57.295779513                /* Umrechnungsfaktor               */
#define STACKSIZE 4096                  /* Stackgrîsse des asynchronen     */
                                        /*  Threads                        */
#define XPIXMAX       1024              /* Grîsstmîgliche Ausdehnung des   */
#define YPIXMAX       768               /* Bildes.                         */

/***************************************************************************/
/* Funktionen (Prototypen), diese werden im Programmtext kommentiert.      */
/***************************************************************************/
VOID cdecl       main(VOID);
BOOL             CreateThread(VOID);
VOID             Error(HAB);
MRESULT EXPENTRY _loadds WinProc (HWND, USHORT, MPARAM, MPARAM);
VOID             Init(HWND);
VOID             Paint(HWND);
MRESULT EXPENTRY _loadds AboutDlgProc(HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY _loadds DlgProc (HWND, USHORT, MPARAM, MPARAM);
VOID             CentreDlgBox (HWND);
VOID             ReadTempDlg (HWND);
BOOL             TestTempDlg (HWND);
VOID             CopyDlg (HWND);
VOID             InputError (HWND, int);
VOID             Help (HWND);
void far cdecl   Calculate ();
VOID             Formula (VOID);
VOID             Point (int, int);

/***************************************************************************/
/* Variabeln zur Steuerung des Presentation-Managers.                      */
/***************************************************************************/
HAB    hab;                             /* PM anchor block Handler         */
HAB    habCalc;                         /* anchor block-Handler des        */
                                        /*   asynchronen Threads           */
HMQ    hmq;                             /* Message queue Handler           */
HWND   hwndClient;                      /* Client area window Handler      */
HWND   hwndFrame;                       /* Frame window Handler            */
HPS    hps;                             /* Presentation Space Handler      */
HPS    hpsMem;                          /* 2. Presntation Space Handler    */
HBITMAP hbm;                            /* Bitmap Handler                  */
ULONG  flCreate = FCF_STANDARD;         /* Window Kontroll-Flags           */
CHAR   titleBar[80] = "P3DF";           /* Title-Bar Text                  */
CHAR   tempString[STRINGLENGTH];        /* TemporÑrer String               */
TID    tidCalc;                         /* Thread-ID                       */

/***************************************************************************/
/* Sonstige Steuervariabeln.                                               */
/***************************************************************************/
BOOL   fErrMem = FALSE;                 /* Flag fÅr Memory-Fehler          */
BOOL   busy = FALSE;                    /* Flag fÅr asynchr. Thread        */
ULONG  inputReady;                      /* Semaphore                       */
ULONG  critical;                        /* Semaphore fÅr gegens. Ausschl.  */
ULONG  repainted;                       /* Semaphore                       */
ULONG  restarted;                       /* Semaphore                       */
ULONG  attention;                       /* Semaphore                       */
BYTE   message;                         /* Meldung an asynch. Thread       */
SHORT  cxClient, cyClient;              /* Grîsse des Windows              */

/***************************************************************************/
/* Variabeln der Applikation.                                              */
/***************************************************************************/
struct {
        char alpha[STRINGLENGTH];
        char beta[STRINGLENGTH];
        char gamma[STRINGLENGTH];
        char min[STRINGLENGTH];
        char max[STRINGLENGTH];
        char vertikal[STRINGLENGTH];
        char ebenen[STRINGLENGTH];
        char dichte[STRINGLENGTH];
        BOOL zwert;
        BOOL grad;
        int  formel;
       } dialog;                      /* Inhalt der Dialog-Box             */
struct {
        char alpha[STRINGLENGTH];
        char beta[STRINGLENGTH];
        char gamma[STRINGLENGTH];
        char min[STRINGLENGTH];
        char max[STRINGLENGTH];
        char vertikal[STRINGLENGTH];
        char ebenen[STRINGLENGTH];
        char dichte[STRINGLENGTH];
       } tDialog;                     /* TemporÑre Dialog-Box              */
typedef xarr[XPIXMAX];
xarr   sMin, sMax;                    /* Sichtbarkeitsgrenzen              */
int    xPixMax;                       /* Hîchster X-Achsen-Wert            */
int    yPixMax;                       /* Hîchster Y-Achsen-Wert            */
double alpha;                         /* Vertikalwinkel                    */
double beta;                          /* Richtungswinkel                   */
double gamma;                         /* Vertikalîffnungswinkel            */
double delta;                         /* Horizontalîffnungswinkel          */
double sinBeta, cosBeta;              /* Hilfsvariablen                    */
double zEye;                          /* Augenhîhe                         */
double zMin, zMax;                    /* Z-Grenzwerte                      */
double r1, r2;                        /* Hilfsvariablen                    */
double d, deltaD, dEye;               /* Distanzwerte                      */
double xsm, ysm, xsl;                 /* Schnittpunktkoordinaten           */
double ysl, xsr, ysr;                 /* Schnittpunktkoordinaten           */
double deltaX, deltaY, deltaZ;        /* Schrittweiten                     */
double xw, yw, zw, z2;                /* Raumkoordinaten                   */
int    zPix;                          /* Z-Wert in Pixels                  */
double dMin, dMax;                    /* Distanzgrenzen                    */
int    deltaP;                        /* Anzahl Bildschnitte               */
int    xPixAnz;                       /* X-Schrittweite                    */
int    formNr;                        /* Formel-Nummer                     */
int    tempFormNr;                    /* Hilfswert fÅr Formelnummer        */
int    i1, i2;                        /* Hilfsvariablen                    */
int    cutNr;                         /* Ebenenanzahl                      */
BOOL   secondZ;                       /* zweiter Z-Wert gewÅnscht?         */
BOOL   see;                           /* Punkt sichtbar?                   */
BOOL   zExist, z2Exist;               /* Z-Wert existiert?                 */
BOOL   degree;                        /* Eingabe in Grad?                  */


/**********************  Start der "main"-Prozedur  ************************/
/*                                                                         */
/* Die main() Prozedur hat 3 Aufgaben:                                     */
/*  -Initialisieren des ganzen Systems (Fenster erstellen,                 */
/*                                      Threads starten, etc.).            */
/*  -Weiterleiten der Systemmeldungen zum richtigen Fenster.               */
/*  -Abschliessen und beenden der Applikation.                             */
/***************************************************************************/

VOID cdecl main()                       /* Die main()-Prozedur muss so     */
                                        /* deklariert werden               */
{
  QMSG qmsg;                            /* Message aus der Message-queue   */

/***********  Initialisierungsteil der main()-Routine  *********************/
  hab = WinInitialize(NULL);            /* PM initialisieren, NULL ist die */
  if (hab == NULL) Error(hab);          /*  einzige Option bei PM.         */
  hmq = WinCreateMsgQueue(hab, 0);      /* Message-Queue erstellen, Queue  */
  if (hmq == NULL) Error(hab);          /*  Grîsse = Default               */

  if (!WinRegisterClass(                /* Window-Klasse registrieren      */
        hab,                             /* Anchor-block Handler           */
        "P3DFWindow",                    /* Klassen-Name                   */
        WinProc,                         /* Adresse der Window-Prozedur    */
        CS_SIZEREDRAW,                   /* Stil der Klasse                */
        0                                /* Keine speziellen Window-Words  */
                       ))
    Error(hab);

   DosSemSet(&inputReady);              /* Semaphoren initialisieren       */
   DosSemSet(&critical);

   hwndFrame = WinCreateStdWindow(      /* Erstellen des Windows           */
               HWND_DESKTOP,             /* Parent ist das Desktop-Window  */
               WS_VISIBLE,               /* Das Fenster ist sichtbar       */
               (PULONG)&flCreate,        /* Frame control flag             */
               "P3DFWindow",             /* Klassen-Name Client-Window     */
               "",                       /* Fenster-Titel                  */
               0L,                       /* Kein spezieller Stil           */
               NULL,                     /* Alle Ressourcen im EXE-File    */
               ID_RESOURCE,              /* Ressourcen (Symbolisch)        */
               (PHWND)&hwndClient        /* "Client window" Handler        */
               );
   if (hwndFrame == (HWND)NULL)
     Error(hab);

   if (!CreateThread())                      /* Starten des Threads fÅr    */
    Error(hab);                              /* die Berechnung             */

/***************************************************************************/
/* Hauptaufgabe der main-Prozedur: Meldungen des PM empfangen und          */
/* weiterleiten an die Window-Prozedur. Dies, bis eine WM_QUIT-Message     */
/* empfangen wird. Dann wird die Applikation beendet.                      */
/***************************************************************************/
  while (WinGetMsg(hab, (PQMSG)&qmsg, (HWND)NULL, 0, 0))
    WinDispatchMsg(hab, (PQMSG)&qmsg);

/***************  "AufrÑum-Teil"  der main()-Routine  **********************/
  WinDestroyWindow(hwndFrame);          /* Fenster zerstîren.              */
  WinDestroyMsgQueue(hmq);              /* Message-Queue zerstîren.        */
  WinTerminate(hab);                    /* Applikation beenden.            */
}
/***********************  Ende der main-Prozedur  **************************/


/******************  Beginn der CreateThread Funktion  *********************/
/*                                                                         */
/* Die Funktion CreateThread() startet den Thread fÅr die Berechnung.      */
/* Dazu wird zuerst Speicher alloziert (fÅr den Stack), anschliessend      */
/* wird die C-Funktion _beginthread aufgerufen, um den Thread zu starten.  */
/*                                                                         */
/***************************************************************************/

BOOL CreateThread()
{
  SEL   segment;         /* Selector zum allozierten Speichersegment       */
  PBYTE stkBot;          /* Pointer zum Stack                              */



  DosAllocSeg(STACKSIZE, (PSEL)&segment, SEG_NONSHARED);
  SELECTOROF(stkBot) = segment;       /* SELECTOROF kopiert den Selector-  */
                                      /* Teil in einen Pointer.            */
                                      /* Ein Selector entspricht einem     */
                                      /* Segment des 80286 Real-Mode.      */
  OFFSETOF(stkBot) = 0;               /* OFFSETOF kopiert den Offset-Teil. */
  tidCalc = _beginthread(Calculate,   /* Adresse der Function, die asyn-   */
                                      /* chron gestartet werden soll.      */
                         stkBot,      /* Pointer zum Stack.                */
                         STACKSIZE-2, /* Grîsse des Stacks.                */
                         NULL);       /* Argumente (hier keine).           */
  return TRUE;
}

/*******************  Ende der CreateThread Funktion  **********************/


/********************  Start der ReportError Prozedur  *********************/
/*                                                                         */
/* Die Funktion ReportError ist eine reine Debug-Hilfe. Sie zeigt den      */
/* letzten aufgetretenen Fehler an.                                        */
/* Wenn der Fehler wegen zu wenig Memory auftritt, kann die Funktion       */
/* WinGetErrInfo nicht verwendet werden. Deshalb muss noch mit einem       */
/* Flag (fErrMem) gearbeitet werden.                                       */
/* Leider ist es keine ganz befriedigende Lîsung: Wenn sie z.B wÑhrend     */
/* einer WM_PAINT-Message aufgerufen wird, fÅhrt dies zu einem Deadlock.   */
/* Es ist mir allerdings nichts besseres eingefallen. Wenn das Programm    */
/* einmal funktioniert, kann die Routine ohnehin entfernt werden. Sie ist  */
/* hier nur noch als Beispiel vorhanden.                                   */
/*                                                                         */
/***************************************************************************/

VOID       Error(HAB hab)

{
PERRINFO  perriBlk;    /* Variable fÅr die Fehlerinformationen */
PSZ       pszErrMsg;   /* String fÅr die Fehlermeldung.        */
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
/********************  Ende der ReportError Funktion  **********************/


/*********************  Start der Window-Prozedur  *************************/
/*                                                                         */
/* Die Window-Prozedur bearbeitet die Meldungen, die von OS/2 gesendet     */
/* werden. Dabei werden -mittels einer Reihe von CASE-Statements- nur      */
/* diejenigen Meldungen bearbeitet, an denen die Applikation interessiert  */
/* ist. Alle anderen werden zur Behandlung an OS/2 zurÅckgeschickt.        */
/*                                                                         */
/***************************************************************************/
MRESULT EXPENTRY _loadds WinProc( HWND hwnd, USHORT msg,
                                  MPARAM mp1, MPARAM mp2 )
{
  switch(msg)                      /* Die Variable msg enthÑlt die Meldung */
  {
    case WM_CREATE:                /* Fenster wurde soeben erstellt.       */
      Init(hwnd);                  /* --> Function Init()                  */
      return 0;
    case WM_COMMAND:               /* Eine Menuauswahl wurde getroffen.    */
      switch (SHORT1FROMMP(mp1))    /* Welche Menuauswahl wurde getroffen? */
      {
                                    /* Alle Auswahlen ausser die folgenden */
                                    /* sind "disabled" und mÅssen nicht    */
                                    /* geprÅft werden.                     */
        case IDM_QUIT:              /* Bei Quit wird die Applikat. beendet.*/
          WinPostMsg(hwnd, WM_CLOSE, 0L, 0L);
          return 0;
        case IDM_ABOUT:
          WinDlgBox(HWND_DESKTOP,
                    hwnd,
                    AboutDlgProc,
                    NULL,
                    IDD_ABOUT,
                    NULL);
          return 0;
        case IDM_PARAM:                   /* Dialog-Box aufrufen           */
          WinDlgBox(HWND_DESKTOP,         /* Desktop-Window ist Parent     */
                    hwndFrame,            /* Frame-Window ist Besitzer     */
                    DlgProc,              /* Adresse der Dialog-Prozedur   */
                    NULL,                 /* Ressourcen im EXE-File        */
                    ID_DIALOG,            /* ID des Dialogs                */
                    NULL);                /* Keine Parameter               */
          return 0;                       /* Return-Wert nicht geprÅft.    */
      }
      break;
    case WM_HELP:
      Help(hwnd);
      return 0;
    case WM_ERASEBACKGROUND:       /* Der Hintergrund muss gelîscht werden.*/
      return (MRESULT)(FALSE);     /* --> Wird von Applikation erledigt.   */
    case WM_PAINT:                 /* Das Fenster muss neu gezeichnet      */
      Paint(hwnd);                 /*  werden.                             */
      return 0;
    case WM_SIZE:
      cxClient = SHORT1FROMMP(mp2);
      cyClient = SHORT2FROMMP(mp2);
      return 0;
    default:
      return WinDefWindowProc( hwnd, msg, mp1, mp2 ); /* Die anderen Mel-  */
  }                                /* dungen werden an OS/2 geschickt.     */
}
/**********************  Ende der Window Prozedur  *************************/


/*******************  Start der Funktion Init  *****************************/
/*                                                                         */
/*  Initialisierungen beim Erîffnen des Fensters.                          */
/*                                                                         */
/*  Alle Initialisierungen werden hier erledigt und nicht in der main()-   */
/*  Prozedur. Dies deshalb, weil erst hier das Fenster erstellt ist und so */
/*  z.B dessen Grîsse abgefragt werden kann.                               */
/*                                                                         */
/***************************************************************************/

VOID Init(HWND hwnd)    /* Der Window-Handler wird als Parameter Åbergeben */
                        /* die restlichen Elemente der WM_CREATE-Message   */
                        /* sind hier nicht von Bedeutung.                  */
{
  HDC              hdc;            /* Device Context Handler fÅr Window-DC.*/
  HDC              hdcMem;         /* Device Context Handler fÅr Memory-DC.*/
  BITMAPINFOHEADER bmHeader;       /* Header des Bitmaps.                  */
  SIZEL            size;           /* Grîsse der Presentationspaces.       */
  RECTL            rcl;
  PSZ              dcdatablk[9];   /* Datenblock fÅr Memory-Device.        */

  xPixMax = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CXFULLSCREEN );
  if (xPixMax > XPIXMAX) xPixMax = XPIXMAX;
  yPixMax = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYFULLSCREEN );
  if (yPixMax > YPIXMAX) yPixMax = YPIXMAX; /* Abfrage der Window-Grîsse,  */
                                            /* falls das Fenster Full-     */
                                            /* Screen ist. Die maximalen   */
                                            /* x- und y-Werte werden dann  */
                                            /* entsprechend gesetzt.       */
                                            /* Grîsser als XPIXMAX bzw.    */
                                            /* YPIXMAX kann das Bild       */
                                            /* allerdings nicht werden.    */
  dcdatablk[0] = (PSZ)0;          /* Keine Adresse          */
  dcdatablk[1] = (PSZ)"DISPLAY";  /* Display Device Driver  */
  dcdatablk[2] = (PSZ)0;          /* Restliche Daten werden */
  dcdatablk[3] = (PSZ)0;          /*  nicht benîtigt.       */
  dcdatablk[4] = (PSZ)0;
  dcdatablk[5] = (PSZ)0;
  dcdatablk[6] = (PSZ)0;
  dcdatablk[7] = (PSZ)0;
  dcdatablk[8] = (PSZ)0;
  hdcMem = DevOpenDC(hab,                     /* Anchor Block Handler      */
                     OD_MEMORY,               /* Memory Device Context     */
                     (PSZ)"*",                /* keine Device-Information  */
                     8L,                      /* 8 Datenblîcke             */
                     (PDEVOPENDATA)dcdatablk, /* Datenblîcke               */
                     (HDC)NULL                /* Kompatibel zu Screen-DC   */
                    );
  if (hdcMem == (HDC)NULL)
    Error(hab);
  hdc = WinOpenWindowDC(hwnd);                /* Window-Device-Context     */
  if (hdc == (HDC)NULL)
    Error(hab);
  size.cx = xPixMax;                          /* Grîsse der Presentation-  */
  size.cy = yPixMax;                          /* spaces.                   */
  hpsMem = GpiCreatePS(hab,                   /* Presentation Space fÅr    */
                       hdcMem,                /*  Memory Dev. Context      */
                       (PSIZEL)&size,
                       (LONG)PU_PELS | GPIT_NORMAL | GPIA_ASSOC
                      );
  if (hpsMem == (HPS)NULL)
    Error(hab);
  hps = GpiCreatePS(habCalc,                  /* Window-Presentationspace  */
                    hdc,
                    (PSIZEL)&size,
                    (LONG)PU_PELS | GPIT_MICRO | GPIA_ASSOC
                   );
  if (hps == (HPS)NULL)
    Error(hab);
  bmHeader.cbFix = 12L;                       /* 12 Datenbytes             */
  bmHeader.cx    = (USHORT) xPixMax;
  bmHeader.cy    = (USHORT) yPixMax;
  bmHeader.cPlanes = 1L;                      /* Anzahl Planes (immer 1)   */
  bmHeader.cBitCount = 1L;                    /* Bits/Punkt (Monochrom).   */
  hbm = GpiCreateBitmap(hpsMem,               /* Erstellen des Bitmaps.    */
                        &bmHeader,
                        0L,
                        (PBYTE)NULL,
                        (PBITMAPINFO)NULL
                       );
  if (hbm == (HBITMAP)NULL)
    Error(hab);
  if (GpiSetBitmap(hpsMem, hbm) == HBM_ERROR) /* Bitmap wird angewÑhlt.    */
    Error(hab);
  rcl.xLeft = 0;
  rcl.yBottom = 0;
  rcl.xRight = xPixMax;
  rcl.yTop = yPixMax;
  WinFillRect(hpsMem, &rcl, CLR_FALSE);
  GpiSetColor(hpsMem, CLR_TRUE);
  GpiSetBackColor(hpsMem, CLR_FALSE);
  GpiSetBackMix(hpsMem, BM_OVERPAINT);
  GpiSetColor(hps, CLR_CYAN);                 /* Malen mit Cyan auf Schwarz*/
  GpiSetBackColor(hps, CLR_BLACK);
  GpiSetBackMix(hps, BM_OVERPAINT);
  strcpy(dialog.alpha,"35");                  /* Initialwerte fÅr die      */
  strcpy(dialog.beta,"0");                    /* Dialog-Box.               */
  strcpy(dialog.gamma,"40");
  strcpy(dialog.min,"-1000");
  strcpy(dialog.max,"1000");
  strcpy(dialog.vertikal,"-2");
  strcpy(dialog.ebenen,"100");
  strcpy(dialog.dichte,"10");
  dialog.zwert = FALSE;
  dialog.grad = TRUE;
  dialog.formel = 1;
  DosSemClear(&critical);                     /* Ende der kritischen       */
                                              /* Sektion (Semaphore von    */
                                              /* main() gesetzt).          */
}
/******************  Ende der Function Init  *******************************/


/*****************  Beginn der Prozedur Paint  *****************************/
/*                                                                         */
/*  Bearbeitet die WM_PAINT-Message der Window-Prozedur                    */
/*                                                                         */
/***************************************************************************/

VOID       Paint(HWND hwnd)  /* Nur der Window-Handler wird Åbergeben,     */
                             /* die anderen Parameter der WM_PAINT-Message */
                             /* sind NULL und somit nicht interessant.     */
{
  RECTL rc;                  /* Update-Region.                             */
  POINTL pt[4];          /* Rechtecke fÅr GpiPlayBitmap                */

  WinQueryUpdateRect(hwnd, &rc);   /* Neuzumalendes Rechteck.              */
  DosSemRequest(&critical, -1L);   /* Synchronisation mit Calculate().     */
  if (busy) {                      /* Calculate() am Rechnen?              */
    message = MSG_REPAINT;         /* ja: Repaint-Message setzen.          */
    DosSemSet(&repainted);         /* Siehe ÅbernÑchste Zeile.             */
    DosSemSet(&attention);         /* "Achtung, Meldung parat".            */
    DosSemWait(&repainted, -1L);   /* Warten, bis Calculate() gemalt hat.  */
  } else {
    pt[0].x = 0;                     /* Calculate() nicht am rechnen -->   */
    pt[0].y = 0;                     /* Bitmap selber neu malen.           */
    pt[1].x = cxClient;
    pt[1].y = cyClient;
    pt[2].x = 0;
    pt[2].y = 0;
    pt[3].x = xPixMax;
    pt[3].y = yPixMax;
    GpiBitBlt(hps,        /* d.H. Bitmap in Window Presentation   */
              hpsMem,       /*  Space kopieren.                     */
              4L,
              pt,
              ROP_SRCCOPY,
              BBO_OR
             );
  }
  DosSemClear(&critical);          /* Ende Synchronisation.                */
  WinValidateRect(hwnd, &rc, TRUE);/* Das Rechteck als gÅltig erklÑren.    */
}
/*******************  Ende der Prozedur Paint  *****************************/


/*******************  Start der About-Box Prozedur  ************************/
/*                                                                         */
/* Behandelt die Messages an die "about"-Box.                              */
/*                                                                         */
/***************************************************************************/

MRESULT EXPENTRY _loadds AboutDlgProc(HWND hwnd, USHORT msg,
                                      MPARAM mp1, MPARAM mp2)
{
  switch(msg)
  {
    case WM_COMMAND:
      switch(COMMANDMSG(&msg)->cmd)
      {
        case DID_OK:
        case DID_CANCEL:
          WinDismissDlg(hwnd, TRUE);
      }
      break;
  }
  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}
/*******************  Ende der Prozedur AboutBox  **************************/


/*******************  Start der Dialogprozedur  ****************************/
/*                                                                         */
/* Ein Dialog-Fenster ist, wie der name schon sagt, ebenfalls ein Fenster  */
/* und hat deshalb auch eine "Window-Procedure". Diese heisst hier DlgPrc. */
/*                                                                         */
/* In dieser Dialogprozedur werden wiederum alle Meldungen abgearbeitet,   */
/* die von OS/2 via die main()-Prozedur gesendet werden.                   */
/*                                                                         */
/***************************************************************************/
MRESULT EXPENTRY _loadds DlgProc(HWND hwndDlg, USHORT msg,
                                 MPARAM mp1, MPARAM mp2)
{
int    tempClicked;                   /* Hilfswert fÅr Formelnummer        */

  switch (msg)
  {
    case WM_INITDLG:                          /* Bei Erîffnung des Dialogs.*/
      CentreDlgBox(hwndDlg);                  /* Box zentrieren.           */
      WinSetDlgItemText(hwndDlg, IDD_ALPHA,   /* Die Dialog-Box wird mit   */
                        dialog.alpha);        /* den Werten aus der Vari-  */
      WinSetDlgItemText(hwndDlg, IDD_BETA,    /* able "dialog" gefÅllt.    */
                        dialog.beta);         /* Bei Text-Feldern kann dazu*/
      WinSetDlgItemText(hwndDlg, IDD_GAMMA,   /* WinSetDlgItemText ver-    */
                        dialog.gamma);        /* wendet werden.            */
      if (dialog.grad)                        /* Bei Check-Buttons muss    */
        WinSendDlgItemMsg(hwndDlg,            /* eine Meldung an den Button*/
                          IDD_GRAD,           /* gesendet werden.          */
                          BM_SETCHECK,
                          MPFROM2SHORT(TRUE,0),
                          0L);
      WinSetDlgItemText(hwndDlg, IDD_MIN,
                        dialog.min);
      WinSetDlgItemText(hwndDlg, IDD_MAX,
                        dialog.max);
      WinSetDlgItemText(hwndDlg, IDD_VERTIKAL,
                        dialog.vertikal);
      WinSetDlgItemText(hwndDlg, IDD_EBENEN,
                        dialog.ebenen);
      WinSetDlgItemText(hwndDlg, IDD_DICHTE,
                        dialog.dichte);
      if (dialog.zwert)
        WinSendDlgItemMsg(hwndDlg,
                          IDD_ZWERT,
                          BM_SETCHECK,
                          MPFROM2SHORT(TRUE,0),
                          0L);
      WinSendDlgItemMsg(hwndDlg,                   /* Dasselbe gilt fÅr    */
                        IDD_FORMEL + dialog.formel,/* Radiobuttons         */
                        BM_CLICK,
                        MPFROM2SHORT(TRUE,0),
                        0L);
      return 0;
    case WM_CONTROL:                          /* Etwas wurde angeklickt.   */
      if (SHORT2FROMMP(mp1) == BN_CLICKED)
        tempClicked = SHORT1FROMMP(mp1) - IDD_FORMEL;/* War es einer der   */
        else break;                                  /* Radiobuttons?      */
      if ((tempClicked > 0) && (tempClicked < 9))
        tempFormNr = tempClicked;             /* Der Wert wird gesichert,  */
        else break;                           /* bis Ok gedrÅckt wird.     */
      return 0;
    case WM_HELP:                        /* Der Help-Button wurde gedrÅckt.*/
      Help(hwndDlg);
      return 0;
    case WM_COMMAND:                          /* Einer der Kommandobuttons */
      switch(SHORT1FROMMP (mp1))              /* wurde angeklickt.         */
      {
        case DID_OK:                      /* Ok-Pushbutton gedrÅckt.       */
          ReadTempDlg(hwndDlg);           /* Dialog-Box auslesen.          */
          if(TestTempDlg(hwndDlg)) break; /* Eingaben prÅfen.              */
          DosSemRequest(&critical,-1L);   /* Synchronisation mit Calculate */
          if (busy) {                     /* Calculate() am Rechnen?       */
            message = MSG_RESTART;        /* ja: Meldung RESTART setzen.   */
            DosSemSet(&restarted);
            DosSemSet(&attention);        /* Calculate() unterbrechen.     */
            DosSemWait(&restarted,-1L);   /* warten, bis Calculate() bereit*/
          };
          CopyDlg(hwndDlg);               /* Variabeln setzen.             */
          DosSemClear(&inputReady);       /* Eingaben bereit.              */
          DosSemClear(&critical);         /* Ende der Synchronisation.     */
          WinDismissDlg(hwndDlg, TRUE);   /* Dialogbox schliessen          */
        case DID_CANCEL:                  /* Cancel-Button oder ESC-Taste. */
          WinDismissDlg(hwndDlg, FALSE);  /* Dialog-Box schliessen.        */
      }
      break;
    default:                              /* Alle anderen Meldongen zu OS/2*/
      return WinDefDlgProc(hwndDlg, msg, mp1, mp2);
  }
}
/*******************  Ende der Dialogprozedur  *****************************/


/****************  Start der Funktion CentreDialogBox  *********************/
/*                                                                         */
/*  Die Funktion CentreDialogBox zentriert die Dialog-Box in der           */
/*  Mitte des Bildschirms.                                                 */
/*                                                                         */
/***************************************************************************/

VOID CentreDlgBox(HWND hwnd)        /* Der Window-Handler der Dialog-Box   */
                                    /* wird als Parameter Åbergeben.       */
{
  SHORT ix, iy;                     /* Neue Position der Dialog-Box.       */
  SHORT iwidth, idepth;             /* Grîsse des Bildschirms              */
  SWP   swp;                        /* Window-Informationen                */

  /* Grîsse des Bildschirms abfragem.                                      */
  iwidth = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
  idepth = (SHORT)WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

  /* Window-Informationen der Dialog-Box fragen.                           */
  WinQueryWindowPos( hwnd, (PSWP)&swp );

  /* Dialog-Box zentrieren.                                                */
  ix = (iwidth - swp.cx) / 2;
  iy = (idepth - swp.cy) / 2;
  WinSetWindowPos( hwnd, HWND_TOP, ix, iy, 0, 0, SWP_MOVE );
}
/****************** Ende der Funktion CentreDlgBox *************************/


/**************  Beginn der Prozedur ReadTempDlg  **************************/
/*                                                                         */
/*  Liest die Dialog-Box in einen temporÑren Speicherbereich aus.          */
/*                                                                         */
/***************************************************************************/

VOID ReadTempDlg(HWND hwndDlg)       /* Der Window-Handler wird Åbergeben. */
{
  WinQueryDlgItemText(hwndDlg, IDD_ALPHA,          /* Die Werte werden in  */
                      STRINGLENGTH, tDialog.alpha);/* die Variable tDialog */
  WinQueryDlgItemText(hwndDlg, IDD_BETA,           /* abgelegt.            */
                      STRINGLENGTH, tDialog.beta); /* dort bleiben sie, bis*/
  WinQueryDlgItemText(hwndDlg, IDD_GAMMA,          /* die Eingaben Åber-   */
                      STRINGLENGTH, tDialog.gamma);/* prÅft worden sind.   */
  WinQueryDlgItemText(hwndDlg, IDD_MIN,
                      STRINGLENGTH, tDialog.min);
  WinQueryDlgItemText(hwndDlg, IDD_MAX,
                      STRINGLENGTH, tDialog.max);
  WinQueryDlgItemText(hwndDlg, IDD_VERTIKAL,
                      STRINGLENGTH, tDialog.vertikal);
  WinQueryDlgItemText(hwndDlg, IDD_EBENEN,
                      STRINGLENGTH, tDialog.ebenen);
  WinQueryDlgItemText(hwndDlg, IDD_DICHTE,
                      STRINGLENGTH, tDialog.dichte);
}
/******************  Ende der Prozedur ReadTempDlg  ************************/


/**************  Beginn der Prozedur TestTempDlg  **************************/
/*                                                                         */
/*  Testet den Inhalt der Dialog-Box.                                      */
/*                                                                         */
/***************************************************************************/
BOOL TestTempDlg(HWND hwndDlg)
{
  double testVar;
  int    testVar2;

  testVar = atof(tDialog.alpha);                   /* Wenn z.B. ein Text   */
  if ((testVar < -45.0) || (testVar > 45.0)) {     /* Eingegeben wird, ist */
    InputError(hwndDlg,IDS_EALPHA);                /* die Variable 0.      */
    return TRUE;
  }
  testVar = atof(tDialog.beta);
  if ((testVar < 0.0) || (testVar > 360.0)) {
    InputError(hwndDlg,IDS_EBETA);
    return TRUE;
  }
  testVar = atof(tDialog.gamma);
  if ((testVar < 10.0) || (testVar > 45.0)) {
    InputError(hwndDlg,IDS_EGAMMA);
    return TRUE;
  }
  testVar = atof(tDialog.max) - atof(tDialog.min);
  if (testVar <= 0.0) {
    InputError(hwndDlg,IDS_EMINMAX);
    return TRUE;
  }
  testVar = atof(tDialog.vertikal);
  if ((testVar < -100.0) || (testVar > 100.0)) {
    InputError(hwndDlg,IDS_EVERTIKAL);
    return TRUE;
  }
  testVar2 = atoi(tDialog.ebenen);
  if ((testVar2 < 3) || (testVar2 > 300)) {
    InputError(hwndDlg,IDS_EEBENEN);
    return TRUE;
  }
  testVar2 = atoi(tDialog.dichte);
  if ((testVar2 < 1) || (testVar2 > 10)) {
    InputError(hwndDlg,IDS_EDICHTE);
    return TRUE;
  }
  return FALSE;
}
/******************  Ende der Prozedur TestTempDlg  ************************/


/**************  Beginn der Prozedur CopyDlg  ******************************/
/*                                                                         */
/*  Kopiert den Inhalt von tDialog in dialog.                              */
/*                                                                         */
/***************************************************************************/
VOID CopyDlg(HWND hwndDlg)
{
  strcpy(dialog.alpha, tDialog.alpha);
  strcpy(dialog.beta, tDialog.beta);
  strcpy(dialog.gamma, tDialog.gamma);
  strcpy(dialog.min, tDialog.min);
  strcpy(dialog.max, tDialog.max);
  strcpy(dialog.vertikal, tDialog.vertikal);
  strcpy(dialog.ebenen, tDialog.ebenen);
  strcpy(dialog.dichte, tDialog.dichte);
  /* Die Inhalte der Buttons kînnen nicht so einfach ausgelesen werden,    */
  /* sondern mÅssen mittels einer Message abgefragt werden. Die Message    */
  /* heisst BM_QUERYCHECK und wird an den Button selbst gesendet. Wenn als */
  /* Antwort TRUE kommt, ist der Button angeklickt.                        */
  dialog.grad = SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwndDlg, IDD_GRAD),
                             BM_QUERYCHECK,
                             0L,
                             0L));
  dialog.zwert  = SHORT1FROMMR(WinSendMsg(WinWindowFromID(hwndDlg, IDD_ZWERT),
                               BM_QUERYCHECK,
                               0L,
                               0L));
  dialog.formel = tempFormNr;    /* Die Formelnummer kann jetzt            */
                                 /* definitiv gesetzt werden, da           */
                                 /* der Ok-Button gedrÅckt worden ist.     */
  alpha = atof(dialog.alpha);    /* Umwandeln der Strings in double's.     */
  alpha = alpha / RAD;           /* Alle Winkel werden in Radiant          */
  beta = atof(dialog.beta);      /*  umgerechnet.                          */
  beta = beta / RAD;
  gamma = atof(dialog.gamma);
  gamma = gamma / RAD;
  delta = gamma * 1.5;           /* Vertikalîffnungswinkel.                */
  degree = dialog.grad;
  dMin = atof(dialog.min);
  dMax = atof(dialog.max);
  d = dMax-dMin;
  if (degree) {
    dMin = dMin / RAD;
    dMax = dMax / RAD;
    d    = d    / RAD;
  }
  zEye = d * tan(alpha);
  r1 = atof(dialog.vertikal);
  zEye = zEye + r1;
  cutNr = atoi(dialog.ebenen);
  deltaP = atoi(dialog.dichte);
  xPixAnz = xPixMax / deltaP;
  deltaD = d/cutNr;
  secondZ = dialog.zwert;
  formNr = dialog.formel;
}
/******************  Ende der Prozedur CopyDlg  ****************************/

/**************  Beginn der Prozedur InputErrror  **************************/
/*                                                                         */
/*  Zeigt eine Fehlermeldung an.                                           */
/***************************************************************************/
VOID InputError(HWND hwndDlg, int msg)    /* msg ist ein im Ressourcen-File*/
{                                         /* definierter String.           */
char szTitle[20];
char szText[150];

WinLoadString(hab, 0L, msg, 150, szText); /* String laden.                 */
WinLoadString(hab, 0L, IDS_ETITLE, 20, szTitle);
WinMessageBox(HWND_DESKTOP,               /* Anzeigen der Meldung          */
              hwndDlg,
              szText,
              szTitle,
              0L,
              MB_OK | MB_ICONEXCLAMATION  /* Ein Button "Ok"               */
             );                           /* und ein Ausrufezeichen.       */
}
/******************  Ende der Prozedur InputError  *************************/


/*********************  Beginn der Prozedur Help  **************************/
/*                                                                         */
/*  Zeigt eine Hilfemeldung an.                                            */
/*                                                                         */
/*  Der Common-User-Access Teil der System Application Architecture (SAA)  */
/*  von IBM schreibt eigentlich ein indexiertes, kontext-sensitives Hilfe- */
/*  system vor. Dies ist naturgemÑss aufwendig zu programmieren. Ich habe  */
/*  aus ZeitgrÅnden darauf verzichtet.                                     */
/*  Mit der Version 1.2 von OS/2 werden einige Hilfen fÅr die Implemen-    */
/*  tation eines solchen Hilfesystems mitgeliefert. Wie diese aber genau   */
/*  funktionieren und wie einfach diese sind, weiss ich aber noch nicht.   */
/***************************************************************************/
VOID Help(HWND hwnd)
{
  char title[20];
  char help[200];

  WinLoadString(hab,
                0L,
                IDS_HELPTITLE,
                20,
                title
               );
  WinLoadString(hab,
                0L,
                IDS_HELP,
                200,
                help
               );
  WinMessageBox(HWND_DESKTOP,
                hwnd,
                help,
                title,
                0L,
                MB_OK | MB_NOICON
               );
}
/**********************  Ende der Prozedur Help  ***************************/



/**************  Beginn der Funktion Calculate  ****************************/
/*                                                                         */
/*  PRIVATE FUNKTION : Calculate                                           */
/*                                                                         */
/*  Berechnungsteil.                                                       */
/*                                                                         */
/*  Diese Prozedur muss "void far cdecl" deklariert werden, da sie         */
/*  durch _NEWTHREAD() gestartet wird.                                     */
/*                                                                         */
/*  Der Algorithmus selbst stammt -wie am Anfang erwÑhnt- aus der          */
/*  Zeitschrift M+K Computer. Kommentiert sind deshalb nur Elemente,       */
/*  die etwas mit dem Presentation-Manager oder der allgemeinen Ablauf-    */
/*  steuerung zu tun haben.                                                */
/*                                                                         */
/***************************************************************************/

void far cdecl Calculate()
{
  POINTL pt;
  RECTL  rcl;

  habCalc = WinInitialize((USHORT)NULL);
  restart:
  for(;;) {
    DosSemRequest(&critical, -1L);   /* Synchronisation.                   */
    busy = FALSE;                    /* Nicht beschÑftigt.                 */
    DosSemClear(&critical);          /* Ende Synchronisation.              */
    DosSemWait(&inputReady, -1L);    /* Warten, bis Eingaben bereit sind.  */
    DosSemRequest(&critical,-1L);    /* Synchronisation.                   */
    busy = TRUE;                     /* Am Rechnen.                        */
    DosSemSet(&inputReady);          /* Eingabe bereit lîschen.            */
    DosSemClear(&critical);          /* Ende Synchronisation.              */
    rcl.xLeft = 0;
    rcl.yBottom = 0;
    rcl.xRight = xPixMax;
    rcl.yTop = yPixMax;
    WinFillRect(hpsMem, &rcl, CLR_FALSE);
    WinInvalidateRegion(hwndClient, NULL, FALSE);
    for(i1=0; i1<xPixAnz; i1++) {
      sMin[i1] = yPixMax;
      sMax[i1] = -1;
    };
    for (i1=0; i1<cutNr; i1++) {
      dEye = d/2 + deltaD*i1;
      r1 = dEye * tan(delta/2);
      r2 = fabs(dMin+deltaD*i1);
      sinBeta = sin(beta);
      cosBeta = cos(beta);
      xsm = r2*cosBeta;
      ysm = r2*sinBeta;
      xsl = xsm+r1*sinBeta;
      ysl = ysm-r1*cosBeta;
      xsr = xsm-r1*sinBeta;
      ysr = ysm+r1*cosBeta;
      deltaX = (xsr-xsl)/xPixAnz;
      deltaY = (ysr-ysl)/xPixAnz;
      xw = xsl;
      yw = ysl;
      zMin = zEye-(dEye*tan(alpha+gamma/2));
      zMax = zEye-(dEye*tan(alpha-gamma/2));
      deltaZ = (zMax-zMin)/yPixMax;
      for (i2=0; i2<xPixAnz; i2++) {
        Formula();
        if (zExist)
          do {
            zPix = (int)((zw-zMin)/deltaZ);
            if (z2Exist) z2Exist = FALSE;
            else z2Exist = TRUE;
            see = FALSE;
            if (zPix > sMax[i2]) {
              sMax[i2] = zPix;
              see = TRUE;
            };
            if (zPix < sMin[i2]) {
              sMin[i2] = zPix;
              see = TRUE;
            };
            if ((zPix > 0) && (zPix < yPixMax) && see)
              Point(i2*deltaP,zPix);
            zw = z2;
          }
          while (!z2Exist);
        xw += deltaX;
        yw += deltaY;
                        /* Ist eine Meldung bereit?                        */
        if ((DosSemWait(&attention,1L)) == ERROR_SEM_TIMEOUT)
          switch (message) {      /* ja, welche?                           */
            case MSG_REPAINT:     /* muss Bitmap neu anzeigen              */
              pt.x = 0L;          /* und zwar ganzer Bereich (dies geht so */
              pt.y = 0L;          /* schnell, dass es sich nicht lohnt,    */
                                  /* nur die "update-region" anzuzeigen.   */
              WinDrawBitmap(hps,
                            hbm,
                            (PRECTL)NULL,
                            &pt,
                            CLR_CYAN,  /* Bits 1: Cyan                     */
                            CLR_BLACK, /* Bits 0: Schwarz                  */
                            DBM_NORMAL
                           );
              DosSemClear(&attention); /* "Achtung"-Flag lîschen           */
              DosSemClear(&repainted); /* Anzeige wieder i.O.              */
              break;
            case MSG_RESTART:   /* Die Prozedur muss neu starten (neue     */
                                /* Eingaben wurden gemacht).               */
              DosSemClear(&attention); /* "Achtung"-Flag lîschen.          */
              busy = FALSE;            /* Nicht mehr am rechnen.           */
              DosSemClear(&restarted); /* Ok, neu gestartet.               */
              goto restart;            /* Alle Loops verlassen.            */
              break;
            default:                   /* Nur zur Sicherheit...            */
              DosSemClear(&attention);
          } /* Ende von switch                                             */
          /* Ende von if                                                     */
      } /* Ende des inneren Loops (x-Werte)                                */
    } /* Ende des Ñusseren Loops (Ebenenschnitte)                          */
  } /* Ende der Endlos-Schleife                                            */
} /* Ende der Ñussersten Schleife (wird nie erreicht!)                     */
/****************  Ende der Prozedur Calculate  ****************************/


/**************  Beginn der Funktion Formula  ******************************/
/*                                                                         */
/*  PRIVATE FUNKTION : Formula                                             */
/*                                                                         */
/*  Gibt den Funktionswert (Z-Wert) zurÅck.                                */
/*                                                                         */
/***************************************************************************/

VOID       Formula(VOID)
{
  double radius,helpr;

  zExist = TRUE;
  z2Exist = FALSE;
  switch (formNr)
  {
    case 1:
      radius = sqrt(xw*xw+yw*yw);
      if (radius != 0.0L)
        zw = sin(radius)/radius*10;
      else
        zw = 5.0L;
      break;
    case 2:
      helpr = sqrt(xw*xw+yw*yw);
      zw = 1-cos(4*helpr)*exp(xw/8)*0.5*(1+cos(helpr));
      break;
    case 3:
      zw = 4*(fabs(cos(xw))+fabs(cos(yw)));
      if (zw < 1.0) zw = 1.0;
      break;
    case 4:
      helpr = 1/(1+xw*xw/5);
      zw = cos(yw/(1.3-helpr))*helpr;
      break;
    case 5:
      helpr = 1/(1+xw*xw/5);
      zw = cos(yw/helpr)*helpr;
      break;
    case 6:
      radius = sqrt(xw*xw+yw*yw);
      if (radius == 0.0) zw = 0.0;
      else zw=((fabs(xw)+fabs(yw))/radius-1)*10;
      break;
    case 7:
      helpr=100-xw*xw-yw*yw;
      if (helpr >= 0.0) {
        zw = sqrt(helpr);
        if (secondZ) {
          z2 = -zw;
          z2Exist = TRUE;
        };
      } else zExist = FALSE;
      break;
    case 8:
      radius = sqrt(xw*xw+yw*yw);
      if (radius > 5.0) zExist = FALSE; else {
        zw = sqrt(100-xw*xw-yw*yw);
        if (secondZ) {
          z2 = radius * 1.7320508;
          z2Exist = TRUE;
        }
      }
    default: /* Nur, um sicher zu sein...                                  */
      zw = 1.0L;
  }
}
/***************  Ende der Funktion Formula  *******************************/


/***************  Beginn der Funktion Point  *******************************/
/*                                                                         */
/*  PRIVATE FUNKTION : Point                                               */
/*                                                                         */
/*  Plotten eines Punktes.                                                 */
/*                                                                         */
/***************************************************************************/

VOID       Point(int x, int y)
{
  POINTL point;
  point.x = x;
  point.y = y;
  GpiSetPel(hpsMem, &point);
  GpiSetPel(hps, &point);
}
/***************  Ende der Function Point  *********************************/

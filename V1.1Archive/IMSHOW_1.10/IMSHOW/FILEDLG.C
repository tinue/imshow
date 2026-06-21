/******************  Modul "FILEDLG.C" Source Code File  *******************/
/*                                                                         */
/* MODUL: FILEDLG (Dialog-Box fÅr Fileauswahl)                             */
/*                                                                         */
/*                                                                         */
/* PROGRAMMIERT VON:                                                       */
/* -----------------                                                       */
/*  Martin Erzberger, 1989/90                                              */
/*  Adaptiert aus: "Programming The OS/2 Presentation Manager" von         */
/*                  Charles Petzold, S. 655-667                            */
/*                  (Microsoft Press, 1989, ISBN: 1-55615-170-5)           */
/*                                                                         */
/*                                                                         */
/* VERSION: 1.10, 1/90                                                     */
/* --------                                                                */
/*                                                                         */
/* ZWECK DES MODULS:                                                       */
/* -----------------                                                       */
/*  - Dialogbox zur Auswahl eines Files.                                   */
/*                                                                         */
/* WIE WIRD DAS PROGRAMM COMPILIERT:                                       */
/* ---------------------------------                                       */
/*                                                                         */
/*  Mit dem MAKEFILE von IMSHOW                                            */
/*                                                                         */
/*  BENôTIGTE SOURCEFILES:                                                 */
/*  ----------------------                                                 */
/*                                                                         */
/*   Siehe bei IMSHOW.C                                                    */
/*                                                                         */
/*                                                                         */
/*  BENôTIGTE LIBRARIES:                                                   */
/*  --------------------                                                   */
/*                                                                         */
/*    Siehe Bei IMSHOW.C                                                   */
/*                                                                         */
/*  BENôTIGTE SOFTWARE:                                                    */
/*  -------------------                                                    */
/*                                                                         */
/*    Siehe bei IMSHOW.C                                                   */
/*                                                                         */
/***************************************************************************/


#define INCL_WIN                       /* Window-Funktionen                */
#include <os2.h>                       /* OS/2 Header-File                 */

#include <mt\stdio.h>                  /* Multithread Include-Files        */
#include <mt\string.h>

#include "imshow.h"                    /* IMSHOW.C Header-File             */


/***************************************************************************/
/* Funktionen (Prototypen), diese werden im Programmtext kommentiert.      */
/***************************************************************************/

VOID             FillDirListBox (HWND, CHAR *);
VOID             FillFileListBox (HWND);
MRESULT EXPENTRY OpenDlgProc (HWND, USHORT, MPARAM, MPARAM);
SHORT            ParseFileName (CHAR *, CHAR *);
BOOL cdecl       GetFileName (CHAR *, HWND, HWND, BOOL);

/***************************************************************************/
/* Variabeln der Applikation.                                              */
/***************************************************************************/

CHAR szTemp[80];                       /* TemporÑrstring                   */
BOOL bOpenSave;                        /* Datei îffnen oder schreiben?     */

extern HAB  hab;                       /* Anchor Block Handler aus IMSHOW  */




/*******************  Beginn der Prozedur FillDirListBox  ******************/
/*                                                                         */
/* Die Prozedur FillDirListBox fÅllt die verfÅgbaren Directories in das    */
/* entsprechende Feld der Dialog-Box ein.                                  */
/*                                                                         */
/***************************************************************************/

VOID FillDirListBox (HWND hwnd, CHAR *pcCurrentPath)
{
  static CHAR szDrive[]="  :";                /* Platz fÅr Driveletter     */
  FILEFINDBUF findbuf;                        /* gefundene Files           */
  HDIR        hDir = 1;                       /* Directory Handler         */
  SHORT       sDrive;                         /* Drive-Nummer              */
  USHORT      usDriveNum;                     /* Drive-Nummer              */
  USHORT      usCurPathLen;                   /* Path-LÑnge                */
  USHORT      usSearchCount = 1;              /* Gefundene Files           */
  ULONG       ulDriveMap;                     /* alle Drives im System     */

  DosQCurDisk (&usDriveNum, &ulDriveMap);     /* Current Drive und Drivemap*/
  pcCurrentPath[0] = (CHAR) usDriveNum + '@'; /* Umwandlung in char        */
  pcCurrentPath[1] = ':';                     /* ':\' anhÑngen             */
  pcCurrentPath[2] = '\\';
  usCurPathLen = 64;                          /* Path 64 Bytes lang        */
  DosQCurDir (0, pcCurrentPath + 3, &usCurPathLen); /* Current dir.        */
                                                    /* anhÑngen            */

  WinSetDlgItemText (hwnd, IDD_PATH, pcCurrentPath); /* In Dialogbox kop.  */

                                            /* dir-Liste in Box lîschen:   */
  WinSendDlgItemMsg (hwnd, IDD_DIRLIST, LM_DELETEALL, NULL, NULL);

  for (sDrive = 0; sDrive < 26; sDrive++)    /* Alle Drives eintragen      */
    if (ulDriveMap & 1L << sDrive) {         /* Drive in Drive-Map?        */
      szDrive[1] = (CHAR) sDrive + 'A';      /* In Char umwandeln          */

      WinSendDlgItemMsg (hwnd, IDD_DIRLIST, LM_INSERTITEM, /* In Box       */
                         MPFROM2SHORT (LIT_END, 0),        /* schreiben    */
                         MPFROMP (szDrive));
    }

  /* Alle Directories suchen und eintragen:                                */
  DosFindFirst ("*.*", &hDir, 0x0017, &findbuf, sizeof findbuf,
                &usSearchCount, 0L);
  while (usSearchCount) {
    if (findbuf.attrFile & 0x0010 &&   /* Nur Directories eintragen        */
                                       /* aber '.' auslassen:              */
        (findbuf.achName[0] != '.' || findbuf.achName[1]))
      WinSendDlgItemMsg (hwnd, IDD_DIRLIST, LM_INSERTITEM,
                         MPFROM2SHORT (LIT_SORTASCENDING, 0),
                         MPFROMP (findbuf.achName));

    DosFindNext (hDir, &findbuf, sizeof findbuf, &usSearchCount);
  } /* Ende while */
} /* Ende for */
/*******************  Ende der Prozedur FillDirListBox  ********************/



/******************  Beginn der Prozedur FillFileListBox  ******************/
/*                                                                         */
/* Die Prozedur FillFileListBox fÅllt die verfÅgbaren Files in das         */
/* entsprechende Feld der Dialog-Box ein.                                  */
/*                                                                         */
/***************************************************************************/

VOID FillFileListBox (HWND hwnd)
{
  FILEFINDBUF findbuf;               /* Gefundene Files                    */
  HDIR        hDir = 1;              /* Directory-Handler                  */
  USHORT      usSearchCount = 1;     /* Anzahl gefundene Files             */

  /* Feld lîschen:                                                         */
  WinSendDlgItemMsg (hwnd, IDD_FILELIST, LM_DELETEALL, NULL, NULL);

  /* Files suchen (auch Read-Only Files werden gesucht):                   */
  DosFindFirst ("*.*", &hDir, 0x001, &findbuf, sizeof findbuf,
                    &usSearchCount, 0L);
  while (usSearchCount) {
    WinSendDlgItemMsg (hwnd, IDD_FILELIST, LM_INSERTITEM,   /* Files ein-  */
                       MPFROM2SHORT (LIT_SORTASCENDING, 0), /* tragen      */
                       MPFROMP (findbuf.achName));          /* (sortiert)  */

    DosFindNext (hDir, &findbuf, sizeof findbuf, &usSearchCount);
  }
}
/******************  Ende der Prozedur FillFileListBox  ********************/



/*********************  Beginn der Dialog-Prozedur  ************************/
/*                                                                         */
/* Die Dialogprozedur behandelt alle Meldungen an die Dialog-Box.          */
/*                                                                         */
/***************************************************************************/

MRESULT EXPENTRY OpenDlgProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
static CHAR szCurrentPath[80], szBuffer[80];   /* Strings                  */
SHORT       sSelect;                           /* Selektiertes Item        */

switch (msg) {
  case WM_INITDLG:                             /* Dielogbox wurde erîffnet */
    if (!bOpenSave) {                          /* Als Resultat von "File-  */
                                               /* Open" oder "File Save"?  */
      WinLoadString(hab,                       /* Bei "File-Open" einigen  */
                    0L,                        /* Text Ñndern.             */
                    IDS_FILEOPEN,              /* 1. Titel                 */
                    20,
                    szTemp
                   );
      WinSetDlgItemText (hwnd, IDD_FILEOPEN, szTemp);

      WinLoadString(hab,
                    0L,
                    IDS_DID_OK,                /* 2. Pushbutton            */
                    20,
                    szTemp
                   );
      WinSetDlgItemText (hwnd, DID_OK, szTemp);
    }
    FillDirListBox (hwnd, szCurrentPath);  /* Directories einfÅllen        */
    FillFileListBox (hwnd);                /* Files einfÅllen              */

                                           /* FeldlÑnge begrenzen:         */
    WinSendDlgItemMsg (hwnd, IDD_FILEEDIT, EM_SETTEXTLIMIT,
                       MPFROM2SHORT (80,0), NULL);
    return 0;

  case WM_CONTROL:
    /* Das angewÑhlte File oder Directory wird in die Variable szBuffer    */
    /* kopiert:                                                            */
    if (SHORT1FROMMP (mp1) == IDD_DIRLIST ||
        SHORT1FROMMP (mp1) == IDD_FILELIST) {
          sSelect = (USHORT) WinSendDlgItemMsg (hwnd, /* Nummer bestimmen  */
                                 SHORT1FROMMP (mp1),
                                 LM_QUERYSELECTION, 0L, 0L);
          WinSendDlgItemMsg (hwnd, SHORT1FROMMP (mp1), /* Kopieren         */
                             LM_QUERYITEMTEXT,
                             MPFROM2SHORT (sSelect, sizeof szBuffer),
                             MPFROMP (szBuffer));
    }

    switch (SHORT1FROMMP (mp1)) {
      case IDD_DIRLIST:                /* Meldung von der Directory-Liste: */
        switch (SHORT2FROMMP (mp1)) {
          case LN_ENTER:               /* Enter oder Doubleclick           */
            if (szBuffer[0] == ' ')    /* Erstes Zeichen leer? --> war     */
              DosSelectDisk (szBuffer[1] - '@');  /* Laufwerk              */
            else
              DosChDir (szBuffer, 0L);            /* nein--> war Directory */

            FillDirListBox (hwnd, szCurrentPath); /* Felder neu fÅllen     */
            FillFileListBox (hwnd);

            WinSetDlgItemText (hwnd, IDD_FILEEDIT, "");
            return 0;
        }
        break;

      case IDD_FILELIST:             /* Meldung von der File-Liste:        */
        switch (SHORT2FROMMP (mp1)) {
          case LN_SELECT:            /* Einfackclick                       */
            WinSetDlgItemText (hwnd, IDD_FILEEDIT, szBuffer); /* In Edit-  */
            return 0;                                 /* Feld kopieren     */

          case LN_ENTER:             /* Enter oder Doubleclick             */
            ParseFileName (szTemp, szBuffer);  /* Filenamen parsen         */
            WinDismissDlg (hwnd, TRUE);        /* Dialog schliessen        */
            return 0;                          /* Nur zur Sicherheit       */
        }
        break;
    }
    break;

  case WM_COMMAND:
    switch (COMMANDMSG(&msg)->cmd) {
      case DID_OK:                             /* Enter-Button oder -Taste */
        WinQueryDlgItemText (hwnd, IDD_FILEEDIT, /* Editfeld auslesen      */
                             sizeof szBuffer, szBuffer);

        switch (ParseFileName (szCurrentPath, szBuffer)) {  /* Parsen      */
          case 0:                                  /* Gab einen Fehler     */
            WinAlarm (HWND_DESKTOP, WA_ERROR);     /* Beep                 */
            FillDirListBox (hwnd, szCurrentPath);  /* Felder wieder fÅllen */
            FillFileListBox (hwnd);
            return 0;

          case 1:                                  /* war Directory        */
            FillDirListBox (hwnd, szCurrentPath);  /* Felder neu fÅllen    */
            FillFileListBox (hwnd);
            WinSetDlgItemText (hwnd, IDD_FILEEDIT, "");
            return 0;

          case 2:                                  /* War ein File         */
            strcpy (szTemp, szCurrentPath);        /* Name kopieren        */
            WinDismissDlg (hwnd, TRUE);            /* Dialog beenden       */
            return 0;                              /* Nur zur Sicherheit   */
        }
        break;

      case DID_CANCEL:                             /* Cancel-Button        */
        WinDismissDlg (hwnd, FALSE);               /* Dialog beenden       */
        return 0;                                  /* Nur zur Sicherheit   */
    }
    break;
  }
  return WinDefDlgProc (hwnd, msg, mp1, mp2);      /* Rest an PM zurÅck    */
}
/*********************  Ende der Dialog-Prozedur  **************************/



/*******************  Beginn der Prozedur ParseFileName  *******************/
/*                                                                         */
/* Die Prozedur ParseFileName nimmt den String pcIn entgegen und gibt ihn  */
/* mit Laufwerk und Pfad versehen in pcOut zurÅck.                         */
/*                                                                         */
/* Return-Codes: 0 - pcIn war ungÅltig                                     */
/*               1 - pcIn war leer oder enthielt keinen Filenamen          */
/*               2 - pcIn war gÅltig, pcOut zeigt auf vollen Filenamen     */
/*                                                                         */
/***************************************************************************/

SHORT ParseFileName (CHAR *pcOut, CHAR *pcIn)
{
  CHAR   *pcLastSlash, *pcFileOnly;
  ULONG  ulDriveMap;
  USHORT usDriveNum, usDirLen = 64;

  strupr (pcIn);               /* String in Grossbuchstaben umwandeln      */

  if (pcIn[0] == '\0')         /* String leer?                             */
    return 1;

  if (pcIn[1] == ':') {        /* Mîglicherweise Drivename?                */
    if (DosSelectDisk (pcIn[0] - '@'))  /* Testen, wenn falsch, return 0   */
      return 0;
    pcIn += 2;                 /* sonst Drive wechseln, Pointer erhîhen    */
  }

  DosQCurDisk (&usDriveNum, &ulDriveMap);  /* Neuen Curr. Drive fragen     */

  *pcOut++ = (CHAR) usDriveNum + '@';      /* Mit ':\' versehen            */
  *pcOut++ = ':';
  *pcOut++ = '\\';

  if (pcIn[0] == '\0')                  /* Ausser Drive noch mehr in pcIn? */
    return 1;

  if (NULL == (pcLastSlash = strrchr (pcIn, '\\'))) {  /* hat es einen \   */
                                                       /* in pcIn?         */
    if (!DosChDir (pcIn, 0L))          /* nein: evtl. Directory, Test mit  */
      return 1;                        /* Dir. wehseln. Wenn gut: return 1 */
                                       /* Wenn Fehler, ist es evtl. File   */
    DosQCurDir (0, pcOut, &usDirLen);  /* Current Dir. anhÑngen.           */

    if (strlen (pcIn) > 12)    /* Rest > 12 Zeichen? (8 Name, 3 Extension) */
      return 0;                /* ja: Fehler, return 0                     */

    if (*(pcOut + strlen (pcOut) - 1) != '\\') /* Nein: evtl. noch \ an-   */
      strcat (pcOut++, "\\");                  /* hÑngen,                  */

    strcat (pcOut, pcIn);                      /* File anhÑngen            */
    return 2;                                  /* zurÅck                   */
  } /* Ende if --> es hat einen \ in pcIn */

  if (pcIn == pcLastSlash) {          /* \ erstes Zeichen?                 */
    DosChDir ("\\", 0L);              /* Zu Root wechseln                  */

    if (pcIn[1] == '\0')              /* \ auch einziges Zeichen?          */
      return 1;                       /* ja --> return 1                   */

    strcpy (pcOut, pcIn + 1);         /* sonst alles ausser \ kopieren     */
    return 2;                         /* zurÅck                            */
  } /* Ende if --> \ ist nicht erstes Zeichen */

  *pcLastSlash = '\0';                /* Alles hinter letztem \ lîschen    */

  if (DosChDir (pcIn, 0L))            /* Versuch, Verzeichnis wechseln     */
    return 0;                         /* Falls unmîglich --> Fehler        */

  DosQCurDir (0, pcOut, &usDirLen);   /* Sonst Directory anhÑngen          */

  pcFileOnly = pcLastSlash + 1;       /* Rest (=Filename) kopieren         */

  if (*pcFileOnly == '\0')            /* leer?                             */
    return 1;                         /* --> da war kein Filename          */

  if (strlen (pcFileOnly) > 12)       /* Sonst Test ob < 12 Zeichen        */
    return 0;                         /* Nein: Fehler                      */

  if (*(pcOut + strlen (pcOut) - 1) != '\\') /* Evtl. \ anhÑngen           */
    strcat (pcOut++, "\\");

  strcat (pcOut, pcFileOnly);         /* Filename anhÑngen                 */
  return 2;                           /* Und fertig                        */
}
/*********************  Ende der Prozedur ParseFileName  *******************/



/*******************  Beginn der Prozedur GetFileName  *********************/
/*                                                                         */
/* Die Prozedur GetFileName dient als Modulschnittstelle. Als Input er-    */
/* hÑlt sie die Window-Handler fÅr die Dialogbox (Parent- und Owner-       */
/* Handle).                                                                */
/* Als Output gibt sie den ausgewÑhlten Filenamen zurÅck.                  */
/*                                                                         */
/* Return-Codes: TRUE -  File wurde ausgewÑhlt.                            */
/*               FALSE - Fehler oder Cancel                                */
/*                                                                         */
/***************************************************************************/

BOOL cdecl GetFileName (CHAR *szFileName, HWND hwndParent,
                        HWND hwndOwner, BOOL bOpen)
{
  BOOL bReturn;

  bOpenSave = bOpen;                          /* Variable umkopieren       */

  bReturn = WinDlgBox (hwndParent, hwndOwner, OpenDlgProc,
                       NULL, IDD_OPEN, NULL);
  strcpy (szFileName, szTemp);
  return bReturn;
}
/*********************  Ende der Prozedur GetFileName  *********************/

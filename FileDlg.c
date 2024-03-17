/*****************  Module "FILEDLG.C" Source Code File  *******************/
/*                                                                         */
/* MODULE: FileDlg.c (Routines to choose a file)                           */
/*                                                                         */
/*                                                                         */
/* DEVELOPED BY:                                                           */
/* -------------                                                           */
/*  Martin Erzberger, 1989/93                                              */
/*                                                                         */
/* VERSION: 2.02, 1/93                                                     */
/* --------                                                                */
/*                                                                         */
/* PURPOSE OF MODULE:                                                      */
/* ------------------                                                      */
/*  - Routines to choose a file to load.                                   */
/*                                                                         */
/***************************************************************************/


#define INCL_WINSTDFILE                /* File dialog                      */
#include <os2.h>                       /* Main OS/2 header file            */

#include <stdio.h>
#include <string.h>
#include <memory.h>


/********************  Start of procedure GetFileName  *********************/
/*                                                                         */
/* The procedure GetFileName is the moduleinterface. It gets the parent-   */
/* and ownerhandle of the dialog box as input, and giver back the          */
/* choosen filename.                                                       */
/* The flag bOpen is not currently used.                                   */
/*                                                                         */
/* Returncodes: TRUE -  A filename was choosen                             */
/*              FALSE - There was an error or the user chose "cancel"      */
/*                                                                         */
/***************************************************************************/

BOOL GetFileName (CHAR *szFileName, HWND hwndParent,
                        HWND hwndOwner, BOOL bOpen)
{
  FILEDLG  fild;
  CHAR tempFileName[255];

  memset (&fild, 0, sizeof(FILEDLG));

  fild.cbSize = sizeof(FILEDLG);

  if (bOpen) {
     fild.fl = FDS_OPEN_DIALOG | FDS_CENTER;
     fild.pszTitle = "Open im or Bitmap File:";
  } else {
     fild.fl = FDS_SAVEAS_DIALOG | FDS_CENTER;
     fild.pszTitle = "Save im or Bitmap File:";
  } /* endif */

  WinFileDlg (hwndParent, hwndOwner, &fild);
  if (fild.lReturn == DID_OK) {                  /* There was a selection  */
     strcpy (szFileName, fild.szFullFile);       /* Get the filename       */
     /* I want to change AND STAY into a directory, so there is some       */
     /* more coding to do:                                                 */
     DosSetDefaultDisk ((ULONG)(szFileName[0]-'@'));  /* Set drive         */
     strcpy (tempFileName, szFileName);               /* Save filename     */
     *(strrchr(tempFileName, '\\')) = '\0';           /* Cut of filename   */
     if (tempFileName[0] == '\0')                     /* Was file in root? */
       DosSetCurrentDir ("\\");                       /* ok, so set it     */
     else
       DosSetCurrentDir (tempFileName);               /* else set dir.     */
     return TRUE;                                    /* Give back success  */
  } else {
     return FALSE;                                   /* or failure...      */
  } /* endif */
}
/*********************  End of procedure GetFileName  *********************/

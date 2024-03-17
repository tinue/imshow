/*****************************  IMSHOW.H  **********************************/
/*                                                                         */
/*  imShow.h contains the symbolic names of the main source file           */
/*  (imShow.c) and of the ressource file (imShow.rc).                      */
/*                                                                         */
/*  imShow.h, Version 2.02, 1/93                                           */
/*                                                                         */
/*  Variables, procedures and constants are declarated in the source       */
/*  code itself.                                                           */
/*                                                                         */
/***************************************************************************/


#define ID_RESOURCE   1           /* ID of application ressources          */

#define IDD_ABOUT     2           /* About dialog box                      */
#define IDD_ERRORS    3           /* Error dialogboxes                     */
#define IDD_CANCEL    4           /* Cancel dialogbox                      */
#define IDD_PERCENT   41
#define IDD_PROP      5           /* Parameter dialogbox                   */
#define IDD_TYPE      51           /* Some textfields of the param. box    */
#define IDD_STORAGE   52
#define IDD_SIZEX     53
#define IDD_SIZEY     54
#define IDD_SAMPLES   55
#define IDD_DEPTH     56
#define IDD_COLORS    57

#define IDM_FILE      1           /* Some menuchoices                      */
#define IDM_OPEN      11
#define IDM_WRITE     12
#define IDM_QUIT      13
#define IDM_VIEW      30
#define IDM_FIT       31
#define IDM_25        32
#define IDM_50        33
#define IDM_75        34
#define IDM_100       35
#define IDM_200       36
#define IDM_400       37
#define IDM_800       38
#define IDM_PROP      4
#define IDM_HELP      5
#define IDM_HELPHELP  51
#define IDM_HELPEXT   52
#define IDM_HELPKEYS  53
#define IDM_HELPIND   54
#define IDM_ABOUT     55

#define IDS_HELPTITLE 1           /* Symbols for the strings               */
#define IDS_HELP      10
#define IDS_ERRTITLE  2
#define IDS_FILEERR   20
#define IDS_TYPEERR   21
#define IDS_FLTERR    22
#define IDS_STARTERR  23
#define IDS_IMTITLE   3
#define IDS_CONVERT   30
#define IDS_FILEOPEN  40
#define IDS_DID_OK    41
#define IDS_GIFERR    50
#define IDS_GIFUNEX   51
#define IDS_GIF_NOIMG 52
#define IDS_GIF_BOGUS 53
#define IDS_GIF_INTL  54

#define WM_FINISHED   (WM_USER + 0) /* Symbol for a user message           */
#define WM_PROGRESS   (WM_USER + 1)

#define UNKNOWN       10            /* Symbols for filetypes               */
#define BITMAP        20
#define OS2_101       21
#define WIN_3         22
#define OS2_200       23
#define GIF           30
#define IM            40
#define IMERROR       41

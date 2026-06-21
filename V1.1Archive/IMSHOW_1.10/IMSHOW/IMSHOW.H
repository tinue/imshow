/*****************************  IMSHOW.H  **********************************/
/*                                                                         */
/*  Das Headerfile enthÑlt sie symbolischen Bezeichner des Source-Codes    */
/*  (IMSHOW.C) und des Ressource-Files (IMSHOW.RC).                        */
/*                                                                         */
/*  IMSHOW.H, Version 1.10, 1/90                                           */
/*                                                                         */
/*  Variabeln , Prozeduren und Konstanten sind im Source-Code selbst       */
/*  deklariert.                                                            */
/*                                                                         */
/***************************************************************************/


#define ID_RESOURCE   1           /* ID der Applikations-Ressourcen        */

#define IDD_ABOUT     2           /* About-Box                             */
#define IDD_OPEN      3           /* Open-File Box                         */
#define IDD_FILEOPEN  31           /* Diverse Felder der Open-File Box     */
#define IDD_PATH      32
#define IDD_FILEEDIT  33
#define IDD_DIRLIST   34
#define IDD_FILELIST  35
#define IDD_CANCEL    4           /* Cancel-Box                            */
#define IDD_PARAM     5           /* Parameter-Box                         */
#define IDD_TYPE      51           /* Diverse Felder der Parameter Box     */
#define IDD_STORAGE   52
#define IDD_SIZEX     53
#define IDD_SIZEY     54
#define IDD_SAMPLES   55
#define IDD_DEPTH     56
#define IDD_COLORS    57

#define IDM_FILE      1           /* Diverse MenÅauswahlen                 */
#define IDM_OPEN      11
#define IDM_WRITE     12
#define IDM_QUIT      13
#define IDM_CLIPBOARD 2
#define IDM_COPY      21
#define IDM_OPTIONS   3
#define IDM_SCROLL    31
#define IDM_COMP      32
#define IDM_PARAM     4
#define IDM_HELP      5
#define IDM_HELPHELP  51
#define IDM_HELPEXT   52
#define IDM_HELPKEYS  53
#define IDM_HELPIND   54
#define IDM_ABOUT     55

#define IDS_HELPTITLE 1           /* Bezeichner fÅr Strings                */
#define IDS_HELP      10
#define IDS_ERRTITLE  2
#define IDS_FILEERR   20
#define IDS_TYPEERR   21
#define IDS_FLTERR    22
#define IDS_IMTITLE   3
#define IDS_CONVERT   30
#define IDS_FILEOPEN  40
#define IDS_DID_OK    41

#define WM_FINISHED   (WM_USER + 0) /* Bezeichner fÅr eine Message         */

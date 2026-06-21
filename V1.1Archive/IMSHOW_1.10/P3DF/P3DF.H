/********************************  P3DF.H  *********************************/
/*                                                                         */
/*  Das Headerfile enthÑlt sie symbolischen Bezeichner des Source-Codes    */
/*  (P3DF.C) und des Ressource-Files (P3DF.RC).                            */
/*                                                                         */
/*  P3DF.H, Version 1.00, 8/89                                             */
/*                                                                         */
/*  Variabeln, Prozeduren und Konstanten sind im Source-Code selbst        */
/*  deklariert.                                                            */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


#define ID_RESOURCE   1           /* ID der Applikations-Ressourcen        */
#define IDD_ABOUT     2           /* About-Box                             */
#define ID_DIALOG     3           /* Dialog-Box                            */

#define MSG_REPAINT   (BYTE)1     /* Meldungen an den asynchronen Thread   */
#define MSG_RESTART   (BYTE)2

#define IDM_FILE      1           /* Diverse MenÅauswahlen                 */
#define IDM_NEW       10
#define IDM_OPEN      11
#define IDM_SAVE      12
#define IDM_SAVEAS    13
#define IDM_SAVEQUIT  14
#define IDM_QUIT      15
#define IDM_PRINT     16
#define IDM_ABOUT     17
#define IDM_PARAM     2
#define IDM_HELP      3

#define IDS_ETITLE    1           /* Die Strings selbst sind im Ressourcen */
#define IDS_EALPHA    2           /* File definiert.                       */
#define IDS_EBETA     3
#define IDS_EGAMMA    4
#define IDS_EMINMAX   5
#define IDS_EVERTIKAL 6
#define IDS_EEBENEN   7
#define IDS_EDICHTE   8
#define IDS_HELP      9
#define IDS_HELPTITLE 10

#define IDD_ALPHA     1           /* Felder der Dialog-Box                 */
#define IDD_BETA      2
#define IDD_GAMMA     3
#define IDD_GRAD      4
#define IDD_MIN       5
#define IDD_DICHTE    6
#define IDD_EBENEN    7
#define IDD_HILFE     8
#define IDD_MAX       9
#define IDD_VERTIKAL  10
#define IDD_ZWERT     11

/* Die folgenden Werte mÅssen aufeinanderfolgend sein!                     */
/* Die Reihenfolge der Bezeichner darf nicht geÑndert werden!              */
/* Im Code werden die nachfolgenden Werte nicht einzeln getestet (mittels  */
/* case-Statements), sondern der Bezeichner IDD_FORMEL wird als Index auf  */
/* die anderen Bezeichner gebraucht. Deshalb gilt die obgenannte Ein-      */
/* schrÑnkung.                                                             */
#define IDD_FORMEL    30
#define IDD_WELLEN1   31
#define IDD_WELLEN2   32
#define IDD_GEWOELBE  33
#define IDD_FALTBLATT 34
#define IDD_BERGE     35
#define IDD_DUENEN    36
#define IDD_KUGEL     37
#define IDD_SEKTOR    38

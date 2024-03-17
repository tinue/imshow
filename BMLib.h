/******************************  BMLIB.H  **********************************/
/*                                                                         */
/* BMLib.h: Headerfile to BMLib.c, Version 2.02, 1/93                      */
/*                                                                         */
/***************************************************************************/

extern HFILE  bmopen (CHAR *, CHAR *);
extern VOID   bmclose (HFILE);
extern VOID   bmrhdr (HFILE , PBITMAPINFO2);
extern VOID   bmwhdr (HFILE , PBITMAPINFOHEADER2, RGB2 *);
extern BYTE * bmrowsalloc (ULONG);
extern VOID   bmrrows (HFILE , BYTE *, ULONG, ULONG);
extern VOID   bmwrows (HFILE , BYTE *, ULONG, ULONG);

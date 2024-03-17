/*****************************  GIFLIB.H  **********************************/
/*                                                                         */
/* GIFLib.h: Headerfile to GIFLib.c, Version 2.02, 1/93                    */
/*                                                                         */
/***************************************************************************/

extern HFILE  gifopen (CHAR *, CHAR *);
extern VOID   gifclose (HFILE);
extern LONG   gifrhdr (HFILE , PBITMAPINFO2);
extern BYTE * gifrowsalloc (ULONG);
extern VOID   gifrrows (HFILE, BYTE *, ULONG, ULONG);
extern int    LWZReadByte(HFILE, int, int);

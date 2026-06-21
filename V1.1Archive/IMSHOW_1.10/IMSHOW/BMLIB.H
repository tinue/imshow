/******************************  BMLIB.H  **********************************/
/*                                                                         */
/* BMLIB.H: Header-File zu BMLIB.C, Version 1.10, 1/90                     */
/*                                                                         */
/***************************************************************************/

extern FILE * far pascal bmopen (CHAR *, CHAR *);
extern VOID   far pascal bmclose (FILE *);
extern VOID   far pascal bmrhdr (FILE *, PBITMAPINFO);
extern VOID   far pascal bmwhdr (FILE *, PBITMAPINFOHEADER, RGB *);
extern BYTE * far pascal bmrowalloc (VOID);
extern VOID   far pascal bmrrow (FILE *, BYTE *, USHORT);
extern VOID   far pascal bmwrow (FILE *, BYTE *, USHORT);

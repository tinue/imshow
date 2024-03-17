# IBM Developer's Workframe/2 Make File Creation run at 09:22:12 on 01/05/93

# Make File Creation run in directory:
#   C:\DATEN\SOURCE\IMSHOW;

.SUFFIXES:

.SUFFIXES: .c .rc

ALL: IMSHOW.EXE \
     imShow.RES

imShow.exe:  \
  BMLib.OBJ \
  FileDlg.OBJ \
  GIFLib.OBJ \
  imShow.OBJ \
  imShow.RES \
  IMSHOW.MAK
   @REM @<<IMSHOW.@0
     /NOI /NOL /PM:PM +
     BMLib.OBJ +
     FileDlg.OBJ +
     GIFLib.OBJ +
     imShow.OBJ
     imShow.exe
     
     imlib.lib
     ;
<<
   LINK386.EXE @IMSHOW.@0
   RC imShow.RES imShow.exe

{.}.rc.res:
   RC -r .\$*.RC

{.}.c.obj:
   ICC.EXE /Q /Kbap /O /W2 /Gm /G4 /C .\$*.c

!include IMSHOW.DEP

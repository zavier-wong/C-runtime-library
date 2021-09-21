cl /c /DWIN32 /utf-8 /GS- entry.c heap.c  stdio.c string.c
cl /c /DWIN32 /utf-8 /GS-  /GR- new_delete.cpp

lib entry.obj heap.obj stdio.obj string.obj new_delete.obj /OUT:crt.lib

cl /c /DWIN32 /GR- /utf-8 main.cpp
link main.obj crt.lib kernel32.lib /NODEFAULTLIB /DEBUG /entry:entry  /OUT:main.exe
del *.obj *.ilk *.pdb
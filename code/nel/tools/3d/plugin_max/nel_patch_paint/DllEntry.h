#ifndef __MODS__H
#define __MODS__H

#pragma warning (disable : 4786)
#include "Max.h"
//#include "reslib.h"


TCHAR *GetString(int id);

extern ClassDesc* GetEditPatchModDesc();

// in mods.cpp
extern HINSTANCE hInstance;

// For 'Supports Object of Type' rollups
extern BOOL CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

#define BIGFLOAT	float(999999)

#define NEWSWMCAT	_T("Modifiers")
extern int *meshSubTypeToolbarIDs;

#endif


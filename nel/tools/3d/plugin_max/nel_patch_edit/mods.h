/**********************************************************************
 *<
	FILE: mods.h

	DESCRIPTION:

	CREATED BY: Rolf Berteig (based on prim.h)

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#ifndef __MODS__H
#define __MODS__H

#pragma warning (disable : 4786)
#include "Max.h"
//#include "reslib.h"

TCHAR *GetString(int id);

extern ClassDesc* GetEditPatchModDesc();

// This is just temporary to make some extra mods so I can
// implement the 'more' system in the modify panel.
extern ClassDesc* GetBendModDesc2();
extern ClassDesc* GetBendModDesc3();
extern ClassDesc* GetBendModDesc4();
extern ClassDesc* GetBendModDesc5();
extern ClassDesc* GetSDeleteModDesc();

// in mods.cpp
extern HINSTANCE hInstance;

// For 'Supports Object of Type' rollups
extern BOOL CALLBACK DefaultSOTProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

#define BIGFLOAT	float(999999)

#define NEWSWMCAT	_T("Modifiers")

// Image list used for mesh sub-object toolbar in Edit Mesh, Mesh Select:
class MeshSelImageHandler {
public:
	HIMAGELIST images;
	MeshSelImageHandler () { images = NULL; }
	~MeshSelImageHandler () { if (images) ImageList_Destroy (images); }
	HIMAGELIST LoadImages ();
};
#define IDC_SELVERTEX 0x3260
#define IDC_SELEDGE 0x3261
#define IDC_SELFACE 0x3262
#define IDC_SELPOLY 0x3263
#define IDC_SELELEMENT 0x3264
extern int *meshSubTypeToolbarIDs;

#endif


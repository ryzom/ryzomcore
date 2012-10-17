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

/*#define EDITMESH_CLASS_ID			0x00050
#define EDITSPLINE_CLASS_ID			0x00060
#define EDITPATCH_CLASS_ID			0x00070
#define EDITLOFT_CLASS_ID			0x00080

#define CLUSTOSM_CLASS_ID			0x25215824

#define RESET_XFORM_CLASS_ID		0x8d562b81
#define CLUSTNODEOSM_CLASS_ID		0xc4d33*/


/*extern ClassDesc* GetBombObjDesc();
extern ClassDesc* GetBombModDesc();

extern ClassDesc* GetBendModDesc();
extern ClassDesc* GetTaperModDesc();
extern ClassDesc* GetSinWaveObjDesc();
extern ClassDesc* GetSinWaveModDesc();
extern ClassDesc* GetLinWaveObjDesc();
extern ClassDesc* GetLinWaveModDesc();
extern ClassDesc* GetEditMeshModDesc();
extern ClassDesc* GetEditSplineModDesc();*/
extern ClassDesc* GetEditPatchModDesc();
/*extern ClassDesc* GetTwistModDesc();
extern ClassDesc* GetTwistModDesc2();
extern ClassDesc* GetExtrudeModDesc();
extern ClassDesc* GetClustModDesc();
extern ClassDesc* GetSkewModDesc();
extern ClassDesc* GetNoiseModDesc();
extern ClassDesc* GetSinWaveOModDesc();
extern ClassDesc* GetLinWaveOModDesc();
extern ClassDesc* GetOptModDesc();
extern ClassDesc* GetDispModDesc();
extern ClassDesc* GetClustNodeModDesc();
extern ClassDesc* GetGravityObjDesc();
extern ClassDesc* GetGravityModDesc();
extern ClassDesc* GetWindObjDesc();
extern ClassDesc* GetWindModDesc();
extern ClassDesc* GetDispObjDesc();
extern ClassDesc* GetDispWSModDesc();
extern ClassDesc* GetDeflectObjDesc();
extern ClassDesc* GetDeflectModDesc();
extern ClassDesc* GetUVWMapModDesc();
extern ClassDesc* GetSelModDesc();
extern ClassDesc* GetSmoothModDesc();
extern ClassDesc* GetMatModDesc();
extern ClassDesc* GetNormalModDesc();
extern ClassDesc* GetSurfrevModDesc();
extern ClassDesc* GetResetXFormDesc();
extern ClassDesc* GetAFRModDesc();
extern ClassDesc* GetTessModDesc();
extern ClassDesc* GetDeleteModDesc();
extern ClassDesc* GetMeshSelModDesc();
extern ClassDesc* GetFaceExtrudeModDesc();
extern ClassDesc* GetUVWXFormModDesc();
extern ClassDesc* GetMirrorModDesc();
extern ClassDesc* GetUnwrapModDesc();
extern ClassDesc* GetBendWSMDesc();
extern ClassDesc* GetTwistWSMDesc();
extern ClassDesc* GetTaperWSMDesc();
extern ClassDesc* GetSkewWSMDesc();
extern ClassDesc* GetNoiseWSMDesc();
extern ClassDesc* GetDispApproxModDesc();
extern ClassDesc* GetMeshMesherWSMDesc();
extern ClassDesc* GetNormalizeSplineDesc();*/


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


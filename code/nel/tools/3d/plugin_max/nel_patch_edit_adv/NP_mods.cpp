/**********************************************************************
 *<
	FILE: mods.cpp

	DESCRIPTION:   DLL implementation of modifiers

	CREATED BY: Rolf Berteig (based on prim.cpp)

	HISTORY: created 30 January 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#include "stdafx.h"
#include "editpat.h"
#include "nel/misc/debug.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

using namespace NLMISC;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	hInstance = hinstDLL;

	if ( !controlsInit ) 
	{
		controlsInit = TRUE;
		
		// jaguar controls
		InitCustomControls(hInstance);

#ifdef OLD3DCONTROLS
		// initialize 3D controls
		Ctl3dRegister(hinstDLL);
		Ctl3dAutoSubclass(hinstDLL);
#endif
		
		// initialize Chicago controls
		InitCommonControls();
		}

	switch(fdwReason) {
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		}
	return(TRUE);
	}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() 
{ 
	return "Ryzom patch"; 
}


#ifndef DESIGN_VER

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 1;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetEditPatchModDesc();
			/*
		case 0: return GetBendModDesc();
		case 1: return GetTaperModDesc();
		case 2: return GetSinWaveObjDesc();
		case 3: return GetSinWaveModDesc();
		case 4: return GetEditMeshModDesc();
		case 5: return GetEditSplineModDesc();
		case 6: return GetEditPatchModDesc();
		case 7: return GetTwistModDesc();
		case 8: return GetExtrudeModDesc();
		case 9: return GetBombObjDesc();
		case 10: return GetBombModDesc();		
		case 11: return GetClustModDesc();
		case 12: return GetSkewModDesc();
		case 13: return GetNoiseModDesc();
		case 14: return GetSinWaveOModDesc();
		case 15: return GetLinWaveObjDesc();
		case 16: return GetLinWaveModDesc();
		case 17: return GetLinWaveOModDesc();
		case 18: return GetOptModDesc();
		case 19: return GetDispModDesc();
		case 20: return GetClustNodeModDesc();
		case 21: return GetGravityObjDesc();
		case 22: return GetGravityModDesc();
		case 23: return GetWindObjDesc();
		case 24: return GetWindModDesc();
		case 25: return GetDispObjDesc();
		case 26: return GetDispWSModDesc();
		case 27: return GetDeflectObjDesc();
		case 28: return GetDeflectModDesc();
		case 29: return GetUVWMapModDesc();
		case 30: return GetSelModDesc();
		case 31: return GetSmoothModDesc();
		case 32: return GetMatModDesc();
		case 33: return GetNormalModDesc();
		case 34: return GetSurfrevModDesc();
		case 35: return GetResetXFormDesc();
		case 36: return GetAFRModDesc();
		case 37: return GetTessModDesc();
		case 38: return GetDeleteModDesc();
		case 39: return GetMeshSelModDesc();
		case 40: return GetFaceExtrudeModDesc();
		case 41: return GetUVWXFormModDesc();
		case 42: return GetMirrorModDesc();
		case 43: return GetUnwrapModDesc();
		case 44: return GetBendWSMDesc();
		case 45: return GetTwistWSMDesc();
		case 46: return GetTaperWSMDesc();
		case 47: return GetSkewWSMDesc();
		case 48: return GetNoiseWSMDesc();
		case 49: return GetSDeleteModDesc();
		case 50: return GetDispApproxModDesc();
		case 51: return GetMeshMesherWSMDesc();
		case 52: return GetNormalizeSplineDesc();*/
		default: return 0;
		}

	}

#else

//
// DESIGN VERSION EXCLUDES SOME PLUG_INS
//

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 1;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetEditPatchModDesc();
			/*
		case 0: return GetBendModDesc();
		case 1: return GetTaperModDesc();
		//case 2: return GetSinWaveObjDesc();
		//case 3: return GetSinWaveModDesc();
		case 2: return GetEditMeshModDesc();
		case 3: return GetEditSplineModDesc();
		case 4: return GetEditPatchModDesc();
		case 5: return GetTwistModDesc();
		case 6: return GetExtrudeModDesc();
		//case 9: return GetBombObjDesc();
		//case 10: return GetBombModDesc();		
		case 7: return GetClustModDesc();
		case 8: return GetSkewModDesc();
		case 9: return GetNoiseModDesc();
		case 10: return GetSinWaveOModDesc();
		//case 15: return GetLinWaveObjDesc();
		//case 16: return GetLinWaveModDesc();
		case 11: return GetLinWaveOModDesc();
		case 12: return GetOptModDesc();
		case 13: return GetDispModDesc();
		case 14: return GetClustNodeModDesc();
		//case 21: return GetGravityObjDesc();
		//case 22: return GetGravityModDesc();
		//case 23: return GetWindObjDesc();
		//case 24: return GetWindModDesc();
		//case 25: return GetDispObjDesc();
		//case 26: return GetDispWSModDesc();
		//case 27: return GetDeflectObjDesc();
		//case 28: return GetDeflectModDesc();
		case 15: return GetUVWMapModDesc();
		case 16: return GetSelModDesc();
		case 17: return GetSmoothModDesc();
		case 18: return GetMatModDesc();
		case 19: return GetNormalModDesc();
		case 20: return GetSurfrevModDesc();
		case 21: return GetResetXFormDesc();
		case 22: return GetAFRModDesc();
		case 23: return GetTessModDesc();
		case 24: return GetDeleteModDesc();
		case 25: return GetMeshSelModDesc();
		case 26: return GetFaceExtrudeModDesc();
		case 27: return GetUVWXFormModDesc();
		case 28: return GetMirrorModDesc();
//		case 29: return GetUnwrapModDesc();
		case 29: return GetBendWSMDesc();
		case 30: return GetTwistWSMDesc();
		case 31: return GetTaperWSMDesc();
		case 32: return GetSkewWSMDesc();
		case 33: return GetNoiseWSMDesc();
		case 34: return GetSDeleteModDesc();
		case 35: return GetDispApproxModDesc();
		case 36: return GetMeshMesherWSMDesc();
		case 37: return GetNormalizeSplineDesc();
*/
		default: return 0;
		}

	}

#endif



// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() { return VERSION_3DSMAX; }

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

BOOL CALLBACK DefaultSOTProc(
		HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
	{
	IObjParam *ip = (IObjParam*)GetWindowLongPtr(hWnd,GWLP_USERDATA);

	switch (msg) {
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd,GWLP_USERDATA,lParam);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			if (ip) ip->RollupMouseMessage(hWnd,msg,wParam,lParam);
			return FALSE;

		default:
			return FALSE;
		}
	return TRUE;
	}

TCHAR *GetString(int id)
	{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
	}

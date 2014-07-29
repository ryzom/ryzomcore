#include "stdafx.h"
#include "nel_patch_paint.h"
#include "nel/misc/debug.h"
#include "nel/misc/app_context.h"
#include "../nel_3dsmax_shared/nel_3dsmax_shared.h"
#include <maxversion.h>
#include "nel/misc/sheet_id.h"

HINSTANCE hInstance;
int controlsInit = FALSE;

using namespace NLMISC;

/** public functions **/
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved) 
{
	// initialize nel context
	if (!NLMISC::INelContext::isContextInitialised())
	{
		new NLMISC::CLibraryContext(GetSharedNelContext());
		nldebug("NeL Patch Paint: DllMain");
		NLMISC::CSheetId::initWithoutSheet();
	}
			
	hInstance = hinstDLL;

	if ( !controlsInit ) 
	{
		controlsInit = TRUE;
		
		// jaguar controls
#if MAX_VERSION_MAJOR < 14
		InitCustomControls(hInstance);
#endif

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
	return "NeL patch painter"; 
}


#ifndef DESIGN_VER

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() {return 1;}

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) {
	switch(i) {
		case 0: return GetEditPatchModDesc();
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

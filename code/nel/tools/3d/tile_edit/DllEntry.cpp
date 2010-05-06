/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION:Contains the Dll Entry stuff

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "stdafx.h"
#include "tile_edit.h"

extern ClassDesc2* GetTile_editDesc();

HINSTANCE hInstance;
int controlsInit = FALSE;

// This function is called by Windows when the DLL is loaded.  This 
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


class CMfcdllApp : public CWinApp
{
public:
	CMfcdllApp();
	virtual BOOL InitInstance( );
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMfcdllApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CMfcdllApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CMfcdllApp

BEGIN_MESSAGE_MAP(CMfcdllApp, CWinApp)
	//{{AFX_MSG_MAP(CMfcdllApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMfcdllApp construction

CMfcdllApp::CMfcdllApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMfcdllApp object

CMfcdllApp theApp;

BOOL CMfcdllApp::InitInstance( )
{
	hInstance = m_hInstance;				// Hang on to this DLL's instance handle.

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
	}
			
	return (TRUE);
}

// This function returns a string that describes the DLL and where the user
// could purchase the DLL if they don't have it.
__declspec( dllexport ) const TCHAR* LibDescription()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	return GetString(IDS_LIBDESCRIPTION);
}

// This function returns the number of plug-in classes this DLL
//TODO: Must change this number when adding a new class
__declspec( dllexport ) int LibNumberClasses()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
	
	return 1;
}

// This function returns the number of plug-in classes this DLL
__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	switch(i) 
	{
		case 0: return GetTile_editDesc();
		default: return 0;
	}
}

// This function returns a pre-defined constant indicating the version of 
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.
__declspec( dllexport ) ULONG LibVersion()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


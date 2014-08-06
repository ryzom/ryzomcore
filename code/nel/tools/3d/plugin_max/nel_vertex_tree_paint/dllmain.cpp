#include "vertex_tree_paint.h"
#include "../nel_3dsmax_shared/nel_3dsmax_shared.h"
#include <maxversion.h>
#include "nel/misc/sheet_id.h"

HINSTANCE hInstance;


BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	// initialize nel context
	if (!NLMISC::INelContext::isContextInitialised())
	{
		new NLMISC::CLibraryContext(GetSharedNelContext());
		nldebug("NeL Vertex Tree Paint: DllMain");
		NLMISC::CSheetId::initWithoutSheet();
	}

	hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
#if MAX_VERSION_MAJOR < 14
			InitCustomControls(hInstance);	// Initialize MAX's custom controls
#endif
			InitCommonControls();			// Initialize Win95 controls
			break;
		}
			
	return (TRUE);
}

__declspec( dllexport ) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}


__declspec( dllexport ) int LibNumberClasses()
{
	return 1;
}

__declspec( dllexport ) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetVertexPaintDesc();
		default: return 0;
		}
}

__declspec( dllexport ) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

// Let the plug-in register itself for deferred loading
__declspec( dllexport ) ULONG CanAutoDefer()
{
	return 1;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


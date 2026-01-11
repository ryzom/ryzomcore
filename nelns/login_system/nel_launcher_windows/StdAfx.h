// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3553FEAD_1991_4F77_91FE_541702A9DEB5__INCLUDED_)
#define AFX_STDAFX_H__3553FEAD_1991_4F77_91FE_541702A9DEB5__INCLUDED_

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <Wininet.h>
#include <io.h>
#include <process.h>
#include <direct.h>
#include <sys/utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <nel/misc/types_nl.h>

#include <queue>
#include <string>

#include <zlib.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__3553FEAD_1991_4F77_91FE_541702A9DEB5__INCLUDED_)

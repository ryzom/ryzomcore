// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__75A839E8_589B_11D2_AB9F_C40300C10000__INCLUDED_)
#define AFX_STDAFX_H__75A839E8_589B_11D2_AB9F_C40300C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#if _MSC_VER >= 1200 // VC6
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#ifndef TB_SETEXTENDEDSTYLE
// You can download the platform SDK from microsoft's site:
// http://msdn.microsoft.com/developer/sdk/default.htm
#error CJ60Lib requires a newer version of the SDK than you have!
#endif

#include <afxpriv.h>
#include <afxtempl.h>
#include <..\src\afximpl.h>

#define AFX_IDW_SIZEBAR_LEFT	AFX_IDW_DOCKBAR_LEFT	+ 4
#define AFX_IDW_SIZEBAR_RIGHT	AFX_IDW_DOCKBAR_RIGHT	+ 5
#define AFX_IDW_SIZEBAR_TOP		AFX_IDW_DOCKBAR_TOP		+ 6
#define AFX_IDW_SIZEBAR_BOTTOM	AFX_IDW_DOCKBAR_BOTTOM	+ 7

const DWORD dwSizeBarMap[4][2] =
{
	{ AFX_IDW_SIZEBAR_TOP,      CBRS_TOP    },
	{ AFX_IDW_SIZEBAR_BOTTOM,   CBRS_BOTTOM },
	{ AFX_IDW_SIZEBAR_LEFT,     CBRS_LEFT   },
	{ AFX_IDW_SIZEBAR_RIGHT,    CBRS_RIGHT  },
};

#define safe_delete(p){if(p){delete p;p=NULL;}}

#include "Globals.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__75A839E8_589B_11D2_AB9F_C40300C10000__INCLUDED_)

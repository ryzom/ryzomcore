// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#if !defined(AFX_STDAFX_H__0B961212_9577_4F64_A497_B29F1567876C__INCLUDED_)
#define AFX_STDAFX_H__0B961212_9577_4F64_A497_B29F1567876C__INCLUDED_
#define _AFXDLL
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
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

#undef min
#undef max

#include <nel/misc/types_nl.h>

#include "resource.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0B961212_9577_4F64_A497_B29F1567876C__INCLUDED_)

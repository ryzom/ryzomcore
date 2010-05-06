// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#if !defined(AFX_STDAFX2_H__9B22CB86_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_STDAFX2_H__9B22CB86_1929_11D5_9CD4_0050DAC3A412__INCLUDED_

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

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


//#include "resource.h"

#include "nel/misc/common.h"
#include "nel/misc/stream.h"
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"

#include "3d/particle_system.h"
#include "3d/ps_located.h"
#include "3d/ps_particle.h"
#include "3d/ps_force.h"
#include "3d/ps_emitter.h"
#include "3d/ps_zone.h"


#include <vector>
#include <map>
#include <algorithm>

#include "animation_dlg.h"
#include "animation_set_dlg.h"
#include "main_dlg.h"
#include "object_viewer.h"
#include "object_viewer_interface.h"
#include "select_string.h"




//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__9B22CB86_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)


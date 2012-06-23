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

#if !defined(AFX_STDAFX_H__E96DCAA4_6BCD_45D1_8188_DEB4E925DDDA__INCLUDED_)
#define AFX_STDAFX_H__E96DCAA4_6BCD_45D1_8188_DEB4E925DDDA__INCLUDED_

#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#define _WIN32_WINNT 0x0500

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

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

// Windows
#include <process.h>

// MFC
#include <afxdlgs.h>
#include <afxcview.h>

// STL
#include <map>
#include <list>
#include <set>
#include <string>
#include <vector>

// NLMISC
#include <nel/misc/types_nl.h>
#include <nel/misc/bitmap.h>
#include <nel/misc/common.h>
#include <nel/misc/config_file.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/matrix.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/path.h>
#include <nel/misc/rgba.h>
#include <nel/misc/smart_ptr.h>
#include <nel/misc/uv.h>
#include <nel/misc/vector.h>
#include <nel/misc/progress_callback.h>

// NL3D
#include <nel/3d/quad_grid.h>
#include <nel/3d/driver.h>
#include <nel/3d/font_manager.h>
#include <nel/3d/init_3d.h>
#include <nel/3d/material.h>
#include <nel/3d/nelu.h>
#include <nel/3d/index_buffer.h>
#include <nel/3d/register_3d.h>
#include <nel/3d/scene.h>
#include <nel/3d/texture_file.h>
#include <nel/3d/texture_mem.h>
#include <nel/3d/texture_blank.h>
#include <nel/3d/text_context.h>
#include <nel/3d/vertex_buffer.h>
#include <nel/3d/landscape.h>

// NLGEORGES
#include <nel/georges/u_form.h>
#include <nel/georges/u_form_elm.h>
#include <nel/georges/u_form_loader.h>

// NLLIGO
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>

// NLPACS
#include <nel/pacs/u_global_retriever.h>
#include <nel/ligo/ligo_config.h>

#include <nel/ligo/zone_bank.h>
#include <nel/ligo/zone_region.h>

extern bool	DontUse3D;


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__E96DCAA4_6BCD_45D1_8188_DEB4E925DDDA__INCLUDED_)

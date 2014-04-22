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

// world_editor_graph_plugin.h : main header file for the world_editor_graph_plugin application
//

#if !defined(AFX_WORLDEDITORGRAPHPLUGIN_H__4CC93544_46D9_11D5_AE4A_0080ADB4DF70__INCLUDED_)
#define AFX_WORLDEDITORGRAPHPLUGIN_H__4CC93544_46D9_11D5_AE4A_0080ADB4DF70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorGraphPluginApp:
// See world_editor_graph_plugin.cpp for the implementation of this class
//

class CWorldEditorGraphPluginApp : public CWinApp
{
public:
	CWorldEditorGraphPluginApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditorGraphPluginApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWorldEditorGraphPluginApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDEDITORGRAPHPLUGIN_H__4CC93544_46D9_11D5_AE4A_0080ADB4DF70__INCLUDED_)

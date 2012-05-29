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

// world_editor.h : main header file for the WORLD_EDITOR application
//

#if !defined(AFX_WORLD_EDITOR_H__534BE790_9E70_4D87_B709_1445991D5824__INCLUDED_)
#define AFX_WORLD_EDITOR_H__534BE790_9E70_4D87_B709_1445991D5824__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include <nel/ligo/primitive_class.h>
#include "imagelist_ex.h"
#include "plugin_interface.h"
//#include "color_edit_wnd.h"
#include "nel/ligo/ligo_config.h"

// Base registry key
#define BASE_REGISTRY_KEY "Software\\Nevrax\\NeL World Editor"

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp:
// See world_editor.cpp for the implementation of this class
//

class CMyLigoConfig : public NLLIGO::CLigoConfig
{
	virtual void errorMessage (const char *format, ... );
};

class CWorldEditorApp : public CWinApp
{
	friend class CProjectSettings;
public:
	CWorldEditorApp();
	~CWorldEditorApp();

	// Helpers
	bool		yesNoMessage (const char *format, ... );
	void		errorMessage (const char *format, ... );
	void		infoMessage (const char *format, ... );
	void		syntaxError (const char *filename, xmlNodePtr, const char *format, ...);
	bool		getPropertyString (std::string &result, const char *filename, xmlNodePtr xmlNode, const char *propName);

	// The image list
	CImageListEx	ImageList;

	// Exe path
	std::string		ExePath;

	// Doc path (for node class html help file)
	std::string		DocPath;

	// Load any extension plugin
	virtual void loadPlugins();

	/// Configuration file for plugins
	NLMISC::CConfigFile				PluginConfig;

	/// Vector of loaded plugins.
	std::vector<IPluginCallback*>	Plugins;

	/// list of primitives associated to plugin displayer
	std::map<std::string, IPrimitiveDisplayer*>	PrimitiveDisplayers;

	/// The ligo config
	CMyLigoConfig					Config;

	/// Get the first active configuration of a primitive. Return -1 else.
	sint	getActiveConfiguration (const NLLIGO::IPrimitive &primitive, uint searchBegin = 0) const;

	/// Reload the configuration file
	bool reloadPrimitiveConfiguration ();

	/// Configuration
	class CConfiguration
	{
	public:
		CConfiguration()
		{
			Activated = false;
		}

		bool	Activated;
	};
	std::vector<CConfiguration>		Configurations;

	

private:

	// Read the path config file
	bool initPath (const char *filename, class CSplashScreen &splashScreen);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldEditorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CWorldEditorApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// for main frame window only
	void deletePlugins();
};

extern CWorldEditorApp theApp;
	class CMainFrame *getMainFrame ();
void invalidateLeftView ();
std::string standardizePath (const char *str);
std::string formatString (const char *str);
inline void transformVector (NLMISC::CVector &toTransform, float angle, const NLMISC::CVector &pivot)
{
	float cosa = (float)cos (angle);
	float sina = (float)sin (angle);
	toTransform -= pivot;
	float temp = cosa * toTransform.x - sina * toTransform.y;
	toTransform.y = sina * toTransform.x + cosa * toTransform.y;
	toTransform.x = temp;
	toTransform += pivot;
}
std::string numberize (const char *oldString, uint value);
bool getZoneNameFromXY (sint32 x, sint32 y, std::string &zoneName);
bool openFile (const char *url);
uint getRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);
void setEditTextMultiLine (CEdit &edit, const char *text);
void setEditTextMultiLine (CEdit &edit, const std::vector<std::string> &vect);
//void setEditTextMultiLine (ColorEditWnd &edit, const char *text);
//void setEditTextMultiLine (ColorEditWnd &edit, const std::vector<std::string> &vect);
//void setEditTextMultiLine (CListBox &listBox, const char *text);
void setEditTextMultiLine (CListBox &listBox, const std::vector<std::string> &vect);
bool setWindowTextUTF8 (HWND hwnd, const char *textUtf8);
bool getWindowTextUTF8 (HWND hwnd, CString &textUtf8);
HTREEITEM insertItemUTF8 (HWND hwnd, const char *textUtf8, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
HTREEITEM insertItemUTF8 (HWND hwnd, const char *textUtf8, int nImage, int nSelectedImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hInsertAfter = TVI_LAST);
bool setItemTextUTF8 ( HWND hwnd, HTREEITEM hItem, LPCTSTR lpszItem );
bool isPrimitiveVisible (const NLLIGO::IPrimitive *primitive);

std::string getTextureFile(const std::string &filename);

// ***************************************************************************

// Class to enable / disable interaction
class CNoInteraction
{
public:
	CNoInteraction ();
	~CNoInteraction ();
};
	

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLD_EDITOR_H__534BE790_9E70_4D87_B709_1445991D5824__INCLUDED_)

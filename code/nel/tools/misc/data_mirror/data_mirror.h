// data_mirror.h : main header file for the DATA_MIRROR application
//

#if !defined(AFX_DATA_MIRROR_H__A4CAF592_6AED_44B9_870E_1EB47C6AA8CE__INCLUDED_)
#define AFX_DATA_MIRROR_H__A4CAF592_6AED_44B9_870E_1EB47C6AA8CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CData_mirrorApp:
// See data_mirror.cpp for the implementation of this class
//

class CData_mirrorApp : public CWinApp
{
public:
	CData_mirrorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CData_mirrorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CData_mirrorApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// From the config file
extern std::string				MainDirectory;
extern std::string				MirrorDirectory;
extern std::string				LogDirectory;
extern std::string				IgnoreDirectory;
extern std::string				CurrentDir;
extern std::set<std::string>	IgnoreFiles;
extern bool						BinaryCompare;

bool RegisterDirectoryAppCommand (const char *appName, const char *command, const char *app);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DATA_MIRROR_H__A4CAF592_6AED_44B9_870E_1EB47C6AA8CE__INCLUDED_)

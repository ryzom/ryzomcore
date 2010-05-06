// tile_edit_exe.h : main header file for the TILE_EDIT_EXE application
//

#if !defined(AFX_TILE_EDIT_EXE_H__254ECFC3_F100_41C1_8D5E_BA741F24687D__INCLUDED_)
#define AFX_TILE_EDIT_EXE_H__254ECFC3_F100_41C1_8D5E_BA741F24687D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTile_edit_exeApp:
// See tile_edit_exe.cpp for the implementation of this class
//

class CTile_edit_exeApp : public CWinApp
{
public:
	CTile_edit_exeApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTile_edit_exeApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTile_edit_exeApp)
	afx_msg void OnSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TILE_EDIT_EXE_H__254ECFC3_F100_41C1_8D5E_BA741F24687D__INCLUDED_)

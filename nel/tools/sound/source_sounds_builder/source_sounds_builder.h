// source_sounds_builder.h : main header file for the SOURCE_SOUNDS_BUILDER application
//

#if !defined(AFX_SOURCE_SOUNDS_BUILDER_H__BFC2A702_D6CA_46F2_BCC5_E46A78437CCA__INCLUDED_)
#define AFX_SOURCE_SOUNDS_BUILDER_H__BFC2A702_D6CA_46F2_BCC5_E46A78437CCA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSource_sounds_builderApp:
// See source_sounds_builder.cpp for the implementation of this class
//

class CSource_sounds_builderApp : public CWinApp
{
public:
	CSource_sounds_builderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSource_sounds_builderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSource_sounds_builderApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOURCE_SOUNDS_BUILDER_H__BFC2A702_D6CA_46F2_BCC5_E46A78437CCA__INCLUDED_)

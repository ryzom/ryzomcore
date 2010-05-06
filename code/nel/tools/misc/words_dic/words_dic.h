// words_dic.h : main header file for the WORDS_DIC application
//

#if !defined(AFX_WORDS_DIC_H__6715B68F_151F_41A0_9A34_A06A9093983D__INCLUDED_)
#define AFX_WORDS_DIC_H__6715B68F_151F_41A0_9A34_A06A9093983D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWords_dicApp:
// See words_dic.cpp for the implementation of this class
//

class CWords_dicApp : public CWinApp
{
public:
	CWords_dicApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWords_dicApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWords_dicApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORDS_DIC_H__6715B68F_151F_41A0_9A34_A06A9093983D__INCLUDED_)

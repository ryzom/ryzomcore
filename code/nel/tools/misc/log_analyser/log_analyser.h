// log_analyser.h : main header file for the LOG_ANALYSER application
//

#if !defined(AFX_LOG_ANALYSER_H__58E99147_C539_4AD8_B80B_A4B11BBF60D1__INCLUDED_)
#define AFX_LOG_ANALYSER_H__58E99147_C539_4AD8_B80B_A4B11BBF60D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CLog_analyserApp:
// See log_analyser.cpp for the implementation of this class
//

class CLog_analyserApp : public CWinApp
{
public:
	CLog_analyserApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLog_analyserApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLog_analyserApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOG_ANALYSER_H__58E99147_C539_4AD8_B80B_A4B11BBF60D1__INCLUDED_)

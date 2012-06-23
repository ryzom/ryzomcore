// logic.h : main header file for the LOGIC DLL
//

#if !defined(AFX_LOGIC_H__AACA5CA3_6F95_4278_9DF5_6ED0839DBBDC__INCLUDED_)
#define AFX_LOGIC_H__AACA5CA3_6F95_4278_9DF5_6ED0839DBBDC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "logic_editor_interface.h"

/////////////////////////////////////////////////////////////////////////////
// CLogic_editorApp
// See logic.cpp for the implementation of this class
//

class CLogic_editorApp : public CWinApp
{

	CMultiDocTemplate* _DocTemplate;

public:
	CLogic_editorApp();
	void newDoc ();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogic_editorApp)
	public:
	//virtual BOOL InitInstance();
	//}}AFX_VIRTUAL
	virtual BOOL initInstance (int x=0, int y=0, int cx=0, int cy=0);

// Implementation
	//{{AFX_MSG(CLogic_editorApp)
	//afx_msg void OnAppAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};




/**
 *	CLogicEditor
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CLogicEditor : public ILogicEditor
{

public:

	/**
	 *	CLogicEditor 
	 */
	CLogicEditor();	

	/**
	 *	Init the UI
	 */
	void initUI (HWND parent=NULL);

	/**
	 *	Init the UI Light version
	 */
	virtual void initUILight (int x, int y, int cx, int cy);

	/**
	 *	Go
	 */
	void go ();

	/**
	 *	Get the main frame
	 */
	virtual void*getMainFrame ();

	/**
	 *	load file
	 */
	virtual void loadFile( const char * fileName );

	/**
	 * create a default file
	 */
	virtual void createDefaultFile( const char * filename = "logic.logic ");
	

	/**
	 *	Release the UI
	 */
	void releaseUI ();
};




/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGIC_H__AACA5CA3_6F95_4278_9DF5_6ED0839DBBDC__INCLUDED_)

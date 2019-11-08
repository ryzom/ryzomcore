#if !defined(AFX_RICH_EDIT_CTRL_EX_H__92FAEFC4_7921_42E9_89FC_BA9B1A5710F8__INCLUDED_)
#define AFX_RICH_EDIT_CTRL_EX_H__92FAEFC4_7921_42E9_89FC_BA9B1A5710F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// rich_edit_ctrl_ex.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRichEditCtrlEx window

class CRichEditCtrlEx : public CRichEditCtrl
{
// Construction
public:
	CRichEditCtrlEx();

// Attributes
public:

// Operations
public:


	// set font to be applied to the next 'append' operation
	void setFont(LONG size, const char *fontName, DWORD effects);
	// Append a new ucstring to the rich edit ctrl, using the last settings of 'setfont'
	void append(const ucstring &str);	
	void clear();



// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRichEditCtrlEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRichEditCtrlEx();

	CHARFORMAT _CharFormat;


	// Generated message map functions
protected:
	//{{AFX_MSG(CRichEditCtrlEx)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RICH_EDIT_CTRL_EX_H__92FAEFC4_7921_42E9_89FC_BA9B1A5710F8__INCLUDED_)

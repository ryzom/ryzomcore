#if !defined(AFX_CHOOSE_FRAME_DELAY_H__F07A2A7E_FB96_4856_99B5_46DE3B05CD6D__INCLUDED_)
#define AFX_CHOOSE_FRAME_DELAY_H__F07A2A7E_FB96_4856_99B5_46DE3B05CD6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// choose_frame_delay.h : header file
//


#include "editable_range.h"
#include "object_viewer.h"
#include "particle_workspace.h"



/////////////////////////////////////////////////////////////////////////////
// CChooseLag dialog

class CChooseFrameDelay : public CDialog, public CObjectViewer::IMainLoopCallBack
{
// Construction
public:
	CChooseFrameDelay(CObjectViewer *objectViewer, CWnd* pParent);   // standard constructor
	~CChooseFrameDelay();


	// Lock frame delay to current particle system
	void lockToPS(bool lock);
	bool isLockedToPS() const { return _LockToPS; }


// Dialog Data
	//{{AFX_DATA(CChooseLag)
	enum { IDD = IDD_CHOOSE_FRAME_DELAY };	
	//}}A FX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseLag)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:	
	bool				_LockToPS;
	CEditableRangeUInt  *_ER;
	// Generated message map functions
	//{{AFX_MSG(CChooseFrameDelay)	
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void updateValueFromText();
	void updateTextFromValue(uint value);
	struct CCFDWrapper : public IPSWrapperUInt
	{
		CObjectViewer *OV;		
		uint32 get(void) const;
		virtual void set(const uint32 &value);
	} _CFDWrapper;
	// From CObjectViewer::IMainLoopCallBack
	virtual void goPreRender();
	virtual void goPostRender() {}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSE_FRAME_DELAY_H__F07A2A7E_FB96_4856_99B5_46DE3B05CD6D__INCLUDED_)

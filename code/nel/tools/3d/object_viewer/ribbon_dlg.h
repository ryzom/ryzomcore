#if !defined(AFX_RIBBON_DLG_H__6BE29749_979B_4066_9CA2_6EDDDBDADA9C__INCLUDED_)
#define AFX_RIBBON_DLG_H__6BE29749_979B_4066_9CA2_6EDDDBDADA9C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "ps_wrapper.h"

namespace NL3D
{
	class CPSRibbonBase;
}

#include "editable_range.h"
#include "particle_workspace.h"

class CRibbonDlg : public CDialog
{
// Construction
public:
	CRibbonDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSRibbonBase *ribbon, CWnd* pParent = NULL);   // standard constructor
	~CRibbonDlg();
	void init(CWnd *pParent, sint x, sint y);	

// Dialog Data
	//{{AFX_DATA(CRibbonDlg)
	enum { IDD = IDD_RIBBON_DLG };
	BOOL	m_UseHermitteInterpolation;
	BOOL	m_ConstantLength;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRibbonDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CParticleWorkspace::CNode *_Node;
	NL3D::CPSRibbonBase		  *_Ribbon; // the ribbon being edited
	CEditableRangeFloat		  *_RibbonLengthDlg;
	CEditableRangeFloat		  *_LODDegradationDlg;


	// Generated message map functions
	//{{AFX_MSG(CRibbonDlg)
	afx_msg void OnUseHermitteInterpolation();
	afx_msg void OnConstantLength();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTrailCoordSystem();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	struct CRibbonLengthWrapper : IPSWrapperFloat
	{
		NL3D::CPSRibbonBase *R;
		float get() const;
		void  set(const float &v);
	} _RibbonLengthWrapper;


	struct CLODDegradationWrapper : IPSWrapperFloat
	{
		NL3D::CPSRibbonBase *R;
		float get() const;
		void  set(const float &v);
	} _LODDegradationWrapper;

	void updateState();
	void updateModifiedFlag() { if (_Node) _Node->setModified(true); }
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RIBBON_DLG_H__6BE29749_979B_4066_9CA2_6EDDDBDADA9C__INCLUDED_)

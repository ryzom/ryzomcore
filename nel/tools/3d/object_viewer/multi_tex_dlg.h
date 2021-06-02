// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


//

#if !defined(AFX_MULTI_TEX_DLG_H__5D160C02_0FF4_4DF3_A804_90CE96C4F448__INCLUDED_)
#define AFX_MULTI_TEX_DLG_H__5D160C02_0FF4_4DF3_A804_90CE96C4F448__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "ps_wrapper.h"
#include "particle_workspace.h"

namespace NL3D
{
	class CPSMultiTexturedParticle;
}

class CTextureChooser;
struct IPopupNotify;


/////////////////////////////////////////////////////////////////////////////
// CMultiTexDlg dialog

class CMultiTexDlg : public CDialog
{
// Construction
public:
	CMultiTexDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSMultiTexturedParticle *mtp, IPopupNotify* pn, CWnd *pParent);   // standard constructor
	~CMultiTexDlg();

	/// create and show this dialog
	void init(CWnd *pParent);
// Dialog Data
	//{{AFX_DATA(CMultiTexDlg)
	enum { IDD = IDD_MULTITEX };
	CButton	m_UseParticleDateAltCtrl;
	CButton	m_UseParticleDateCtrl;
	CButton	m_ForceBasicCapsCtrl;
	CButton	m_AltTexCtrl;
	CComboBox	m_AlternateOpCtrl;
	CComboBox	m_MainOpCtrl;
	BOOL	m_ForceBasicCaps;
	BOOL	m_UseParticleDateAlt;
	BOOL	m_UseParticleDate;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiTexDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IPopupNotify	*_PN;
	// the multitextured particle being edited
	NL3D::CPSMultiTexturedParticle *_MTP;
	CParticleWorkspace::CNode	   *_Node;

	// Generated message map functions
	//{{AFX_MSG(CMultiTexDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnEnableAlternate();
	afx_msg void OnUpdateSpeed();
	afx_msg void OnUpdateSpeedAlternate();
	afx_msg void OnSelchangeAlternateOp();
	afx_msg void OnSelchangeMainOp();
	afx_msg void OnForceBasicCaps();
	afx_msg void OnUseParticleDate();
	afx_msg void OnUseParticleDateAlt();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()	
	// texture wrappers
	struct CMainTexWrapper : IPSWrapperTexture
	{
		NL3D::CPSMultiTexturedParticle *MTP;
		virtual NL3D::ITexture *get(void);
		virtual void set(NL3D::ITexture *);
	} _TexWrapper;

	struct CAlternateTexWrapper : IPSWrapperTexture
	{
		NL3D::CPSMultiTexturedParticle *MTP;
		virtual NL3D::ITexture *get(void);
		virtual void set(NL3D::ITexture *);
	} _AlternateTexWrapper;
	CTextureChooser *_MainTexDlg, *_AltTexDlg;
	void readValues(bool alternate); // get the values from the particle
	void writeValues(bool alternate);	
	void updateAlternate();
	void updateTexOp();
	void updateBumpFactorEnabled();
	void updateModifiedFlag() { if (_Node) _Node->setModified(true); }	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTI_TEX_DLG_H__5D160C02_0FF4_4DF3_A804_90CE96C4F448__INCLUDED_)

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

#if !defined(AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_)
#define AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// texture_chooser.h : header file
//


#include "edit_attrib_dlg.h"
#include "popup_notify.h"
#include "ps_wrapper.h"
#include "particle_workspace.h"
//
#include "nel/misc/smart_ptr.h"
//
#include "nel/3d/texture.h"

using NLMISC::CSmartPtr ;

namespace NL3D
{
	class CPSMultiTexturedParticle;
}

class CMultiTexDlg;


/////////////////////////////////////////////////////////////////////////////
// CTextureChooser dialog

class CTextureChooser : public CEditAttribDlg, IPopupNotify
{
// Construction
public:
	// construct the object with the given texture
	CTextureChooser(NL3D::CPSMultiTexturedParticle *mtp, CParticleWorkspace::CNode *ownerNode);   // standard constructor

	~CTextureChooser();

	/// when initing, you can also provide a point to a mutltitextured particle
	virtual void init(uint32 x, uint32 y, CWnd *pParent = NULL) ;
	
	BOOL EnableWindow( BOOL bEnable);

	// set a wrapper to get the datas
	void setWrapper(IPSWrapperTexture *wrapper) { _Wrapper = wrapper ; }


	/// enable to remove texture. the default is false
	void enableRemoveButton(void) { _EnableRemoveButton = true ; }
// Dialog Data
	//{{AFX_DATA(CTextureChooser)
	enum { IDD = IDD_TEXTURE_CHOOSER };
	CButton	m_MultiTexCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureChooser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool _EnableRemoveButton;
	CParticleWorkspace::CNode *_Node;
	IPSWrapperTexture *_Wrapper ;
	NL3D::CPSMultiTexturedParticle *_MTP;
	CMultiTexDlg	  *_MultiTexDlg;
	// handle to the current bitmap being displayed
	HBITMAP _CurrBitmap ;

	// update the current bitmap
	void textureToBitmap() ;

	// the current texture
	CSmartPtr<NL3D::ITexture> _Texture ;

	// Generated message map functions
	//{{AFX_MSG(CTextureChooser)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnRemoveTexture();
	afx_msg void OnEditMultitexturing();
	afx_msg void OnEnableMultitexturing();
	afx_msg void OnBrowseTexture();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP() ;

	/// inherited from IPopupNotify
	void childPopupClosed(CWnd *child);

	void updateMultiTexCtrl();
	
} ;




//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURE_CHOOSER_H__FE10F78E_0B69_4EB0_8FC7_A48FAEB904FD__INCLUDED_)

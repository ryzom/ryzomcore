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

#if !defined(AFX_TEXTURE_ANIM_DLG_H__4A689FB0_93B9_4F5A_8075_8006D8FD19B2__INCLUDED_)
#define AFX_TEXTURE_ANIM_DLG_H__4A689FB0_93B9_4F5A_8075_8006D8FD19B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "ps_wrapper.h"
#include "nel/3d/ps_particle.h"
#include "value_gradient_dlg.h"
#include "popup_notify.h"
#include "particle_workspace.h"

namespace NL3D
{
	class CPSTexturedParticle;
	class CPSMultiTexturedParticle;

}

class CTextureChooser;
class CAttribDlgInt;
class CValueGradientDlg;
class CMultiTexDlg;

/////////////////////////////////////////////////////////////////////////////
// CTextureAnimDlg dialog

class CTextureAnimDlg : public CDialog, IPopupNotify
{
// Construction
public:
	CTextureAnimDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSTexturedParticle *p, NL3D::CPSMultiTexturedParticle *mtp = NULL);   // standard constructor
	~CTextureAnimDlg();

	void init(sint x, sint y, CWnd *pParent);
// Dialog Data
	//{{AFX_DATA(CTextureAnimDlg)
	enum { IDD = IDD_TEXTURE_ANIM };
	CButton	m_ChooseTextures;
	BOOL	m_EnableTextureAnim;
	BOOL	m_MultiTexEnable;
	//}}AFX_DATA


	BOOL EnableWindow( BOOL bEnable);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureAnimDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NL3D::CPSTexturedParticle *_EditedParticle;
	NL3D::CPSMultiTexturedParticle *_MTP;
	CParticleWorkspace::CNode	   *_Node;		
	// dialog to choose a constant texture
	CTextureChooser *_TextureChooser;
	// dialog to have a theme or constant value for texture id
	CAttribDlgInt *_TextureIndexDialog;	
	CMultiTexDlg *_MultiTexDlg;
	// Generated message map functions
	//{{AFX_MSG(CTextureAnimDlg)
	afx_msg void OnChooseTextures();
	afx_msg void OnEnableTextureAnim();
	afx_msg void OnMultiTex();
	afx_msg void OnEditMultitex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    // delete created controls
	void cleanCtrl(void);
	// create the right controls, depending on the fact that a an,imated texture is used or not
	void setupCtrl(void);
	//////////////////////////////////
	//   wrapper for single texture //
	//////////////////////////////////
			
		struct CTextureWrapper : public IPSWrapperTexture
		{
			NL3D::CPSTexturedParticle *P;
			NL3D::ITexture *get(void)  { return P->getTexture(); }
			void set(NL3D::ITexture *t) { P->setTexture(t); }
		} _TextureWrapper;

	///////////////////////////////////////
	// wrapper for texture anim sequence //
	///////////////////////////////////////

		struct CTextureIndexWrapper : public IPSWrapper<sint32>, IPSSchemeWrapper<sint32>
		{
		   NL3D::CPSTexturedParticle *P;		   
		   sint32 get(void) const { return P->getTextureIndex(); }
		   void set(const sint32 &v) { P->setTextureIndex(v); }
		   scheme_type *getScheme(void) const { return P->getTextureIndexScheme(); }
		   void setScheme(scheme_type *s) { P->setTextureIndexScheme(s); }
		} _TextureIndexWrapper;

	/**
	 * the implementation of this struct tells the gradient dialog bow how to edit a texture list
	 */

	struct CGradientInterface : public IValueGradientDlgClient
	{	
		// the particle being edited
		CValueGradientDlg *Dlg;
		NL3D::CPSTexturedParticle *P;	
		// all method inherited from IValueGradientDlgClient
		virtual CEditAttribDlg *createDialog(uint index, CValueGradientDlg *grad, CParticleWorkspace::CNode *ownerNode);
		virtual void modifyGradient(TAction, uint index);	
		virtual void displayValue(CDC *dc, uint index, sint x, sint y);	
		virtual uint32 getSchemeSize(void) const;	
		virtual uint32 getNbSteps(void) const;	
		virtual void setNbSteps(uint32 value);

		/////////////////////////////////////////////////
		// wrapper for the texture chooser             //
		// that allows to choose a texture in the list //
		/////////////////////////////////////////////////

		struct CTextureWrapper : public IPSWrapperTexture
		{
			CValueGradientDlg *Dlg;
			NL3D::CPSTexturedParticle *P;			
			// index of the particle in the list
			uint32 Index;
			NL3D::ITexture *get(void);
			void set(NL3D::ITexture *t);
		} _TextureWrapper;

	} _GradientInterface;


	/// inherited from IPopuNotify
	void childPopupClosed(CWnd *child);
	void updateModifiedFlag() { if (_Node) _Node->setModified(true); }
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTURE_ANIM_DLG_H__4A689FB0_93B9_4F5A_8075_8006D8FD19B2__INCLUDED_)

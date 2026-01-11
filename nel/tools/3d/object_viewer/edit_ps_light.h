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

#if !defined(AFX_EDIT_PS_LIGHT_H__6F00004D_F2A5_452E_9FC2_8188F9164613__INCLUDED_)
#define AFX_EDIT_PS_LIGHT_H__6F00004D_F2A5_452E_9FC2_8188F9164613__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// edit_ps_light.h : header file
//
#include "nel/3d/ps_light.h"
//
#include "ps_wrapper.h"
#include "particle_workspace.h"



class CAttribDlgFloat;
class CAttribDlgRGBA;


/* Edition of dynamic lights attributes in a particle system
 *
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2003
 */
class CEditPSLight : public CDialog
{
// Construction
public:
	CEditPSLight(CParticleWorkspace::CNode *ownerNode, NL3D::CPSLight *light);   // standard constructor
	~CEditPSLight();

	void init(CWnd* pParent = NULL);
// Dialog Data
	//{{AFX_DATA(CEditPSLight)
	enum { IDD = IDD_PS_LIGHT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditPSLight)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CParticleWorkspace::CNode *_Node;
	NL3D::CPSLight			  *_Light;
	CAttribDlgRGBA            *_ColorDlg;			
	CAttribDlgFloat           *_AttenStartDlg;	
	CAttribDlgFloat		      *_AttenEndDlg;
	// Generated message map functions
	//{{AFX_MSG(CEditPSLight)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	///////////////////////////////////////
	// wrapper to set the color of light //
	///////////////////////////////////////
	struct CColorWrapper : public IPSWrapperRGBA, IPSSchemeWrapperRGBA
	{
		NL3D::CPSLight *L;
		NLMISC::CRGBA get(void) const { return L->getColor(); }
		void set(const NLMISC::CRGBA &v) { L->setColor(v); }
		scheme_type *getScheme(void) const { return L->getColorScheme(); }
		void setScheme(scheme_type *s) { L->setColorScheme(s); }
	} _ColorWrapper;
	///////////////////////////////////////
	// wrapper to set start atten radius //
	///////////////////////////////////////
	struct CAttenStartWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSLight *L;
		float get(void) const { return L->getAttenStart(); }
		void set(const float &v) { L->setAttenStart(v); }
		scheme_type *getScheme(void) const { return L->getAttenStartScheme(); }
		void setScheme(scheme_type *s) { L->setAttenStartScheme(s); }
	} _AttenStartWrapper;
	///////////////////////////////////////
	// wrapper to set end atten radius //
	///////////////////////////////////////
	struct CAttenEndWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSLight *L;
		float get(void) const { return L->getAttenEnd(); }
		void set(const float &v) { L->setAttenEnd(v); }
		scheme_type *getScheme(void) const { return L->getAttenEndScheme(); }
		void setScheme(scheme_type *s) { L->setAttenEndScheme(s); }
	} _AttenEndWrapper;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_PS_LIGHT_H__6F00004D_F2A5_452E_9FC2_8188F9164613__INCLUDED_)

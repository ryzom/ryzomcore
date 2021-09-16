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


#if !defined(AFX_EDIT_FOLLOW_PATH_H__80C5C9BA_85D1_48C5_8D8F_2D5CDE5FFB78__INCLUDED_)
#define AFX_EDIT_FOLLOW_PATH_H__80C5C9BA_85D1_48C5_8D8F_2D5CDE5FFB78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// edit_follow_path.h : header file
//

#include "particle_workspace.h"

struct IPopupNotify;

namespace NL3D
{
	class CPSPlaneBasisFollowSpeed;
}


/** Dialog to edit the parameters of the 'Follow Path' modifier
  * 
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CEditFollowPath : public CDialog
{
// Construction
public:
	CEditFollowPath(NL3D::CPSPlaneBasisFollowSpeed *pbfs, CParticleWorkspace::CNode *ownerNode, CWnd* pParent, IPopupNotify *pn);   // standard constructor

	/// init this dialog
	void init(CWnd *pParent);
// Dialog Data
	//{{AFX_DATA(CEditFollowPath)
	enum { IDD = IDD_FOLLOWPATH };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditFollowPath)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NL3D::CPSPlaneBasisFollowSpeed *_FollowPath;
	IPopupNotify				   *_PN;
	CParticleWorkspace::CNode	   *_Node;
	// Generated message map functions
	//{{AFX_MSG(CEditFollowPath)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnSelchangeProjectionMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_FOLLOW_PATH_H__80C5C9BA_85D1_48C5_8D8F_2D5CDE5FFB78__INCLUDED_)

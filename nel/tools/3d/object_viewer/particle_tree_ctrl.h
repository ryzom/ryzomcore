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


#if !defined(AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_)
#define AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif


#include "nel/misc/matrix.h"
//
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_edit.h"
#include "nel/3d/ps_located.h"
//
#include "particle_workspace.h"
//
#include <algorithm>
#include <memory>

class CParticleDlg;
class CParticleWorkspace;


namespace NL3D
{
	class CParticleSystem;
	class CParticleSystemModel;
}




/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl window

class CParticleTreeCtrl : public CTreeCtrl, public CParticleWorkspace::IModificationCallback
{

// Construction
public:
	CParticleTreeCtrl(CParticleDlg *);
	virtual ~CParticleTreeCtrl();
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParticleTreeCtrl)	
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

	// build the whole tree from a workspace
	void					buildTreeFromWorkSpace(CParticleWorkspace &ws);
	/** build a portion of the tree using the given particle system
	  * \return root of the built tree
      */
	HTREEITEM				buildTreeFromPS(CParticleWorkspace::CNode &node, HTREEITEM rootHandle, HTREEITEM prevSibling = TVI_LAST);
	/// Add a node from the given lcoated
	void					createNodeFromLocated(NL3D::CPSLocated *loc, HTREEITEM rootHandle);
	/// Add a node from the given located bindable
	void					createNodeFromLocatedBindable(NL3D::CPSLocatedBindable *lb, HTREEITEM rootHandle);
	// rebuild the located instance in the tree (after loading for example)
	void					rebuildLocatedInstance(CParticleWorkspace::CNode &node);  
	/// suppress located instance item, so that they don't have higher index than the new size
	void					suppressLocatedInstanceNbItem(CParticleWorkspace::CNode &node, uint32 newSize);
	//
	void					init(void);
	// move the current element by using the given matrix
	void					moveElement(const NLMISC::CMatrix &mat);
	// get the matrix of the current element being selected, or identity if there's none
	NLMISC::CMatrix			getElementMatrix(void) const;
	// reset the list of node in the tree (but don't delete the tree)
	void					reset();
	//
	CParticleDlg		   *getParticleDlg() const { return _ParticleDlg; }
protected:
	//{{AFX_MSG(CParticleTreeCtrl)
	afx_msg void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);	
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	CImageList _ImageList;  // the image list containing the icons


	// the dialog that contain us
	CParticleDlg *_ParticleDlg;

public:
	/** this struct is used to identify the type of each node	
	 */
	struct CNodeType
	{	
		enum { located, particleSystem, locatedBindable, locatedInstance, workspace  } Type;
		union
		{
			CParticleWorkspace		  *WS;
			NL3D::CPSLocated		  *Loc;
			NL3D::CPSLocatedBindable  *Bind;
			CParticleWorkspace::CNode *PS;
		};		
		// for the located instance type, this is the index of the instance
		uint32 LocatedInstanceIndex;		
		// build node for a workspace
		CNodeType(CParticleWorkspace *ws) { nlassert(ws); WS = ws; Type = workspace; }		
		// build node for a located
		CNodeType(NL3D::CPSLocated *loc) { nlassert(loc); Loc = loc; Type = located; }
		// build node for an instance of a located
		CNodeType(NL3D::CPSLocated *loc, uint32 index) 
		{ 
			nlassert(loc);
			Loc = loc; 
			Type = locatedInstance; 
			LocatedInstanceIndex = index; 				
		}
		CNodeType(CParticleWorkspace::CNode *node) 
		{ 				
			PS = node;
			Type = particleSystem;
		}
		CNodeType(NL3D::CPSLocatedBindable *lb) { Bind = lb; Type = locatedBindable; }

		// Get the ps that owns that node (or NULL if it is a worspace)
		NL3D::CParticleSystem *getOwnerPS() const;
	};

	void setViewFilenameFlag(bool enabled);
	bool getViewFilenameFlag() const {	return _ViewFilenameFlag; }	
	// Update caption of a node
	void		updateCaption(CParticleWorkspace::CNode &node);
private:
	// instanciate a located in the given system , and return its nodetype and htreeitem
	std::pair<CNodeType *, HTREEITEM> createLocated(NL3D::CParticleSystem *ps, HTREEITEM headItem);
	// Compute caption to display for a particle system
	std::string computeCaption(CParticleWorkspace::CNode &node);
	// Compute caption to display for a workspace
	std::string computeCaption(CParticleWorkspace &workspace);
	// Compute a node caption from its filename, username & modified state
	std::string computeCaption(const std::string &path, const std::string &userName, bool modified);
	
	// Allow user to insert multiple PS in the workspace (prompt a file dialog to chose them)
	void insertNewPS(CParticleWorkspace &pws);
	// allow user to create a new particle system in the workspace
	void createNewPS(CParticleWorkspace &pws);	
	// remove part of the tree and the associated CNodeType objects. IMPORTANT : DOES NOT update the matching elements in the ps that is being edited.
	void removeTreePart(HTREEITEM root);
	// get tree item from its matching CNodeType object
	HTREEITEM getTreeItem(CNodeType *nt) const;
	// get a tree item from a workspace node
	HTREEITEM getTreeItem(CParticleWorkspace::CNode *node) const;
	// Get the parent node in the workspace for the given element in the tree
	CParticleWorkspace::CNode *getOwnerNode(CNodeType *nt) const;
	// the last ps that had a selected instance in it
	NLMISC::CRefPtr<NL3D::CParticleSystem> _LastClickedPS;
	// Update right pane to edit the given element
	void updateRightPane(CNodeType &nt);
	// Matching infos for each nodes in the CTreeCtrl
	std::vector<CNodeType *> _NodeTypes;
	//
	CUniquePtr<NL3D::CPSLocated>			_LocatedCopy;
	CUniquePtr<NL3D::CPSLocatedBindable>	_LocatedBindableCopy;
	//
	DECLARE_MESSAGE_MAP()
	// from CParticleWorkspace::IModificationCallback
	virtual void nodeModifiedFlagChanged(CParticleWorkspace::CNode &node);
	// from CParticleWorkspace::IModificationCallback
	virtual void workspaceModifiedFlagChanged(CParticleWorkspace &ws);
	// from CParticleWorkspace::IModificationCallback
	virtual void nodeSkelParentChanged(CParticleWorkspace::CNode &node);

	HTREEITEM	_LastActiveNode;

	bool		_ViewFilenameFlag;
public:
	// Update currently active node. Its node in the tree is displayed with bold characters.
	void setActiveNode(CParticleWorkspace::CNode *node);
	void touchPSState(CNodeType *nt);
	void sortWorkspace(CParticleWorkspace &ws, CParticleWorkspace::ISort &sorter);
	// delete the current selected node, and update the edited ps acoordingly
	void deleteSelection();
	void expandRoot();
	void updateAllCaptions();
	void removeAllPS(CParticleWorkspace &ws);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLE_TREE_CTRL_H__9A55D046_3E95_49CC_99AC_8A4F268EDD33__INCLUDED_)

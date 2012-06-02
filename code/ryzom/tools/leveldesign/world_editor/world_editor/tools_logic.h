// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef TOOLSLOGIC_H
#define TOOLSLOGIC_H

// ***************************************************************************

#include <afxcview.h>
#include "resource.h"
#include "color_button.h"
#include "main_frm.h"

// ***************************************************************************

class CMainFrame;
class CToolsLogic;

// ***************************************************************************

class CToolsLogicTree : public CTreeCtrl
{
	struct SRegionInfo
	{
		std::string			Name;
		HTREEITEM			RegionItem;
		//HTREEITEM		PointItem, PathItem, ZoneItem;
		std::vector<HTREEITEM>	Groups;
	};

	CToolsLogic					*_ToolLogic;
	CMainFrame					*_MainFrame;
	std::vector<SRegionInfo>	_RegionsInfo;

	// for items drag-and-drop
	bool			m_boDragging;
	HTREEITEM       m_hDragItem;
	HTREEITEM       m_hDragTarget;
	CImageList*		m_pDragImgList;

public:

	// Begin selection mark (select with SHIFT)
	HTREEITEM					SelectMark;

	// Old stuff

	CToolsLogicTree();
	
	void init (CMainFrame *pMF);
	void reset ();
	void uninit ();
	void setTool (CToolsLogic *pTools);

	uint32 createNewRegion (const std::string &sRegionName);
	uint32 createNewGroup (uint32 nRegion, const std::string &sGroupName);
	HTREEITEM addPoint (uint32 nRegion, uint32 nGroup, const std::string &sPointName, bool bHide=false);
	HTREEITEM addPath  (uint32 nRegion, uint32 nGroup, const std::string &sPathName, bool bHide=false);
	HTREEITEM addZone  (uint32 nRegion, uint32 nGroup, const std::string &sZoneName, bool bHide=false);
	void expandAll (uint32 nRegion);
	void SuccessfulDrag( HTREEITEM hDest,HTREEITEM hSrc );

	HTREEITEM InsertItemAndSubtree( HTREEITEM hDest, HTREEITEM hSrc );
	void CopySubtree( HTREEITEM hDest, HTREEITEM hSrc );
	void CopyItem( HTREEITEM hDest, HTREEITEM hSrc );

	virtual BOOL PreCreateWindow (CREATESTRUCT& cs);

	afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown (UINT nFlags, CPoint point);	

	afx_msg void OnSelChanged ();

	// Contextual menu handlers
	afx_msg void OnMenuRegionCreateGroup ();
	afx_msg void OnMenuRegionHideAll ();
	afx_msg void OnMenuRegionUnhideAll ();
	afx_msg void OnMenuRegionHideType ();
	afx_msg void OnMenuRegionUnhideType ();

	afx_msg void OnMenuGroupCreatePoint ();
	afx_msg void OnMenuGroupCreatePath ();
	afx_msg void OnMenuGroupCreateZone ();
	afx_msg void OnMenuGroupDelete ();
	afx_msg void OnMenuGroupProperties ();
	afx_msg void OnMenuGroupHide ();
	afx_msg void OnMenuGroupUnhide ();
	afx_msg void OnMenuGroupTransfertAppend ();
	afx_msg void OnMenuGroupTransfertReplace ();

	afx_msg void OnMenuPrimDelete ();
	afx_msg void OnMenuPrimProperties ();
	afx_msg void OnProjectNewPrimitive ();
	afx_msg void OnViewShow ();
	afx_msg void OnViewHide ();
	afx_msg void OnEditSelectChildren ();
	afx_msg void OnAddPrimitive (UINT nID);
	afx_msg void OnGeneratePrimitive (UINT nID);
	afx_msg void OnOpenFile (UINT nID);
	afx_msg void OnEditExpand ();
	afx_msg void OnEditCollapse ();
	afx_msg void OnRenameSelected();
	afx_msg void OnRepairSelected();
	afx_msg void OnTreeHelp();

	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		
	DECLARE_MESSAGE_MAP ()
};

// ***************************************************************************

class CToolsLogic : public CFormView
{
	DECLARE_DYNCREATE(CToolsLogic)

	CToolsLogicTree	*_Tree;
	bool			_TreeCreated;

	CMainFrame		*_MainFrame;
	
public:

	// \name Modify tree functions

	// Add a primitive
	HTREEITEM addPrimitive (HTREEITEM parentItem, HTREEITEM lastBrother, const CDatabaseLocatorPointer &locator);

	// Update primitive parameters
	void updatePrimitive (HTREEITEM item, const CDatabaseLocatorPointer &locator);

	void ensureVisible (class IPrimitiveEditor *primitive);

private:

	// Data are validated
	bool		_ValidStruct;
	bool		_ValidParam;
	bool		_ValidTitles;
	
public:
	
	CToolsLogic();

	CToolsLogicTree *GetTreeCtrl();
	
	void init (CMainFrame *pMF);
	void reset ();
	void uninit ();

	uint32 createNewRegion (const std::string &sRegionName);
	uint32 createNewGroup (uint32 nRegion, const std::string &sGroupName);
	HTREEITEM addPoint (uint32 nRegion, uint32 nGroup, const std::string &name, bool bHide=false);
	HTREEITEM addPath  (uint32 nRegion, uint32 nGroup, const std::string &name, bool bHide=false);
	HTREEITEM addZone  (uint32 nRegion, uint32 nGroup, const std::string &name, bool bHide=false);
	void expandAll (uint32 nRegion);

	// Event handlers
	afx_msg int OnCreate (LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg void OnPaint ();

	afx_msg void OnNewGroup ();
	afx_msg void OnNewPatat ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolsLogic)
	protected:
	virtual BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
	//}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
};

// ***************************************************************************

class CCreateDialog : public CDialog
{
	
public:

	char			Name[128], 
					LayerName[128];

	std::vector<SType>	*TypesForInit;

	std::string RegionPost;
	CMainFrame	*MainFrame;

	std::string PropName;
	std::string PropType;


	CComboBox		ComboType;

public:

	CCreateDialog(CWnd*pParent);

	virtual BOOL OnInitDialog ();
	virtual void DoDataExchange (CDataExchange* pDX);
	virtual void OnOK();

	void setRegionName(const std::string &rn);

	afx_msg void OnSelChange();


	DECLARE_MESSAGE_MAP()

};

// ***************************************************************************

class CEditStringDlg : public CDialog
{
	
public:

	CString	Name;

public:

	CEditStringDlg (CWnd*pParent);

	virtual BOOL OnInitDialog ();
	virtual void DoDataExchange (CDataExchange* pDX);
	virtual void OnOK();

};

// ***************************************************************************

#endif // TOOLSLOGIC_H

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

#if !defined(AFX_VEGETABLE_DLG_H__4E65E757_88C9_4FCE_94CC_9E429D8DFD68__INCLUDED_)
#define AFX_VEGETABLE_DLG_H__4E65E757_88C9_4FCE_94CC_9E429D8DFD68__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// vegetable_dlg.h : header file
//

#include "ps_wrapper.h"
#include "vegetable_refresh.h"
#include "vegetable_list_box.h"


class	CObjectViewer;
class	CVegetableApperancePage;
class	CVegetableDensityPage;
class	CVegetableScalePage;
class	CVegetableRotatePage;
class	CVegetableWindDlg;

namespace	NL3D
{
	class	CVegetable;
	class	CTileVegetableDesc;
}


/////////////////////////////////////////////////////////////////////////////
// CVegetableDlg dialog

class CVegetableDlg : public CDialog, public IVegetableRefresh
{
// Construction
public:
	CVegetableDlg(CObjectViewer *viewer, CWnd* pParent = NULL);   // standard constructor

	~CVegetableDlg();

// Dialog Data
	//{{AFX_DATA(CVegetableDlg)
	enum { IDD = IDD_VEGETABLE_DLG };
	CVegetableListBox	VegetableList;
	CStatic	StaticPolyCount;
	CButton	CheckSnapToGround;
	CButton	CheckEnableVegetable;
	CButton	CheckAutomaticRefresh;
	CButton	ButtonRefreshLandscape;
	CButton	CheckShowLandscape;
	CStatic	SelectVegetableStaticText;
	//}}AFX_DATA


	// IVegetableRefresh implementation: only if automatic refresh is checked
	virtual	void		refreshVegetableDisplay();


public:
	
	// get count of vegetable in the dlg
	uint				getNumVegetables() const;
	std::string			getVegetableName(uint id) const;
	void				updateCurSelVegetableName();
	NL3D::CVegetable	*getVegetable(uint id) const;

	// Change the hiden state of the id vegetable.
	void				setShowHideVegetable (uint id, bool visible, bool refreshDisplay);
	void				swapShowHideVegetable (uint id);
	bool				isVegetableVisible (uint id);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVegetableDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	/** set the vegetble to edit. NULL will hide all the windows.
	 *	Called by ListBox selection.
	 */
	void			setVegetableToEdit(NL3D::CVegetable *vegetable);

// Implementation
protected:
	CObjectViewer				*_ObjView;
	friend	class	CVegetableListBox;

	// Name to save.
	std::string					_LastVegetSetName;

	// A desc of vegetable to edit.
	struct	CVegetableDesc
	{
		// The vegetable
		NL3D::CVegetable		*Vegetable;
		// The name of this vegetable.
		std::string				VegetableName;
		// Visibility. Editor feature only
		bool					Visible;

		CVegetableDesc();

		// init the vegetable
		void		initDefaultVegetable();
		void		initVegetable(const NL3D::CVegetable &vegetable);
		// update VegetableName according to Vegetable
		void		updateVegetableName();
		// delete the vegetable
		void		deleteVegetable();
	};

	// The vegetable List.
	std::vector<CVegetableDesc>	_Vegetables;

	// The property sheet.
	CPropertySheet				*_PropertySheet;
	CVegetableDensityPage		*_VegetableDensityPage;
	CVegetableApperancePage		*_VegetableApperancePage;
	CVegetableScalePage			*_VegetableScalePage;
	CVegetableRotatePage		*_VegetableRotatePage;

	// Extra wind dlg
	CVegetableWindDlg			*_VegetableWindDlg;

	// refresh vegetable display even if box unchecked.
	void			doRefreshVegetableDisplay();

	// clear all vegetables.
	void			clearVegetables();
	// load a vegetSet with a FileDialog
	bool			loadVegetableSet(NL3D::CTileVegetableDesc &vegetSet, const char *title);
	/** build the vegetSet from the current _Vegetables
	 * NB: transform Rotate Angle in Radians.
	 * \param keepDefaultShapeName if true, then vegetables with a ShapeName=="" are kept.
	 * \param keepHiden if true, then vegetables maked as hiden in ObjectViewer are kept.
	 */
	void			buildVegetableSet(NL3D::CTileVegetableDesc &vegetSet, bool keepDefaultShapeName= true, bool keepHiden= true );
	// append the vegetSet to the current _Vegetables
	// NB: transform Rotate Angle in Degrees.
	void			appendVegetableSet(NL3D::CTileVegetableDesc &vegetSet);

	// Generated message map functions
	//{{AFX_MSG(CVegetableDlg)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListVegetable();
	afx_msg void OnButtonVegetableAdd();
	afx_msg void OnButtonVegetableClear();
	afx_msg void OnButtonVegetableInsert();
	afx_msg void OnButtonVegetableLoadDesc();
	afx_msg void OnButtonVegetableLoadSet();
	afx_msg void OnButtonVegetableRemove();
	afx_msg void OnButtonVegetableSaveDesc();
	afx_msg void OnButtonVegetableSaveSet();
	afx_msg void OnButtonVegetableAppendSet();
	afx_msg void OnButtonVegetableCopy();
	afx_msg void OnButtonVegetableRefresh();
	afx_msg void OnCheckVegetableShow();
	afx_msg void OnButtonVegetableSetupWind();
	afx_msg void OnCheckVegetableAutomatic();
	afx_msg void OnCheckVegetableEnable();
	afx_msg void OnCheckVegetableSnaptoground();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VEGETABLE_DLG_H__4E65E757_88C9_4FCE_94CC_9E429D8DFD68__INCLUDED_)

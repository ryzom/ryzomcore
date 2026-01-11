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

#if !defined(AFX_SKELETON_SCALE_DLG_H__4684D63E_9A55_47E7_BE43_F5C4CD86F211__INCLUDED_)
#define AFX_SKELETON_SCALE_DLG_H__4684D63E_9A55_47E7_BE43_F5C4CD86F211__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// skeleton_scale_dlg.h : header file
//

#include "nel/misc/smart_ptr.h"

namespace NL3D
{
	class CSkeletonModel;
};

/////////////////////////////////////////////////////////////////////////////
// CSkeletonScaleDlg dialog

// Define the decimal precision
#define NL_SSD_SCALE_PRECISION	1000

class CSkeletonScaleDlg : public CDialog
{
// Construction
public:
	CSkeletonScaleDlg(CObjectViewer *viewer, CWnd* pParent = NULL);   // standard constructor
	~CSkeletonScaleDlg();

// Dialog Data
	//{{AFX_DATA(CSkeletonScaleDlg)
	enum { IDD = IDD_SKELETON_SCALE_DLG };
	CSliderCtrl	_SliderSkinZ;
	CSliderCtrl	_SliderSkinY;
	CSliderCtrl	_SliderSkinX;
	CSliderCtrl	_SliderBoneZ;
	CSliderCtrl	_SliderBoneY;
	CSliderCtrl	_SliderBoneX;
	CListBox	_BoneList;
	CString	_StaticFileName;
	CString	_EditBoneSX;
	CString	_EditBoneSY;
	CString	_EditBoneSZ;
	CString	_EditSkinSX;
	CString	_EditSkinSY;
	CString	_EditSkinSZ;
	CStatic	_StaticScaleMarkerBoneSX;
	CStatic	_StaticScaleMarkerBoneSY;
	CStatic	_StaticScaleMarkerBoneSZ;
	CStatic	_StaticScaleMarkerSkinSX;
	CStatic	_StaticScaleMarkerSkinSY;
	CStatic	_StaticScaleMarkerSkinSZ;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSkeletonScaleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


public:
	// call when a skeleton is loaded. set NULL when released
	void		setSkeletonToEdit(NL3D::CSkeletonModel *skel, const std::string &fileName);

	// call each frame to display scaled bboxes around selected bones
	void		drawSelection();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSkeletonScaleDlg)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnReleasedcaptureSsdSliderBoneSx(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSsdSliderBoneSy(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSsdSliderBoneSz(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSsdSliderSkinSx(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSsdSliderSkinSy(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReleasedcaptureSsdSliderSkinSz(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeSsdEditBoneSx();
	afx_msg void OnChangeSsdEditBoneSy();
	afx_msg void OnChangeSsdEditBoneSz();
	afx_msg void OnChangeSsdEditSkinSx();
	afx_msg void OnChangeSsdEditSkinSy();
	afx_msg void OnChangeSsdEditSkinSz();
	afx_msg void OnKillfocusSsdEditBoneSx();
	afx_msg void OnKillfocusSsdEditBoneSy();
	afx_msg void OnKillfocusSsdEditBoneSz();
	afx_msg void OnKillfocusSsdEditSkinSx();
	afx_msg void OnKillfocusSsdEditSkinSy();
	afx_msg void OnKillfocusSsdEditSkinSz();
	afx_msg void OnSetfocusSsdEditBoneSx();
	afx_msg void OnSetfocusSsdEditBoneSy();
	afx_msg void OnSetfocusSsdEditBoneSz();
	afx_msg void OnSetfocusSsdEditSkinSx();
	afx_msg void OnSetfocusSsdEditSkinSy();
	afx_msg void OnSetfocusSsdEditSkinSz();
	afx_msg void OnSelchangeSsdList();
	afx_msg void OnSsdButtonUndo();
	afx_msg void OnSsdButtonRedo();
	afx_msg void OnSsdButtonSave();
	afx_msg void OnSsdButtonSaveas();
	afx_msg void OnSsdButtonMirror();
	afx_msg void OnSsdButtonSaveScale();
	afx_msg void OnSsdButtonLoadScale();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	enum	TScaleId
	{
		SidBoneX= 0,
		SidBoneY,
		SidBoneZ,
		SidSkinX,
		SidSkinY,
		SidSkinZ,
		SidCount,
		SidNone= SidCount
	};
	
private:
	CObjectViewer								*_ObjViewer;
	NLMISC::CRefPtr<NL3D::CSkeletonModel>		_SkeletonModel;
	std::string									_SkeletonFileName;
	TScaleId									_SliderEdited;
	bool										_SaveDirty;

	// A mirror to the list of bone
	struct CBoneMirror 
	{
		CBoneMirror()
		{
			SkinScale= BoneScale= NLMISC::CVector(1.f,1.f,1.f);
			Selected= false;
		}

		// Current Scale * NL_SSD_SCALE_PRECISION, and rounded
		NLMISC::CVector		SkinScale;
		NLMISC::CVector		BoneScale;
		// If the bone is selected in the multi List
		bool		Selected;
	};
	typedef std::vector<CBoneMirror>	TBoneMirrorArray;
	// The current bones
	TBoneMirrorArray							_Bones;
	// Backup of bones when slider start moving
	TBoneMirrorArray							_BkupBones;


	// For selection drawing, the local bbox
	std::vector<NLMISC::CAABBox>				_BoneBBoxes;
	bool										_BoneBBoxNeedRecompute;


	// points to controls for each ScaleId
	CSliderCtrl			*_ScaleSliders[SidCount];
	CString				*_ScaleEdits[SidCount];
	CStatic				*_StaticScaleMarkers[SidCount];
	
	void		applyScaleSlider(sint scrollValue);
	void		onSliderReleased(TScaleId sid);
	void		applyMirrorToSkeleton();
	void		applySkeletonToMirror();
	void		refreshTextViews();
	void		refreshTextViewWithScale(TScaleId sid, float scale, float diff);
	void		roundClampScale(NLMISC::CVector &v) const;
	TScaleId	getScaleIdFromSliderCtrl(CSliderCtrl	*sliderCtrl) const;
	TScaleId	getScaleIdFromEditId(UINT ctrlId) const;
	void		onSelectEditText(UINT ctrlId);
	void		onChangeEditText(UINT ctrlId);
	void		onQuitEditText(UINT ctrlId);
	void		updateScalesFromText(UINT ctrlId);
	void		applySelectionToView();
	void		refreshUndoRedoView();
	bool		saveCurrentInStream(NLMISC::IStream &f);
	void		refreshSaveButton();
	sint		getBoneForMirror(uint boneId, std::string &mirrorName);
	bool		saveSkelScaleInStream(NLMISC::IStream &f);
	bool		loadSkelScaleFromStream(NLMISC::IStream &f);
	
	/// \name undo/redo mgt
	// @{
	enum	{MaxUndoRedo= 100};
	std::deque<TBoneMirrorArray>				_UndoQueue;
	std::deque<TBoneMirrorArray>				_RedoQueue;

	// bkup the current _Bones in the undo queue, and clear the redo. dirtSave indicate the skel need saving
	// NB: compare precState with _Bones. if same, then no-op!
	void		pushUndoState(const TBoneMirrorArray &precState, bool dirtSave);
	// undo the last change, and store it in the redo queue
	void		undo();
	// redo the last undoed change, and restore it in the undo queue
	void		redo();

	// @}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKELETON_SCALE_DLG_H__4684D63E_9A55_47E7_BE43_F5C4CD86F211__INCLUDED_)

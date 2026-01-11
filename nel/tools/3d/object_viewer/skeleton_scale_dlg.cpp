// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// skeleton_scale_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "skeleton_scale_dlg.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/misc/algo.h"
#include "main_frame.h"


// ***************************************************************************
#define NL_SSD_SLIDER_SIZE			1000
#define NL_SSD_SLIDER_THRESHOLD		0.03f
#define NL_SSD_SLIDER_SCALE			2


/////////////////////////////////////////////////////////////////////////////
// CSkeletonScaleDlg dialog


CSkeletonScaleDlg::CSkeletonScaleDlg(CObjectViewer *viewer, CWnd* pParent /*=NULL*/)
	: CDialog(CSkeletonScaleDlg::IDD, pParent) ,_ObjViewer(viewer)
{
	//{{AFX_DATA_INIT(CSkeletonScaleDlg)
	_StaticFileName = _T("");
	_EditBoneSX = _T("");
	_EditBoneSY = _T("");
	_EditBoneSZ = _T("");
	_EditSkinSX = _T("");
	_EditSkinSY = _T("");
	_EditSkinSZ = _T("");
	//}}AFX_DATA_INIT

	// Init Scale Sliders ptrs
	nlassert(SidCount==6);
	_ScaleSliders[SidBoneX]= &_SliderBoneX;
	_ScaleSliders[SidBoneY]= &_SliderBoneY;
	_ScaleSliders[SidBoneZ]= &_SliderBoneZ;
	_ScaleSliders[SidSkinX]= &_SliderSkinX;
	_ScaleSliders[SidSkinY]= &_SliderSkinY;
	_ScaleSliders[SidSkinZ]= &_SliderSkinZ;
	_ScaleEdits[SidBoneX]= &_EditBoneSX;
	_ScaleEdits[SidBoneY]= &_EditBoneSY;
	_ScaleEdits[SidBoneZ]= &_EditBoneSZ;
	_ScaleEdits[SidSkinX]= &_EditSkinSX;
	_ScaleEdits[SidSkinY]= &_EditSkinSY;
	_ScaleEdits[SidSkinZ]= &_EditSkinSZ;
	_StaticScaleMarkers[SidBoneX]= &_StaticScaleMarkerBoneSX;
	_StaticScaleMarkers[SidBoneY]= &_StaticScaleMarkerBoneSY;
	_StaticScaleMarkers[SidBoneZ]= &_StaticScaleMarkerBoneSZ;
	_StaticScaleMarkers[SidSkinX]= &_StaticScaleMarkerSkinSX;
	_StaticScaleMarkers[SidSkinY]= &_StaticScaleMarkerSkinSY;
	_StaticScaleMarkers[SidSkinZ]= &_StaticScaleMarkerSkinSZ;
	

	_SliderEdited= SidNone;
	_SaveDirty= false;

	// avoid realloc
	_UndoQueue.resize(MaxUndoRedo);
	_RedoQueue.resize(MaxUndoRedo);
	_UndoQueue.clear();
	_RedoQueue.clear();

	_BoneBBoxNeedRecompute= false;
}

CSkeletonScaleDlg::~CSkeletonScaleDlg()
{
}


void CSkeletonScaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkeletonScaleDlg)
	DDX_Control(pDX, IDC_SSD_SLIDER_SKIN_SZ, _SliderSkinZ);
	DDX_Control(pDX, IDC_SSD_SLIDER_SKIN_SY, _SliderSkinY);
	DDX_Control(pDX, IDC_SSD_SLIDER_SKIN_SX, _SliderSkinX);
	DDX_Control(pDX, IDC_SSD_SLIDER_BONE_SZ, _SliderBoneZ);
	DDX_Control(pDX, IDC_SSD_SLIDER_BONE_SY, _SliderBoneY);
	DDX_Control(pDX, IDC_SSD_SLIDER_BONE_SX, _SliderBoneX);
	DDX_Control(pDX, IDC_SSD_LIST, _BoneList);
	DDX_Text(pDX, IDC_SSD_STATIC_FILENAME, _StaticFileName);
	DDX_Text(pDX, IDC_SSD_EDIT_BONE_SX, _EditBoneSX);
	DDX_Text(pDX, IDC_SSD_EDIT_BONE_SY, _EditBoneSY);
	DDX_Text(pDX, IDC_SSD_EDIT_BONE_SZ, _EditBoneSZ);
	DDX_Text(pDX, IDC_SSD_EDIT_SKIN_SX, _EditSkinSX);
	DDX_Text(pDX, IDC_SSD_EDIT_SKIN_SY, _EditSkinSY);
	DDX_Text(pDX, IDC_SSD_EDIT_SKIN_SZ, _EditSkinSZ);
	DDX_Control(pDX, IDC_SSD_STATIC_SKIN_SZ, _StaticScaleMarkerSkinSZ);
	DDX_Control(pDX, IDC_SSD_STATIC_SKIN_SY, _StaticScaleMarkerSkinSY);
	DDX_Control(pDX, IDC_SSD_STATIC_SKIN_SX, _StaticScaleMarkerSkinSX);
	DDX_Control(pDX, IDC_SSD_STATIC_BONE_SZ, _StaticScaleMarkerBoneSZ);
	DDX_Control(pDX, IDC_SSD_STATIC_BONE_SY, _StaticScaleMarkerBoneSY);
	DDX_Control(pDX, IDC_SSD_STATIC_BONE_SX, _StaticScaleMarkerBoneSX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSkeletonScaleDlg, CDialog)
	//{{AFX_MSG_MAP(CSkeletonScaleDlg)
	ON_WM_DESTROY()
	ON_WM_VSCROLL()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SSD_SLIDER_BONE_SX, OnReleasedcaptureSsdSliderBoneSx)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SSD_SLIDER_BONE_SY, OnReleasedcaptureSsdSliderBoneSy)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SSD_SLIDER_BONE_SZ, OnReleasedcaptureSsdSliderBoneSz)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SSD_SLIDER_SKIN_SX, OnReleasedcaptureSsdSliderSkinSx)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SSD_SLIDER_SKIN_SY, OnReleasedcaptureSsdSliderSkinSy)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SSD_SLIDER_SKIN_SZ, OnReleasedcaptureSsdSliderSkinSz)
	ON_EN_CHANGE(IDC_SSD_EDIT_BONE_SX, OnChangeSsdEditBoneSx)
	ON_EN_CHANGE(IDC_SSD_EDIT_BONE_SY, OnChangeSsdEditBoneSy)
	ON_EN_CHANGE(IDC_SSD_EDIT_BONE_SZ, OnChangeSsdEditBoneSz)
	ON_EN_CHANGE(IDC_SSD_EDIT_SKIN_SX, OnChangeSsdEditSkinSx)
	ON_EN_CHANGE(IDC_SSD_EDIT_SKIN_SY, OnChangeSsdEditSkinSy)
	ON_EN_CHANGE(IDC_SSD_EDIT_SKIN_SZ, OnChangeSsdEditSkinSz)
	ON_EN_KILLFOCUS(IDC_SSD_EDIT_BONE_SX, OnKillfocusSsdEditBoneSx)
	ON_EN_KILLFOCUS(IDC_SSD_EDIT_BONE_SY, OnKillfocusSsdEditBoneSy)
	ON_EN_KILLFOCUS(IDC_SSD_EDIT_BONE_SZ, OnKillfocusSsdEditBoneSz)
	ON_EN_KILLFOCUS(IDC_SSD_EDIT_SKIN_SX, OnKillfocusSsdEditSkinSx)
	ON_EN_KILLFOCUS(IDC_SSD_EDIT_SKIN_SY, OnKillfocusSsdEditSkinSy)
	ON_EN_KILLFOCUS(IDC_SSD_EDIT_SKIN_SZ, OnKillfocusSsdEditSkinSz)
	ON_EN_SETFOCUS(IDC_SSD_EDIT_BONE_SX, OnSetfocusSsdEditBoneSx)
	ON_EN_SETFOCUS(IDC_SSD_EDIT_BONE_SY, OnSetfocusSsdEditBoneSy)
	ON_EN_SETFOCUS(IDC_SSD_EDIT_BONE_SZ, OnSetfocusSsdEditBoneSz)
	ON_EN_SETFOCUS(IDC_SSD_EDIT_SKIN_SX, OnSetfocusSsdEditSkinSx)
	ON_EN_SETFOCUS(IDC_SSD_EDIT_SKIN_SY, OnSetfocusSsdEditSkinSy)
	ON_EN_SETFOCUS(IDC_SSD_EDIT_SKIN_SZ, OnSetfocusSsdEditSkinSz)
	ON_LBN_SELCHANGE(IDC_SSD_LIST, OnSelchangeSsdList)
	ON_BN_CLICKED(IDC_SSD_BUTTON_UNDO, OnSsdButtonUndo)
	ON_BN_CLICKED(IDC_SSD_BUTTON_REDO, OnSsdButtonRedo)
	ON_BN_CLICKED(IDC_SSD_BUTTON_SAVE, OnSsdButtonSave)
	ON_BN_CLICKED(IDC_SSD_BUTTON_SAVEAS, OnSsdButtonSaveas)
	ON_BN_CLICKED(IDC_SSD_BUTTON_MIRROR, OnSsdButtonMirror)
	ON_BN_CLICKED(IDC_SSD_BUTTON_SAVE_SCALE, OnSsdButtonSaveScale)
	ON_BN_CLICKED(IDC_SSD_BUTTON_LOAD_SCALE, OnSsdButtonLoadScale)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkeletonScaleDlg message handlers

void CSkeletonScaleDlg::OnDestroy() 
{
	setRegisterWindowState (this, REGKEY_SKELETON_SCALE_DLG);		
	CDialog::OnDestroy();	
}



// ***************************************************************************
void		CSkeletonScaleDlg::setSkeletonToEdit(NL3D::CSkeletonModel *skel, const std::string &fileName)
{
	uint	i;

	_SkeletonModel= skel;
	_SkeletonFileName= fileName;


	// **** Setup File name
	_StaticFileName= fileName.c_str();

	// **** Setup Bone mirror
	_Bones.clear();
	if(_SkeletonModel)
	{
		_Bones.resize(_SkeletonModel->Bones.size());
		// copy from skel to mirror
		applySkeletonToMirror();
	}

	// **** reset bone bbox here
	_BoneBBoxes.clear();
	_BoneBBoxes.resize(_Bones.size());
	// delegate to drawSelection(), cause skins not still bound
	_BoneBBoxNeedRecompute= true;

	// **** Setup Bone List
	_BoneList.ResetContent();
	if(_SkeletonModel)
	{
		for(uint i=0;i<_SkeletonModel->Bones.size();i++)
		{
			const std::string tabStr = "   ";
			std::string name = _SkeletonModel->Bones[i].getBoneName();

			// append a tab for easy hierarchy
			uint boneId = i;

			while((boneId=_SkeletonModel->Bones[boneId].getFatherId())!=-1)
				name = tabStr + name;

			// append to the list
			_BoneList.AddString(nlUtf8ToTStr(name));
		}
	}


	// **** unselect all widgets
	for(i=0;i<SidCount;i++)
	{
		_ScaleSliders[i]->SetRange(0, NL_SSD_SLIDER_SIZE);
		_ScaleSliders[i]->SetPos(NL_SSD_SLIDER_SIZE/2);
		_ScaleEdits[i]->Empty();
		_StaticScaleMarkers[i]->SetWindowText(_T("100%"));
	}
	// ensure no problem with scale and ALT-TAB
	_SliderEdited= SidNone;
	

	// **** clean undo/redo
	_UndoQueue.clear();
	_RedoQueue.clear();
	refreshUndoRedoView();


	// **** clear save button
	_SaveDirty= false;
	refreshSaveButton();


	// refresh
	UpdateData(FALSE);
}


// ***************************************************************************
BOOL CSkeletonScaleDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	setSkeletonToEdit(NULL, "");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



// ***************************************************************************
void CSkeletonScaleDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CSliderCtrl	*sliderCtrl= (CSliderCtrl*)pScrollBar;
	TScaleId	sliderId= getScaleIdFromSliderCtrl(sliderCtrl);
	if(sliderId!=SidNone && nSBCode==SB_THUMBPOSITION || nSBCode==SB_THUMBTRACK)
	{
		// If the user press ALT-Tab while dragging an old slider, the old slider is not released.
		// ThereFore, release the old one now
		if(_SliderEdited!=SidNone && _SliderEdited!=sliderId)
		{
			onSliderReleased(_SliderEdited);
		}
		
		// if begin of slide, bkup state
		if(_SliderEdited==SidNone)
		{
			_SliderEdited= sliderId;
			// Bkup all scales (dont bother selected bones or which scale is edited...)
			_BkupBones= _Bones;
		}
		
		//applyScale
		applyScaleSlider(nPos);
	}
	else
		CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

// ***************************************************************************
void CSkeletonScaleDlg::OnReleasedcaptureSsdSliderBoneSx(NMHDR* pNMHDR, LRESULT* pResult) 
{
	onSliderReleased(SidBoneX);
	*pResult = 0;
}
void CSkeletonScaleDlg::OnReleasedcaptureSsdSliderBoneSy(NMHDR* pNMHDR, LRESULT* pResult) 
{
	onSliderReleased(SidBoneY);
	*pResult = 0;
}
void CSkeletonScaleDlg::OnReleasedcaptureSsdSliderBoneSz(NMHDR* pNMHDR, LRESULT* pResult) 
{
	onSliderReleased(SidBoneZ);
	*pResult = 0;
}
void CSkeletonScaleDlg::OnReleasedcaptureSsdSliderSkinSx(NMHDR* pNMHDR, LRESULT* pResult) 
{
	onSliderReleased(SidSkinX);
	*pResult = 0;
}
void CSkeletonScaleDlg::OnReleasedcaptureSsdSliderSkinSy(NMHDR* pNMHDR, LRESULT* pResult) 
{
	onSliderReleased(SidSkinY);
	*pResult = 0;
}
void CSkeletonScaleDlg::OnReleasedcaptureSsdSliderSkinSz(NMHDR* pNMHDR, LRESULT* pResult) 
{
	onSliderReleased(SidSkinZ);
	*pResult = 0;
}

void		CSkeletonScaleDlg::onSliderReleased(TScaleId sid)
{
	// Get value from slider
	sint	value= _ScaleSliders[sid]->GetPos();
	
	// If the user press ALT-Tab while dragging an old slider, the old slider is not released.
	// ThereFore, release the old one now
	if(_SliderEdited!=SidNone && _SliderEdited!=sid)
	{
		onSliderReleased(_SliderEdited);
	}
	
	//applyScale
	if(_SliderEdited==SidNone)
	{
		_SliderEdited= sid;
		// Bkup all scales (dont bother selected bones or which scale is edited...)
		_BkupBones= _Bones;
	}

	// apply the final value
	applyScaleSlider(value);
	
	// And reset the slider
	_ScaleSliders[_SliderEdited]->SetPos(NL_SSD_SLIDER_SIZE/2);
	_StaticScaleMarkers[_SliderEdited]->SetWindowText(_T("100%"));
	_SliderEdited= SidNone;

	// push an undo/redo only at release of slide. push the value of scale before slide
	pushUndoState(_BkupBones, true);
}


// ***************************************************************************
void		CSkeletonScaleDlg::applyScaleSlider(sint scrollValue)
{
	// get scale beetween -1 and 1.
	float	scale= (NL_SSD_SLIDER_SIZE/2-scrollValue)/(float)(NL_SSD_SLIDER_SIZE/2);
	NLMISC::clamp(scale, -1.f, 1.f);
	float	factor;
	
	// no scale
	if(fabs(scale)<NL_SSD_SLIDER_THRESHOLD)
		factor=1;
	// scale down
	else if(scale<0)
	{
		float	minv= 1.0f / NL_SSD_SLIDER_SCALE;
		factor= minv*(-scale) + 1.0f*(1+scale);
	}
	// scale up
	else
	{
		float	maxv= NL_SSD_SLIDER_SCALE;
		factor= maxv*(scale) + 1.0f*(1-scale);
	}
	
	// Apply the noise to each selected bones
	for(uint i=0;i<_Bones.size();i++)
	{
		CBoneMirror	&bone= _Bones[i];
		CBoneMirror	&bkup= _BkupBones[i];
		if(bone.Selected)
		{
			// apply to the scaled component
			switch(_SliderEdited)
			{
			case SidBoneX:	bone.BoneScale.x= bkup.BoneScale.x *factor; break;
			case SidBoneY:	bone.BoneScale.y= bkup.BoneScale.y *factor; break;
			case SidBoneZ:	bone.BoneScale.z= bkup.BoneScale.z *factor; break;
			case SidSkinX:	bone.SkinScale.x= bkup.SkinScale.x *factor; break;
			case SidSkinY:	bone.SkinScale.y= bkup.SkinScale.y *factor; break;
			case SidSkinZ:	bone.SkinScale.z= bkup.SkinScale.z *factor; break;
			};
			// round result
			roundClampScale(bone.BoneScale);
			roundClampScale(bone.SkinScale);
		}
	}

	// update the skeleton view
	applyMirrorToSkeleton();

	// refresh text views
	refreshTextViews();

	// update marker text
	std::string	str = NLMISC::toString("%d%%", (sint)(factor*100));
	_StaticScaleMarkers[_SliderEdited]->SetWindowText(nlUtf8ToTStr(str));
}

// ***************************************************************************
void		CSkeletonScaleDlg::applyMirrorToSkeleton()
{
	if(_SkeletonModel)
	{
		nlassert(_SkeletonModel->Bones.size()==_Bones.size());
		for(uint i=0;i<_Bones.size();i++)
		{
			// unmul from precision
			_SkeletonModel->Bones[i].setScale(_Bones[i].BoneScale / NL_SSD_SCALE_PRECISION);
			_SkeletonModel->Bones[i].setSkinScale(_Bones[i].SkinScale / NL_SSD_SCALE_PRECISION);
		}
	}
}


// ***************************************************************************
void		CSkeletonScaleDlg::applySkeletonToMirror()
{
	if(_SkeletonModel)
	{
		nlassert(_SkeletonModel->Bones.size()==_Bones.size());
		for(uint i=0;i<_SkeletonModel->Bones.size();i++)
		{
			// mul by precision, and round
			_Bones[i].SkinScale= _SkeletonModel->Bones[i].getSkinScale() * NL_SSD_SCALE_PRECISION;
			_Bones[i].BoneScale= _SkeletonModel->Bones[i].getScale() * NL_SSD_SCALE_PRECISION;
			roundClampScale(_Bones[i].SkinScale);
			roundClampScale(_Bones[i].BoneScale);
		}
	}
}


// ***************************************************************************
void		CSkeletonScaleDlg::refreshTextViews()
{
	// 1.f for each component if multiple selection is different, else 0.f
	NLMISC::CVector		boneScaleDiff= NLMISC::CVector::Null;
	NLMISC::CVector		skinScaleDiff= NLMISC::CVector::Null;
	// valid if scale of each bone component is same
	NLMISC::CVector		boneScaleAll= NLMISC::CVector::Null;
	NLMISC::CVector		skinScaleAll= NLMISC::CVector::Null;
	bool		someSelected= false;
	
	// For all bones selected
	for(uint i=0;i<_Bones.size();i++)
	{
		CBoneMirror	&bone= _Bones[i];
		if(bone.Selected)
		{
			if(!someSelected)
			{
				someSelected= true;
				// just bkup
				boneScaleAll= bone.BoneScale;
				skinScaleAll= bone.SkinScale;
			}
			else
			{
				// compare each component, if different, flag
				if(boneScaleAll.x!= bone.BoneScale.x)	boneScaleDiff.x= 1.f;
				if(boneScaleAll.y!= bone.BoneScale.y)	boneScaleDiff.y= 1.f;
				if(boneScaleAll.z!= bone.BoneScale.z)	boneScaleDiff.z= 1.f;
				if(skinScaleAll.x!= bone.SkinScale.x)	skinScaleDiff.x= 1.f;
				if(skinScaleAll.y!= bone.SkinScale.y)	skinScaleDiff.y= 1.f;
				if(skinScaleAll.z!= bone.SkinScale.z)	skinScaleDiff.z= 1.f;
			}
		}
	}
	
	// if none selected, force empty text
	if(!someSelected)
	{
		boneScaleDiff.set(1.f,1.f,1.f);
		skinScaleDiff.set(1.f,1.f,1.f);
	}

	// All component refresh or only one refresh?
	nlassert(SidCount==6);
	refreshTextViewWithScale(SidBoneX, boneScaleAll.x, boneScaleDiff.x);
	refreshTextViewWithScale(SidBoneY, boneScaleAll.y, boneScaleDiff.y);
	refreshTextViewWithScale(SidBoneZ, boneScaleAll.z, boneScaleDiff.z);
	refreshTextViewWithScale(SidSkinX, skinScaleAll.x, skinScaleDiff.x);
	refreshTextViewWithScale(SidSkinY, skinScaleAll.y, skinScaleDiff.y);
	refreshTextViewWithScale(SidSkinZ, skinScaleAll.z, skinScaleDiff.z);

	// refresh
	UpdateData(FALSE);
}


// ***************************************************************************
void		CSkeletonScaleDlg::refreshTextViewWithScale(TScaleId sid, float scale, float diff)
{
	// if different values selected, diff
	if(diff)
	{
		_ScaleEdits[sid]->Empty();
	}
	// else display text
	else
	{
		char	str[256];
		// ensure correct display with no precision problem
		sint	value= uint(floor(scale+0.5f));
		sint	whole= value*100/NL_SSD_SCALE_PRECISION;
		sint	decimal= value - whole*(NL_SSD_SCALE_PRECISION/100);
		sprintf(str, "%d.%d", whole, decimal);
		*_ScaleEdits[sid]= str;
	}
}


// ***************************************************************************
void		CSkeletonScaleDlg::roundClampScale(NLMISC::CVector &v) const
{
	v.x+= 0.5f;
	v.y+= 0.5f;
	v.z+= 0.5f;
	v.x= (float)floor(v.x);
	v.y= (float)floor(v.y);
	v.z= (float)floor(v.z);
	// Minimum is 1 (avoid 0 scale)
	v.maxof(v, NLMISC::CVector(1.f,1.f,1.f));
}

// ***************************************************************************
CSkeletonScaleDlg::TScaleId	CSkeletonScaleDlg::getScaleIdFromSliderCtrl(CSliderCtrl	*sliderCtrl) const
{
	for(uint i=0;i<SidCount;i++)
	{
		if(sliderCtrl==_ScaleSliders[i])
			return (TScaleId)i;
	}

	return SidNone;
}


// ***************************************************************************
CSkeletonScaleDlg::TScaleId	CSkeletonScaleDlg::getScaleIdFromEditId(UINT ctrlId) const
{
	nlctassert(SidCount==6);
	if(ctrlId==IDC_SSD_EDIT_BONE_SX)	return SidBoneX;
	if(ctrlId==IDC_SSD_EDIT_BONE_SY)	return SidBoneY;
	if(ctrlId==IDC_SSD_EDIT_BONE_SZ)	return SidBoneZ;
	if(ctrlId==IDC_SSD_EDIT_SKIN_SX)	return SidSkinX;
	if(ctrlId==IDC_SSD_EDIT_SKIN_SY)	return SidSkinY;
	if(ctrlId==IDC_SSD_EDIT_SKIN_SZ)	return SidSkinZ;
	
	return SidNone;
}


// ***************************************************************************
void CSkeletonScaleDlg::OnChangeSsdEditBoneSx() 
{
	onChangeEditText(IDC_SSD_EDIT_BONE_SX);
}
void CSkeletonScaleDlg::OnChangeSsdEditBoneSy() 
{
	onChangeEditText(IDC_SSD_EDIT_BONE_SY);
}
void CSkeletonScaleDlg::OnChangeSsdEditBoneSz() 
{
	onChangeEditText(IDC_SSD_EDIT_BONE_SZ);
}
void CSkeletonScaleDlg::OnChangeSsdEditSkinSx() 
{
	onChangeEditText(IDC_SSD_EDIT_SKIN_SX);
}
void CSkeletonScaleDlg::OnChangeSsdEditSkinSy() 
{
	onChangeEditText(IDC_SSD_EDIT_SKIN_SY);
}
void CSkeletonScaleDlg::OnChangeSsdEditSkinSz() 
{
	onChangeEditText(IDC_SSD_EDIT_SKIN_SZ);
}


static	void concatEdit2Lines(CEdit &edit)
{
	const	uint lineLen= 1000;
	uint	n;
	// retrieve the 2 lines.
	TCHAR	tmp0[2*lineLen];
	TCHAR	tmp1[lineLen];
	n= edit.GetLine(0, tmp0, lineLen);	tmp0[n]= 0;
	n= edit.GetLine(1, tmp1, lineLen);	tmp1[n]= 0;
	// concat and update the CEdit.
	edit.SetWindowText(_tcscat(tmp0, tmp1));
}


void CSkeletonScaleDlg::onChangeEditText(UINT ctrlId)
{
	CEdit *ce = (CEdit*)GetDlgItem(ctrlId);
	if(ce)
	{
		UpdateData();	
		// Trick to track "Enter" keypress: CEdit are multiline. If GetLineCount()>1, then
		// user has press enter.
		if(ce->GetLineCount()>1)
		{
			// must ccat 2 lines of the CEdit.
			concatEdit2Lines(*ce);
			// update text members
			UpdateData(FALSE);
			UpdateData();

			// update scale values from the CEdit
			updateScalesFromText(ctrlId);

			// then re-update CEdit from scale values
			refreshTextViews();
		}
	}
}


// ***************************************************************************
void CSkeletonScaleDlg::OnKillfocusSsdEditBoneSx() 
{
	onQuitEditText(IDC_SSD_EDIT_BONE_SX);
}
void CSkeletonScaleDlg::OnKillfocusSsdEditBoneSy() 
{
	onQuitEditText(IDC_SSD_EDIT_BONE_SY);
}
void CSkeletonScaleDlg::OnKillfocusSsdEditBoneSz() 
{
	onQuitEditText(IDC_SSD_EDIT_BONE_SZ);
}
void CSkeletonScaleDlg::OnKillfocusSsdEditSkinSx() 
{
	onQuitEditText(IDC_SSD_EDIT_SKIN_SX);
}
void CSkeletonScaleDlg::OnKillfocusSsdEditSkinSy() 
{
	onQuitEditText(IDC_SSD_EDIT_SKIN_SY);
}
void CSkeletonScaleDlg::OnKillfocusSsdEditSkinSz() 
{
	onQuitEditText(IDC_SSD_EDIT_SKIN_SZ);
}


void CSkeletonScaleDlg::onQuitEditText(UINT ctrlId)
{
	CEdit *ce = (CEdit*)GetDlgItem(ctrlId);
	if(ce)
	{
		UpdateData();	
			
		// update scale values from the CEdit
		updateScalesFromText(ctrlId);
		
		// then re-update CEdit from scale values
		refreshTextViews();
	}
}


// ***************************************************************************
void CSkeletonScaleDlg::OnSetfocusSsdEditBoneSx() 
{
	onSelectEditText(IDC_SSD_EDIT_BONE_SX);
}
void CSkeletonScaleDlg::OnSetfocusSsdEditBoneSy() 
{
	onSelectEditText(IDC_SSD_EDIT_BONE_SY);
}
void CSkeletonScaleDlg::OnSetfocusSsdEditBoneSz() 
{
	onSelectEditText(IDC_SSD_EDIT_BONE_SZ);
}
void CSkeletonScaleDlg::OnSetfocusSsdEditSkinSx() 
{
	onSelectEditText(IDC_SSD_EDIT_SKIN_SX);
}
void CSkeletonScaleDlg::OnSetfocusSsdEditSkinSy() 
{
	onSelectEditText(IDC_SSD_EDIT_SKIN_SY);
}
void CSkeletonScaleDlg::OnSetfocusSsdEditSkinSz() 
{
	onSelectEditText(IDC_SSD_EDIT_SKIN_SZ);
}

void CSkeletonScaleDlg::onSelectEditText(UINT ctrlId)
{
	CEdit *ce = (CEdit*)GetDlgItem(ctrlId);
	if(ce)
	{
		ce->PostMessage(EM_SETSEL, 0, -1);	
		ce->Invalidate();
	}
}


// ***************************************************************************
void CSkeletonScaleDlg::updateScalesFromText(UINT ctrlId)
{
	TScaleId	sid= getScaleIdFromEditId(ctrlId);
	if(sid==SidNone)
		return;

	// get the scale info
	std::string str = NLMISC::tStrToUtf8(*_ScaleEdits[sid]);
	if(str.empty())
		return;
	float	f;
	sscanf(str.c_str(), "%f", &f);
	// edit a %age
	f*= NL_SSD_SCALE_PRECISION/100;
	f= (float)floor(f+0.5f);

	// bkup for undo
	static TBoneMirrorArray		precState;
	precState= _Bones;
	
	// For all bones selected, set the edited value
	for(uint i=0;i<_Bones.size();i++)
	{
		CBoneMirror	&bone= _Bones[i];
		if(bone.Selected)
		{
			switch(sid)
			{
			case SidBoneX:	bone.BoneScale.x= f; break;
			case SidBoneY:	bone.BoneScale.y= f; break;
			case SidBoneZ:	bone.BoneScale.z= f; break;
			case SidSkinX:	bone.SkinScale.x= f; break;
			case SidSkinY:	bone.SkinScale.y= f; break;
			case SidSkinZ:	bone.SkinScale.z= f; break;
			};
			// normalize
			roundClampScale(bone.BoneScale);
			roundClampScale(bone.SkinScale);
		}
	}

	// change => bkup for undo
	pushUndoState(precState, true);
	
	// mirror changed => update skeleton
	applyMirrorToSkeleton();
}



// ***************************************************************************
void CSkeletonScaleDlg::OnSelchangeSsdList() 
{
	UpdateData();

	// **** Retrieve List of selected bones.
	uint	count= _BoneList.GetSelCount();
	std::vector<int>	items;
	if(count)
	{
		items.resize(count);
		_BoneList.GetSelItems(count, &items[0]);
	}

	// **** update the Selection array
	// bkup for undo
	static TBoneMirrorArray		precState;
	precState= _Bones;

	// identify selected items in a set
	std::set<int>	selSet;
	uint	i;
	for(i=0;i<count;i++)
		selSet.insert(items[i]);

	// change selection of Bones
	for(i=0;i<_Bones.size();i++)
	{
		if(selSet.find(i)!=selSet.end())
			_Bones[i].Selected= true;
		else
			_Bones[i].Selected= false;
	}

	// **** undo-redo
	// selection change => no need to dirt save
	pushUndoState(precState, false);

	// **** refresh text views
	refreshTextViews();
}


// ***************************************************************************
void CSkeletonScaleDlg::OnSsdButtonUndo() 
{
	undo();
}

void CSkeletonScaleDlg::OnSsdButtonRedo() 
{
	redo();
}


// ***************************************************************************
void CSkeletonScaleDlg::OnSsdButtonSave() 
{
	// if no skeleton edited, quit
	if(!_SkeletonModel)
		return;

	// save the file
	NLMISC::COFile	f;
	if( f.open(_SkeletonFileName) )
	{
		if(saveCurrentInStream(f))
		{
			// no more need to save (done)
			_SaveDirty= false;
			refreshSaveButton();
		}
	}
	else
	{
		MessageBox(_T("Failed to open file for write!"));
	}
}

void CSkeletonScaleDlg::OnSsdButtonSaveas() 
{
	// if no skeleton edited, quit
	if(!_SkeletonModel)
		return;

	// choose the file
	CFileDialog fd(FALSE, _T("skel"), nlUtf8ToTStr(_SkeletonFileName), OFN_OVERWRITEPROMPT, _T("SkelFiles (*.skel)|*.skel|All Files (*.*)|*.*||"), this);
	fd.m_ofn.lpstrTitle = _T("Save As Skeleton");
	if (fd.DoModal() == IDOK)
	{
		NLMISC::COFile	f;
		
		if (f.open(NLMISC::tStrToUtf8(fd.GetPathName())))
		{
			if(saveCurrentInStream(f))
			{
				// no more need to save (done)
				_SaveDirty= false;
				refreshSaveButton();
			}

			// bkup the valid fileName (new file edited)
			_SkeletonFileName = NLMISC::tStrToUtf8(fd.GetPathName());
			_StaticFileName= _SkeletonFileName.c_str();
			UpdateData(FALSE);
		}
		else
		{
			MessageBox(_T("Failed to open file for write!"));
		}
	}
}

// ***************************************************************************
bool CSkeletonScaleDlg::saveCurrentInStream(NLMISC::IStream &f)
{
	try
	{
		nlassert(_SkeletonModel);
		nlassert(_SkeletonModel->Shape);
		
		// Retrieve boneBase definition from the current skeleton
		std::vector<NL3D::CBoneBase>	boneBases;
		(NLMISC::safe_cast<NL3D::CSkeletonShape*>((NL3D::IShape*)_SkeletonModel->Shape))->retrieve(boneBases);
		
		// Copies bone scales from the model 
		nlassert(boneBases.size()==_SkeletonModel->Bones.size());
		for(uint i=0;i<_SkeletonModel->Bones.size();i++)
		{
			NL3D::CBone		&bone= _SkeletonModel->Bones[i];
			NL3D::CBoneBase	&boneBase= boneBases[i];

			boneBase.SkinScale= bone.getSkinScale();
			boneBase.DefaultScale= bone.getScale();
		}
		
		// build a new Skeleton shape
		NL3D::CSkeletonShape	*skelShape= new NL3D::CSkeletonShape;
		skelShape->build(boneBases);
		
		
		// save the vegetable
		NL3D::CShapeStream	ss;
		ss.setShapePointer(skelShape);
		ss.serial(f);
		delete skelShape;
	}
	catch(const NLMISC::EStream &)
	{
		MessageBox(_T("Failed to save file!"));
		return false;
	}

	return true;
}


// ***************************************************************************
void		CSkeletonScaleDlg::pushUndoState(const TBoneMirrorArray &precState, bool dirtSave)
{
	// **** test if real change
	nlassert(precState.size()==_Bones.size());
	bool	change= false;
	for(uint i=0;i<_Bones.size();i++)
	{
		if( _Bones[i].BoneScale!=precState[i].BoneScale ||
			_Bones[i].SkinScale!=precState[i].SkinScale || 
			_Bones[i].Selected!=precState[i].Selected)
		{
			change= true;
			break;
		}
	}
	// no change? no op
	if(!change)
		return;
	

	// **** then bkup for undo
	// change => the redo list is lost
	_RedoQueue.clear();

	// if not enough space, the last undo is lost
	if(_UndoQueue.size()==MaxUndoRedo)
		_UndoQueue.pop_front();

	// add the precedent state to the undo queue
	_UndoQueue.push_back(precState);

	// refresh buttons
	refreshUndoRedoView();

	// refresh save button
	if(dirtSave && !_SaveDirty)
	{
		_SaveDirty= true;
		refreshSaveButton();
	}
}

// ***************************************************************************
void		CSkeletonScaleDlg::undo()
{
	nlassert(_UndoQueue.size()+_RedoQueue.size()<=MaxUndoRedo);

	// is some undoable
	if(_UndoQueue.size())
	{
		// current goes into the redo queue
		_RedoQueue.push_front(_Bones);
		// restore
		_Bones= _UndoQueue.back();
		// pop
		_UndoQueue.pop_back();

		// refresh display
		applyMirrorToSkeleton();
		refreshTextViews();
		applySelectionToView();

		// refresh buttons
		refreshUndoRedoView();

		// change => must save
		_SaveDirty= true;
		refreshSaveButton();
	}
}

// ***************************************************************************
void		CSkeletonScaleDlg::redo()
{
	nlassert(_UndoQueue.size()+_RedoQueue.size()<=MaxUndoRedo);

	// is some redoable
	if(_RedoQueue.size())
	{
		// current goes into the undo queue
		_UndoQueue.push_back(_Bones);
		// restore
		_Bones= _RedoQueue.front();
		// pop
		_RedoQueue.pop_front();
		
		// refresh display
		applyMirrorToSkeleton();
		refreshTextViews();
		applySelectionToView();

		// refresh buttons
		refreshUndoRedoView();

		// change => must save
		_SaveDirty= true;
		refreshSaveButton();
	}
}


// ***************************************************************************
void		CSkeletonScaleDlg::applySelectionToView()
{
	// update list box selection according to state
	nlassert(_Bones.size()==(uint)_BoneList.GetCount());
	for(uint i=0;i<_Bones.size();i++)
	{
		_BoneList.SetSel(i, _Bones[i].Selected);
	}
	UpdateData(FALSE);
}

// ***************************************************************************
void		CSkeletonScaleDlg::refreshUndoRedoView()
{
	CWnd	*wnd;
	wnd= GetDlgItem(IDC_SSD_BUTTON_UNDO);
	if(wnd)
		wnd->EnableWindow(!_UndoQueue.empty());
	wnd= GetDlgItem(IDC_SSD_BUTTON_REDO);
	if(wnd)
		wnd->EnableWindow(!_RedoQueue.empty());
}


// ***************************************************************************
void		CSkeletonScaleDlg::refreshSaveButton()
{
	// SaveAs is always available
	CWnd	*wnd= GetDlgItem(IDC_SSD_BUTTON_SAVE);
	if(wnd)
		wnd->EnableWindow(_SaveDirty);
}


// ***************************************************************************
sint CSkeletonScaleDlg::getBoneForMirror(uint boneId, std::string &mirrorName)
{
	sint	side= 0;
	std::string::size_type pos;
	nlassert(_SkeletonModel && boneId<_SkeletonModel->Bones.size());
	mirrorName= _SkeletonModel->Bones[boneId].getBoneName();

	if((pos= mirrorName.find(" R "))!=std::string::npos)
	{
		side= 1;
		mirrorName[pos+1]= 'L';
	}
	else if((pos= mirrorName.find(" L "))!=std::string::npos)
	{
		side= -1;
		mirrorName[pos+1]= 'R';
	}
	
	return side;
}


// ***************************************************************************
void CSkeletonScaleDlg::OnSsdButtonMirror() 
{
	// bkup for undo
	static TBoneMirrorArray		precState;
	precState= _Bones;
	nlassert(_SkeletonModel);

	// for each bone selected
	bool	change= false;
	for(uint i=0;i<_Bones.size();i++)
	{
		CBoneMirror	&bone= _Bones[i];
		if(bone.Selected)
		{
			// get the bone side and mirrored name
			std::string		mirrorName;
			sint	side= getBoneForMirror(i, mirrorName);
			// if not a "centered" bone
			if(side!=0)
			{
				// get the bone with mirrored name
				sint	mirrorId= _SkeletonModel->getBoneIdByName(mirrorName);
				if(mirrorId<0)
				{
					nlinfo("MirrorScale: Didn't found %s", mirrorName.c_str());
				}
				else
				{
					// copy setup from the dest bone
					nlassert(mirrorId<(sint)_Bones.size());
					_Bones[mirrorId].BoneScale= bone.BoneScale;
					_Bones[mirrorId].SkinScale= bone.SkinScale;
				}
			}
		}
	}

	// refresh display
	applyMirrorToSkeleton();
	refreshTextViews();
	
	// if some change, bkup for undo/redo
	pushUndoState(precState, true);
}



// ***************************************************************************
void	CSkeletonScaleDlg::drawSelection()
{
	if(!_SkeletonModel)
		return;

	nlassert(_SkeletonModel->Bones.size()==_Bones.size());
	
	// **** Need recompute Bones bbox?
	if(_BoneBBoxNeedRecompute)
	{
		_BoneBBoxNeedRecompute= false;

		// for all bones
		for(uint i=0;i<_SkeletonModel->Bones.size();i++)
		{
			NLMISC::CAABBox		boneBBox;
			bool				empty= true;
			
			// for all instances skinned
			const std::set<NL3D::CTransform*>		&skins= _SkeletonModel->getSkins();
			std::set<NL3D::CTransform*>::const_iterator		it;
			for(it=skins.begin();it!=skins.end();it++)
			{
				NL3D::CTransform	*skin= *it;
				NLMISC::CAABBox		skinBoneBBox;
				// if the skin is skinned to this bone
				if(skin->getSkinBoneBBox(skinBoneBBox, i))
				{
					// set or enlarge the bone bbox
					if(empty)
					{
						empty= false;
						boneBBox= skinBoneBBox;
					}
					else
					{
						boneBBox= NLMISC::CAABBox::computeAABBoxUnion(boneBBox, skinBoneBBox);
					}
				}
			}
			
			// if there is no skin influence on this bone, still display a tiny bbox, to see the scale
			const	float	defSize= 0.05f;
			if(empty)
			{
				boneBBox.setCenter(NLMISC::CVector::Null);
				boneBBox.setSize(NLMISC::CVector(defSize, defSize, defSize));
			}
			
			// assign
			_BoneBBoxes[i]= boneBBox;
		}
	}


	// **** Draw each selected bone
	for(uint i=0;i<_SkeletonModel->Bones.size();i++)
	{
		// if bone not selected, skip
		if(!_Bones[i].Selected)
			continue;

		// get the bone "Skin Matrix"
		NL3D::CBone		&bone= _SkeletonModel->Bones[i];
		// force compute of this bone
		if(!_SkeletonModel->isBoneComputed(i))
			_SkeletonModel->forceComputeBone(i);
		NLMISC::CMatrix		worldSkinMat= bone.getWorldMatrix();
		// scale with skin scale, because the localskeleton and world matrix do not have this scale
		worldSkinMat.scale(bone.getSkinScale());

		// Transform the local bbox in its associated matrix
		NLMISC::CMatrix		matBBox;
		NLMISC::CAABBox		bbox= _BoneBBoxes[i];
		matBBox.setPos(bbox.getCenter());
		matBBox.setRot(
			NLMISC::CVector::I * bbox.getSize().x, 
			NLMISC::CVector::J * bbox.getSize().y, 
			NLMISC::CVector::K * bbox.getSize().z);
		NLMISC::CMatrix		finalMat;
		finalMat.setMulMatrixNoProj(worldSkinMat, matBBox);

		//Draw a wired bbox with this bone
		NLMISC::CVector		corner= finalMat.getPos() - finalMat.getI()/2 - finalMat.getJ()/2 - finalMat.getK()/2;
		NL3D::IDriver		*driver= NL3D::CNELU::Driver;
		driver->setupModelMatrix(NLMISC::CMatrix::Identity);
		NL3D::CDRU::drawWiredBox(corner, finalMat.getI(), finalMat.getJ(), finalMat.getK(), NLMISC::CRGBA::White, *driver);
	}
}


// ***************************************************************************
void CSkeletonScaleDlg::OnSsdButtonSaveScale() 
{
	// if no skeleton edited, quit
	if(!_SkeletonModel)
		return;
	
	// choose the file
	std::string	defaultFileName = _SkeletonFileName;
	NLMISC::strFindReplace(defaultFileName, ".skel", ".scale");

	CFileDialog fd(FALSE, _T("scale"), nlUtf8ToTStr(defaultFileName), OFN_OVERWRITEPROMPT, _T("SkelScaleFiles (*.scale)|*.scale|All Files (*.*)|*.*||"), this);
	fd.m_ofn.lpstrTitle = _T("Save As Skeleton Scale File");

	if (fd.DoModal() == IDOK)
	{
		NLMISC::COFile	f;
		if (f.open(NLMISC::tStrToUtf8(fd.GetPathName())))
		{
			saveSkelScaleInStream(f);
		}
		else
		{
			MessageBox(_T("Failed to open file for write!"));
		}
	}
}

// ***************************************************************************
void CSkeletonScaleDlg::OnSsdButtonLoadScale() 
{
	// if no skeleton edited, quit
	if(!_SkeletonModel)
		return;
	
	// choose the file
	std::string	defaultFileName= _SkeletonFileName;
	NLMISC::strFindReplace(defaultFileName, ".skel", ".scale");

	CFileDialog fd(TRUE, _T("scale"), nlUtf8ToTStr(defaultFileName), 0, _T("SkelScaleFiles (*.scale)|*.scale|All Files (*.*)|*.*||"), this);
	fd.m_ofn.lpstrTitle= _T("Load a Skeleton Scale File");

	if (fd.DoModal() == IDOK)
	{
		NLMISC::CIFile	f;
		if (f.open(NLMISC::tStrToUtf8(fd.GetPathName())))
		{
			loadSkelScaleFromStream(f);
		}
		else
		{
			MessageBox(_T("Failed to open file for read!"));
		}
	}
}


// ***************************************************************************
struct CBoneScaleInfo
{
	std::string	Name;
	NLMISC::CVector	Scale;
	NLMISC::CVector	SkinScale;

	void	serial(NLMISC::IStream &f)
	{
		sint32	ver= f.serialVersion(0);
		f.serial(Name, Scale, SkinScale);
	}
};

// ***************************************************************************
bool	CSkeletonScaleDlg::saveSkelScaleInStream(NLMISC::IStream &f)
{
	try
	{
		nlassert(_SkeletonModel);
		
		// Copies bone scales from the model 
		std::vector<CBoneScaleInfo>	boneScales;
		boneScales.resize(_SkeletonModel->Bones.size());
		for(uint i=0;i<boneScales.size();i++)
		{
			NL3D::CBone		&bone= _SkeletonModel->Bones[i];
			CBoneScaleInfo	&boneScale= boneScales[i];
			
			// get scale info from current edited skeleton
			boneScale.Name= bone.getBoneName();
			boneScale.Scale= bone.getScale();
			boneScale.SkinScale= bone.getSkinScale();
		}
		
		// save the file
		sint32	ver= f.serialVersion(0);
		f.serialCont(boneScales);
	}
	catch(const NLMISC::EStream &)
	{
		MessageBox(_T("Failed to save file!"));
		return false;
	}
	
	return true;
}

// ***************************************************************************
bool	CSkeletonScaleDlg::loadSkelScaleFromStream(NLMISC::IStream &f)
{
	try
	{
		nlassert(_SkeletonModel);
		
		// load the file
		sint32	ver= f.serialVersion(0);
		std::vector<CBoneScaleInfo>	boneScales;
		f.serialCont(boneScales);

		// apply to the current skeleton
		for(uint i=0;i<boneScales.size();i++)
		{
			sint32	boneId= _SkeletonModel->getBoneIdByName(boneScales[i].Name);
			if(boneId>=0 && boneId<(sint32)_SkeletonModel->Bones.size())
			{
				CBoneScaleInfo	&boneScale= boneScales[i];
				_SkeletonModel->Bones[boneId].setScale(boneScale.Scale);
				_SkeletonModel->Bones[boneId].setSkinScale(boneScale.SkinScale);
			}
		}

		// Bkup _Bones, for undo
		static TBoneMirrorArray		precState;
		precState= _Bones;
		
		// Then reapply to the mirror
		applySkeletonToMirror();

		// change => must save
		pushUndoState(precState, true);
		
		// and update display
		refreshTextViews();
	}
	catch(const NLMISC::EStream &)
	{
		MessageBox(_T("Failed to save file!"));
		return false;
	}
	
	return true;
}


void CSkeletonScaleDlg::OnClose() 
{
	_ObjViewer->getMainFrame()->OnWindowSkeletonScale();
}

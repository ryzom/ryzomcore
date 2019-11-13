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

#include "std_afx.h"
#include "object_viewer.h"
#include "select_string.h"
#include "slot_dlg.h"
#include <nel/misc/common.h>


#define DELTA_BLEND 0.1f
#define DELTA_TIME (getTimeIncrement ())
#define DELTA_MUL 0.1f

using namespace NLMISC;
using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CSlotDlg dialog


CSlotDlg::CSlotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSlotDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSlotDlg)
	EndBlend = 1.0f;
	Smoothness = 0.0f;
	SpeedFactor = 1.0f;
	StartBlend = 1.0f;
	ClampMode = 0;
	SkeletonWeightInverted = FALSE;
	Offset = 0;
	StartTime = 0;
	EndTime = 0;
	StartAnimTime=0.f;
	EndAnimTime=1.f;
	enable = TRUE;
	//}}AFX_DATA_INIT
	MainDlg=NULL;
}

void CSlotDlg::init (uint id, CObjectViewer* mainDlg)
{
	Id=id;
	MainDlg=mainDlg;
}

void CSlotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSlotDlg)
	DDX_Control(pDX, IDC_INVERT_SKELETON_WEIGHT, InvertSkeletonWeightCtrl);
	DDX_Control(pDX, IDC_ALIGN_BLEND, AlignBlendCtrl);
	DDX_Control(pDX, IDC_CLAMP, ClampCtrl);
	DDX_Control(pDX, IDC_SCROLLBAR, ScrollBarCtrl);
	DDX_Control(pDX, IDC_OFFSET_SPIN, OffsetSpinCtrl);
	DDX_Control(pDX, IDC_OFFSET, OffsetCtrl);
	DDX_Control(pDX, IDC_START_TIME_SPIN, StartTimeSpinCtrl);
	DDX_Control(pDX, IDC_START_BLEND_SPIN, StartBlendSpinCtrl);
	DDX_Control(pDX, IDC_SPEED_FACTOR_SPIN, SpeedFactorSpinCtrl);
	DDX_Control(pDX, IDC_SMOOTHNESS_SPIN, SmoothnessSpinCtrl);
	DDX_Control(pDX, IDC_END_TIME_SPIN, EndTimeSpinCtrl);
	DDX_Control(pDX, IDC_END_BLEND_SPIN, EndBlendSpinCtrl);
	DDX_Control(pDX, IDC_START_TIME, StartTimeCtrl);
	DDX_Control(pDX, IDC_START_BLEND, StartBlendCtrl);
	DDX_Control(pDX, IDC_SPEED_FACTOR, SpeddFactorCtrl);
	DDX_Control(pDX, IDC_SMOOTHNESS, SmoothnessCtrl);
	DDX_Control(pDX, IDC_END_TIME, EndTimeCtrl);
	DDX_Control(pDX, IDC_END_BLEND, EndBlendCtrl);
	DDX_Text(pDX, IDC_END_BLEND, EndBlend);
	DDX_Text(pDX, IDC_SMOOTHNESS, Smoothness);
	DDV_MinMaxFloat(pDX, Smoothness, 0.f, 1.f);
	DDX_Text(pDX, IDC_SPEED_FACTOR, SpeedFactor);
	DDV_MinMaxFloat(pDX, SpeedFactor, 1.e-002f, 100.f);
	DDX_Text(pDX, IDC_START_BLEND, StartBlend);
	DDV_MinMaxFloat(pDX, StartBlend, 0.f, 1.f);
	DDX_Radio(pDX, IDC_CLAMP, ClampMode);
	DDX_Check(pDX, IDC_INVERT_SKELETON_WEIGHT, SkeletonWeightInverted);
	DDX_Text(pDX, IDC_OFFSET, Offset);
	DDX_Text(pDX, IDC_START_TIME, StartTime);
	DDX_Text(pDX, IDC_END_TIME, EndTime);
	DDX_Check(pDX, IDC_ENABLE, enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSlotDlg, CDialog)
	//{{AFX_MSG_MAP(CSlotDlg)
	ON_WM_PAINT()
	ON_NOTIFY(UDN_DELTAPOS, IDC_END_BLEND_SPIN, OnDeltaposEndBlendSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_END_TIME_SPIN, OnDeltaposEndTimeSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SMOOTHNESS_SPIN, OnDeltaposSmoothnessSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPEED_FACTOR_SPIN, OnDeltaposSpeedFactorSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_START_BLEND_SPIN, OnDeltaposStartBlendSpin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_START_TIME_SPIN, OnDeltaposStartTimeSpin)
	ON_EN_CHANGE(IDC_END_BLEND, OnChangeEndBlend)
	ON_EN_CHANGE(IDC_END_TIME, OnChangeEndTime)
	ON_EN_CHANGE(IDC_SMOOTHNESS, OnChangeSmoothness)
	ON_EN_CHANGE(IDC_SPEED_FACTOR, OnChangeSpeedFactor)
	ON_EN_CHANGE(IDC_START_BLEND, OnChangeStartBlend)
	ON_EN_CHANGE(IDC_START_TIME, OnChangeStartTime)
	ON_BN_CLICKED(IDC_SET_ANIMATION, OnSetAnimation)
	ON_BN_CLICKED(IDC_SET_SKELETON, OnSetSkeleton)
	ON_EN_CHANGE(IDC_OFFSET, OnChangeOffset)
	ON_NOTIFY(UDN_DELTAPOS, IDC_OFFSET_SPIN, OnDeltaposOffsetSpin)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CLAMP, OnClamp)
	ON_BN_CLICKED(IDC_REPEAT, OnRepeat)
	ON_BN_CLICKED(IDC_DISABLE, OnDisable)
	ON_BN_CLICKED(IDC_ALIGN_BLEND, OnAlignBlend)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ENABLE, OnEnable)
	ON_BN_CLICKED(IDC_INVERT_SKELETON_WEIGHT, OnInvertSkeletonWeight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSlotDlg message handlers

BOOL CSlotDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Init the blend window
	setWindowName ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// ***************************************************************************

void CSlotDlg::OnPaint() 
{
	//CPaintDC dc(this); // device context for painting
	PAINTSTRUCT paint;
	CDC* pDc=BeginPaint(&paint);
	
	// TODO: Add your message handler code here
	// Draw the blend
	RECT rect;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&rect);
	ScreenToClient (&rect);
	Blend.OnPaint (rect, pDc, StartBlend, EndBlend, (float)StartTime, (float)EndTime, 
		Smoothness, StartAnimTime, EndAnimTime, !isEmpty());
	
	// Do not call CDialog::OnPaint() for painting messages
	EndPaint(&paint);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposEndBlendSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		EndBlend+=DELTA_BLEND;
	if (pNMUpDown->iDelta>0)
		EndBlend-=DELTA_BLEND;
	clamp (EndBlend, 0.f, 1.f);

	refresh  (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposEndTimeSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		EndTime++;
	if (pNMUpDown->iDelta>0)
		EndTime--;
	clamp (EndTime, (int)StartAnimTime, (int)EndAnimTime);
	if (EndTime<StartTime)
		StartTime=EndTime;

	refresh  (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposSmoothnessSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		Smoothness+=DELTA_BLEND;
	if (pNMUpDown->iDelta>0)
		Smoothness-=DELTA_BLEND;
	clamp (Smoothness, 0.f, 1.f);

	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposSpeedFactorSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		SpeedFactor+=DELTA_MUL;
	if (pNMUpDown->iDelta>0)
		SpeedFactor-=DELTA_MUL;
	clamp (SpeedFactor, 0.01f, 100.f);

	refresh (FALSE);

	validateTime ();
	updateScrollBar ();
}

// ***************************************************************************

void CSlotDlg::OnDeltaposStartBlendSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		StartBlend+=DELTA_BLEND;
	if (pNMUpDown->iDelta>0)
		StartBlend-=DELTA_BLEND;
	clamp (StartBlend, 0.f, 1.f);

	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposStartTimeSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;

	UpdateData ();

	if (pNMUpDown->iDelta<0)
		StartTime++;
	if (pNMUpDown->iDelta>0)
		StartTime--;
	clamp (StartTime, (int)StartAnimTime, (int)EndAnimTime);
	if (EndTime<StartTime)
		EndTime=StartTime;

	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnChangeEndBlend() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (EndBlend, 0.f, 1.f);

	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnChangeEndTime() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (EndTime, (int)StartAnimTime, (int)EndAnimTime);
	if (EndTime<StartTime)
		StartTime=EndTime;

	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnChangeSmoothness() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (Smoothness, 0.f, 1.f);

	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnChangeSpeedFactor() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (SpeedFactor, 0.01f, 100.f);

	refresh (FALSE);
	validateTime ();
	updateScrollBar ();
}

// ***************************************************************************

void CSlotDlg::OnChangeStartBlend() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (StartBlend, 0.f, 1.f);

	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnChangeStartTime() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData ();

	clamp (StartTime, (int)StartAnimTime, (int)EndAnimTime);
	if (EndTime<StartTime)
		EndTime=StartTime;

	refresh (FALSE);
	
}

// ***************************************************************************

void CSlotDlg::OnSetAnimation() 
{
	// List of string
	std::vector<std::string> vectString;

	// Build a list of string
	for (uint a=0; a<getAnimationSetPointer()->getNumAnimation (); a++)
		vectString.push_back (getAnimationSetPointer()->getAnimationName (a));

	// Select a string
	CSelectString select (vectString, "Select your animation", this, true);
	if (select.DoModal ()==IDOK)
	{
		// Set the animation
		if (select.Selection!=-1)
		{
			getSlotInformation ()->Animation = vectString[select.Selection];
			validateTime ();
			updateScrollBar ();
		}
		else
			getSlotInformation ()->Animation.clear();
	}
	refresh (TRUE);
}

// ***************************************************************************

void CSlotDlg::OnSetSkeleton() 
{
	// List of string
	std::vector<std::string> vectString;

	// Build a list of string
	for (uint s=0; s<getAnimationSetPointer()->getNumSkeletonWeight (); s++)
		vectString.push_back (getAnimationSetPointer()->getSkeletonWeightName (s));

	// Select a string
	CSelectString select (vectString, "Select your skeleton weight template", this, true);
	if (select.DoModal ()==IDOK)
	{
		// Set the animation
		if (select.Selection!=-1)
			getSlotInformation ()->Skeleton = vectString[select.Selection].c_str();
		else
			getSlotInformation ()->Skeleton.clear();

		setWindowName ();
		Invalidate ();
	}
}

// ***************************************************************************

void CSlotDlg::setWindowName ()
{
	std::string tmp = NLMISC::toString("Slot %d : ", Id);

	if (isEmpty())
		tmp += "empty";
	else
		tmp += getSlotInformation ()->Animation;

	CSlotInfo *information = getSlotInformation ();
	if (information)
	{
		std::string SkeletonName = information->Skeleton;
		if (!SkeletonName.empty())	
		{
			tmp += " (" + SkeletonName + ")";
		}
	}

	GetDlgItem(IDC_SLOT_NAME)->SetWindowText(nlUtf8ToTStr(tmp));
}

// ***************************************************************************

void CSlotDlg::OnChangeOffset() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	validateTime ();
	updateScrollBar ();

	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnDeltaposOffsetSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here

	UpdateData ();
	if (pNMUpDown->iDelta<0)
		Offset++;
	if (pNMUpDown->iDelta>0)
		Offset--;

	validateTime ();
	updateScrollBar ();

	*pResult = 0;
	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::validateTime ()
{
	bool reinit=false;
	if ((float)Offset<StartAnimTime)
	{
		StartAnimTime=(float)Offset;
		reinit=true;
	}
	if ((float)Offset+AnimationLength/SpeedFactor>EndAnimTime)
	{
		EndAnimTime=(float)Offset+AnimationLength/SpeedFactor;
		reinit=true;
	}
	if (reinit)
		MainDlg->setAnimTime (StartAnimTime, EndAnimTime);
}

// ***************************************************************************

void CSlotDlg::updateScrollBar ()
{
	// Scroll info
	SCROLLINFO info;
	memset (&info, 0, sizeof (info));

	// Fill the infos
	info.fMask=SIF_ALL;
	info.nMin=0;
	info.nMax=10000;
	if (fabs(EndAnimTime-StartAnimTime)<0.00001f)
	{
		info.nPage=1;
		info.nPos=0;
	}
	else
	{
		info.nPage=(int)(10000.f*(AnimationLength/SpeedFactor)/(EndAnimTime-StartAnimTime));
		info.nPos=(int)(10000.f*((float)Offset-StartAnimTime)/(EndAnimTime-StartAnimTime));
	}

	// Set scrollbar infos
	ScrollBarCtrl.SetScrollInfo (&info, TRUE);

	// Invalidate blend bar
	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

void CSlotDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	if (pScrollBar==GetDlgItem (IDC_SCROLLBAR))
	{
		// Only drag and drop
		if (nSBCode==SB_THUMBTRACK)
		{
			int DeltaOffset=Offset;
			UpdateData ();
			Offset=(int)((EndAnimTime-StartAnimTime)*(float)nPos/10000.f+StartAnimTime);
			DeltaOffset=Offset-DeltaOffset;
			StartTime+=DeltaOffset;
			EndTime+=DeltaOffset;
			refresh (FALSE);
			updateScrollBar ();
		}
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// ***************************************************************************

void CSlotDlg::setAnimTime (float animStart, float animEnd)
{
	StartAnimTime=animStart;
	EndAnimTime=animEnd;
	updateScrollBar ();
	computeLength ();
}

// ***************************************************************************

float CSlotDlg::getTimeIncrement ()
{
	return (EndAnimTime-StartAnimTime)/100.f;
}

// ***************************************************************************

void CSlotDlg::OnClamp() 
{
	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnRepeat() 
{
	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnDisable() 
{
	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnAlignBlend() 
{
	// Refresh data
	UpdateData ();

	// Change some of them
	StartTime=Offset;
	EndTime=(int)((float)Offset+AnimationLength/SpeedFactor);

	// Invalidate UI
	refresh (FALSE);

	// Invalidate blend bar
	RECT bar;
	GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
	ScreenToClient (&bar);
	InvalidateRect (&bar);
}

// ***************************************************************************

float CSlotDlg::getStartTime ()
{
	return (float)StartTime/MainDlg->getFrameRate ();
}

// ***************************************************************************

float CSlotDlg::getEndTime ()
{
	return (float)EndTime/MainDlg->getFrameRate ();
}

// ***************************************************************************

void CSlotDlg::computeLength ()
{
	if (getAnimationPointer())
		AnimationLength=(getAnimationPointer()->getEndTime()-getAnimationPointer()->getBeginTime())*MainDlg->getFrameRate();
}

// ***************************************************************************

void CSlotDlg::OnDestroy() 
{
	// TODO: Add your message handler code here
	CDialog::OnDestroy();
}

// ***************************************************************************

void CSlotDlg::refresh (BOOL update)
{
	CSlotInfo *slotInfo = getSlotInformation ();

	if (update)
	{
		CDialog::UpdateData (update);

		// Update from slot information
		if (slotInfo)
		{
			EndBlend = slotInfo->EndBlend;
			Smoothness = slotInfo->Smoothness;
			SpeedFactor = slotInfo->SpeedFactor;
			StartBlend = slotInfo->StartBlend;
			ClampMode = slotInfo->ClampMode;
			SkeletonWeightInverted = slotInfo->SkeletonInverted?TRUE:FALSE;
			StartTime = slotInfo->StartTime;
			Offset = slotInfo->Offset;
			EndTime = slotInfo->EndTime;
			enable = slotInfo->Enable?TRUE:FALSE;
		}
		else
		{
			EndBlend = 1;
			Smoothness = 1;
			SpeedFactor = 1;
			StartBlend = 1;
			ClampMode = 0;
			SkeletonWeightInverted = FALSE;
			StartTime = 0;
			Offset = 0;
			EndTime = 0;
			enable = TRUE;
		}

		// Compute length
		computeLength ();

		// Slot frozen
		bool frozen = (slotInfo == NULL) || (getAnimationPointer () == NULL);

		// Enable / disable windows
		OffsetCtrl.EnableWindow (!frozen);
		StartTimeCtrl.EnableWindow (!frozen);
		StartBlendCtrl.EnableWindow (!frozen);
		SpeddFactorCtrl.EnableWindow (!frozen);
		SmoothnessCtrl.EnableWindow (!frozen);
		EndTimeCtrl.EnableWindow (!frozen);
		EndBlendCtrl.EnableWindow (!frozen);
		OffsetSpinCtrl.EnableWindow (!frozen);
		StartTimeSpinCtrl.EnableWindow (!frozen);
		StartBlendSpinCtrl.EnableWindow (!frozen);
		SpeedFactorSpinCtrl.EnableWindow (!frozen);
		SmoothnessSpinCtrl.EnableWindow (!frozen);
		EndTimeSpinCtrl.EnableWindow (!frozen);
		EndBlendSpinCtrl.EnableWindow (!frozen);
		ScrollBarCtrl.EnableWindow (!frozen);
		AlignBlendCtrl.EnableWindow (!frozen);
		InvertSkeletonWeightCtrl.EnableWindow (!frozen);
		GetDlgItem (IDC_CLAMP)->EnableWindow (!frozen);
		GetDlgItem (IDC_REPEAT)->EnableWindow (!frozen);
		GetDlgItem (IDC_DISABLE)->EnableWindow (!frozen);
		setWindowName ();
		updateScrollBar ();

		RECT bar;
		GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
		ScreenToClient (&bar);
		InvalidateRect (&bar);

		CDialog::UpdateData (FALSE);
	}
	else
	{
		CDialog::UpdateData (TRUE);

		// Update from slot information
		slotInfo->EndBlend = EndBlend;
		slotInfo->Smoothness = Smoothness;
		slotInfo->SpeedFactor = SpeedFactor;
		slotInfo->StartBlend = StartBlend;
		slotInfo->ClampMode = ClampMode;
		slotInfo->SkeletonInverted = SkeletonWeightInverted?true:false;
		slotInfo->StartTime = StartTime;
		slotInfo->Offset = Offset;
		slotInfo->EndTime = EndTime;
		slotInfo->Enable = enable?true:false;

		CDialog::UpdateData (update);

		RECT bar;
		GetDlgItem (IDC_DOOMY_BLEND)->GetWindowRect (&bar);
		ScreenToClient (&bar);
		InvalidateRect (&bar);
	}
}

// ***************************************************************************

const CAnimation *CSlotDlg::getAnimationPointer () const
{
	// The animation pointer
	const CAnimation *pointer = NULL;

	// Get an instance pointer
	CInstanceInfo *instance = getInstanceInformation ();
	if (instance)
	{
		// Get a slot pointer
		CSlotInfo *slot = getSlotInformation ();
		uint animId = instance->AnimationSet.getAnimationIdByName (slot->Animation);
		if (animId != CAnimationSet::NotFound)
		{
			// Get the animation pointer
			pointer = instance->AnimationSet.getAnimation (animId);
		}
	}

	// Return the pointer
	return pointer;
}

// ***************************************************************************

const CSkeletonWeight *CSlotDlg::getSkeletonPointer () const
{
	// The skeleton pointer
	const CSkeletonWeight *pointer = NULL;

	// Get an instance pointer
	CInstanceInfo *instance = getInstanceInformation ();
	if (instance)
	{
		// Get a slot pointer
		CSlotInfo *slot = getSlotInformation ();
		uint animId = instance->AnimationSet.getSkeletonWeightIdByName (slot->Skeleton);
		if (animId != CAnimationSet::NotFound)
		{
			// Get the skeleton pointer
			pointer = instance->AnimationSet.getSkeletonWeight (animId);
		}
	}

	// Return the pointer
	return pointer;
}

// ***************************************************************************

const NL3D::CAnimationSet *CSlotDlg::getAnimationSetPointer () const
{
	// Get an instance pointer
	CInstanceInfo *instance = getInstanceInformation ();
	if (instance)
		return &instance->AnimationSet;
	else
		return NULL;
}

// ***************************************************************************

CSlotInfo *CSlotDlg::getSlotInformation () const
{
	// Get the instance
	CInstanceInfo *instance = getInstanceInformation ();
	if (instance)
	{
		return &instance->Saved.SlotInfo[Id];
	}
	else
		return NULL;
}

// ***************************************************************************

CInstanceInfo *CSlotDlg::getInstanceInformation () const
{
	if (MainDlg)
	{
		uint instance = MainDlg->getEditedObject ();
		if (instance != 0xffffffff)
			return MainDlg->getInstance (instance);
	}
	return NULL;
}

// ***************************************************************************

bool CSlotDlg::isEmpty()
{
	return (getInstanceInformation () == NULL) || (getAnimationPointer () == NULL);
}

// ***************************************************************************

void CSlotDlg::OnEnable() 
{
	refresh (FALSE);
}

// ***************************************************************************

void CSlotDlg::OnInvertSkeletonWeight() 
{
	refresh (FALSE);
}

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

// editable_range.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "editable_range.h"



#include "range_manager.h"
#include "range_selector.h"


/////////////////////////////////////////////////////////////////////////////
// CEditableRange dialog


CEditableRange::CEditableRange(const std::string &id, CParticleWorkspace::CNode *node) 
	: _Id(id), _Node(node)
{
	//{{AFX_DATA_INIT(CEditableRange)
	m_MinRange = _T("");
	m_MaxRange = _T("");
	m_Value = _T("");
	m_SliderPos = 0;
	//}}AFX_DATA_INIT
			


}


void CEditableRange::update()
{
	updateRange();
	updateValueFromReader();
}

BOOL CEditableRange::EnableWindow( BOOL bEnable)
{	
	m_ValueCtrl.EnableWindow(bEnable);
	m_SliderCtrl.EnableWindow(bEnable);
	m_UpdateValue.EnableWindow(bEnable);
	m_SelectRange.EnableWindow(bEnable);

	UpdateData(FALSE);

	return CEditAttribDlg::EnableWindow(bEnable);
}

void CEditableRange::init(uint32 x, uint32 y, CWnd *pParent)
{	
	Create(IDD_EDITABLE_RANGE, pParent);
	RECT r;
	GetClientRect(&r);
	MoveWindow(x, y, r.right, r.bottom);
	// set the slider size
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_SLIDER);	
	ShowWindow(SW_SHOW);
}


void CEditableRange::DoDataExchange(CDataExchange* pDX)
{	
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditableRange)
	DDX_Control(pDX, IDC_SLIDER, m_SliderCtrl);
	DDX_Control(pDX, IDC_VALUE, m_ValueCtrl);
	DDX_Control(pDX, IDC_UPDATE_VALUE, m_UpdateValue);
	DDX_Control(pDX, IDC_SELECT_RANGE, m_SelectRange);
	DDX_Text(pDX, IDC_MIN_RANGE, m_MinRange);
	DDX_Text(pDX, IDC_MAX_RANGE, m_MaxRange);
	DDX_Text(pDX, IDC_VALUE, m_Value);
	DDX_Slider(pDX, IDC_SLIDER, m_SliderPos);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CEditableRange, CDialog)
	//{{AFX_MSG_MAP(CEditableRange)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER, OnReleasedcaptureSlider)
	ON_BN_CLICKED(IDC_SELECT_RANGE, OnSelectRange)	
	ON_EN_SETFOCUS(IDC_VALUE, OnSetfocusValue)
	ON_BN_CLICKED(IDC_UPDATE_VALUE, OnUpdateValue)
	ON_EN_KILLFOCUS(IDC_VALUE, OnKillfocusValue)
	ON_EN_CHANGE(IDC_VALUE, OnChangeValue)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditableRange message handlers






BOOL CEditableRange::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	updateRange();
	updateValueFromReader();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEditableRange::OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	UpdateData();
	CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_SLIDER);	
	if (
		(sl->GetRangeMax() -  sl->GetRangeMin()) != 0
		)
	{
		updateValueFromSlider(m_SliderPos * 1.f / (sl->GetRangeMax() -  sl->GetRangeMin()));		
	}
	else
	{
		updateValueFromSlider(0);
	}
	*pResult = 0;
}

void CEditableRange::OnSelectRange() 
{
	selectRange();
}



void CEditableRange::OnUpdateValue() 
{
	UpdateData();	
	updateValueFromText();
}


void CEditableRange::emptyDialog(void)
{
	m_Value = CString("");
	m_SliderPos = 0;
	UpdateData(FALSE);
}

void CEditableRange::OnSetfocusValue() 
{
	CEdit *ce = (CEdit *) GetDlgItem(IDC_VALUE);
	ce->PostMessage(EM_SETSEL, 0, -1);	
	ce->Invalidate();
}


void CEditableRange::OnKillfocusValue() 
{
	// When kill Focus from the edit text, update the value.
	/*UpdateData();	
	updateValueFromText();		*/
}


static	void concatEdit2Lines(CEdit &edit)
{
	const	uint lineLen= 1000;
	uint	n;
	// retrieve the 2 lines.
	char	tmp0[2*lineLen];
	char	tmp1[lineLen];
	n= edit.GetLine(0, tmp0, lineLen);	tmp0[n]= 0;
	n= edit.GetLine(1, tmp1, lineLen);	tmp1[n]= 0;
	// concat and update the CEdit.
	edit.SetWindowText(strcat(tmp0, tmp1));
}


void CEditableRange::OnChangeValue() 
{
	UpdateData();	
	// Trick to track "Enter" keypress: CEdit are multiline. If GetLineCount()>1, then
	// user has press enter.
	if(m_ValueCtrl.GetLineCount()>1)
	{
		// must ccat 2 lines of the CEdit.
		concatEdit2Lines(m_ValueCtrl);
		m_ValueCtrl.GetWindowText(m_Value);
		updateValueFromText();		
	}
}

void CEditableRange::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{	
	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK || nSBCode == SB_LINERIGHT || nSBCode == SB_LINELEFT)
	{
		UpdateData(TRUE);
		if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
		{		
			m_SliderPos = nPos;
			UpdateData(FALSE);
		}

		CSliderCtrl *sl = (CSliderCtrl *) GetDlgItem(IDC_SLIDER);	
		if (
			(sl->GetRangeMax() -  sl->GetRangeMin()) != 0
		   )
		{
			updateValueFromSlider(m_SliderPos * 1.f / (sl->GetRangeMax() -  sl->GetRangeMin()));		
		}
		else
		{
			updateValueFromSlider(0);
		}

		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}		
}

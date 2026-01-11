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
#include "color_edit.h"
#include "color_button.h"


//==================================================================================
CColorEdit::CColorEdit(CWnd* pParent /*=NULL*/)	
{
	// we don't use the first parameter here, it is for template compatibility with CEditAttribDlg

	//{{AFX_DATA_INIT(CColorEdit)
	//}}AFX_DATA_INIT

	m_BlueEditCtrl.setListener(this);
	m_AlphaEditCtrl.setListener(this);
	m_GreenEditCtrl.setListener(this);
	m_RedEditCtrl.setListener(this);

	m_BlueEditCtrl.setType(CEditEx::UIntType);
	m_AlphaEditCtrl.setType(CEditEx::UIntType);
	m_GreenEditCtrl.setType(CEditEx::UIntType);
	m_RedEditCtrl.setType(CEditEx::UIntType);
}


//==================================================================================
void CColorEdit::init(uint32 x, uint32 y, CWnd *pParent)
{
	Create(IDD_COLOR_EDIT, pParent);
	RECT r;
	GetClientRect(&r);

	m_RedCtrl.SetScrollRange(0, 255);
	m_GreenCtrl.SetScrollRange(0, 255);
	m_BlueCtrl.SetScrollRange(0, 255);
	m_AlphaCtrl.SetScrollRange(0, 255);

	MoveWindow(x, y, r.right, r.bottom);	
	updateColorFromReader();
	ShowWindow(SW_SHOW);	
	UpdateData(FALSE);
}


//==================================================================================
void CColorEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorEdit)
	DDX_Control(pDX, IDC_BLUE_EDIT, m_BlueEditCtrl);
	DDX_Control(pDX, IDC_ALPHA_EDIT, m_AlphaEditCtrl);
	DDX_Control(pDX, IDC_GREEN_EDIT, m_GreenEditCtrl);
	DDX_Control(pDX, IDC_RED_EDIT, m_RedEditCtrl);
	DDX_Control(pDX, IDC_ALPHA_AMOUNT, m_AlphaCtrl);
	DDX_Control(pDX, IDC_GREEN_AMOUNT, m_GreenCtrl);
	DDX_Control(pDX, IDC_BLUE_AMOUNT, m_BlueCtrl);
	DDX_Control(pDX, IDC_RED_AMOUNT, m_RedCtrl);
	DDX_Control(pDX, IDC_PARTICLE_COLOR, m_Color);
	//}}AFX_DATA_MAP
}

//==================================================================================
void CColorEdit::updateEdits()
{
	NLMISC::CRGBA col = _Wrapper->get();
	m_RedEditCtrl.setUInt((uint8) col.R);
	m_GreenEditCtrl.setUInt((uint8) col.G);
	m_BlueEditCtrl.setUInt((uint8) col.B);
	m_AlphaEditCtrl.setUInt((uint8) col.A);
}

//==================================================================================
void CColorEdit::updateColorFromReader(void)
{
	if (_Wrapper)
	{
		CRGBA col = _Wrapper->get();
		m_RedCtrl.SetScrollPos(col.R);
		m_GreenCtrl.SetScrollPos(col.G);
		m_BlueCtrl.SetScrollPos(col.B);
		m_AlphaCtrl.SetScrollPos(col.A);
		m_Color.setColor(col);
		updateEdits();
		UpdateData(FALSE);	
	}
}



BEGIN_MESSAGE_MAP(CColorEdit, CDialog)
	//{{AFX_MSG_MAP(CColorEdit)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BROWSE_COLOR, OnBrowseColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//==================================================================================
void CColorEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		UpdateData(TRUE);
		nlassert(_Wrapper);

		CRGBA col = _Wrapper->get();

		if (pScrollBar == &m_RedCtrl)
		{
			col.R = nPos;
			m_RedCtrl.SetScrollPos(nPos);
		}
		else
		if (pScrollBar == &m_GreenCtrl)
		{
			col.G = nPos;
			m_GreenCtrl.SetScrollPos(nPos);
		}
		else
		if (pScrollBar == &m_BlueCtrl)
		{
			col.B = nPos;
			m_BlueCtrl.SetScrollPos(nPos);
		}	
		if (pScrollBar == &m_AlphaCtrl)
		{
			col.A = nPos;
			m_AlphaCtrl.SetScrollPos(nPos);
		}	
		
		m_Color.setColor(col);
		_Wrapper->setAndUpdateModifiedFlag(col);
		updateEdits();
		UpdateData(FALSE);
		CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	}	
}


//==================================================================================
void CColorEdit::OnBrowseColor() 
{
	static COLORREF colTab[16] = { 0, 0xff0000, 0x00ff00, 0xffff00, 0x0000ff, 0xff00ff, 0x00ffff, 0xffffff
								   , 0x7f7f7f, 0xff7f7f, 0x7fff7f, 0xffff7f, 0x7f7fff, 0xff7fff, 0x7fffff, 0xff7f00 };
	nlassert(_Wrapper);
	CRGBA col = _Wrapper->get();
	CHOOSECOLOR cc;
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = this->m_hWnd;
	cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;	
	cc.rgbResult = RGB(col.R, col.G, col.B);
	cc.lpCustColors = colTab;

	if (::ChooseColor(&cc) == IDOK)
	{		
		col.R = (uint8) (cc.rgbResult & 0xff);
		col.G = (uint8) ((cc.rgbResult & 0xff00) >> 8);
		col.B = (uint8) ((cc.rgbResult & 0xff0000) >> 16);
		_Wrapper->setAndUpdateModifiedFlag(col);
		updateColorFromReader();
	}
}

//==================================================================================
void CColorEdit::editExValueChanged(CEditEx *ctrl)
{
	if (ctrl == &m_BlueEditCtrl)
	{
		uint value = std::min((uint) 255, m_BlueEditCtrl.getUInt());			
		NLMISC::CRGBA oldVal = _Wrapper->get();
		oldVal.B = (uint8) value;
		_Wrapper->setAndUpdateModifiedFlag(oldVal);
		updateColorFromReader();
		return;
	}
		
	if (ctrl == &m_AlphaEditCtrl)
	{
		uint value = std::min((uint) 255, m_AlphaEditCtrl.getUInt());			
		NLMISC::CRGBA oldVal = _Wrapper->get();
		oldVal.A = (uint8) value;
		_Wrapper->setAndUpdateModifiedFlag(oldVal);
		updateColorFromReader();
		return;
	}
	
	if (ctrl == &m_GreenEditCtrl)
	{
		uint value = std::min((uint) 255, m_GreenEditCtrl.getUInt());			
		NLMISC::CRGBA oldVal = _Wrapper->get();
		oldVal.G = (uint8) value;
		_Wrapper->setAndUpdateModifiedFlag(oldVal);
		updateColorFromReader();
		return;
	}
	
	if (ctrl == &m_RedEditCtrl)
	{
		uint value = std::min((uint) 255, m_RedEditCtrl.getUInt());			
		NLMISC::CRGBA oldVal = _Wrapper->get();
		oldVal.R = (uint8) value;
		_Wrapper->setAndUpdateModifiedFlag(oldVal);
		updateColorFromReader();
		return;
	}
	nlassert(0);		
}

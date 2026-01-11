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

// list_box_color.cpp : implementation file
//

#include "stdafx.h"
#include "list_box_color.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/file.h"


const uint COLOR_BOX_SIZE = 16;
const uint COLOR_BOX_SIZE_GAP = 4;

/////////////////////////////////////////////////////////////////////////////
// CListBoxColor

//**********************************************************************************************************************
CListBoxColor::CListBoxColor()
{	
	_CheckBoxEnabled = true;
	_IconsEnabled =  true;
	_IconSize = 0;
	_IconsBitmapLoaded = false;
}

//**********************************************************************************************************************
CListBoxColor::~CListBoxColor()
{
}


BEGIN_MESSAGE_MAP(CListBoxColor, CCheckListBox)
	//{{AFX_MSG_MAP(CListBoxColor)
	ON_WM_DRAWITEM_REFLECT()
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//**********************************************************************************************************************
CRect CListBoxColor::OnGetCheckPosition( CRect rectItem, CRect rectCheckBox )
{
	CRect result;
	result.left = rectCheckBox.left;
	result.right = rectCheckBox.right;
	result.top = (rectCheckBox.top + rectCheckBox.bottom - rectCheckBox.Height()) / 2;
	result.bottom = result.top + rectCheckBox.Height();
	return result;
}

//**********************************************************************************************************************
BOOL CListBoxColor::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{	
	if (message == WM_DRAWITEM)
	{
		if (_CheckBoxEnabled)
		{
			CCheckListBox::PreDrawItem((LPDRAWITEMSTRUCT)lParam);
			return CCheckListBox::OnChildNotify(message, wParam, lParam, pResult);	
		}
		else
		{
			DrawItem((LPDRAWITEMSTRUCT)lParam);
			return TRUE;
		}		
	}	
	return CCheckListBox::OnChildNotify(message, wParam, lParam, pResult);		
}

//**********************************************************************************************************************
void CListBoxColor::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);	

	
	
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);
	CString text;
	GetText(lpDrawItemStruct->itemID, text);	

    // Save these values to restore them when done drawing.
    COLORREF crOldTextColor = dc.GetTextColor();
    COLORREF crOldBkColor = dc.GetBkColor();

    // If this item is selected, set the background color 
    // and the text color to appropriate values. Also, erase
    // rect by filling it with the background color.
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
		(lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		dc.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		dc.SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		dc.FillSolidRect(&lpDrawItemStruct->rcItem, 
		  ::GetSysColor(COLOR_HIGHLIGHT));
	}
	else
	{
		dc.FillSolidRect(&lpDrawItemStruct->rcItem, crOldBkColor);
	}

			
	DWORD gap = 4;
	lpDrawItemStruct->rcItem.left += gap;
	lpDrawItemStruct->rcItem.right += gap;	

	RECT r;

	r = lpDrawItemStruct->rcItem;
	r.left  += COLOR_BOX_SIZE + COLOR_BOX_SIZE_GAP;
	r.right += COLOR_BOX_SIZE + COLOR_BOX_SIZE_GAP;

	// if icons are enabled, draw them
	if (_IconsEnabled)
	{
		if (_IconsBitmapLoaded)
		{
			sint x, y;
			getIcon(lpDrawItemStruct->itemID, x, y);
			if (x != -1 && y !=-1)
			{
				CDC srcDC;
				srcDC.CreateCompatibleDC(&dc);
				srcDC.SelectObject(_PackedIcons);
				BLENDFUNCTION bf;
				bf.BlendOp = AC_SRC_OVER;
				bf.BlendFlags = 0;
				//bf.SourceConstantAlpha = GetCheck(lpDrawItemStruct->itemID) ? 255 : 127;
				bf.SourceConstantAlpha = 255;
				bf.AlphaFormat = 0x01; // AC_SRC_ALPHA
				BOOL res = AlphaBlend(dc.m_hDC, r.left, r.top, _IconSize, _IconSize, srcDC.m_hDC, (int) x * _IconSize, (int) y * _IconSize, _IconSize, _IconSize, bf);				
				nlassert(res != ERROR_INVALID_PARAMETER);
			}
		}
		r.left  += _IconSize + COLOR_BOX_SIZE_GAP;
		r.right += _IconSize + COLOR_BOX_SIZE_GAP;
	}

	// Draw the text.
	dc.DrawText(
    (LPCTSTR) text,
    text.GetLength(),
    &r,
    DT_LEFT|DT_SINGLELINE|DT_VCENTER);

	// Reset the background color and the text color back to their
	// original values.
	dc.SetTextColor(crOldTextColor);
	dc.SetBkColor(crOldBkColor);	
	
	r.left  = lpDrawItemStruct->rcItem.left;
	r.right = r.left + COLOR_BOX_SIZE;
	r.top  = (lpDrawItemStruct->rcItem.top + lpDrawItemStruct->rcItem.bottom - COLOR_BOX_SIZE) / 2;
	r.bottom  = (lpDrawItemStruct->rcItem.top + lpDrawItemStruct->rcItem.bottom + COLOR_BOX_SIZE) / 2;	

	// color stored in item data
	{
		CBrush b(getColor(lpDrawItemStruct->itemID));	
		dc.FillRect(&r, &b);		
	}

	{
		CBrush b(RGB(0, 0, 0));
		dc.FrameRect(&r, &b);	
	}	
	dc.Detach();	
}

//**********************************************************************************************************************
void CListBoxColor::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{			
	lpMeasureItemStruct->itemHeight = COLOR_BOX_SIZE + COLOR_BOX_SIZE_GAP;
	if (_IconsEnabled)
	{
		lpMeasureItemStruct->itemHeight = std::max((UINT) _IconSize, lpMeasureItemStruct->itemHeight);
	}
}

//**********************************************************************************************************************
void CListBoxColor::setColor(uint index, COLORREF col)
{
	nlassert(index < (uint) GetCount());
	_Colors.resize(GetCount());
	_Colors[index] = col;
}

//**********************************************************************************************************************
void CListBoxColor::setIcon(uint index, sint srcX, sint srcY)
{
	nlassert(index < (uint) GetCount());
	_Icons.resize(GetCount());
	_Icons[index].SrcX = srcX;
	_Icons[index].SrcY = srcY;
}

//**********************************************************************************************************************
void CListBoxColor::getIcon(uint index, sint &x, sint &y)
{
	nlassert(index < (uint) GetCount());
	if (index < _Icons.size())
	{
		x = _Icons[index].SrcX;
		y = _Icons[index].SrcY;
		return;
	}
	x = -1;
	y = -1;
}

//**********************************************************************************************************************
COLORREF CListBoxColor::getColor(uint index) const
{
	nlassert(index < (uint) GetCount());
	if (index < _Colors.size()) return _Colors[index];
	return 0;
}

//**********************************************************************************************************************
void CListBoxColor::setIconBitmap(const NLMISC::CBitmap &bm, uint iconSize)
{
	NLMISC::CBitmap tmpBM = bm;
	tmpBM.convertToType(NLMISC::CBitmap::RGBA);
	uint8 *firstPix  = (uint8 *) &(tmpBM.getPixels()[0]);
	const uint8 *lastPix = firstPix + bm.getWidth() * bm.getHeight() * 4;	
	// convert from rgba to bgra, & premultiply by alpha (required by windows alpha blend)
	uint8 *currPix = firstPix;
	while (currPix != lastPix)
	{
		std::swap(currPix[0], currPix[2]); // swap B & R
		currPix[0] = (uint8) (((uint16) currPix[3] * (uint16) currPix[0]) >> 8);
		currPix[1] = (uint8) (((uint16) currPix[3] * (uint16) currPix[1]) >> 8);
		currPix[2] = (uint8) (((uint16) currPix[3] * (uint16) currPix[2]) >> 8);
		currPix += 4;
	}
	// 
	_PackedIcons.CreateBitmap(tmpBM.getWidth(), tmpBM.getHeight(), 1, 32, firstPix);
	_IconsBitmapLoaded = true;
	_IconSize = iconSize;
}



/*
struct CListBoxIdent
{
	HWND Wnd;
	WPARAM CtrlID;
};
inline static bool operator < (const CListBoxIdent &lhs, const CListBoxIdent &rhs)
{
	if (lhs.Wnd != rhs.Wnd) return lhs.Wnd < rhs.Wnd;
	return lhs.CtrlID < rhs.CtrlID;
}

struct CListBoxInfo
{
	WNDPROC		  OldProc;
	CListBoxColor *ListBox;
};


static std::map<CListBoxIdent, CListBoxInfo> ListBoxMap;

// When a check box is checked the whole list box item has to be updated. Can't do it in SetCheck because it is not virtual...
static LRESULT CALLBACK catchCheckBoxsProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	CListBoxIdent ident;
	ident.Wnd = hwnd;
	ident.WAPRAM = wParam;
	nlassert(ListBoxMap.count(ident));
	CListBoxInfo &lbInfo = ListBoxMap[ident].second;
		lbInfo.ListBox->Invalidate();
	if (uMsg == CLBN_CHKCHANGE)
	{
		
	}
	return ::CallWindowProc(catchCheckBoxsProc, OldParenProcMap[hwnd], hwnd, uMsg, wParam, lParam);
}

*/

int CListBoxColor::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CCheckListBox::OnCreate(lpCreateStruct) == -1)
		return -1;
/*		
	if (GetParent())
	{		
		WNDPROC oldParentWndProc = GetWindowLong(GetParent()->m_hWnd, GWL_WNDPROC);				
		CListBoxIdent ident;
		ident.Wnd = GetParent()->m_hWnd;
		ident.CtrlId = GetCtrlID();	
		CListBoxInfo lbInfo;
		lbInfo.OldProc = oldParentWndProc;
		ListBoxColorMap[ident] = lbinfo;
		// Intercept msg from parent
		SubclassWindow(GetParent()->m_hWnd, catchCheckBoxsProc);
	}	
	*/
	return 0;
}




void CListBoxColor::OnDestroy() 
{
	/*
	std::map<HWND, WNDPROC>::iterator it = ListBoxColorMap.find(GetParent()->m_hWnd);
	if (it != ListBoxColorMap.end())
	{
		SubclassWindow((GetParent()->m_hWnd, (*it)->_OldParentWndProc);
		ListBoxColorMap.erase(*it);
	}
	*/	
	CCheckListBox::OnDestroy();		
}




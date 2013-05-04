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

#include "stdafx.h"
#include "tools_zone.h"

#include "main_frm.h"

#include <string>

using namespace std;

// ---------------------------------------------------------------------------

//#define IDC_LIST			0x10000


// ***************************************************************************
// CToolsZoneList
// ***************************************************************************

// ---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CToolsZoneList, CListBox)
	ON_WM_LBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
	ON_WM_MOUSEMOVE ()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
CToolsZoneList::CToolsZoneList()
{
	_MouseLDown = false;
}

// ---------------------------------------------------------------------------
void CToolsZoneList::OnLButtonDown	(UINT nFlags, CPoint point)
{
	CListBox::OnLButtonDown (nFlags, point);
	_MouseLDown = true;
	notifyParent();
}

// ---------------------------------------------------------------------------
void CToolsZoneList::OnLButtonUp	(UINT nFlags, CPoint point)
{
	CListBox::OnLButtonUp (nFlags, point);
	_MouseLDown = false;
}

// ---------------------------------------------------------------------------
void CToolsZoneList::OnMouseMove	(UINT nFlags, CPoint point)
{
	CListBox::OnMouseMove (nFlags, point);
	if (_MouseLDown)
		notifyParent();
}

// ---------------------------------------------------------------------------
void CToolsZoneList::setTool (CToolsZone *pTool)
{
	_Tools = pTool;
}

// ---------------------------------------------------------------------------
void CToolsZoneList::notifyParent ()
{
	int nIndex = GetCurSel();
	_Tools->OnSelChange();
}

// ---------------------------------------------------------------------------
void CToolsZoneList::setImages (std::vector<CBitmap*> &vBitmaps)
{
	_BitmapList = vBitmaps;
	for (uint32 i = 0; i < _BitmapList.size(); ++i)
	{
		BITMAP bitmap;
		_BitmapList[i]->GetBitmap (&bitmap);
		SetItemHeight (i, bitmap.bmHeight);
	}
}

// ---------------------------------------------------------------------------
void CToolsZoneList::reset()
{
	ResetContent();
	_ItemNames.clear();
}

// ---------------------------------------------------------------------------
void CToolsZoneList::addItem (const string &itemName)
{
	_ItemNames.push_back (itemName);
	InsertString (-1, itemName.c_str());
}

// ---------------------------------------------------------------------------
const string &CToolsZoneList::getItem (uint32 nIndex)
{
	return _ItemNames[nIndex];
}

// ---------------------------------------------------------------------------
void CToolsZoneList::DrawItem (LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	ASSERT(lpDrawItemStruct->CtlType == ODT_LISTBOX);
	LPCTSTR lpszText = (LPCTSTR) lpDrawItemStruct->itemData;
	if (lpszText == NULL)
		return;
	CDC dc;

	if (lpDrawItemStruct->itemID >= _BitmapList.size())
		return;

	dc.Attach (lpDrawItemStruct->hDC);

	// Draw the image
	CBitmap *p = _BitmapList[lpDrawItemStruct->itemID];
	BITMAP bitmap;
	p->GetBitmap (&bitmap);
	dc.DrawState (CPoint(lpDrawItemStruct->rcItem.left,lpDrawItemStruct->rcItem.top),
					CSize(bitmap.bmWidth, bitmap.bmHeight) , p, DSS_NORMAL);

	// Reduce the rectangle to display selection box and item text
	CRect rectLeft = lpDrawItemStruct->rcItem;
	rectLeft.left += bitmap.bmWidth;

	// Save these value to restore them when done drawing.
	COLORREF crOldTextColor = dc.GetTextColor();
	COLORREF crOldBkColor = dc.GetBkColor();

	// If this item is selected, set the background color 
	// and the text color to appropriate values. Also, erase
	// rect by filling it with the background color.
	if ((lpDrawItemStruct->itemAction | ODA_SELECT) &&
		(lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		dc.SetTextColor (::GetSysColor(COLOR_HIGHLIGHTTEXT));
		dc.SetBkColor (::GetSysColor(COLOR_HIGHLIGHT));
		dc.FillSolidRect (&rectLeft, ::GetSysColor(COLOR_HIGHLIGHT));
	}
	else
	{
		dc.FillSolidRect (&rectLeft, crOldBkColor);
	}

	// If this item has the focus, draw a red frame around the
	// item's rect.
	if ((lpDrawItemStruct->itemAction | ODA_FOCUS) &&
		(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		CBrush br(RGB(255, 0, 0));
		dc.FrameRect (&rectLeft, &br);
	}

	// Draw the text.
	dc.DrawText (lpszText, strlen(lpszText), &rectLeft, DT_CENTER|DT_SINGLELINE|DT_VCENTER);

	// Reset the background color and the text color back to their original values.
	dc.SetTextColor (crOldTextColor);
	dc.SetBkColor (crOldBkColor);

	dc.Detach ();
}

// ---------------------------------------------------------------------------
void CToolsZoneList::MeasureItem (LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (lpMeasureItemStruct->itemID >= 0)
	{
		if (lpMeasureItemStruct->itemID < _BitmapList.size())
		{
			CBitmap *p = _BitmapList[lpMeasureItemStruct->itemID];
			BITMAP b;
			p->GetBitmap (&b);
			lpMeasureItemStruct->itemHeight = b.bmHeight;
		}
	}
}

// ---------------------------------------------------------------------------
int	 CToolsZoneList::CompareItem (LPCOMPAREITEMSTRUCT)
{
	return 0;
}

// ***************************************************************************
// CToolsZone
// ***************************************************************************

// ---------------------------------------------------------------------------

IMPLEMENT_DYNCREATE (CToolsZone, CFormView)

// ---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CToolsZone, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE ()
	ON_WM_PAINT ()
	ON_CBN_SELCHANGE (IDC_CATTYPE1, OnSelectCatType1)
	ON_CBN_SELCHANGE (IDC_CATTYPE2, OnSelectCatType2)
	ON_CBN_SELCHANGE (IDC_CATTYPE3, OnSelectCatType3)
	ON_CBN_SELCHANGE (IDC_CATTYPE4, OnSelectCatType4)
	ON_CBN_SELCHANGE (IDC_CATVALUE1, OnSelectCatValue1)
	ON_CBN_SELCHANGE (IDC_CATVALUE2, OnSelectCatValue2)
	ON_CBN_SELCHANGE (IDC_CATVALUE3, OnSelectCatValue3)
	ON_CBN_SELCHANGE (IDC_CATVALUE4, OnSelectCatValue4)
	ON_BN_CLICKED (IDC_AND2, OnSelectAnd2)
	ON_BN_CLICKED (IDC_OR2, OnSelectOr2)
	ON_BN_CLICKED (IDC_AND3, OnSelectAnd3)
	ON_BN_CLICKED (IDC_OR3, OnSelectOr3)
	ON_BN_CLICKED (IDC_AND4, OnSelectAnd4)
	ON_BN_CLICKED (IDC_OR4, OnSelectOr4)
	ON_BN_CLICKED (IDC_RANDOM, OnSelectRandom)
	ON_BN_CLICKED (IDC_FULL_CYCLE, OnSelectCycle)
	ON_BN_CLICKED (IDC_NOT_PROPAGATE, OnSelectNotPropagate)
	ON_BN_CLICKED (IDC_FORCE, OnSelectForce)

	ON_BN_CLICKED (IDC_ROT0, OnSelectRot0)
	ON_BN_CLICKED (IDC_ROT90, OnSelectRot90)
	ON_BN_CLICKED (IDC_ROT180, OnSelectRot180)
	ON_BN_CLICKED (IDC_ROT270, OnSelectRot270)
	ON_BN_CLICKED (IDC_ROTRANDOM, OnSelectRotRan)
	ON_BN_CLICKED (IDC_ROTCYCLE, OnSelectRotCycle)
	ON_BN_CLICKED (IDC_FLIPNO, OnSelectFlipNo)
	ON_BN_CLICKED (IDC_FLIPYES, OnSelectFlipYes)
	ON_BN_CLICKED (IDC_FLIPRANDOM, OnSelectFlipRan)
	ON_BN_CLICKED (IDC_FLIPCYCLE, OnSelectFlipCycle)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
CToolsZone::CToolsZone() : CFormView(IDD_TOOLS_ZONE)
{
	_ListCreated = false;
}

// ---------------------------------------------------------------------------
CToolsZoneList *CToolsZone::getListCtrl()
{
	return &_List;
}

// ---------------------------------------------------------------------------
void CToolsZone::addToAllCatTypeCB (const string &Name)
{
	CComboBox* pCB;
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE1);
	pCB->AddString (Name.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE2);
	pCB->AddString (Name.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE3);
	pCB->AddString (Name.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE4);
	pCB->AddString (Name.c_str());
}

// ---------------------------------------------------------------------------
void CToolsZone::init (CMainFrame *pMF)
{
	_MainFrame = pMF;
	_MainFrame->_ZoneBuilder->setToolsZone (this);

	addToAllCatTypeCB (STRING_UNUSED);
	vector<string> allCategoryTypes;
	_MainFrame->_ZoneBuilder->getZoneBank().getCategoriesType (allCategoryTypes);
	for(uint32 i = 0; i < allCategoryTypes.size(); ++i)
		addToAllCatTypeCB (allCategoryTypes[i]);

	// Select right category types
	CComboBox* pCB;
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE1);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder->_FilterType1.c_str());	
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE2);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder->_FilterType2.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE3);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder->_FilterType3.c_str());
	pCB = (CComboBox*)GetDlgItem (IDC_CATTYPE4);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder->_FilterType4.c_str());	

	updateComboPairAndFilter (IDC_CATTYPE1, IDC_CATVALUE1, &_MainFrame->_ZoneBuilder->_FilterType1);
	pCB = (CComboBox*)GetDlgItem (IDC_CATVALUE1);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder->_FilterValue1.c_str());
	updateComboPairAndFilter (IDC_CATTYPE2, IDC_CATVALUE2, &_MainFrame->_ZoneBuilder->_FilterType2);
	pCB = (CComboBox*)GetDlgItem (IDC_CATVALUE2);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder->_FilterValue2.c_str());
	updateComboPairAndFilter (IDC_CATTYPE3, IDC_CATVALUE3, &_MainFrame->_ZoneBuilder->_FilterType3);
	pCB = (CComboBox*)GetDlgItem (IDC_CATVALUE3);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder->_FilterValue3.c_str());
	updateComboPairAndFilter (IDC_CATTYPE4, IDC_CATVALUE4, &_MainFrame->_ZoneBuilder->_FilterType4);
	pCB = (CComboBox*)GetDlgItem (IDC_CATVALUE4);
	pCB->SelectString (-1, _MainFrame->_ZoneBuilder->_FilterValue4.c_str());

	// Select right operators
	CButton *pButAnd, *pButOr;
	pButAnd = (CButton*)GetDlgItem(IDC_AND2); 
	pButOr =(CButton*)GetDlgItem(IDC_OR2);
	if (_MainFrame->_ZoneBuilder->_FilterOperator2 == 0)
		pButAnd->SetCheck (1);
	else
		pButOr->SetCheck (1);

	pButAnd = (CButton*)GetDlgItem(IDC_AND3); 
	pButOr =(CButton*)GetDlgItem(IDC_OR3);
	if (_MainFrame->_ZoneBuilder->_FilterOperator3 == 0)
		pButAnd->SetCheck (1);
	else
		pButOr->SetCheck (1);

	pButAnd = (CButton*)GetDlgItem(IDC_AND4); 
	pButOr =(CButton*)GetDlgItem(IDC_OR4);
	if (_MainFrame->_ZoneBuilder->_FilterOperator4 == 0)
		pButAnd->SetCheck (1);
	else
		pButOr->SetCheck (1);

	CButton *pButRan = (CButton*)GetDlgItem(IDC_RANDOM);
	if (_MainFrame->_ZoneBuilder->_RandomSelection)
		pButRan->SetCheck (1);
	else
		pButRan->SetCheck (0);

	CButton *pButton;
	if (_MainFrame->_ZoneBuilder->_ApplyRotType == 1) // Random
	{
		pButton = (CButton*)GetDlgItem(IDC_ROTRANDOM);
		pButton->SetCheck (1);
	}
	else if (_MainFrame->_ZoneBuilder->_ApplyRotType == 0) // Normal
	{
		switch (_MainFrame->_ZoneBuilder->_ApplyRot)
		{
			case 0: pButton = (CButton*)GetDlgItem(IDC_ROT0); break;
			case 1: pButton = (CButton*)GetDlgItem(IDC_ROT90); break;
			case 2: pButton = (CButton*)GetDlgItem(IDC_ROT180); break;
			case 3: pButton = (CButton*)GetDlgItem(IDC_ROT270); break;
		}
		pButton->SetCheck (1);
	}
	else if (_MainFrame->_ZoneBuilder->_ApplyRotType == 2) // Cycle
	{
		pButton = (CButton*)GetDlgItem(IDC_ROTCYCLE);
		pButton->SetCheck (1);
	}

	if (_MainFrame->_ZoneBuilder->_ApplyFlipType == 1) // Random
	{
		pButton = (CButton*)GetDlgItem(IDC_FLIPRANDOM);
		pButton->SetCheck (1);
	}
	else if (_MainFrame->_ZoneBuilder->_ApplyFlipType == 0) // Normal
	{
		switch (_MainFrame->_ZoneBuilder->_ApplyFlip)
		{
			case 0: pButton = (CButton*)GetDlgItem(IDC_FLIPNO); break;
			case 1: pButton = (CButton*)GetDlgItem(IDC_FLIPYES); break;
		}
		pButton->SetCheck (1);
	}
	else if (_MainFrame->_ZoneBuilder->_ApplyFlipType == 2) // Cycle
	{
		pButton = (CButton*)GetDlgItem(IDC_FLIPCYCLE);
		pButton->SetCheck (1);
	}

	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

#define LIST_TOP 170

// ---------------------------------------------------------------------------
int CToolsZone::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate (lpCreateStruct) == -1)
		return -1;
	CRect iniRect;
	GetClientRect(&iniRect);
	iniRect.top = LIST_TOP; iniRect.left = 10;
	iniRect.right -= 20; iniRect.bottom -= 20;
	_List.Create (WS_CHILD|WS_VISIBLE|WS_BORDER|WS_HSCROLL|WS_VSCROLL|LBS_OWNERDRAWVARIABLE|LBS_NOTIFY,
				iniRect, this, IDC_LIST);
	_List.setTool (this);
	_ListCreated = true;
	return 0;
}

// ---------------------------------------------------------------------------
void CToolsZone::uninit()
{
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSize (UINT nType, int cx, int cy)
{
	CFormView::OnSize (nType, cx, cy);
	// Resize list ctrl to fill the whole view.
	if (_ListCreated)
	{
		CRect iniRect;
		GetClientRect(&iniRect);
		
		iniRect.top = LIST_TOP; iniRect.left = 10;
		iniRect.right -= 10; iniRect.bottom -= 10;
		
		getListCtrl()->MoveWindow (&iniRect);
	}
}

// ---------------------------------------------------------------------------
void CToolsZone::OnPaint()
{
	getListCtrl()->ShowScrollBar(SB_VERT);
	CFormView::OnPaint();
}

// ---------------------------------------------------------------------------
void CToolsZone::updateComboPairAndFilter (int CatTypeId, int CatValueId, string *pFilterType)
{
	uint32 i;
	char sTmp[256];
	CComboBox *pCBType, *pCBValue;
	pCBType = (CComboBox*)GetDlgItem (CatTypeId);
	pCBType->GetLBText (pCBType->GetCurSel(), sTmp);
	*pFilterType = sTmp;
	pCBValue = (CComboBox*)GetDlgItem (CatValueId);
	pCBValue->ResetContent ();

	if (*pFilterType == STRING_UNUSED)
		return;

	vector<string> allCategoryValues;
	_MainFrame->_ZoneBuilder->getZoneBank().getCategoryValues (*pFilterType, allCategoryValues);
	for(i = 0; i < allCategoryValues.size(); ++i)
		pCBValue->AddString (allCategoryValues[i].c_str());
	pCBValue->SetCurSel (0);
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatType1 ()
{
	updateComboPairAndFilter (IDC_CATTYPE1, IDC_CATVALUE1, &_MainFrame->_ZoneBuilder->_FilterType1);
	OnSelectCatValue1 ();
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatType2 ()
{
	updateComboPairAndFilter (IDC_CATTYPE2, IDC_CATVALUE2, &_MainFrame->_ZoneBuilder->_FilterType2);
	OnSelectCatValue2 ();
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatType3 ()
{
	updateComboPairAndFilter (IDC_CATTYPE3, IDC_CATVALUE3, &_MainFrame->_ZoneBuilder->_FilterType3);
	OnSelectCatValue3 ();
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatType4 ()
{
	updateComboPairAndFilter (IDC_CATTYPE4, IDC_CATVALUE4, &_MainFrame->_ZoneBuilder->_FilterType4);
	OnSelectCatValue4 ();
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatValue1()
{
	char sTmp[256];
	CComboBox *pCBValue = (CComboBox*)GetDlgItem (IDC_CATVALUE1);
	pCBValue->GetLBText (pCBValue->GetCurSel(), sTmp);
	_MainFrame->_ZoneBuilder->_FilterValue1 = sTmp;
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatValue2()
{
	char sTmp[256];
	CComboBox *pCBValue = (CComboBox*)GetDlgItem (IDC_CATVALUE2);
	pCBValue->GetLBText (pCBValue->GetCurSel(), sTmp);
	_MainFrame->_ZoneBuilder->_FilterValue2 = sTmp;
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatValue3()
{
	char sTmp[256];
	CComboBox *pCBValue = (CComboBox*)GetDlgItem (IDC_CATVALUE3);
	pCBValue->GetLBText (pCBValue->GetCurSel(), sTmp);
	_MainFrame->_ZoneBuilder->_FilterValue3 = sTmp;
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCatValue4()
{
	char sTmp[256];
	CComboBox *pCBValue = (CComboBox*)GetDlgItem (IDC_CATVALUE4);
	pCBValue->GetLBText (pCBValue->GetCurSel(), sTmp);
	_MainFrame->_ZoneBuilder->_FilterValue4 = sTmp;
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectAnd2 ()
{
	if (_MainFrame->_ZoneBuilder->_FilterOperator2 == 0) return;
	_MainFrame->_ZoneBuilder->_FilterOperator2 = 0; // And
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectOr2 ()
{
	if (_MainFrame->_ZoneBuilder->_FilterOperator2 == 1) return;
	_MainFrame->_ZoneBuilder->_FilterOperator2 = 1; // Or
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectAnd3 ()
{
	if (_MainFrame->_ZoneBuilder->_FilterOperator3 == 0) return;
	_MainFrame->_ZoneBuilder->_FilterOperator3 = 0; // And
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectOr3 ()
{
	if (_MainFrame->_ZoneBuilder->_FilterOperator3 == 1) return;
	_MainFrame->_ZoneBuilder->_FilterOperator3 = 1; // Or
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectAnd4 ()
{
	if (_MainFrame->_ZoneBuilder->_FilterOperator3 == 0) return;
	_MainFrame->_ZoneBuilder->_FilterOperator4 = 0; // And
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectOr4 ()
{
	if (_MainFrame->_ZoneBuilder->_FilterOperator4 == 1) return;
	_MainFrame->_ZoneBuilder->_FilterOperator4 = 1; // Or
	_MainFrame->_ZoneBuilder->updateToolsZone ();
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectRandom()
{
	_MainFrame->_ZoneBuilder->_RandomSelection = !_MainFrame->_ZoneBuilder->_RandomSelection;
	if (_MainFrame->_ZoneBuilder->_RandomSelection)
	{
		CButton *pBut = (CButton*)GetDlgItem (IDC_FULL_CYCLE);
		pBut->SetCheck (0);
		_MainFrame->_ZoneBuilder->_CycleSelection = false;
	}
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectCycle ()
{
	_MainFrame->_ZoneBuilder->_CycleSelection = !_MainFrame->_ZoneBuilder->_CycleSelection;
	if (_MainFrame->_ZoneBuilder->_CycleSelection)
	{
		CButton *pBut = (CButton*)GetDlgItem (IDC_RANDOM);
		pBut->SetCheck (0);
		_MainFrame->_ZoneBuilder->_RandomSelection = false;
	}
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectNotPropagate ()
{
	_MainFrame->_ZoneBuilder->_NotPropagate = !_MainFrame->_ZoneBuilder->_NotPropagate;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectForce ()
{
	_MainFrame->_ZoneBuilder->_Force = !_MainFrame->_ZoneBuilder->_Force;
	if (_MainFrame->_ZoneBuilder->_Force)
	{
		CButton *pBut = (CButton*)GetDlgItem (IDC_NOT_PROPAGATE);
		pBut->SetCheck (1);
		_MainFrame->_ZoneBuilder->_NotPropagate = true;
	}
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelChange ()
{
	// Select the next item of the currently selected one.
	int nIndex = _List.GetCurSel();
	if (nIndex != LB_ERR)
	{
		// Here for some reason we cant use the GetText(nIndex, str) function...
		_MainFrame->_ZoneBuilder->_CurSelectedZone = nIndex;//_List.getItem (nIndex);
	}
	else
	{
		_MainFrame->_ZoneBuilder->_CurSelectedZone = -1;//STRING_UNUSED;
	}
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectRot0 ()
{
	_MainFrame->_ZoneBuilder->_ApplyRot = 0;
	_MainFrame->_ZoneBuilder->_ApplyRotType = 0;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectRot90 ()
{
	_MainFrame->_ZoneBuilder->_ApplyRot = 1;
	_MainFrame->_ZoneBuilder->_ApplyRotType = 0;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectRot180 ()
{
	_MainFrame->_ZoneBuilder->_ApplyRot = 2;
	_MainFrame->_ZoneBuilder->_ApplyRotType = 0;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectRot270 ()
{
	_MainFrame->_ZoneBuilder->_ApplyRot = 3;
	_MainFrame->_ZoneBuilder->_ApplyRotType = 0;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectRotRan ()
{
	_MainFrame->_ZoneBuilder->_ApplyRotType = 1;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectRotCycle ()
{
	_MainFrame->_ZoneBuilder->_ApplyRotType = 2;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectFlipNo ()
{
	_MainFrame->_ZoneBuilder->_ApplyFlip = 0;
	_MainFrame->_ZoneBuilder->_ApplyFlipType = 0;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectFlipYes ()
{
	_MainFrame->_ZoneBuilder->_ApplyFlip = 1;
	_MainFrame->_ZoneBuilder->_ApplyFlipType = 0;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectFlipRan ()
{
	_MainFrame->_ZoneBuilder->_ApplyFlipType = 1;
}

// ---------------------------------------------------------------------------
void CToolsZone::OnSelectFlipCycle ()
{
	_MainFrame->_ZoneBuilder->_ApplyFlipType = 2;
}
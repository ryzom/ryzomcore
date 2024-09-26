// TMenu.cpp : implementation file
//

#include "stdafx.h"
#include "TMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTMenu

CTMenu::CTMenu()
{
	HFONT hfont = CreatePopupMenuTitleFont();
	ASSERT(hfont);
	m_Font.Attach(hfont);
}

CTMenu::~CTMenu()
{
	m_Font.DeleteObject();
}




/////////////////////////////////////////////////////////////////////////////
// CTMenu message handlers


HFONT CTMenu::CreatePopupMenuTitleFont()
{
	// start by getting the stock menu font
	HFONT hfont = (HFONT)GetStockObject(ANSI_VAR_FONT);
	if (hfont)
	{ 
	    LOGFONT lf; //get the complete LOGFONT describing this font
	    if (GetObject(hfont, sizeof(LOGFONT), &lf))
		{
			lf.lfWeight = FW_BOLD;	// set the weight to bold
			// recreate this font with just the weight changed
			return ::CreateFontIndirect(&lf);
	    }
	}
	return NULL;
}


//
// This function adds a title entry to a popup menu
//
void CTMenu::AddMenuTitle(LPCTSTR lpszTitle)
{
	// insert an empty owner-draw item at top to serve as the title
	// note: item is not selectable (disabled) but not grayed
	m_strTitle=CString(lpszTitle);
	InsertMenu(0, MF_BYPOSITION | MF_OWNERDRAW | MF_STRING | MF_DISABLED, 0);
}



void CTMenu::MeasureItem(LPMEASUREITEMSTRUCT mi)
{
	// get the screen dc to use for retrieving size information
	CDC dc;
	dc.Attach(::GetDC(NULL));
	// select the title font
	HFONT hfontOld = (HFONT)SelectObject(dc.m_hDC, (HFONT)m_Font);
	// compute the size of the title
	CSize size = dc.GetTextExtent(m_strTitle);
	// deselect the title font
	::SelectObject(dc.m_hDC, hfontOld);
	// add in the left margin for the menu item
	size.cx += GetSystemMetrics(SM_CXMENUCHECK)+8;

	//Return the width and height
	//+ include space for border
	const int nBorderSize = 2;
	mi->itemWidth = size.cx + nBorderSize;
	mi->itemHeight = size.cy + nBorderSize;
}


void CTMenu::DrawItem(LPDRAWITEMSTRUCT di)
{
	//Draw the background and a sunken rect...
	COLORREF crOldBk = ::SetBkColor(di->hDC, ::GetSysColor(COLOR_INACTIVECAPTION));
	::ExtTextOut(di->hDC, 0, 0, ETO_OPAQUE, &di->rcItem, NULL, 0, NULL);
	::DrawEdge(di->hDC, &di->rcItem, BDR_SUNKENINNER, BF_RECT);

	int modeOld = ::SetBkMode(di->hDC, TRANSPARENT);
	COLORREF crOld = ::SetTextColor(di->hDC, GetSysColor(COLOR_CAPTIONTEXT));
	// select font into the dc
	HFONT hfontOld = (HFONT)SelectObject(di->hDC, (HFONT)m_Font);

	// add the menu margin offset
	di->rcItem.left += GetSystemMetrics(SM_CXMENUCHECK)+8;
	// draw the text left aligned and vertically centered
	::DrawText(di->hDC, m_strTitle, -1, &di->rcItem, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

	//Restore font and colors...
	::SelectObject(di->hDC, hfontOld);
	::SetBkMode(di->hDC, modeOld);
	::SetBkColor(di->hDC, crOldBk);
	::SetTextColor(di->hDC, crOld);
}
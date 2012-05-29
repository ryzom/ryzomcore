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

// ColumnTreeWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ide2.h"
#include "ColumnTreeWnd.h"

#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef TVS_NOHSCROLL
#define TVS_NOHSCROLL 0x8000	// IE 5.0 or higher required
#endif



BEGIN_MESSAGE_MAP(CColumnTreeWnd, CWnd)
	//{{AFX_MSG_MAP(CColumnTreeWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_NOTIFY(HDN_ITEMCHANGED, HeaderID, OnHeaderItemChanged)
	ON_NOTIFY(NM_CUSTOMDRAW, TreeID, OnTreeCustomDraw)	
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CColumnTreeWnd::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CWnd::Create(NULL, "", WS_VISIBLE|WS_CHILD, rect, pParentWnd, nID);
}




void CColumnTreeWnd::OnPaint()
{
	// do nothing
	CPaintDC dc(this);
}

BOOL CColumnTreeWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CColumnTreeWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	UpdateScroller();
	RepositionControls();
}

void CColumnTreeWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.Width();

	int xLast = m_xPos;

	switch (nSBCode)
	{
	case SB_LINELEFT:
		m_xPos -= 15;
		break;
	case SB_LINERIGHT:
		m_xPos += 15;
		break;
	case SB_PAGELEFT:
		m_xPos -= cx;
		break;
	case SB_PAGERIGHT:
		m_xPos += cx;
		break;
	case SB_LEFT:
		m_xPos = 0;
		break;
	case SB_RIGHT:
		m_xPos = m_cxTotal - cx;
		break;
	case SB_THUMBTRACK:
		m_xPos = nPos;
		break;
	}

	if (m_xPos < 0)
		m_xPos = 0;
	else if (m_xPos > m_cxTotal - cx)
		m_xPos = m_cxTotal - cx;

	if (xLast == m_xPos)
		return;

	SetScrollPos(SB_HORZ, m_xPos);
	RepositionControls();
}


void CColumnTreeWnd::OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateColumns();

	m_Tree.Invalidate();
}

void CColumnTreeWnd::OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMCUSTOMDRAW* pNMCustomDraw = (NMCUSTOMDRAW*)pNMHDR;
	NMTVCUSTOMDRAW* pNMTVCustomDraw = (NMTVCUSTOMDRAW*)pNMHDR;

	switch (pNMCustomDraw->dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
		break;

	case CDDS_ITEMPOSTPAINT:
		{				
			HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;
			CRect rcItem = pNMCustomDraw->rc;

			if (rcItem.IsRectEmpty())
			{
				// nothing to paint
				*pResult = CDRF_DODEFAULT;
				break;
			}

			CDC dc;
			dc.Attach(pNMCustomDraw->hdc);

			CRect rcLabel;
			m_Tree.GetItemRect(hItem, &rcLabel, TRUE);

			COLORREF crTextBk = pNMTVCustomDraw->clrTextBk;
			COLORREF crWnd = GetSysColor(COLOR_WINDOW);

			// clear the original label rectangle
			dc.FillSolidRect(&rcLabel, crWnd);

			int nColsCnt = m_Header.GetItemCount();

			int i;
			if (nColsCnt > 0)
			{
				CRect rcBG = rcItem;
				for (i=0; i<nColsCnt; i++)
				{				
					rcBG.right += m_arrColWidths[i];
				}				
				rcBG.left += m_arrColWidths[0] - 1;
				dc.FillSolidRect(&rcBG, crWnd);
			}

			// draw horizontal lines...
			int xOffset = 0;
			for (i=0; i<nColsCnt; i++)
			{
				xOffset += m_arrColWidths[i];
				rcItem.right = xOffset-1;
				dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_RIGHT);
			}
			// ...and the vertical ones
			dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_BOTTOM);

			CString strText = m_Tree.GetItemText(hItem);
			CString strSub;
			AfxExtractSubString(strSub, strText, 0, '\t');

			// calculate main label's size
			CRect rcText(0,0,0,0);
			dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_CALCRECT);
			rcLabel.right = min(rcLabel.left + rcText.right + 4, m_arrColWidths[0] - 4);

			if (rcLabel.Width() < 0)
				crTextBk = crWnd;
			if (crTextBk != crWnd)	// draw label's background
				dc.FillSolidRect(&rcLabel, crTextBk);

			// draw focus rectangle if necessary
			if (pNMCustomDraw->uItemState & CDIS_FOCUS)
				dc.DrawFocusRect(&rcLabel);

			dc.SetTextColor(pNMTVCustomDraw->clrText);
			// draw main label
			rcText = rcLabel;
			rcText.DeflateRect(2, 1);
			dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_END_ELLIPSIS);

			xOffset = m_arrColWidths[0];
			dc.SetBkMode(TRANSPARENT);

			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			// draw other columns text
			for (i=1; i<nColsCnt; i++)
			{
				if (AfxExtractSubString(strSub, strText, i, '\t'))
				{
					rcText = rcLabel;
					rcText.left = xOffset;
					rcText.right = xOffset + m_arrColWidths[i];
					rcText.DeflateRect(m_xOffset, 1, 2, 1);
					dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_END_ELLIPSIS);
				}
				xOffset += m_arrColWidths[i];
			}
			

			dc.Detach();						
		}
		*pResult = CDRF_DODEFAULT;
		break;

	default:
		*pResult = CDRF_DODEFAULT;
	}
}


void CColumnTreeWnd::UpdateColumns()
{
	m_cxTotal = 0;

	HDITEM hditem;
	hditem.mask = HDI_WIDTH;
	int nCnt = m_Header.GetItemCount();
	if (nCnt > 16)
		nCnt = 16;

	// get column widths from the header control
	for (int i=0; i<nCnt; i++)
	{
		if (m_Header.GetItem(i, &hditem))
		{
			m_cxTotal += m_arrColWidths[i] = hditem.cxy;
			if (i==0)
				m_Tree.m_cxFirstCol = hditem.cxy;
		}
	}

	UpdateScroller();
	RepositionControls();
}

void CColumnTreeWnd::UpdateScroller()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.Width();

	int lx = m_xPos;

	if (m_xPos > m_cxTotal - cx)
		m_xPos = m_cxTotal - cx;
	if (m_xPos < 0)
		m_xPos = 0;

	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(scrinfo);
	scrinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	scrinfo.nPage = cx;
	scrinfo.nMin = 0;
	scrinfo.nMax = m_cxTotal;
	scrinfo.nPos = m_xPos;
	SetScrollInfo(SB_HORZ, &scrinfo);
}

void CColumnTreeWnd::RepositionControls()
{
	// reposition child controls
	if (m_Tree.m_hWnd)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		int cx = rcClient.Width();
		int cy = rcClient.Height();

		// move to a negative offset if scrolled horizontally
		int x = 0;
		if (cx < m_cxTotal)
		{
			x = GetScrollPos(SB_HORZ);
			cx += x;
		}
		m_Header.MoveWindow(-x, 0, cx, m_cyHeader);
		m_Tree.MoveWindow(-x, m_cyHeader, cx, cy-m_cyHeader);
	}
}
 


int CColumnTreeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (m_Tree.m_hWnd)
		return -1;

	// create tree and header controls as children
	m_Tree.Create(WS_CHILD | WS_VISIBLE | TVS_NOHSCROLL | TVS_NOTOOLTIPS, CRect(), this, TreeID);
	m_Header.Create(WS_CHILD | WS_VISIBLE | HDS_FULLDRAG, CRect(), this, HeaderID);

	// set correct font for the header
	CFont* pFont = m_Tree.GetFont();
	m_Header.SetFont(pFont);

	// check if the common controls library version 6.0 is available
	BOOL bIsComCtl6 = FALSE;

	HMODULE hComCtlDll = LoadLibrary("comctl32.dll");

	if (hComCtlDll)
	{
		typedef HRESULT (CALLBACK *PFNDLLGETVERSION)(DLLVERSIONINFO*);

		PFNDLLGETVERSION pfnDllGetVersion = (PFNDLLGETVERSION)GetProcAddress(hComCtlDll, "DllGetVersion");

		if (pfnDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			HRESULT hRes = (*pfnDllGetVersion)(&dvi);

			if (SUCCEEDED(hRes) && dvi.dwMajorVersion >= 6)
				bIsComCtl6 = TRUE;
		}

		FreeLibrary(hComCtlDll);
	}

	// calculate correct header's height
	CDC* pDC = GetDC();
	pDC->SelectObject(pFont);
	CSize szExt = pDC->GetTextExtent("A");
	m_cyHeader = szExt.cy + (bIsComCtl6 ? 7 : 4);
	ReleaseDC(pDC);

	// offset from column start to text start
	m_xOffset = bIsComCtl6 ? 9 : 6;

	m_xPos = 0;
	UpdateColumns();
	
	return 0;
}


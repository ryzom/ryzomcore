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

// HyperLink.h : header file
//
//
// HyperLink static control. Will open the default browser with the given URL
// when the user clicks on the link.
//
// Copyright Chris Maunder, 1997
// Feel free to use and distribute. May not be sold for profit. 

#ifndef __HYPERLINK_H__
#define __HYPERLINK_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CHyperLink window

class AFX_EXT_CLASS CHyperLink : public CStatic
{
// Construction/destruction
public:
    CHyperLink();
    virtual ~CHyperLink();

// Attributes
public:

// Operations
public:

    void SetURL(CString strURL);
    CString GetURL() const;

    void SetColours(COLORREF crLinkColour, COLORREF crVisitedColour, 
                    COLORREF crHoverColour = -1);
    COLORREF GetLinkColour() const;
    COLORREF GetVisitedColour() const;
    COLORREF GetHoverColour() const;

    void SetVisited(BOOL bVisited = TRUE);
    BOOL GetVisited() const;

    void SetLinkCursor(HCURSOR hCursor);
    HCURSOR GetLinkCursor() const;

    void SetUnderline(BOOL bUnderline = TRUE);
    BOOL GetUnderline() const;

    void SetAutoSize(BOOL bAutoSize = TRUE);
    BOOL GetAutoSize() const;

    HINSTANCE GotoURL(LPCTSTR url, int showcmd);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CHyperLink)
    public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    protected:
    virtual void PreSubclassWindow();
    //}}AFX_VIRTUAL

// Implementation
protected:
    void ReportError(int nError);
    LONG GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata);
    void PositionWindow();
    void SetDefaultCursor();

// Protected attributes
protected:
    COLORREF m_crLinkColour, m_crVisitedColour;     // Hyperlink colours
    COLORREF m_crHoverColour;                       // Hover colour
    BOOL     m_bOverControl;                        // cursor over control?
    BOOL     m_bVisited;                            // Has it been visited?
    BOOL     m_bUnderline;                          // underline hyperlink?
    BOOL     m_bAdjustToFit;                        // Adjust window size to fit text?
    CString  m_strURL;                              // hyperlink URL
    CFont    m_Font;                                // Underline font if necessary
    HCURSOR  m_hLinkCursor;                         // Cursor for hyperlink
    CToolTipCtrl m_ToolTip;                         // The tooltip

    // Generated message map functions
protected:
    //{{AFX_MSG(CHyperLink)
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    afx_msg void OnClicked();
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __HYPERLINK_H__

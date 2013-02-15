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

// icon_wnd.h: interface for the CIconWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ICON_WND_H__12893D3C_9A86_4A7A_A972_7965BDDBD2A2__INCLUDED_)
#define AFX_ICON_WND_H__12893D3C_9A86_4A7A_A972_7965BDDBD2A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/bitmap.h"

class CIconWnd : public CWnd
{
// Operations
public:
	CIconWnd();
	void create (DWORD wStyle, RECT &pos, CWnd *window, uint dialogIndex);

private:
	bool updateStr();
	void updateIcon();

	void blendIcons(NLMISC::CBitmap &dst, const NLMISC::CBitmap &src);
	void modulateIcon(NLMISC::CBitmap &dst, const NLMISC::CRGBA &col);

	bool loadIcon(const std::string &filename, NLMISC::CBitmap &bmp);
	bool getColorFromStr(const std::string &s, NLMISC::CRGBA &c);

	void addIconLayer(NLMISC::CBitmap &dst, const std::string iconStr, const std::string iconCol);
	bool updateWnd(CWnd *pWnd, std::string &str);

// Attributes
public:
	uint Id;

	// Pointer to control window
	CWnd *pWndIcon;
	CWnd *pWndIconColor;
	CWnd *pWndIconBack;
	CWnd *pWndIconBackColor;
	CWnd *pWndIconOver;
	CWnd *pWndIconOverColor;
	CWnd *pWndIconOver2;
	CWnd *pWndIconOver2Color;

private:
	// String containing window data
	std::string strIcon;
	std::string strIconColor;
	std::string strIconBack;
	std::string strIconBackColor;
	std::string strIconOver;
	std::string strIconOverColor;
	std::string strIconOver2;
	std::string strIconOver2Color;

	// Bitmap printed on window
	NLMISC::CBitmap bitmap;

	// Directory of icons
	static std::string IconPath;

// Implementation
public:
	virtual ~CIconWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CIconWnd)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_ICON_WND_H__12893D3C_9A86_4A7A_A972_7965BDDBD2A2__INCLUDED_)

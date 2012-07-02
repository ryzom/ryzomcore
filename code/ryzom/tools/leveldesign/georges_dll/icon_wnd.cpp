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

// icon_wnd.cpp: implementation of the CIconWnd class.
//

#include "stdafx.h"
#include "georges_edit.h"
#include "icon_wnd.h"

#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/bitmap.h"

using namespace NLMISC;
using namespace std;

string CIconWnd::IconPath = "";


CIconWnd::CIconWnd()
{
	pWndIcon			= NULL;
	pWndIconColor		= NULL;
	pWndIconBack		= NULL;
	pWndIconBackColor	= NULL;
	pWndIconOver		= NULL;
	pWndIconOverColor	= NULL;
	pWndIconOver2		= NULL;
	pWndIconOver2Color	= NULL;
}

CIconWnd::~CIconWnd()
{

}

BEGIN_MESSAGE_MAP(CIconWnd, CWnd)
	//{{AFX_MSG_MAP(CIconWnd)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CIconWnd::OnPaint() 
{
	CPaintDC dc(this);

	// update data
	if (updateStr())
		updateIcon();
	
	// Get client dc
	RECT client;
	GetClientRect(&client);
	dc.Rectangle(&client);

	for (uint y=0 ; y<bitmap.getHeight() ; y++)
	{
		for (uint x=0 ; x<bitmap.getWidth() ; x++)
		{
			CRGBA c = bitmap.getPixelColor(x, y);
			dc.SetPixel(x, y, RGB(c.R, c.G, c.B));
		}	
	}
}

void CIconWnd::create (DWORD wStyle, RECT &pos, CWnd *parent, uint dialogIndex)
{
	Id = dialogIndex;
	// Register window class
	LPCTSTR className = AfxRegisterWndClass(CS_OWNDC); 

	// Create this window
	CWnd::Create(className, "empty", wStyle, pos, parent, dialogIndex);
}

bool CIconWnd::updateStr()
{
	// check if icons need to be recomputed
	bool needUpdate = false;

	if (updateWnd(pWndIcon, strIcon))
		needUpdate = true;

	if (updateWnd(pWndIconColor, strIconColor))
		needUpdate = true;

	if (updateWnd(pWndIconBack, strIconBack))
		needUpdate = true;

	if (updateWnd(pWndIconBackColor, strIconBackColor))
		needUpdate = true;

	if (updateWnd(pWndIconOver, strIconOver))
		needUpdate = true;

	if (updateWnd(pWndIconOverColor, strIconOverColor))
		needUpdate = true;

	if (updateWnd(pWndIconOver2, strIconOver2))
		needUpdate = true;

	if (updateWnd(pWndIconOver2Color, strIconOver2Color))
		needUpdate = true;

	return needUpdate;
}

void CIconWnd::updateIcon()
{
	// Get IconPath
	if (IconPath == "")
	{
		CConfigFile::CVar *var = theApp.ConfigFile.getVarPtr("IconPath");
		if (var)
		{
			IconPath = var->asString();
			for (uint i=0 ; i<var->size() ; i++)
				CPath::addSearchPath(var->asString(i), true, false, NULL);
		}
		else
		{
			nlinfo("!! IconPath missing in .cfg, Please complete the config file !!");
			IconPath = "\\\\amiga\\3d\\Database\\Interfaces\\";
			nlinfo("default to : IconPath = \"\\\\amiga\\3d\\Database\\Interfaces\\\"");
			CPath::addSearchPath(IconPath, true, false, NULL);
		}
	}

	NLMISC::CBitmap icon;
	NLMISC::CRGBA color;

	bitmap.reset();
	bitmap.convertToType(NLMISC::CBitmap::RGBA);
	bitmap.resample(40, 40);

	// back icon
	if (loadIcon(strIconBack, icon))
	{
		// back icon color
		if (getColorFromStr(strIconBackColor, color))
			modulateIcon(icon, color);

		bitmap = icon;

		// base icon
		addIconLayer(bitmap, strIcon, strIconColor);
	}
	else
	{
		// base icon
		loadIcon(strIcon, icon);

		// base icon color
		if (getColorFromStr(strIconColor, color))
			modulateIcon(icon, color);

		bitmap = icon;
	}

	// overlay icon
	addIconLayer(bitmap, strIconOver, strIconOverColor);

	// overlay 2 icon
	addIconLayer(bitmap, strIconOver2, strIconOver2Color);
}

bool CIconWnd::loadIcon(const std::string &filename, NLMISC::CBitmap &bmp)
{
	// Try to get the file path
	string filepath = CPath::lookup(filename, false, false);
	if (filepath == "")
	{
		bmp.makeDummy();
		bmp.convertToType(NLMISC::CBitmap::RGBA);
		bmp.resample(40, 40);
		return false;
	}

	// load icon
	CIFile f;
	f.open(filepath);

	bmp.load(f);
	bmp.convertToType(NLMISC::CBitmap::RGBA);
	bmp.resample(40, 40);

	f.close();

	return true;
}

bool CIconWnd::getColorFromStr(const std::string &s, NLMISC::CRGBA &c)
{
	// Convert string to color

	sint r, g, b;
	if (sscanf (s.c_str(), "%d,%d,%d", &r, &g, &b) == 3)
	{
		clamp (r, 0, 255);
		clamp (g, 0, 255);
		clamp (b, 0, 255);
		c = CRGBA(r, g, b);
		return true;
	}

	return false;
}

void CIconWnd::blendIcons(NLMISC::CBitmap &dst, const NLMISC::CBitmap &src)
{
	// blend between two icons

	nlassert(dst.getWidth() == src.getWidth());
	nlassert(dst.getHeight() == src.getHeight());

	CObjectVector<uint8> &data = dst.getPixels();

	for (uint y=0 ; y<dst.getHeight() ; y++)
	{
		for (uint x=0 ; x<dst.getWidth() ; x++)
		{
			CRGBA c;
			c.blendFromui(dst.getPixelColor(x, y), src.getPixelColor(x, y), src.getPixelColor(x, y).A);
			
			data[(x+y*dst.getWidth())*4]   = c.R;
			data[(x+y*dst.getWidth())*4+1] = c.G;
			data[(x+y*dst.getWidth())*4+2] = c.B;
			data[(x+y*dst.getWidth())*4+3] = c.A;
			
		}	
	}
}

void CIconWnd::modulateIcon(NLMISC::CBitmap &dst, const NLMISC::CRGBA &col)
{
	// modulate an icon by a color

	CObjectVector<uint8> &data = dst.getPixels();

	for (uint y=0 ; y<dst.getHeight() ; y++)
	{
		for (uint x=0 ; x<dst.getWidth() ; x++)
		{
			CRGBA c;
			c.modulateFromColor(col, dst.getPixelColor(x, y));

			data[(x+y*dst.getWidth())*4]   = c.R;
			data[(x+y*dst.getWidth())*4+1] = c.G;
			data[(x+y*dst.getWidth())*4+2] = c.B;
			data[(x+y*dst.getWidth())*4+3] = dst.getPixelColor(x, y).A;
		}	
	}
}

void CIconWnd::addIconLayer(NLMISC::CBitmap &dst, const std::string iconStr, const std::string iconCol)
{
	NLMISC::CBitmap icon;
	NLMISC::CRGBA color;

	if (loadIcon(iconStr, icon))
	{
		if (getColorFromStr(iconCol, color))
			modulateIcon(icon, color);

		blendIcons(dst, icon);
	}
}

bool CIconWnd::updateWnd(CWnd *pWnd, std::string &str)
{
	char buffer[512];

	if (pWnd)
	{
		pWnd->GetWindowText(buffer, 512);
		if (buffer != str)
		{
			str = buffer;
			return true;
		}
	}

	return false;
}
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
#include "blend_wnd.h"
#include <nel/misc/common.h>
#include <nel/3d/animation_playlist.h>

using namespace NL3D;
using namespace NLMISC;

#define COLOR_BLEND_ENABLE (RGB(192, 43, 223))
#define COLOR_BLEND_DISABLE (RGB(140, 113, 142))
#define SEGMENT_COUNT 10

// ***************************************************************************

CBlendWnd::CBlendWnd()
{
}

// ***************************************************************************

void CBlendWnd::MakePoint (const RECT& src, POINT& dst, float x, float y)
{
	float widthClient=(float)src.right-(float)src.left;
	float heightClient=(float)src.bottom-(float)src.top;
	dst.x=src.left+(int)(widthClient*x);
	dst.y=src.top+(int)(heightClient*y);
}

// ***************************************************************************

void CBlendWnd::MakeRect (const RECT& src, RECT& dst, float x, float y, float width, float height)
{
	float widthClient=(float)src.right-(float)src.left;
	float heightClient=(float)src.bottom-(float)src.top;
	dst.left=src.left+(int)(widthClient*x);
	dst.top=src.top+(int)(heightClient*y);
	dst.right=src.left+(int)(widthClient*(x+width));
	dst.bottom=src.top+(int)(heightClient*(y+height));
}

// ***************************************************************************

void CBlendWnd::OnPaint (const RECT& client, CDC* pDc, float StartBlend, float EndBlend, float StartBlendTime, float EndBlendTime, 
	float Smoothness, float StartTime, float EndTime, bool enabled)
{
	// Get the good color
	COLORREF color=(enabled?COLOR_BLEND_ENABLE:COLOR_BLEND_DISABLE);

	// *** Paint the left rect

	// Offset start
	float offsetLeft=(StartBlendTime-StartTime)/(EndTime-StartTime);
	clamp (offsetLeft, 0, 1);

	// Fill the background
	pDc->FillSolidRect(&client, GetSysColor (COLOR_SCROLLBAR));

	// Make a rect for left
	RECT left;
	MakeRect (client, left, 0.f, 1.f-StartBlend, offsetLeft, StartBlend);
	pDc->FillSolidRect(&left, color);

	// *** Paint the right rect

	// Offset start
	float offsetRight=(EndBlendTime-StartTime)/(EndTime-StartTime);
	clamp (offsetRight, 0, 1);

	// Make a rect for left
	RECT right;
	MakeRect (client, right, offsetRight, 1.f-EndBlend, 1.f-offsetRight, EndBlend);
	pDc->FillSolidRect(&right, color);

	// *** Paint the inter zone

	// Set pen and brush color
    CPen myPen (PS_NULL, 0, color);
    CBrush myBrush (color);
	CPen* oldPen=NULL;
	CBrush* oldBrush=NULL;

    // Then initialize it
	oldPen=pDc->SelectObject (&myPen);
	oldBrush=pDc->SelectObject (&myBrush);

	for (uint i=0; i<SEGMENT_COUNT; i++)
	{	
		// Offset of the polygon
		float firstOffset=offsetLeft+(float)i*(offsetRight-offsetLeft)/(float)SEGMENT_COUNT;
		float nextOffset=offsetLeft+(float)(i+1)*(offsetRight-offsetLeft)/(float)SEGMENT_COUNT;

		// Get time
		float firstTime=StartBlendTime+(float)i*(EndBlendTime-StartBlendTime)/(float)SEGMENT_COUNT;
		float nextTime=StartBlendTime+(float)(i+1)*(EndBlendTime-StartBlendTime)/(float)SEGMENT_COUNT;

		// Get the values
		float firstValue=CAnimationPlaylist::getWeightValue (StartBlendTime, EndBlendTime, firstTime, StartBlend, EndBlend, Smoothness);
		float nextValue=CAnimationPlaylist::getWeightValue (StartBlendTime, EndBlendTime, nextTime, StartBlend, EndBlend, Smoothness);

		// Setup polygon points
		POINT polygon[4];
		MakePoint (client, polygon[0], firstOffset, 1.f);
		MakePoint (client, polygon[1], firstOffset, 1.f-firstValue);
		MakePoint (client, polygon[2], nextOffset, 1.f-nextValue);
		MakePoint (client, polygon[3], nextOffset, 1.f);

		// Draw the polygon
		pDc->Polygon (polygon, 4);
	}

	// Draw limit line
    CPen myBlackPen (PS_SOLID, 1, RGB(0,0,0));
	pDc->SelectObject (&myBlackPen);

	POINT p0;
	POINT p1;
	//MakePoint (client, p0, offsetLeft, 1.f-StartBlend);
	MakePoint (client, p0, offsetLeft, 0.f);
	MakePoint (client, p1, offsetLeft, 1.f);
	pDc->MoveTo (p0);
	pDc->LineTo (p1);
	//MakePoint (client, p0, offsetRight, 1.f-EndBlend);
	MakePoint (client, p0, offsetRight, 0.f);
	MakePoint (client, p1, offsetRight, 1.f);
	pDc->MoveTo (p0);
	pDc->LineTo (p1);

	// Make frame
	pDc->MoveTo (client.left, client.top);
	pDc->LineTo (client.right, client.top);
	pDc->LineTo (client.right, client.bottom);
	pDc->LineTo (client.left, client.bottom);
	pDc->LineTo (client.left, client.top);

    // Then reselect old object
	pDc->SelectObject (oldPen);
	pDc->SelectObject (oldBrush);
}

// ***************************************************************************

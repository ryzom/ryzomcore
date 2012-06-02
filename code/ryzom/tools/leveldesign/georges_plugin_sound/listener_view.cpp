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

#include "listener_view.h"
#include "sound_document_plugin.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/vector.h"
#include <mmsystem.h>
#include <math.h>

using namespace std;
using namespace NLMISC;

namespace NLGEORGES
{


#undef new
IMPLEMENT_DYNCREATE(CListenerView, CView)
#define new NL_NEW

BEGIN_MESSAGE_MAP(CListenerView, CView)
	//{{AFX_MSG_MAP(CListenerView)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

bool			CListenerView::_Registered = false;
CString			CListenerView::_WndClass;
uint			CListenerView::_WndId = 0;
CFont			CListenerView::_Font;
CBrush			CListenerView::_InnerBrush;
CBrush			CListenerView::_OuterBrush;
CBrush			CListenerView::_ListenerBrush;
CPen			CListenerView::_VolumeCurve;



// ***************************************************************************

bool CListenerView::registerClass()
{
	if (_Registered)
	{
		return true;
	}

	try
	{
		LPCTSTR className = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW), (HBRUSH) ::GetStockObject(WHITE_BRUSH));
		_WndClass = className;
	}
	catch (CResourceException* e)
	{
		  AfxMessageBox("Couldn't register class! (Already registered?)");
		  e->Delete();
		  return false;
	}


	_OuterBrush.CreateSolidBrush(RGB(255, 0, 0));
	_InnerBrush.CreateSolidBrush(RGB(0, 255, 0));
	_ListenerBrush.CreateSolidBrush(RGB(0, 0, 0));
	_VolumeCurve.CreatePen(PS_SOLID, 1, RGB(255, 0, 128)); 

	_Font.CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
					CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");

	return true;
}


// ***************************************************************************

void CListenerView::init(CSoundPlugin* plugin, CRect& rect, CWnd* parent)
{

	registerClass();

	_Plugin = plugin;
	_Listener.SetRect(-1, -1, -1, -1);
	_Cones.SetRect(0, 0, 0, 0);
	_OuterAngle = 360;
	_InnerAngle = 360;

	if (!Create((LPCTSTR) _WndClass, "Listener", WS_CHILD | WS_VISIBLE, rect, parent, ++_WndId))
	{
		throw exception("failed to create the listener view");
	}

	EnableWindow ();
	ShowWindow(SW_SHOW);
}

// ***************************************************************************

void CListenerView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (_Listener.PtInRect(point))
	{
		_DragOrigine = point;
		_ListenerOrigine = _Listener.CenterPoint();
		_Dragging = true;
	}
}

// ***************************************************************************

void CListenerView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (_Dragging)
	{
		_Dragging = false;
	}
}

// ***************************************************************************

void CListenerView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (_Dragging)
	{
		CVector pos;
		CPoint center = _ListenerOrigine + point - _DragOrigine;
		CRect updateRect = _Listener;
		_Listener.SetRect(center.x - 5, center.y - 5, center.x + 5, center.y + 5);
		//updateRect |= _Listener;
		//InvalidateRect(&updateRect);
		Invalidate();

		center = _Cones.CenterPoint();
		CPoint listener = _Listener.CenterPoint();
		
		CRect rect;
		GetClientRect(&rect);
		uint fullHeight = rect.bottom - center.y;
		uint fullWidth = rect.right - center.x; 
		
		_X =   _MaxDist * (listener.x - center.x) / fullHeight;
		_Y = (_MaxDist * (listener.y - center.y) / fullHeight);

		pos.set(_X, _Y, 0.0f);

		//nldebug("position=(%.2f, %.2f)", _X, _Y);

		_Plugin->setListenerPos(pos);
		_Plugin->setListenerOrientation(CVector(0,1,0), CVector(0,0,1));
	}
}

// ***************************************************************************

void CListenerView::OnDraw(CDC* dc)
{
	CRect rect;

	GetClientRect(&rect);

	dc->Rectangle(rect);

	// Initialize the position of the rectangle on the first call to draw
	if (_Listener.top < 0)
	{
		CPoint c = rect.CenterPoint();
		_Listener.SetRect(c.x - 5, rect.top + 25, c.x + 5, rect.top + 35);
		_Cones.SetRect(c.x - 20, rect.top + 10, c.x + 20, rect.top + 50);
	}


	if (_Activated)
	{
		CPoint center = _Cones.CenterPoint();

		CBrush *oldBrush = dc->SelectObject(&_OuterBrush);

		double angle = NLMISC::Pi * (360.0 - _OuterAngle) / 2.0 / 180.0;
		dc->Pie(_Cones, 
				CPoint((int) (center.x - 10.0 * sin(angle)), (int) (center.y - 10.0 * cos(angle))), 
				CPoint((int) (center.x + 10.0 * sin(angle)), (int) (center.y - 10.0 * cos(angle))));

		dc->SelectObject(&_InnerBrush);

		angle = NLMISC::Pi * (360.0 - _InnerAngle) / 2.0 / 180.0;
		dc->Pie(_Cones, 
				CPoint((int) (center.x - 15.0 * sin(angle)), (int) (center.y - 15.0 * cos(angle))), 
				CPoint((int) (center.x + 15.0 * sin(angle)), (int) (center.y - 15.0 * cos(angle))));

		dc->SelectObject(&_ListenerBrush);
		//dc->Ellipse(_Listener);

		CPoint pts[4];
		pts[0].x = _Listener.left;
		pts[0].y = _Listener.bottom;

		pts[1].x = _Listener.right;
		pts[1].y = _Listener.bottom;

		pts[2].x = (_Listener.left + _Listener.right) / 2;
		pts[2].y = _Listener.top;

		pts[3].x = _Listener.left;
		pts[3].y = _Listener.bottom;


		dc->Polygon(pts, 4);

		dc->SelectObject(oldBrush);

		char s[256];
		_snprintf(s, 256, "%.1f, %.1f", _X, _Y);

		CFont* oldFont = dc->SelectObject(&_Font);
		dc->DrawText(s, rect, DT_VCENTER | DT_SINGLELINE | DT_CENTER);
		dc->SelectObject(oldFont);

		CPen* oldPen = dc->SelectObject(&_VolumeCurve);
		
		// Draw the volume curve
		CPoint c = _Cones.CenterPoint();
		uint fullHeight = rect.bottom - c.y;
		uint fullWidth = rect.right - c.x; 

		// Draw the min distance
		uint minDistHeight = (uint)(((float) fullHeight * _MinDist) / _MaxDist);

		CPoint p = c;
 		dc->MoveTo(p);
		p.y += minDistHeight;
		dc->LineTo(p);

 
		if (_ShowAlpha)
		{
			// calculate the curve in 10 intervals
			double dx = (_MaxDist - _MinDist) / 100.0;
			double x = _MinDist;
			
			for (uint i = 0; i < 101; i++)
			{
				double y;

				// linearly descending volume on a dB scale
				double db1 = -100.0 * (x - _MinDist) / (_MaxDist - _MinDist);

				if (_Alpha == 0.0)
				{
					y = db1;

				}
				else if (_Alpha > 0.0)
				{
					double amp2 = 0.0001 + 0.9999 * (_MaxDist - x) / (_MaxDist - _MinDist); // linear amp between 0.00001 and 1.0
					double db2 = 20. * log10(amp2); 
					y = ((1.0 - _Alpha) * db1 + _Alpha * db2);

				}
				else if (_Alpha < 0.0)
				{
					double amp3 = _MinDist / x; // linear amplitude is 1/distance
					double db3 = 20.0 * log10(amp3); 
					y = ((1.0 + _Alpha) * db1 - _Alpha * db3);
				}

				clamp(y, -100, 0);

				p.x = c.x + (uint)(fullWidth * (y / -100.0));
				p.y = c.y + (uint)(fullHeight * (x / _MaxDist));
				dc->LineTo(p);
			
				x += dx;
			}
		}

		dc->SelectObject(oldPen);		
	}
}


#ifdef _DEBUG

// ***************************************************************************

void CListenerView::AssertValid() const
{
	CView::AssertValid();
}

// ***************************************************************************

void CListenerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

#endif //_DEBUG


} // namespace NLGEORGES


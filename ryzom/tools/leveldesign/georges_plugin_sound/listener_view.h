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


#ifndef _NLGEORGES_LISTENER_VIEW_H
#define _NLGEORGES_LISTENER_VIEW_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "std_sound_plugin.h"
#include "../georges_dll/plugin_interface.h"


namespace NLGEORGES
{

class CSoundPlugin;


class CListenerView : public CView
{
public:

	CListenerView() : CView(), _Dragging(false), _Activated(false), _X(0), _Y(0) {}
	virtual ~CListenerView() {};

	void					init(CSoundPlugin* plugin, CRect& rect, CWnd* parent);
	void					setAngles(uint32 inner, uint32 outer)				{ _InnerAngle = inner; _OuterAngle = outer; Invalidate(); }
	void					setActive(bool active)								{ _Activated = active; Invalidate(); }
	void					setShowAlpha(bool showAlpha)						{ _ShowAlpha = showAlpha; Invalidate(); }
	void					setAlpha(double alpha)								{ _Alpha = alpha; Invalidate(); }
	void					setMinMaxDistances(float mindist, float maxdist)	{ _MinDist = mindist; _MaxDist = maxdist; Invalidate(); }


protected:

	static bool				registerClass();
	static bool				_Registered;
	static CString			_WndClass;
	static uint				_WndId;
	static CFont			_Font;
	static CBrush			_InnerBrush;
	static CBrush			_OuterBrush;
	static CBrush			_ListenerBrush;
	static CPen				_VolumeCurve;

	CSoundPlugin			*_Plugin;
	CRect					_Listener;
	CRect					_Cones;
	uint32					_InnerAngle;
	uint32					_OuterAngle; 
	bool					_Dragging;
	CPoint					_DragOrigine;
	CPoint					_ListenerOrigine;
	bool					_Activated;
	bool					_ShowAlpha;
	float					_X;
	float					_Y;
	float					_MinDist;
	float					_MaxDist;
	double					_Alpha;


// MFC crap
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListenerView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	DECLARE_DYNCREATE(CListenerView)

	// Generated message map functions
	//{{AFX_MSG(CListenerView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

} // namespace NLGEORGES


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // _NLGEORGES_LISTENER_VIEW_H

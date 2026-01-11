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

#if !defined(AFX_CURVE_EDIT_H__7E8F583B_8E0D_4623_9128_B0FD23E1DC4A__INCLUDED_)
#define AFX_CURVE_EDIT_H__7E8F583B_8E0D_4623_9128_B0FD23E1DC4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif


#include "nel/3d/ps_float.h"
#include "ps_wrapper.h"
#include "popup_notify.h"
#include "editable_range.h"
#include "particle_workspace.h"


class CurveEdit : public CDialog
{
public:
	// ctor
	CurveEdit(NL3D::CPSFloatCurveFunctor *curve, CParticleWorkspace::CNode *ownerNode, IPopupNotify *pn, CWnd* pParent = NULL);
	// dtor
	~CurveEdit();

	// non modal display
	void init(CWnd *pParent);
	

// Dialog Data
	//{{AFX_DATA(CurveEdit)
	enum { IDD = IDD_CURVE_EDIT };
	BOOL	m_DisplayInterpolation;
	BOOL	m_SmoothingOn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CurveEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	public:
	NL3D::CPSFloatCurveFunctor  *Curve; // the scheme being edited
protected:
	
	typedef NL3D::CPSFloatCurveFunctor::CCtrlPoint CCtrlPoint;

	// Generated message map functions
	//{{AFX_MSG(CurveEdit)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnZoomOut();
	afx_msg void OnZoomIn();
	afx_msg void OnGoUp();
	afx_msg void OnGoDown();
	virtual BOOL OnInitDialog();
	afx_msg void OnMovePoint();
	afx_msg void OnAddPoint();
	afx_msg void OnRemovePoint();
	afx_msg void OnDisplayInterpolation();
	afx_msg void OnSmoothingOn();
	afx_msg void OnLastEqualFirst();
	afx_msg void OnCenterCurve();
	afx_msg void OnFirstEqualLast();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// convert a date / value pair to a pos in the drawing canvas
	POINT makePoint(float date, float value) const;
	/// convert from screen coordinate to a <date, value> pair
	CCtrlPoint coordsFromScreen(sint x, sint y) const;
	// test if the given pos intersect a control point
	sint intersectCtrlPoint(sint x, sint y);
	void drawBackGround(CDC &dc);
	void drawCtrlPoints(CDC &dc);
	void drawCurve(CDC &dc);
	void drawInterpolatingCurve(CDC &dc);
	void setupBoundRect(CDC &dc);
	void drawUnits(CDC &dc);
	float getSampledValue(float date) const;
	void scaleMinMax();

	sint								_X, _Y, _Width, _Height; // position and size of the drawing window	
	sint								_SelectedCtrlPoint;  //-1 means none
	enum    { Move,  Moving,
			  Create, Created,
			  Remove, Removing }		_State;	
	float								_Scale, _Origin;
	CEditableRangeUInt					*_NumSamplesDlg;
	CParticleWorkspace::CNode			*_Node;
	
	struct CNumSampleWrapper : public IPSWrapper<uint32>
	{
		CurveEdit *CE;
		uint32 get(void) const
		{
			return CE->Curve->getNumSamples();
		}
		void set(const uint32 &value)
		{
			CE->Curve->setNumSamples(value);
			CE->Invalidate();
		}
	}	_NumSampleWrapper;

	void invalidate();

	IPopupNotify *_PN; // this should be notified when this window is destroyed
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CURVE_EDIT_H__7E8F583B_8E0D_4623_9128_B0FD23E1DC4A__INCLUDED_)

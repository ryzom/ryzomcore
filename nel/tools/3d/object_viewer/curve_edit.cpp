// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//

#include "std_afx.h"
#include "object_viewer.h"
#include "curve_edit.h"
#include "editable_range.h"
#include "nel/3d/ps_float.h"
#include "nel/misc/common.h"
#include "nel/misc/fast_floor.h"

static const uint CtrlPointSize = 3;



CurveEdit::CurveEdit(NL3D::CPSFloatCurveFunctor *curve, CParticleWorkspace::CNode *ownerNode, IPopupNotify *pn, CWnd* pParent /*=NULL*/)
	: CDialog(CurveEdit::IDD, pParent), 
	  _Node(ownerNode),
	  _PN(pn),
	  Curve(curve),
	  _State(Create),
	  _X(10),
	  _Y(10),
	  _Width(350),
	  _Height(200),
	  _SelectedCtrlPoint(-1),
	  _NumSamplesDlg(NULL)

{
	nlassert(Curve);	
	//{{AFX_DATA_INIT(CurveEdit)
	m_DisplayInterpolation = FALSE;
	m_SmoothingOn = FALSE;
	//}}AFX_DATA_INIT
	scaleMinMax();	
}

CurveEdit::~CurveEdit()
{
	if (_NumSamplesDlg)
	{
		_NumSamplesDlg->DestroyWindow();
		delete _NumSamplesDlg;
	}
}


void CurveEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CurveEdit)
	DDX_Check(pDX, IDC_DISPLAY_INTERPOLATION, m_DisplayInterpolation);
	DDX_Check(pDX, IDC_SMOOTHING_ON, m_SmoothingOn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CurveEdit, CDialog)
	//{{AFX_MSG_MAP(CurveEdit)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_ZOOM_OUT, OnZoomOut)
	ON_BN_CLICKED(IDC_ZOOM_IN, OnZoomIn)
	ON_BN_CLICKED(IDC_GO_UP, OnGoUp)
	ON_BN_CLICKED(IDC_GO_DOWN, OnGoDown)
	ON_BN_CLICKED(IDC_MOVE_POINT, OnMovePoint)
	ON_BN_CLICKED(IDC_ADD_POINT, OnAddPoint)
	ON_BN_CLICKED(IDC_REMOVE_POINT, OnRemovePoint)
	ON_BN_CLICKED(IDC_DISPLAY_INTERPOLATION, OnDisplayInterpolation)
	ON_BN_CLICKED(IDC_SMOOTHING_ON, OnSmoothingOn)
	ON_BN_CLICKED(IDC_LAST_EQUAL_FIRST, OnLastEqualFirst)
	ON_BN_CLICKED(IDC_CENTER_CURVE, OnCenterCurve)
	ON_BN_CLICKED(IDC_FIRST_EQUAL_LAST, OnFirstEqualLast)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CurveEdit message handlers

void CurveEdit::OnMouseMove(UINT nFlags, CPoint point) 
{
	switch (_State)
	{
		case Move:
		case Remove:
		{
			sint cpIndex = intersectCtrlPoint(point.x, point.y);
			if (cpIndex != _SelectedCtrlPoint)
			{
				_SelectedCtrlPoint = cpIndex;	
				Invalidate();
			}			
		}
		break;
		case Moving:
		{
			Curve->setCtrlPoint(_SelectedCtrlPoint,coordsFromScreen(point.x, point.y));
			_SelectedCtrlPoint = intersectCtrlPoint(point.x, point.y); // the index of the point we are moving may have changed
			if (_SelectedCtrlPoint == -1) _State = Move;
			Invalidate();
		}
		break;		
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CurveEdit::OnLButtonUp(UINT nFlags, CPoint point) 
{
	switch (_State)
	{
		case Moving:
			_State = Move;
			_SelectedCtrlPoint = intersectCtrlPoint(point.x, point.y);
			Invalidate();
		break;
		case Removing:
			_State = Remove;
		break;
		case Created:
			_State = Create;
		break;
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CurveEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	switch (_State)
	{
		case Move:
		{
			if (_SelectedCtrlPoint != -1) _State = Moving;
		}
		break;
		case Create:
		{
			Curve->addControlPoint(coordsFromScreen(point.x, point.y));
			_State = Created;
			invalidate();
		}
		case Remove:
		{
			if ( _SelectedCtrlPoint == -1 || Curve->getNumCtrlPoints() == 2 ) return;
			Curve->removeCtrlPoint(_SelectedCtrlPoint);
			_State= Removing;
			_SelectedCtrlPoint = -1;
			invalidate();
		}
		break;
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CurveEdit::OnPaint() 
{
	UpdateData();
	CPaintDC dc(this); // device context for painting	
	drawBackGround(dc);
	drawUnits(dc);
	drawCurve(dc);
	if (m_DisplayInterpolation) drawInterpolatingCurve(dc);
	drawCtrlPoints(dc);	
}


void CurveEdit::setupBoundRect(CDC &dc)
{	
	CRgn rgn;
	rgn.CreateRectRgn(_X, _X, _X + _Width, _Y + _Height);
	dc.SelectClipRgn(&rgn, RGN_COPY  );	
}

void CurveEdit::drawBackGround(CDC &dc)
{
	setupBoundRect(dc);	
	dc.FillSolidRect(_X, _Y,  _Width, _Height, 0xffffff);
}



POINT CurveEdit::makePoint(float date, float value) const
{
	POINT p;
	p.x = (int) (_X + date * _Width);
	p.y = (int) (_Y + _Height - _Scale * _Height * (value - _Origin));
	return p;
}


sint CurveEdit::intersectCtrlPoint(sint x, sint y)
{
	uint numPoints = Curve->getNumCtrlPoints();
	for (uint k = 0; k < numPoints; ++k)
	{
		const CCtrlPoint &cp = Curve->getControlPoint(k);
		POINT p = makePoint(cp.Date, cp.Value);
		if (   x >= (sint) (p.x - CtrlPointSize) 
			&& x <= (sint) (p.x + CtrlPointSize)
			&& y >= (sint) (p.y - CtrlPointSize) 
			&& y <= (sint) (p.y + CtrlPointSize) )
		{
			return k;
		}
	}
	return -1;
}

CurveEdit::CCtrlPoint CurveEdit::coordsFromScreen(sint x, sint y) const
{
	float date =(x - _X) / (float) _Width;
	NLMISC::clamp(date, 0.f, 1.f);
	CCtrlPoint pos(
		             date,
					 _Origin + (_Y + _Height - y) / (_Scale * _Height)
				  );	
	return pos;
}


void CurveEdit::drawCurve(CDC &dc)
{
setupBoundRect(dc);	
CPen pen;
pen.CreatePen(PS_SOLID, 1, (COLORREF) 0x000000);
CGdiObject *oldPen = dc.SelectObject(&pen);

	dc.MoveTo(makePoint(0, Curve->getValue(0)));
	for (sint x = 0; x < _Width; ++x)
	{
		const float date = x / (float) _Width;
		dc.LineTo(_X + x, makePoint(date, Curve->getValue(date)).y);
	}
dc.SelectObject(oldPen);
}


void CurveEdit::drawInterpolatingCurve(CDC &dc)
{
setupBoundRect(dc);	
CPen pen;
pen.CreatePen(PS_SOLID, 1, (COLORREF) 0x772211);
CGdiObject *oldPen = dc.SelectObject(&pen);
dc.MoveTo(makePoint(0, getSampledValue(0)));
for (sint x = 0; x < _Width; ++x)
{
	const float date = x / (float) _Width;
	dc.LineTo(_X + x, makePoint(date, getSampledValue(date)).y);
}
dc.SelectObject(oldPen);
}



void CurveEdit::scaleMinMax()
{
	float minValue, maxValue;
	maxValue = minValue = getSampledValue(0);
	for (sint x = 1; x < _Width; ++x)
	{
		const float date = x / (float) _Width;
		const float value = getSampledValue(date);
		minValue = std::min(minValue, value);
		maxValue = std::max(maxValue, value);
	}
	_Origin = (maxValue == minValue) ? minValue - .5f : minValue;
	_Scale = (maxValue == minValue) ? 1.f : 1.f / (maxValue - minValue);
}




void CurveEdit::drawCtrlPoints(CDC &dc)
{
	setupBoundRect(dc);	
	CPen pens[2];
	pens[0].CreatePen(PS_SOLID, 1, (COLORREF) 0x00ff00);
	pens[1].CreatePen(PS_SOLID, 1, (COLORREF) 0x0000ff);

	uint numPoints = Curve->getNumCtrlPoints();
	for (uint k = 0; k < numPoints; ++k)
	{
		CGdiObject *oldPen = dc.SelectObject(&pens[k == (uint) _SelectedCtrlPoint ? 1 : 0]); // slect the red pen if thi ctrl point is selected
		const CCtrlPoint &cp = Curve->getControlPoint(k);
		POINT p = makePoint(cp.Date, cp.Value);
		dc.MoveTo(p.x- CtrlPointSize, p.y - CtrlPointSize);
		dc.LineTo(p.x + CtrlPointSize, p.y - CtrlPointSize);
		dc.LineTo(p.x + CtrlPointSize, p.y + CtrlPointSize);
		dc.LineTo(p.x - CtrlPointSize, p.y + CtrlPointSize);
		dc.LineTo(p.x - CtrlPointSize, p.y - CtrlPointSize);
		dc.SelectObject(oldPen);
	}
}


void CurveEdit::OnZoomOut() 
{
	_Scale *= 1.5f;
	Invalidate();	
}

void CurveEdit::OnZoomIn() 
{
	_Scale *= 0.5f;
	Invalidate();		
}

void CurveEdit::OnGoUp() 
{
	_Origin += (1.f / _Scale) * 0.25f;
	Invalidate();		
}

void CurveEdit::OnGoDown() 
{
	_Origin -= (1.f / _Scale) * 0.25f;	
	Invalidate();		
}


void CurveEdit::drawUnits(CDC &dc)
{
	CPen pens[2];
	pens[0].CreatePen(PS_SOLID, 1, (COLORREF) 0xaaaaaa);
	pens[1].CreatePen(PS_SOLID, 1, (COLORREF) 0xff0011);

	sint upVal = (sint) floorf(coordsFromScreen(0, _X).Value);
	sint downVal = (sint) floorf(coordsFromScreen(0, _X + _Width).Value);

	nlassert(upVal >= downVal);
	for (sint k = downVal ; k <= upVal ; ++k)
	{
		CGdiObject *oldPen = dc.SelectObject(&pens[k == 0 ? 1 : 0]); 
		dc.MoveTo(makePoint(0, (float) k));
		dc.LineTo(makePoint(1, (float) k));
		dc.SelectObject(oldPen);
	}
}

BOOL CurveEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	((CButton *) GetDlgItem(IDC_ADD_POINT))->SetCheck(1);
	
	_NumSamplesDlg = new CEditableRangeUInt(std::string("FLOAT_CURVE_NB_SAMPLE"), _Node, 1, 512);
	_NumSampleWrapper.CE = this;
	_NumSamplesDlg->setWrapper(&_NumSampleWrapper);
	_NumSamplesDlg->init(80, 225, this);
	_NumSamplesDlg->enableLowerBound(1, true);
	m_SmoothingOn = Curve->hasSmoothing();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CurveEdit::OnMovePoint() 
{
	_State = Move;
	_SelectedCtrlPoint = -1;
	Invalidate();
}

void CurveEdit::OnAddPoint() 
{
	_State = Create;
	_SelectedCtrlPoint = -1;
	Invalidate();	
}

void CurveEdit::OnRemovePoint() 
{
	_State = Remove;
	_SelectedCtrlPoint = -1;
	Invalidate();		
}

void CurveEdit::OnDisplayInterpolation() 
{
	Invalidate();	
}



float CurveEdit::getSampledValue(float date) const
{
	nlassert(Curve);
	nlassert(date >=0 && date < 1);
	NLMISC::OptFastFloorBegin();
	float result = (*Curve)(date);
	NLMISC::OptFastFloorEnd();
	return result;
}

void CurveEdit::OnSmoothingOn() 
{
	UpdateData();
	nlassert(Curve);
	Curve->enableSmoothing(m_SmoothingOn ? true : false /* perf; warning */);
	invalidate();
}


void CurveEdit::OnLastEqualFirst()
{
	CCtrlPoint pt = Curve->getControlPoint(0);
	pt.Date = Curve->getControlPoint(Curve->getNumCtrlPoints() - 1).Date;
	Curve->setCtrlPoint(Curve->getNumCtrlPoints() - 1, pt);
	invalidate();	
}

void CurveEdit::OnCenterCurve() 
{
	scaleMinMax();
	Invalidate();
}

void CurveEdit::OnFirstEqualLast() 
{
	CCtrlPoint pt = Curve->getControlPoint(Curve->getNumCtrlPoints() - 1);
	pt.Date = Curve->getControlPoint(0).Date;
	Curve->setCtrlPoint(0, pt);
	invalidate();	
}

void CurveEdit::OnDestroy() 
{
	CDialog::OnDestroy();	
}

void CurveEdit::init(CWnd *pParent)
{	
	CDialog::Create(IDD_CURVE_EDIT, pParent);	
	ShowWindow(SW_SHOW);
}


void CurveEdit::OnClose() 
{
	CDialog::OnClose();	
	_PN->childPopupClosed(this);
}

void CurveEdit::invalidate()
{
	if (_Node)
	{
		_Node->setModified(true);
	}
	Invalidate();
}

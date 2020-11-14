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
#include "object_viewer.h"
#include "basis_edit.h"
#include "nel/misc/matrix.h"
#include "nel/misc/vector.h"

/////////////////////////////////////////////////////////////////////////////
// CBasisEdit dialog


CBasisEdit::CBasisEdit(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(CBasisEdit)
	m_Psi = 0;
	m_Theta = 0;
	m_Phi = 0;
	//}}AFX_DATA_INIT
}


void CBasisEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBasisEdit)
	DDX_Control(pDX, IDC_PHI_SCROLLBAR, m_PhiCtrl);
	DDX_Control(pDX, IDC_PSI_SCROLLBAR, m_PsiCtrl);
	DDX_Control(pDX, IDC_THETA_SCROLLBAR, m_ThetaCtrl);
	DDX_Text(pDX, IDC_PSI_VALUE, m_Psi);
	DDX_Text(pDX, IDC_THETA_VALUE, m_Theta);
	DDX_Text(pDX, IDC_PHI_VALUE, m_Phi);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBasisEdit, CDialog)
	//{{AFX_MSG_MAP(CBasisEdit)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBasisEdit message handlers





void CBasisEdit::init(uint32 x, uint32 y, CWnd *pParent)
{
	nlassert(_Wrapper); // _Wrapper should have been set before init !!
	Create(IDD_BASIS_EDIT, pParent);
	RECT r;
	GetClientRect(&r);

	m_PsiCtrl.SetScrollRange(0, 359);
	m_ThetaCtrl.SetScrollRange(0, 359);
	m_PhiCtrl.SetScrollRange(0, 359);
	
	MoveWindow(x, y, r.right, r.bottom);	
	updateAnglesFromReader();
	ShowWindow(SW_SHOW);	
	UpdateData(FALSE);
}





// build an euler matrix
NLMISC::CMatrix  BuildEulerMatrix(float psi, float theta, float phi)
{
	float ca = cosf(psi), sa = sinf(psi)
		  , cb = cosf(theta), sb = sinf(theta)
		  , cc = cosf(phi), sc = sinf(phi);
	NLMISC::CMatrix m;
	m.identity();
	m.setRot(NLMISC::CVector(ca * cb * cc - sa * sc, -cc * sa - ca * cb *sc, ca * sb)
			 ,NLMISC::CVector(cb * cc * sa + ca * sc, ca * cc - cb * sa * sc, sa *sb)
			 ,NLMISC::CVector(-cc * sb, sb * sc, cb)
			);
	return m;
}

// get back the euler angles from a matrix
NLMISC::CVector GetEulerAngles(const NLMISC::CMatrix &mat)
{
	float m[3][3];
	// we got cos theta = m33
	NLMISC::CVector v[3];
	mat.getRot(v[0], v[1], v[2]);
	for (uint l = 0; l < 3; ++l)
	{
		m[0][l] = v[l].x; m[1][l] = v[l].y; m[2][l] = v[l].z; 
	}
	
	// there are eight triplet that may satisfy the equation
	// we compute them all, and test them against the matrix

	float b0, b1, a0, a1, a2, a3, c0, c1, c2, c3;
	b0 = acosf(m[2][2]);
	b1 = (float) NLMISC::Pi - b0;
	float sb0 = sinf(b0), sb1 = sinf(b1);
	if (fabsf(sb0) > 10E-6)
	{
		a0 = m[2][0] / sb0;
		c0 = m[1][2] / sb0;
	}
	else
	{
		a0 = c0 = 1.f;
	}
	if (fabs(sb1) > 10E-6)
	{
		a1 = m[2][0] / sb1;
		c1 = m[1][2] / sb1;
	}
	else
	{
		a1 = c1 = 1.f;
	}


	a2 = (float) NLMISC::Pi - a0;
	a3 = (float) NLMISC::Pi - a1;
	
	
	c2 = (float) NLMISC::Pi - c0;
	c3 = (float) NLMISC::Pi - c1;

	NLMISC::CVector sol[] =
	{
		NLMISC::CVector(b0, a0, c0)
		,NLMISC::CVector(b0, a2, c0)
		,NLMISC::CVector(b0, a0, c2)
		,NLMISC::CVector(b0, a2, c2)
		,NLMISC::CVector(b1, a1, c1)
		,NLMISC::CVector(b1, a3, c1)
		,NLMISC::CVector(b1, a1, c3)
		,NLMISC::CVector(b1, a3, c3)
	};

	// now we take the triplet that fit best the 6 other equations

	float bestGap = 0.f;
	uint bestIndex;

	for (uint k = 0; k < 8; ++k)
	{
		float ca = cosf(sol[k].x), sa = sinf(sol[k].x)
		  , cb = cosf(sol[k].y), sb = sinf(sol[k].y)
		  , cc = cosf(sol[k].z), sc = sinf(sol[k].z);

		float gap = fabsf(m[0][0] - ca * cb * cc + sa * sc);
		gap += fabsf(m[1][0] + cc * sa + ca * cb *sc);
		gap += fabsf(m[0][1] - cb * cc * sa - ca * sc);
		gap += fabsf(m[0][1] - cb * cc * sa - ca * sc);
		gap += fabsf(m[1][1] - ca * cc + cb * sa * sc);
		gap += fabsf(m[2][1] - sb *ca);
		gap += fabsf(m[0][2] + cc * sb);

		if (k == 0 || gap < bestGap)
		{
			bestGap = gap;
			bestIndex  = k; 

		}
	}
	return sol[bestIndex];
}



void CBasisEdit::updateAnglesFromReader()
{
	nlassert(_Wrapper); // _Wrapper should have been set before init !!
	
	// read plane basis
	NL3D::CPlaneBasis pb = _Wrapper->get();
	NLMISC::CMatrix mat;
	mat.setRot(pb.X, pb.Y, pb.X ^ pb.Y);
	NLMISC::CVector angles = GetEulerAngles(mat);
	m_PsiCtrl.SetScrollPos((uint) (360.f * angles.x / (2.f * (float) NLMISC::Pi)));
	m_ThetaCtrl.SetScrollPos((uint) (360.f * angles.y / (2.f * (float) NLMISC::Pi)));
	m_PhiCtrl.SetScrollPos((uint) (360.f * angles.z / (2.f * (float) NLMISC::Pi)));
	UpdateData(FALSE);
}





// just project a vector with orthogonal proj
static CPoint BasisVectXForm(NLMISC::CVector &v, float size)
{
	const float sq = sqrtf(2.f) / 2.f;
	return CPoint((int) (size * (sq * (v.x + v.y) )), (int) (size * (-v.z + sq * (v.x - v.y))));
}

// draw a basis using the given colors, and hte given dc

void DrawBasisInDC(const CPoint &center, float size, const NLMISC::CMatrix &m, CDC &dc, NLMISC::CRGBA col[3])
{
	// draw the basis
	CPoint px  = center + BasisVectXForm(m.getI(), size);
	CPoint py  = center + BasisVectXForm(m.getJ(), size);
	CPoint pz  = center + BasisVectXForm(m.getK(), size);


	CPen p[3];
	p[0].CreatePen(PS_SOLID, 1, col[0].R + (col[0].G << 8) + (col[0].B << 16) );
	p[1].CreatePen(PS_SOLID, 1, col[1].R + (col[1].G << 8) + (col[1].B << 16) );
	p[2].CreatePen(PS_SOLID, 1, col[2].R + (col[2].G << 8) + (col[2].B << 16) );


	

	

	// draw letters indicating each axis
	// X
	CPen *old = dc.SelectObject(&p[0]);
	dc.MoveTo(center);
	dc.LineTo(px);
	dc.MoveTo(px + CPoint(2, 2));
	dc.LineTo(px + CPoint(5, 5));
	dc.MoveTo(px + CPoint(4, 2));
	dc.LineTo(px + CPoint(3, 5));

	 
	// Y
	dc.SelectObject(&p[1]);	
	dc.MoveTo(center);
	dc.LineTo(py);
	
	dc.MoveTo(py + CPoint(2, 2));
	dc.LineTo(py + CPoint(4, 4));
	dc.MoveTo(py + CPoint(4, 2));
	dc.LineTo(py + CPoint(1, 5));

	// Z
	dc.SelectObject(&p[2]);
	dc.MoveTo(center);
	dc.LineTo(pz);

	dc.MoveTo(pz + CPoint(2, 2));
	dc.LineTo(pz + CPoint(5, 2));
	dc.MoveTo(pz + CPoint(4, 2));
	dc.LineTo(pz + CPoint(1, 5));
	dc.MoveTo(pz + CPoint(2, 4));
	dc.LineTo(pz + CPoint(5, 4));

	dc.SelectObject(old);

}

void CBasisEdit::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	NLMISC::CRGBA c1[] ={ NLMISC::CRGBA::White, NLMISC::CRGBA::White, NLMISC::CRGBA::White };
	NLMISC::CRGBA c2[] ={ NLMISC::CRGBA::Green, NLMISC::CRGBA::Green, NLMISC::CRGBA::Red };

	if (_Wrapper)
	{
		
		// read plane basis
		NL3D::CPlaneBasis pb = _Wrapper->get();

		CPoint center(20, 20);
		// draw a white box on the left				
		dc.FillSolidRect(0, 0, 39, 39, 0x777777);

	
		NLMISC::CMatrix m;
		m.identity();
		DrawBasisInDC(center, 18, m, dc, c1);
		m.setRot(pb.X, pb.Y, pb.X ^ pb.Y);
		DrawBasisInDC(center, 18, m, dc, c2);

	}
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CBasisEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	UpdateData();
	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		NLMISC::CVector angles(2.f * (float) NLMISC::Pi * m_PsiCtrl.GetScrollPos() / 360.f
					   , 2.f * (float) NLMISC::Pi * m_ThetaCtrl.GetScrollPos() / 360.f
					   , 2.f * (float) NLMISC::Pi * m_PhiCtrl.GetScrollPos() / 360.f
					  );
		if (pScrollBar == &m_PsiCtrl)
		{
			angles.x =  2.f * (float) NLMISC::Pi * nPos / 360.f;
			m_PsiCtrl.SetScrollPos(nPos);				
		}

		if (pScrollBar == &m_ThetaCtrl)
		{
			angles.y =  2.f * (float) NLMISC::Pi * nPos / 360.f;
			m_ThetaCtrl.SetScrollPos(nPos);				
		}

		if (pScrollBar == &m_PhiCtrl)
		{
			angles.z =  2.f * (float) NLMISC::Pi * nPos / 360.f;
			m_PhiCtrl.SetScrollPos(nPos);				
		}

		NLMISC::CMatrix mat = BuildEulerMatrix(angles.x, angles.y, angles.z);
		NL3D::CPlaneBasis pb;
		pb.X = mat.getI();
		pb.Y = mat.getJ();
		_Wrapper->setAndUpdateModifiedFlag(pb);
		Invalidate();
	}		
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
	UpdateData(FALSE);
}

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
#include "nel/3d/ps_particle.h"
#include "tail_particle_dlg.h"




/////////////////////////////////////////////////////////////////////////////
// CTailParticleDlg dialog


CTailParticleDlg::CTailParticleDlg(CParticleWorkspace::CNode *ownerNode, NL3D::CPSTailParticle *tp) 
{
	nlassert(tp);
	_TailParticle = tp;
	_Node = ownerNode;
	//{{AFX_DATA_INIT(CTailParticleDlg)
	m_TailFade = tp->getColorFading();	
	m_TailPersistAfterDeath = FALSE;
	//}}AFX_DATA_INIT
}


void CTailParticleDlg::init(CWnd *pParent, sint x, sint y)
{
	Create(IDD_TAIL_PARTICLE, pParent);
	RECT r;
	GetClientRect(&r);
	r.top += y; r.bottom += y;
	r.right += x; r.left += x;
	MoveWindow(&r);

	if (!dynamic_cast<NL3D::CPSRibbon *>(_TailParticle))
	{
		m_TailShape.EnableWindow(FALSE);
		m_TailShape.ShowWindow(FALSE);		
		m_TailPersistAfterDeathCtrl.EnableWindow(FALSE);
		m_TailPersistAfterDeathCtrl.ShowWindow(FALSE);
	}
	else
	{
///		m_TailPersistAfterDeath = (dynamic_cast<NL3D::CPSRibbon *>(_TailParticle))->getPersistAfterDeath();		
	}
	NL3D::CPSRibbon *ribbon = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
	if (ribbon)
	{	
		((CComboBox *) GetDlgItem(IDC_RIBBON_ORIENTATION))->SetCurSel(ribbon->getOrientation());
	}
	else
	{
		GetDlgItem(IDC_RIBBON_ORIENTATION)->ShowWindow(SW_HIDE);
	}
	UpdateData();
	ShowWindow(SW_SHOW);
}

void CTailParticleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTailParticleDlg)
	DDX_Control(pDX, IDC_TAIL_SHAPE, m_TailShape);
	DDX_Control(pDX, IDC_TAIL_PERSIST_AFTER_DEATH, m_TailPersistAfterDeathCtrl);
	DDX_Check(pDX, IDC_TAIL_FADE, m_TailFade);	
	DDX_Check(pDX, IDC_TAIL_PERSIST_AFTER_DEATH, m_TailPersistAfterDeath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTailParticleDlg, CDialog)
	//{{AFX_MSG_MAP(CTailParticleDlg)
	ON_BN_CLICKED(IDC_TAIL_FADE, OnTailFade)	
	ON_BN_CLICKED(IDC_TAIL_PERSIST_AFTER_DEATH, OnTailPersistAfterDeath)
	ON_CBN_SELCHANGE(IDC_TAIL_SHAPE, OnSelchangeTailShape)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_RIBBON_ORIENTATION, OnSelchangeRibbonOrientation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTailParticleDlg message handlers

void CTailParticleDlg::OnTailFade() 
{
	UpdateData();
	_TailParticle->setColorFading(m_TailFade ? true : false);
	_Node->setModified(true);
	UpdateData(FALSE);
	
}


void CTailParticleDlg::OnTailPersistAfterDeath() 
{
	UpdateData();
	nlassert(dynamic_cast<NL3D::CPSRibbon *>(_TailParticle));
///	(dynamic_cast<NL3D::CPSRibbon *>(_TailParticle))->setPersistAfterDeath(m_TailPersistAfterDeath ? true : false);
	_Node->setModified(true);
	UpdateData(FALSE);
}

void CTailParticleDlg::OnSelchangeTailShape() 
{
	UpdateData();
	NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
	nlassert(r);
	switch (m_TailShape.GetCurSel() )
	{
		case 0: // triangle
			r->setShape(NL3D::CPSRibbon::Triangle, NL3D::CPSRibbon::NbVerticesInTriangle);
		break;
		case 1:	// quad
			r->setShape(NL3D::CPSRibbon::Losange, NL3D::CPSRibbon::NbVerticesInLosange);
		break;
		case 2: // octogon
			r->setShape(NL3D::CPSRibbon::HeightSides, NL3D::CPSRibbon::NbVerticesInHeightSide);
		break;
		case 3: // pentagram
			r->setShape(NL3D::CPSRibbon::Pentagram, NL3D::CPSRibbon::NbVerticesInPentagram);
		break;
		case 4: // simple segment x
			r->setShape(NL3D::CPSRibbon::SimpleSegmentX, NL3D::CPSRibbon::NbVerticesInSimpleSegmentX, true);
		break;
		case 5: // simple segment y
			r->setShape(NL3D::CPSRibbon::SimpleSegmentY, NL3D::CPSRibbon::NbVerticesInSimpleSegmentY, true);
		break;
		case 6: // simple segment z
			r->setShape(NL3D::CPSRibbon::SimpleSegmentZ, NL3D::CPSRibbon::NbVerticesInSimpleSegmentZ, true);
		break;
		case 7: // simple brace
			r->setShape(NL3D::CPSRibbon::SimpleBrace, NL3D::CPSRibbon::NbVerticesInSimpleBrace, true);
		break;
	}
	_Node->setModified(true);
	Invalidate();
}

void CTailParticleDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
	// if we're dealing with a ribbon, we draw the shape used for extrusion
	if (r)
	{
		const uint x = 270, y = 15, size = 32;
	

		dc.FillSolidRect(x, y, size, size, 0xffffff);
		
		std::vector<NLMISC::CVector> verts;
		verts.resize(r->getNbVerticesInShape() );
		r->getShape(&verts[0]);

		CPen p;
		p.CreatePen(PS_SOLID, 1, (COLORREF) 0);
		CPen *old = dc.SelectObject(&p);

		if (r->getBraceMode())
		{
			for(uint k = 0; k < verts.size() / 2; ++k)
			{
				dc.MoveTo((int) (x + (size / 2) * (1 + verts[2 * k].x)), (int) (y + (size / 2) * (1 - verts[2 * k].y)));
				dc.LineTo((int) (x + (size / 2) * (1 + verts[2 * k + 1].x)), (int) (y + (size / 2) * (1 - verts[2 * k + 1].y)));
			}			
		}
		else
		{
			dc.MoveTo((int) (x + (size / 2) * (1 + verts[0].x)), (int) (y + (size / 2) * (1 - verts[0].y)));
			for (std::vector<NLMISC::CVector>::const_iterator it = verts.begin(); it != verts.end(); ++it)
			{
				dc.LineTo((int) (x + (size / 2) * (1 + it->x)), (int) (y + (size / 2) * (1 - it->y)));
			}
			dc.LineTo((int) (x + (size / 2) * (1 + verts[0].x)), (int) (y + (size / 2) * (1 - verts[0].y)));
		}		
		dc.SelectObject(old);
	}
}

void CTailParticleDlg::OnSelchangeRibbonOrientation() 
{
	NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
	nlassert(r);
	r->setOrientation((NL3D::CPSRibbon::TOrientation) ((CComboBox *) GetDlgItem(IDC_RIBBON_ORIENTATION))->GetCurSel());
	_Node->setModified(true);
}

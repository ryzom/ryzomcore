/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdpch.h"
#include "tail_particle_widget.h"

// NeL includes
#include "nel/3d/ps_particle.h"

// Projects includes

namespace NLQT {

CTailParticleWidget::CTailParticleWidget(QWidget *parent)
    : QWidget(parent)
{
	_ui.setupUi(this);
	
	_ui.pathWidget->setMode(Mode::RibbonShape);
	connect(_ui.tailFadingCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTailFading(bool)));
	connect(_ui.tailPersistAfterDeathCheckBox, SIGNAL(toggled(bool)), this, SLOT(setPersistAfterDeath(bool)));
	connect(_ui.ribbonOrientationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setRibbonOrientation(int)));
	connect(_ui.tailShapeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setTailShape(int)));
}

CTailParticleWidget::~CTailParticleWidget()
{
}

void CTailParticleWidget::setCurrentTailParticle(CWorkspaceNode *ownerNode, NL3D::CPSTailParticle *tp)
{
	_Node = ownerNode;

	_TailParticle = tp;

	_ui.tailFadingCheckBox->setChecked(tp->getColorFading());
	
	if (!dynamic_cast<NL3D::CPSRibbon *>(_TailParticle))
	{
		_ui.pathWidget->hide();
		_ui.ribbonOrientationComboBox->hide();
		_ui.tailPersistAfterDeathCheckBox->hide();
		_ui.tailShapeComboBox->hide();
	}
	else
	{
		_ui.pathWidget->show();
		_ui.ribbonOrientationComboBox->show();
		_ui.tailPersistAfterDeathCheckBox->show();
		_ui.tailShapeComboBox->show();
		NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
//		_ui.tailPersistAfterDeathCheckBox = (r->getPersistAfterDeath();
		_ui.ribbonOrientationComboBox->setCurrentIndex(r->getOrientation());
		
		// Update graphics widget
		std::vector<NLMISC::CVector> verts;
		verts.resize(r->getNbVerticesInShape());
		r->getShape(&verts[0]);
		_ui.pathWidget->setRibbonShape(verts, r->getBraceMode());
	}
}

void CTailParticleWidget::setTailFading(bool state)
{
	if (state != _TailParticle->getColorFading())
	{
		_TailParticle->setColorFading(state);
		_Node->setModified(true);
	}
}

void CTailParticleWidget::setPersistAfterDeath(bool state)
{
	nlassert(dynamic_cast<NL3D::CPSRibbon *>(_TailParticle));
//	(dynamic_cast<NL3D::CPSRibbon *>(_TailParticle))->setPersistAfterDeath(state);
}

void CTailParticleWidget::setTailShape(int index)
{
	NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
	nlassert(r);
	switch (index)
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
	
	// Update graphics widget
	std::vector<NLMISC::CVector> verts;
	verts.resize(r->getNbVerticesInShape() );
	r->getShape(&verts[0]);
	_ui.pathWidget->setRibbonShape(verts, r->getBraceMode());
}

void CTailParticleWidget::setRibbonOrientation(int index)
{
  	NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
	nlassert(r);
	if (index != r->getOrientation())
	{
		r->setOrientation((NL3D::CPSRibbon::TOrientation) index);
		_Node->setModified(true);
	}
}

} /* namespace NLQT */
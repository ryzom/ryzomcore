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

// Qt includes
#include <QtGui/QPainter>

namespace NLQT
{

CTailParticleWidget::CTailParticleWidget(QWidget *parent)
	: QWidget(parent), _Node(NULL), _TailParticle(NULL)
{
	_ui.setupUi(this);
	_ui.graphicsWidget->installEventFilter(this);

	connect(_ui.tailFadingCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTailFading(bool)));
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
		_ui.graphicsWidget->hide();
		_ui.ribbonOrientationComboBox->hide();
		_ui.tailShapeComboBox->hide();
	}
	else
	{
		_ui.graphicsWidget->show();
		_ui.ribbonOrientationComboBox->show();
		_ui.tailShapeComboBox->show();
		NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
		_ui.ribbonOrientationComboBox->setCurrentIndex(r->getOrientation());

		_ui.graphicsWidget->repaint();
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

	_ui.graphicsWidget->repaint();
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

bool CTailParticleWidget::eventFilter(QObject *object, QEvent *event)
{
	if( event->type() == QEvent::Paint )
	{
		QPainter painter(_ui.graphicsWidget);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setBrush(QBrush(Qt::white));
		painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));
		painter.drawRoundedRect(QRect(3, 3, _ui.graphicsWidget->width() - 6, _ui.graphicsWidget->height() - 6), 3.0, 3.0);

		painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
		painter.scale(0.86, 0.86);
		painter.translate(6, 6);
		NL3D::CPSRibbon *r = dynamic_cast<NL3D::CPSRibbon *>(_TailParticle);
		if (r)
		{
			std::vector<NLMISC::CVector> verts;
			verts.resize(r->getNbVerticesInShape() );
			r->getShape(&verts[0]);
			if (r->getBraceMode())
			{
				for(uint k = 0; k < verts.size() / 2; ++k)
				{
					painter.drawLine(int((_ui.graphicsWidget->width() / 2.0) * (1 + verts[2 * k].x)),
									 int((_ui.graphicsWidget->height() / 2.0) * (1 - verts[2 * k].y)),
									 int((_ui.graphicsWidget->width() / 2.0) * (1 + verts[2 * k + 1].x)),
									 int((_ui.graphicsWidget->height() / 2.0) * (1 - verts[2 * k + 1].y)));
				}
			}
			else
			{
				for(uint k = 1; k < verts.size(); ++k)
				{
					painter.drawLine(int((_ui.graphicsWidget->width() / 2.0) * (1 + verts[k - 1].x)),
									 int((_ui.graphicsWidget->height() / 2.0) * (1 - verts[k - 1].y)),
									 int((_ui.graphicsWidget->width() / 2.0) * (1 + verts[ k].x)),
									 int((_ui.graphicsWidget->height() / 2.0) * (1 - verts[k].y)));
				}
				painter.drawLine(int((_ui.graphicsWidget->width() / 2.0) * (1 + verts[0].x)),
								 int((_ui.graphicsWidget->height() / 2.0) * (1 - verts[0].y)),
								 int((_ui.graphicsWidget->width() / 2.0) * (1 + verts[verts.size() - 1].x)),
								 int((_ui.graphicsWidget->height() / 2.0) * (1 - verts[verts.size() - 1].y)));
			}
		}
	}
	return QWidget::eventFilter(object, event);
}

} /* namespace NLQT */
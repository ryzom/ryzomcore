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

#ifndef TAIL_PARTICLE_WIDGET_H
#define TAIL_PARTICLE_WIDGET_H

#include "ui_tail_form.h"

// STL includes

// Qt includes

// NeL includes

// Project includes
#include "particle_node.h"

namespace NL3D
{
struct CPSTailParticle;
}

namespace NLQT
{

class CTailParticleWidget: public QWidget
{
	Q_OBJECT

public:
	CTailParticleWidget(QWidget *parent = 0);
	~CTailParticleWidget();

	void setCurrentTailParticle(CWorkspaceNode *ownerNode, NL3D::CPSTailParticle *tp);

private Q_SLOTS:
	void setTailFading(bool state);
	void setTailShape(int index);
	void setRibbonOrientation(int index);

private:
	bool eventFilter(QObject *object, QEvent *event);

	CWorkspaceNode *_Node;

	NL3D::CPSTailParticle *_TailParticle;

	Ui::CTailParticleWidget _ui;
}; /* class CTailParticleWidget */

} /* namespace NLQT */

#endif // TAIL_PARTICLE_WIDGET_H

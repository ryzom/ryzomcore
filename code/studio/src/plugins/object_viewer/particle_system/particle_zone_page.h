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

#ifndef PARTICLE_ZONE_PAGE_H
#define PARTICLE_ZONE_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_particle_zone_form.h"

// STL includes

// NeL includes
#include "nel/3d/ps_zone.h"

// Project includes
#include "particle_node.h"

namespace NLQT
{

class CZonePage: public QWidget
{
	Q_OBJECT

public:
	CZonePage(QWidget *parent = 0);
	virtual ~CZonePage();

	void setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable);

private Q_SLOTS:
	void addTarget();
	void removeTarget();
	void setBounce(int index);

	void setBounceFactor(float value);

private:
	void updateTargets();

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	/// the target we're focusing on
	NL3D::CPSTargetLocatedBindable *_LBTarget;

	/// the collision zone being edited
	NL3D::CPSZone *_Zone ;

	CWorkspaceNode *_Node;

	Ui::CZonePage _ui;

}; /* class CZonePage */

} /* namespace NLQT */

#endif // PARTICLE_ZONE_PAGE_H

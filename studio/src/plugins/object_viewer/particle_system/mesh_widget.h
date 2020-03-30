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

#ifndef MESH_WIDGET_H
#define MESH_WIDGET_H

#include "ui_mesh_form.h"

// STL includes

// Qt includes

// NeL includes

// Project includes
#include "particle_node.h"

namespace NL3D
{
struct CPSShapeParticle;
}

namespace NLQT
{

class CMeshWidget: public QGroupBox
{
	Q_OBJECT

public:
	CMeshWidget(QWidget *parent = 0);
	~CMeshWidget();

	void setCurrentShape(CWorkspaceNode *ownerNode, NL3D::CPSShapeParticle *sp);

	QString getShapeErrorString(sint errorCode);

private Q_SLOTS:
	void browseShape();
	void setMorphMesh(bool state);
	void editMorph();

private:
	CWorkspaceNode *_Node;

	NL3D::CPSShapeParticle *_ShapeParticle;

	void updateForMorph();

	void updateMeshErrorString();

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	void touchPSState();

	Ui::CMeshWidget _ui;
}; /* class CMeshWidget */

} /* namespace NLQT */

#endif // MESH_WIDGET_H

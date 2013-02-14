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

#ifndef CONSTRAINT_MESH_WIDGET_H
#define CONSTRAINT_MESH_WIDGET_H

#include "ui_constraint_mesh_form.h"

// STL includes

// Qt includes

// NeL includes

// Project includes
#include "particle_node.h"

namespace NL3D
{
class CPSConstraintMesh;
}

namespace NLQT
{

/**
@class CConstraintMeshWidget
@brief Widget for setup mesh that have very simple geometry.
*/
class CConstraintMeshWidget: public QWidget
{
	Q_OBJECT

public:
	CConstraintMeshWidget(QWidget *parent = 0);
	~CConstraintMeshWidget();

	/// Set the constraint mesh to edit.
	void setCurrentConstraintMesh(CWorkspaceNode *ownerNode, NL3D::CPSConstraintMesh *cm);

private Q_SLOTS:
	void setForceStage0(bool state);
	void setForceStage1(bool state);
	void setForceStage2(bool state);
	void setForceStage3(bool state);
	void setForceVertexColorLighting(bool state);
	void setTexAnimType(int index);
	void setReinitWhenNewElementIsCreated(bool state);
	void setStage(int value);
	void setGlobalTexAnimValue();

private:
	void connectGlobalTexAnim();
	void disconnectGlobalTexAnim();
	void updateGlobalTexAnim(int value);

	CWorkspaceNode *_Node;

	NL3D::CPSConstraintMesh *_CM;

	Ui::CConstraintMeshWidget _ui;
}; /* class CConstraintMeshWidget */

} /* namespace NLQT */

#endif // CONSTRAINT_MESH_WIDGET_H

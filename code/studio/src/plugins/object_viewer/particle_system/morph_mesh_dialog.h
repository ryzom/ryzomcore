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

#ifndef MORPH_MESH_DIALOG_H
#define MORPH_MESH_DIALOG_H

#include "ui_morph_mesh_form.h"

// STL includes

// Qt includes

// NeL includes

// Project includes
#include "ps_wrapper.h"

namespace NL3D
{
class CPSConstraintMesh;
}

namespace NLQT
{

class CMorphMeshDialog: public QDialog
{
	Q_OBJECT

public:
	CMorphMeshDialog(CWorkspaceNode *ownerNode, NL3D::CPSConstraintMesh *cm, QWidget *parent = 0);
	~CMorphMeshDialog();

private Q_SLOTS:
	void add();
	void remove();
	void insert();
	void change();
	void up();
	void down();

private:
	/// fill the mesh list with the mesh names in the object being edited
	void updateMeshList();

	QString getShapeDescStr(uint shapeIndex, sint numVerts) const;

	/// wrapper for the morph scheme
	struct CMorphSchemeWrapper : IPSSchemeWrapperFloat, IPSWrapperFloat
	{
		NL3D::CPSConstraintMesh *CM;
		virtual float get(void) const;
		virtual void set(const float &);
		virtual scheme_type *getScheme(void) const;
		virtual void setScheme(scheme_type *s);
	} _MorphSchemeWrapper;

	void touchPSState();

	CWorkspaceNode *_Node;

	// the constraint mesh being edited
	NL3D::CPSConstraintMesh *_CM;

	Ui::CMorphMeshDialog _ui;
}; /* class CMorphMeshDialog */

} /* namespace NLQT */

#endif // MORPH_MESH_DIALOG_H

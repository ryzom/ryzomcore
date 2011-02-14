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

#ifndef AUTO_LOD_DIALOG_H
#define AUTO_LOD_DIALOG_H

#include "ui_auto_lod_form.h"

// STL includes

// Qt includes

// NeL includes
#include "nel/3d/particle_system.h"

// Project includes
#include "ps_wrapper.h"

namespace NLQT
{

class CAutoLODDialog: public QDialog
{
	Q_OBJECT

public:
	CAutoLODDialog(CWorkspaceNode *ownerNode, NL3D::CParticleSystem *ps, QWidget *parent = 0);
	~CAutoLODDialog();

private Q_SLOTS:
	void setDegradationExponent(int value);
	void setSkipParticles(bool state);
	void setDistRatio(float value);
	void setMaxDistLODBias(float value);

private:
	CWorkspaceNode *_Node;
	NL3D::CParticleSystem *_PS;

	Ui::CAutoLODDialog _ui;
}; /* class CAutoLODDialog */

} /* namespace NLQT */

#endif // AUTO_LOD_DIALOG_H

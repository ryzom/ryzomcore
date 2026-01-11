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

#ifndef PARTICLE_LINK_SKELETON_DIALOG_H
#define PARTICLE_LINK_SKELETON_DIALOG_H

#include "ui_particle_link_skeleton_form.h"

// STL includes

// NeL includes

// Project includes
#include "skeleton_tree_model.h"

namespace NLQT
{

class CParticleLinkDialog: public QDockWidget
{
	Q_OBJECT

public:
	CParticleLinkDialog(CSkeletonTreeModel *model, QWidget *parent = 0);
	~CParticleLinkDialog();

private Q_SLOTS:
	void setLink();
	void setUnlink();
	void resetModel();
	void clickedItem(const QModelIndex &index);

private:

	Ui::CParticleLinkDialog _ui;

}; /* class CParticleLinkDialog */

} /* namespace NLQT */


#endif // PARTICLE_LINK_SKELETON_DIALOG_H

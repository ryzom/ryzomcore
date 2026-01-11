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

#ifndef PARTICLE_WORKSPACE_PAGE_H
#define PARTICLE_WORKSPACE_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_workspace_form.h"

// STL includes

// NeL includes
#include <nel/misc/config_file.h>

// Project includes
#include "particle_tree_model.h"

namespace NLQT
{

/**
@class CWorkspacePage
@brief Page for QStackWidget, to particles workspace operation (new/load/save workspace,
create/insert/remove all particles system to workspace)
*/
class CWorkspacePage: public QWidget
{
	Q_OBJECT

public:
	CWorkspacePage(CParticleTreeModel *treeModel, QWidget *parent = 0);
	~CWorkspacePage();

private Q_SLOTS:
	void newWP();
	void loadWP();
	void saveWP();
	void saveAsWP();
	void insertPS();
	void createPS();
	void removeAllPS();
	void unloadWP();

private:

	Ui::CWorkspacePage _ui;

	CParticleTreeModel *_treeModel;
	friend class CPropertyDialog;
	friend class CParticleWorkspaceDialog;
}; /* class CWorkspacePage */

} /* namespace NLQT */

#endif // PARTICLE_WORKSPACE_PAGE_H

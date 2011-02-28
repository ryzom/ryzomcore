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

#ifndef PARTICLE_PROPERTY_DIALOG_H
#define PARTICLE_PROPERTY_DIALOG_H

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QDockWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QScrollArea>
#include <QtGui/QGridLayout>

// NeL includes
#include "nel/misc/smart_ptr.h"

// Projects includes
#include "particle_system_page.h"
#include "emitter_page.h"
#include "located_page.h"
#include "located_bindable_page.h"
#include "particle_force_page.h"
#include "particle_light_page.h"
#include "particle_zone_page.h"
#include "particle_sound_page.h"
#include "particle_workspace_page.h"
#include "ps_mover_page.h"
#include "particle_tree_model.h"
#include "particle_node.h"

namespace NLQT
{

class CPropertyDialog: public QDockWidget
{
	Q_OBJECT
public:
	CPropertyDialog(CParticleTreeModel *treeModel, QWidget *parent = 0);
	~CPropertyDialog();

	void setCurrentEditedElement(CParticleTreeItem *editedItem);

	CLocatedPage *getLocatedPage() const
	{
		return _locatedPage;
	};
	CPSMoverPage *getMoverPage() const
	{
		return _psMoverPage;
	};

private:
	void setupUi();

	QWidget *_dockWidgetContents;
	QGridLayout *_gridLayout;
	QGridLayout *_pagesGridLayout;
	QScrollArea *_scrollArea;
	QWidget *_scrollAreaWidgetContents;

	QStackedWidget *_stackedWidget;
	CWorkspacePage *_wpPage;
	CParticleSystemPage	*_psPage;
	CLocatedBindablePage *_locatedBindablePage;
	CLocatedPage *_locatedPage;
	CForcePage *_forcePage;
	CLightPage *_lightPage;
	CZonePage *_zonePage;
	CSoundPage *_soundPage;
	CEmitterPage *_emitterPage;
	CPSMoverPage *_psMoverPage;

	CParticleTreeModel *_treeModel;

	friend class CParticleWorkspaceDialog;
}; /* class CPropertyDialog */

} /* namespace NLQT */

#endif // PARTICLE_PROPERTY_DIALOG_H

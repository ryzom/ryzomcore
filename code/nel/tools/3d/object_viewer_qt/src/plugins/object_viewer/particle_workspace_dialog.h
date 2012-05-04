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

#ifndef PARTICLE_WORKSPACE_DIALOG_H
#define PARTICLE_WORKSPACE_DIALOG_H

#include <nel/misc/types_nl.h>
#include "ui_particle_workspace_form.h"

// Qt includes
#include <QtCore/QSignalMapper>

// NeL includes
#include "nel/misc/smart_ptr.h"

// Projects includes
#include "particle_property_dialog.h"
#include "particle_tree_model.h"
#include "particle_node.h"

namespace NLQT
{

/**
@class ParticleWorkspaceDialog
@brief Displays particles workspace in QTreeView, build popur menu (operations with particles system sub-items).
*/
class CParticleWorkspaceDialog: public QDockWidget
{
	Q_OBJECT
public:
	CParticleWorkspaceDialog(QWidget *parent = 0);
	~CParticleWorkspaceDialog();

	void touchPSState(CParticleTreeItem *item);
	CPropertyDialog *getPropertyDialog() const
	{
		return _PropertyDialog;
	}

Q_SIGNALS:
	/// Emits change active particle system node.
	void changeActiveNode();

private Q_SLOTS:
	void clickedItem(const QModelIndex &index);
	void customContextMenu();

	void setActiveNode();
	void savePS();
	void saveAsPS();
	void clearContent();
	void removePS();
	void mergePS();

	void newLocated();
	void pasteLocated();

	void bindNewLocatedBindable(int id);

	void forceZBias();

	void copyLocated();
	void copyBindable();
	void pasteBindable();
	void deleteItem();

	void setInstanciate();
	void setAllLOD();
	void setLOD1();
	void setLOD2();

	void setExternID();

	void setNewState();

	void updateTreeView();

private:
	void buildMenu(QMenu *menu);
	NL3D::CPSLocated *createLocated(NL3D::CParticleSystem *ps);

	CPropertyDialog *_PropertyDialog;
	CParticleTreeModel *_treeModel;

	QSignalMapper *_signalMapper;

	QAction *_setActivePSAction;
	QAction *_savePSAction;
	QAction *_saveAsPSAction;
	QAction *_clearContentAction;
	QAction *_removeFromWSAction;
	QAction *_mergeAction;
	QAction *_newLocatedAction;
	QAction *_pasteLocatedAction;
	QAction *_bindNewLocatedBindable[32];
	QAction *_forceZBiasAction;

	QAction *_instanciateAction;
	QAction *_copyLocatedAction;
	QAction *_copyBindableAction;
	QAction *_pasteBindableAction;
	QAction *_deleteAction;

	QAction *_allLODAction;
	QAction *_lod1Action;
	QAction *_lod2Action;
	QAction *_externIDAction;

	std::auto_ptr<NL3D::CPSLocated>	_LocatedCopy;
	std::auto_ptr<NL3D::CPSLocatedBindable> _LocatedBindableCopy;

	CParticleTreeItem *_currentItem;

	Ui::CParticleWorkspaceDialog _ui;
	friend class CMainWindow;
}; /* class CParticleWorkspaceDialog */

} /* namespace NLQT */

#endif // PARTICLE_WORKSPACE_DIALOG_H

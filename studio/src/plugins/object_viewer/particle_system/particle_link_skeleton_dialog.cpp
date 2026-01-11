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

#include "stdpch.h"
#include "particle_link_skeleton_dialog.h"

// Qt includes
#include <QtGui/QMainWindow>

// Project includes
#include "modules.h"
#include "particle_node.h"

namespace NLQT
{

CParticleLinkDialog::CParticleLinkDialog(CSkeletonTreeModel *model, QWidget *parent)
	: QDockWidget(parent)
{
	_ui.setupUi(this);

	qobject_cast<QMainWindow *>(parent)->addDockWidget(Qt::RightDockWidgetArea, this);

	_ui.treeView->setModel(model);

	connect(model, SIGNAL(modelReset()), this, SLOT(resetModel()));
	connect(_ui.linkPushButton, SIGNAL(clicked()), this, SLOT(setLink()));
	connect(_ui.unlinkPushButton, SIGNAL(clicked()), this, SLOT(setUnlink()));
	connect(_ui.treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(clickedItem(QModelIndex)));
}

CParticleLinkDialog::~CParticleLinkDialog()
{
}

void CParticleLinkDialog::setLink()
{
	CWorkspaceNode *node = Modules::psEdit().getActiveNode();
	if (node == NULL)
		return;

	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;

	CSkeletonTreeItem *item = static_cast<CSkeletonTreeItem *>(_ui.treeView->currentIndex().internalPointer());

	NL3D::CSkeletonModel *skel = Modules::objView().getEntity(curObj).getSkeleton().getObjectPtr();
	uint boneIndex = item->getId();
	std::string parentSkelName = Modules::objView().getEntity(curObj).getFileNameSkeleton();
	std::string parentBoneName = skel->Bones[boneIndex].getBoneName();

	node->stickPSToSkeleton(skel, boneIndex, parentSkelName, parentBoneName);
}

void CParticleLinkDialog::setUnlink()
{
	CWorkspaceNode *node = Modules::psEdit().getActiveNode();
	if (node == NULL)
		return;

	node->unstickPSFromSkeleton();
}

void CParticleLinkDialog::resetModel()
{
	_ui.treeView->expandAll();
	_ui.linkPushButton->setEnabled(false);
	_ui.unlinkPushButton->setEnabled(false);
}

void CParticleLinkDialog::clickedItem(const QModelIndex &index)
{
	_ui.linkPushButton->setEnabled(true);
	_ui.unlinkPushButton->setEnabled(true);
}

} /* namespace NLQT */
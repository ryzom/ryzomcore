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
#include "particle_workspace_page.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// NeL includes

// Project includes
#include "modules.h"

namespace NLQT
{

CWorkspacePage::CWorkspacePage(CParticleTreeModel *treeModel, QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_treeModel = treeModel;

	connect(_ui.newToolButton, SIGNAL(clicked()), this, SLOT(newWP()));
	connect(_ui.loadToolButton, SIGNAL(clicked()), this, SLOT(loadWP()));
	connect(_ui.saveToolButton, SIGNAL(clicked()), this, SLOT(saveWP()));
	connect(_ui.saveAsToolButton, SIGNAL(clicked()), this, SLOT(saveAsWP()));
	connect(_ui.insertToolButton, SIGNAL(clicked()), this, SLOT(insertPS()));
	connect(_ui.createToolButton, SIGNAL(clicked()), this, SLOT(createPS()));
	connect(_ui.resetToolButton, SIGNAL(clicked()), this, SLOT(removeAllPS()));
	connect(_ui.unloadToolButton, SIGNAL(clicked()), this, SLOT(unloadWP()));
}

CWorkspacePage::~CWorkspacePage()
{
}

void CWorkspacePage::newWP()
{
	//checkModifiedWorkSpace();
	// ask name of the new workspace to create
	QString fileName = QFileDialog::getSaveFileName(this, tr("Create new pws file"),
					   ".",
					   tr("pws files (*.pws)"));
	if (!fileName.isEmpty())
	{
		Modules::psEdit().createNewWorkspace(fileName.toUtf8().constData());
		_treeModel->setupModelFromWorkSpace();
		_ui.saveToolButton->setEnabled(true);
		_ui.saveAsToolButton->setEnabled(true);
		_ui.insertToolButton->setEnabled(true);
		_ui.resetToolButton->setEnabled(true);
		_ui.createToolButton->setEnabled(true);
	}
}

void CWorkspacePage::loadWP()
{
	//checkModifiedWorkSpace();
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open NeL data file"), ".",
					   tr("Particle Workspace file (*.pws);;"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		Modules::psEdit().loadWorkspace(fileName.toUtf8().constData());
		_treeModel->setupModelFromWorkSpace();
		_ui.unloadToolButton->setEnabled(true);
		_ui.saveToolButton->setEnabled(true);
		_ui.saveAsToolButton->setEnabled(true);
		_ui.insertToolButton->setEnabled(true);
		_ui.resetToolButton->setEnabled(true);
		_ui.createToolButton->setEnabled(true);
	}
	setCursor(Qt::ArrowCursor);
}

void CWorkspacePage::saveWP()
{
	//checkModifiedWorkSpace();
	Modules::psEdit().saveWorkspaceStructure();
	Modules::psEdit().saveWorkspaceContent();
}

void CWorkspacePage::saveAsWP()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save as pws file"),
					   ".",
					   tr("pws files (*.pws)"));
	if (!fileName.isEmpty())
	{
		Modules::psEdit().getParticleWorkspace()->setFileName(fileName.toUtf8().constData());
		Modules::psEdit().saveWorkspaceStructure();
		Modules::psEdit().saveWorkspaceContent();
		_treeModel->setupModelFromWorkSpace();
	}
}

void CWorkspacePage::insertPS()
{
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open NeL data file"), ".",
					   tr("Particle System file (*.ps);;"));

	if (!fileName.isEmpty())
	{
		// TODO: create method particle editor insertNewPS and multiple add
		CWorkspaceNode *node = Modules::psEdit().getParticleWorkspace()->addNode(NLMISC::CFile::getFilename(fileName.toUtf8().constData()));
		if (node != 0)
		{
			try
			{
				node->loadPS();
			}
			catch(NLMISC::EStream &e)
			{
				QMessageBox::critical(this, tr("NeL particle system editor"),
									  QString(e.what()),
									  QMessageBox::Ok);
			}
			if (!node->isLoaded())
				Modules::psEdit().getParticleWorkspace()->removeNode(Modules::psEdit().getParticleWorkspace()->getNumNode() - 1);
			else
			{
				QModelIndex index = _treeModel->index(0, 0);
				_treeModel->insertRows(node, static_cast<CParticleTreeItem *>(index.internalPointer())->childCount(), index);
			}
		}
	}
}

void CWorkspacePage::createPS()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Create new particle system file"),
					   ".",
					   tr("ps files (*.ps)"));
	if (!fileName.isEmpty())
	{

		// TODO: create method particle editor createNewPS
		if (Modules::psEdit().getParticleWorkspace()->containsFile(NLMISC::CFile::getFilename(fileName.toUtf8().constData())))
		{
			QMessageBox::critical(this, tr("NeL particle system editor"),
								  tr("Failed to create new particle system"),
								  QMessageBox::Ok);
			return;
		}
		CWorkspaceNode *node = Modules::psEdit().getParticleWorkspace()->addNode(NLMISC::CFile::getFilename(fileName.toUtf8().constData()));
		// should always succeed because we tested if file already exists
		nlassert(node);
		node->createEmptyPS();
		try
		{
			node->savePS();
			node->setModified(false);
		}
		catch (NLMISC::Exception &e)
		{
			QMessageBox::critical(this, tr("NeL particle system editor"),
								  QString(e.what()),
								  QMessageBox::Ok);
			return;
		}
		QModelIndex index = _treeModel->index(0, 0);
		_treeModel->insertRows(node, static_cast<CParticleTreeItem *>(index.internalPointer())->childCount(), index);
	}
}

void CWorkspacePage::removeAllPS()
{
	Modules::psEdit().setActiveNode(NULL);
	// TODO: create method particle editor clearParticleWorkspace
	uint numNodes = Modules::psEdit().getParticleWorkspace()->getNumNode();
	for(uint k = 0; k < numNodes; ++k)
		Modules::psEdit().getParticleWorkspace()->removeNode((uint) 0);

	_treeModel->setupModelFromWorkSpace();
}

void CWorkspacePage::unloadWP()
{
	//checkModifiedWorkSpace();
	Modules::psEdit().closeWorkspace();
	_treeModel->setupModelFromWorkSpace();
	_ui.unloadToolButton->setEnabled(false);
	_ui.saveToolButton->setEnabled(false);
	_ui.saveAsToolButton->setEnabled(false);
	_ui.insertToolButton->setEnabled(false);
	_ui.resetToolButton->setEnabled(false);
	_ui.createToolButton->setEnabled(false);
}

} /* namespace NLQT */
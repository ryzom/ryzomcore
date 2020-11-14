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
#include "particle_property_dialog.h"

// Qt includes
// Project includes
#include "modules.h"

namespace NLQT
{

CPropertyDialog::CPropertyDialog(CParticleTreeModel *treeModel, QWidget *parent)
	: QDockWidget(parent)
{
	_treeModel = treeModel;

	setupUi();
}
CPropertyDialog::~CPropertyDialog()
{
}

void CPropertyDialog::setupUi()
{
	setObjectName(QString::fromUtf8("CPropertyDialog"));
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/pqrticles.png"), QSize(), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon);

	_dockWidgetContents = new QWidget();
	_gridLayout = new QGridLayout(_dockWidgetContents);
	_gridLayout->setContentsMargins(6, 6, 6, 6);
	_scrollArea = new QScrollArea(_dockWidgetContents);
	_scrollArea->setWidgetResizable(true);
	_scrollAreaWidgetContents = new QWidget();

	_pagesGridLayout = new QGridLayout(_scrollAreaWidgetContents);
	_pagesGridLayout->setContentsMargins(0, 0, 0, 0);

	_stackedWidget = new QStackedWidget(_scrollAreaWidgetContents);

	_wpPage = new CWorkspacePage(_treeModel);
	_stackedWidget->addWidget(_wpPage);
	_psPage = new  CParticleSystemPage(_stackedWidget);
	_stackedWidget->addWidget(_psPage);
	_locatedBindablePage = new CLocatedBindablePage(_stackedWidget);
	_stackedWidget->addWidget(_locatedBindablePage);
	_locatedPage = new CLocatedPage(_stackedWidget);
	_stackedWidget->addWidget(_locatedPage);
	_forcePage = new CForcePage(_stackedWidget);
	_stackedWidget->addWidget(_forcePage);
	_lightPage = new CLightPage(_stackedWidget);
	_stackedWidget->addWidget(_lightPage);
	_zonePage = new CZonePage(_stackedWidget);
	_stackedWidget->addWidget(_zonePage);
	_soundPage = new CSoundPage(_stackedWidget);
	_stackedWidget->addWidget(_soundPage);
	_emitterPage = new CEmitterPage(_stackedWidget);
	_stackedWidget->addWidget(_emitterPage);
	_psMoverPage = new CPSMoverPage(_stackedWidget);
	_stackedWidget->addWidget(_psMoverPage);

	_pagesGridLayout->addWidget(_stackedWidget, 0, 0, 1, 1);

	_scrollArea->setWidget(_scrollAreaWidgetContents);

	_gridLayout->addWidget(_scrollArea, 0, 0, 1, 1);

	setWidget(_dockWidgetContents);
	setWindowTitle(tr("Property editor"));
}

void CPropertyDialog::setCurrentEditedElement(CParticleTreeItem *editedItem)
{
	// Update ui property editor
	switch (editedItem->itemType())
	{
	case ItemType::ParticleSystem:
		_psPage->setEditedParticleSystem(editedItem->getNode());
		_stackedWidget->setCurrentWidget(_psPage);
		break;
	case ItemType::Particle:
		_locatedBindablePage->setEditedItem(_treeModel->getOwnerNode(editedItem) ,editedItem->getBind());
		_stackedWidget->setCurrentWidget(_locatedBindablePage);
		break;
	case ItemType::Emitter:
		_emitterPage->setEditedItem(_treeModel->getOwnerNode(editedItem) ,editedItem->getBind());
		_stackedWidget->setCurrentWidget(_emitterPage);
		break;
	case ItemType::Force:
		_forcePage->setEditedItem(_treeModel->getOwnerNode(editedItem) ,editedItem->getBind());
		_stackedWidget->setCurrentWidget(_forcePage);
		break;
	case ItemType::Light:
		_lightPage->setEditedItem(_treeModel->getOwnerNode(editedItem) ,editedItem->getBind());
		_stackedWidget->setCurrentWidget(_lightPage);
		break;
	case ItemType::Sound:
		_soundPage->setEditedItem(_treeModel->getOwnerNode(editedItem) ,editedItem->getBind());
		_stackedWidget->setCurrentWidget(_soundPage);
		break;
	case ItemType::Located:
		_locatedPage->setEditedItem(_treeModel->getOwnerNode(editedItem) ,editedItem->getLoc());
		_stackedWidget->setCurrentWidget(_locatedPage);
		break;
	case ItemType::CollisionZone:
		_zonePage->setEditedItem(_treeModel->getOwnerNode(editedItem) ,editedItem->getBind());
		_stackedWidget->setCurrentWidget(_zonePage);
		break;
	case ItemType::LocatedInstance:
		_psMoverPage->setEditedItem(_treeModel->getOwnerNode(editedItem) ,editedItem->getLoc(), editedItem->getLocatedInstanceIndex());
		_stackedWidget->setCurrentWidget(_psMoverPage);
		_treeModel->getOwnerNode(editedItem)->getPSPointer()->setCurrentEditedElement(editedItem->getLoc(),
				editedItem->getLocatedInstanceIndex(),
				_psMoverPage->getLocatedBindable());
		break;
	default:
		_stackedWidget->setCurrentWidget(_wpPage);
	}
}

} /* namespace NLQT */
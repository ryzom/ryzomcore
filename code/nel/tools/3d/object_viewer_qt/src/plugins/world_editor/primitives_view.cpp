// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Project includes
#include "primitives_view.h"
#include "primitives_model.h"
#include "world_editor_actions.h"
#include "world_editor_constants.h"

#include "../core/core_constants.h"
#include "../landscape_editor/landscape_editor_constants.h"
#include "../landscape_editor/builder_zone_base.h"

// NeL includes
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>
#include <nel/ligo/primitive_class.h>
#include <nel/ligo/primitive_utils.h>

// Qt includes
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QApplication>
#include <QtGui/QMenu>
#include <QtGui/QFileDialog>

namespace WorldEditor
{

PrimitivesView::PrimitivesView(QWidget *parent)
	: QTreeView(parent),
	  m_undoStack(0),
	  m_worldEditorScene(0),
	  m_zoneBuilder(0),
	  m_primitivesTreeModel(0)
{
	setContextMenuPolicy(Qt::DefaultContextMenu);

	m_unloadAction = new QAction("Unload", this);

	m_saveAction = new QAction("Save", this);
	m_saveAction->setIcon(QIcon(Core::Constants::ICON_SAVE));

	m_saveAsAction = new QAction("Save As...", this);
	m_saveAsAction->setIcon(QIcon(Core::Constants::ICON_SAVE_AS));

	m_loadLandAction = new QAction("Load landscape file", this);
	m_loadLandAction->setIcon(QIcon(LandscapeEditor::Constants::ICON_ZONE_ITEM));

	m_loadPrimitiveAction = new QAction("Load primitive file", this);
	m_loadPrimitiveAction->setIcon(QIcon(Constants::ICON_ROOT_PRIMITIVE));

	m_newPrimitiveAction = new QAction("New primitive", this);

	m_deleteAction = new QAction("Delete", this);

	m_selectChildrenAction = new QAction("Select children", this);

	m_helpAction = new QAction("Help", this);
	m_helpAction->setEnabled(false);

	m_showAction = new QAction("Show", this);
	m_showAction->setEnabled(false);

	m_hideAction = new QAction("Hide", this);
	m_hideAction->setEnabled(false);

	connect(m_loadLandAction, SIGNAL(triggered()), this, SLOT(loadLandscape()));
	connect(m_loadPrimitiveAction, SIGNAL(triggered()), this, SLOT(loadRootPrimitive()));
	connect(m_newPrimitiveAction, SIGNAL(triggered()), this, SLOT(createRootPrimitive()));
	connect(m_selectChildrenAction, SIGNAL(triggered()), this, SLOT(selectChildren()));
	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deletePrimitives()));
	connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));
	connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(m_unloadAction, SIGNAL(triggered()), this, SLOT(unload()));
	connect(m_showAction, SIGNAL(triggered()), this, SLOT(showPrimitive()));
	connect(m_hideAction, SIGNAL(triggered()), this, SLOT(hidePrimitive()));

#ifdef Q_OS_DARWIN
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#endif
}

PrimitivesView::~PrimitivesView()
{
}

void PrimitivesView::setUndoStack(QUndoStack *undoStack)
{
	m_undoStack = undoStack;
}

void PrimitivesView::setZoneBuilder(LandscapeEditor::ZoneBuilderBase *zoneBuilder)
{
	m_zoneBuilder = zoneBuilder;
}

void PrimitivesView::setWorldScene(WorldEditorScene *worldEditorScene)
{
	m_worldEditorScene = worldEditorScene;
}

void PrimitivesView::setModel(PrimitivesTreeModel *model)
{
	QTreeView::setModel(model);
	m_primitivesTreeModel = model;
}

void PrimitivesView::loadRootPrimitive()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo primitive file"), m_lastDir,
							tr("All NeL Ligo primitive files (*.primitive)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		if (fileNames.count() > 1)
			m_undoStack->beginMacro(tr("Load primitive files"));

		Q_FOREACH(QString fileName, fileNames)
		{
			m_lastDir = QFileInfo(fileName).absolutePath();
			m_undoStack->push(new LoadRootPrimitiveCommand(fileName, m_worldEditorScene, m_primitivesTreeModel, this));
		}

		if (fileNames.count() > 1)
			m_undoStack->endMacro();
	}
	setCursor(Qt::ArrowCursor);
}

void PrimitivesView::loadLandscape()
{
	nlassert(m_undoStack);
	nlassert(m_zoneBuilder);
	nlassert(m_primitivesTreeModel);

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo land file"), m_lastDir,
							tr("All NeL Ligo land files (*.land)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		if (fileNames.count() > 1)
			m_undoStack->beginMacro(tr("Load land files"));

		Q_FOREACH(QString fileName, fileNames)
		{
			m_lastDir = QFileInfo(fileName).absolutePath();
			m_undoStack->push(new LoadLandscapeCommand(fileName, m_primitivesTreeModel, m_zoneBuilder));
		}

		if (fileNames.count() > 1)
			m_undoStack->endMacro();
	}
	setCursor(Qt::ArrowCursor);
}

void PrimitivesView::createRootPrimitive()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	m_undoStack->push(new CreateRootPrimitiveCommand("NewPrimitive", m_primitivesTreeModel));
}

void PrimitivesView::selectChildren()
{
	QModelIndexList indexList = selectionModel()->selectedRows();
	QModelIndex parentIndex = indexList.first();

	selectionModel()->clearSelection();

	QItemSelection itemSelection;
	selectChildren(parentIndex, itemSelection);
	selectionModel()->select(itemSelection, QItemSelectionModel::Select);
}

void PrimitivesView::save()
{
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();
	QModelIndex index = indexList.first();

	RootPrimitiveNode *node = static_cast<RootPrimitiveNode *>(index.internalPointer());

	if (node->data(Constants::PRIMITIVE_FILE_IS_CREATED).toBool())
	{
		if (!NLLIGO::saveXmlPrimitiveFile(*node->primitives(), node->fileName().toStdString()))
			QMessageBox::warning(this, "World Editor Qt", tr("Error writing output file: %1").arg(node->fileName()));
		else
			node->setData(Constants::PRIMITIVE_IS_MODIFIED, false);
	}
	else
		saveAs();
}

void PrimitivesView::saveAs()
{
	nlassert(m_primitivesTreeModel);

	QString fileName = QFileDialog::getSaveFileName(this,
					   tr("Save NeL Ligo primitive file"), m_lastDir,
					   tr("NeL Ligo primitive file (*.primitive)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		QModelIndexList indexList = selectionModel()->selectedRows();
		QModelIndex index = indexList.first();

		RootPrimitiveNode *node = static_cast<RootPrimitiveNode *>(index.internalPointer());

		if (!NLLIGO::saveXmlPrimitiveFile(*node->primitives(), fileName.toStdString()))
			QMessageBox::warning(this, "World Editor Qt", tr("Error writing output file: %1").arg(fileName));
		else
		{
			node->setFileName(fileName);
			node->setData(Constants::PRIMITIVE_FILE_IS_CREATED, true);
			node->setData(Constants::PRIMITIVE_IS_MODIFIED, false);
		}
	}
	setCursor(Qt::ArrowCursor);
}

void PrimitivesView::deletePrimitives()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();

	QModelIndex index = indexList.first();

	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());

	if (node->primitiveClass()->Deletable)
		m_undoStack->push(new DeletePrimitiveCommand(index, m_primitivesTreeModel, m_worldEditorScene, this));

}

void PrimitivesView::unload()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();
	QModelIndex index = indexList.first();
	Node *node = static_cast<Node *>(index.internalPointer());
	switch (node->type())
	{
	case Node::WorldEditNodeType:
	{
		break;
	}
	case Node::LandscapeNodeType:
	{
		m_undoStack->push(new UnloadLandscapeCommand(index, m_primitivesTreeModel, m_zoneBuilder));
		break;
	}
	case Node::RootPrimitiveNodeType:
	{
		m_undoStack->push(new UnloadRootPrimitiveCommand(index, m_worldEditorScene, m_primitivesTreeModel, this));
		break;
	}
	}
}

void PrimitivesView::showPrimitive()
{
}

void PrimitivesView::hidePrimitive()
{
}

void PrimitivesView::addNewPrimitiveByClass(int value)
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();

	PrimitiveNode *node = static_cast<PrimitiveNode *>(indexList.first().internalPointer());

	// Get class name
	QString className = node->primitiveClass()->DynamicChildren[value].ClassName.c_str();

	m_undoStack->push(new AddPrimitiveByClassCommand(className, m_primitivesTreeModel->pathFromIndex(indexList.first()),
					  m_worldEditorScene, m_primitivesTreeModel, this));
}

void PrimitivesView::generatePrimitives(int value)
{
}

void PrimitivesView::openItem(int value)
{
}

void PrimitivesView::contextMenuEvent(QContextMenuEvent *event)
{
	QWidget::contextMenuEvent(event);
	QModelIndexList indexList = selectionModel()->selectedRows();
	if (indexList.size() == 0)
		return;

	QMenu *popurMenu = new QMenu(this);

	if (indexList.size() == 1)
	{
		Node *node = static_cast<Node *>(indexList.first().internalPointer());
		switch (node->type())
		{
		case Node::WorldEditNodeType:
			fillMenu_WorldEdit(popurMenu);
			break;
		case Node::RootPrimitiveNodeType:
			fillMenu_RootPrimitive(popurMenu, indexList.first());
			break;
		case Node::LandscapeNodeType:
			fillMenu_Landscape(popurMenu);
			break;
		case Node::PrimitiveNodeType:
			fillMenu_Primitive(popurMenu, indexList.first());
			break;
		};
	}

	popurMenu->exec(event->globalPos());
	delete popurMenu;
	event->accept();
}

void PrimitivesView::selectChildren(const QModelIndex &parent, QItemSelection &itemSelection)
{
	const int rowCount = model()->rowCount(parent);

	QItemSelection mergeItemSelection(parent.child(0, 0), parent.child(rowCount - 1, 0));
	itemSelection.merge(mergeItemSelection, QItemSelectionModel::Select);

	for (int i = 0; i < rowCount; ++i)
	{
		QModelIndex childIndex = parent.child(i, 0);
		if (model()->rowCount(childIndex) != 0)
			selectChildren(childIndex, itemSelection);
	}
}

void PrimitivesView::fillMenu_WorldEdit(QMenu *menu)
{
	//menu->addAction(m_unloadAction);
	//menu->addAction(m_saveAction);
	//menu->addAction(m_saveAsAction);
	menu->addSeparator();
	menu->addAction(m_loadLandAction);
	menu->addAction(m_loadPrimitiveAction);
	menu->addAction(m_newPrimitiveAction);
	menu->addSeparator();
	menu->addAction(m_helpAction);
}

void PrimitivesView::fillMenu_Landscape(QMenu *menu)
{
	menu->addAction(m_unloadAction);
	menu->addSeparator();
	menu->addAction(m_showAction);
	menu->addAction(m_hideAction);
}

void PrimitivesView::fillMenu_RootPrimitive(QMenu *menu, const QModelIndex &index)
{
	menu->addAction(m_saveAction);
	menu->addAction(m_saveAsAction);
	menu->addAction(m_unloadAction);
	fillMenu_Primitive(menu, index);
	menu->removeAction(m_deleteAction);
}

void PrimitivesView::fillMenu_Primitive(QMenu *menu, const QModelIndex &index)
{
	menu->addAction(m_deleteAction);
	menu->addAction(m_selectChildrenAction);
	menu->addAction(m_helpAction);
	menu->addSeparator();
	menu->addAction(m_showAction);
	menu->addAction(m_hideAction);

	QSignalMapper *addSignalMapper = new QSignalMapper(menu);
	QSignalMapper *generateSignalMapper = new QSignalMapper(menu);
	//QSignalMapper *openSignalMapper = new QSignalMapper(menu);
	connect(addSignalMapper, SIGNAL(mapped(int)), this, SLOT(addNewPrimitiveByClass(int)));
	connect(generateSignalMapper, SIGNAL(mapped(int)), this, SLOT(generatePrimitives(int)));
	//connect(openSignalMapper, SIGNAL(mapped(int)), this, SLOT(openItem(int)));

	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());
	const NLLIGO::CPrimitiveClass *primClass = node->primitiveClass();

	// What class is it ?
	if (primClass && primClass->DynamicChildren.size())
	{
		menu->addSeparator();

		// For each child, add a create method
		for (size_t i = 0; i < primClass->DynamicChildren.size(); i++)
		{
			// Get class name
			QString className = primClass->DynamicChildren[i].ClassName.c_str();

			// Get icon
			QIcon icon(QString("%1/%2.ico").arg(Constants::PATH_TO_OLD_ICONS).arg(className));

			// Create and add action in popur menu
			QAction *action = menu->addAction(icon, tr("Add %1").arg(className));
			addSignalMapper->setMapping(action, i);
			connect(action, SIGNAL(triggered()), addSignalMapper, SLOT(map()));
		}
	}

	// What class is it ?
	if (primClass && primClass->GeneratedChildren.size())
	{
		menu->addSeparator();

		// For each child, add a create method
		for (size_t i = 0; i < primClass->GeneratedChildren.size(); i++)
		{
			// Get class name
			QString childName = primClass->GeneratedChildren[i].ClassName.c_str();

			// Create and add action in popur menu
			QAction *action = menu->addAction(tr("Generate %1").arg(childName));
			generateSignalMapper->setMapping(action, i);
			connect(action, SIGNAL(triggered()), generateSignalMapper, SLOT(map()));
		}
	}
}

} /* namespace WorldEditor */
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
//#include "primitive_item.h"
#include "primitives_model.h"

// NeL includes
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>
#include <nel/ligo/primitive_class.h>

// Qt includes
#include <QContextMenuEvent>
#include <QtGui/QMenu>

namespace WorldEditor
{

PrimitivesView::PrimitivesView(QWidget *parent)
	: QTreeView(parent),
	  m_primitivesTreeModel(0)
{
	setContextMenuPolicy(Qt::DefaultContextMenu);

	m_deleteAction = new QAction("Delete", this);
	m_selectChildrenAction = new QAction("Select children", this);
	m_helpAction = new QAction("Help", this);
	m_showAction = new QAction("Show", this);
	m_hideAction = new QAction("Hide", this);

	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deletePrimitives()));

	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(clickedItem(QModelIndex)));

#ifdef Q_OS_DARWIN
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#endif
}

PrimitivesView::~PrimitivesView()
{
}

void PrimitivesView::setModel(PrimitivesTreeModel *model)
{
	QTreeView::setModel(model);
	m_primitivesTreeModel = model;
}

void PrimitivesView::clickedItem(const QModelIndex &index)
{
}

void PrimitivesView::deletePrimitives()
{
	QModelIndexList indexList = selectionModel()->selectedRows();

	// TODO: use QPersistentModelIndex for deleting several items
	m_primitivesTreeModel->deletePrimitiveWithoutUndo(indexList.first());
}

void PrimitivesView::addNewPrimitive(int value)
{
	QModelIndexList indexList = selectionModel()->selectedRows();

	const NLLIGO::CPrimitiveClass *primClass = m_primitivesTreeModel->primitiveClass(indexList.first());

	// Get class name
	QString className = primClass->DynamicChildren[value].ClassName.c_str();

	m_primitivesTreeModel->newPrimitiveWithoutUndo(className, value, indexList.first());
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
	popurMenu->addAction(m_deleteAction);
	popurMenu->addAction(m_selectChildrenAction);
	popurMenu->addAction(m_helpAction);
	popurMenu->addSeparator();
	popurMenu->addAction(m_showAction);
	popurMenu->addAction(m_hideAction);
	popurMenu->addSeparator();

	QSignalMapper *addSignalMapper = new QSignalMapper(this);
	QSignalMapper *generateSignalMapper = new QSignalMapper(this);
	QSignalMapper *openSignalMapper = new QSignalMapper(this);
	connect(addSignalMapper, SIGNAL(mapped(int)), this, SLOT(addNewPrimitive(int)));
	connect(generateSignalMapper, SIGNAL(mapped(int)), this, SLOT(generatePrimitives(int)));
	//connect(openSignalMapper, SIGNAL(mapped(int)), this, SLOT(openItem(int)));

	if (indexList.size() == 1)
	{
		const NLLIGO::CPrimitiveClass *primClass = m_primitivesTreeModel->primitiveClass(indexList.first());

		// What class is it ?
		if (primClass && primClass->DynamicChildren.size())
		{
			popurMenu->addSeparator();

			// For each child, add a create method
			for (size_t i = 0; i < primClass->DynamicChildren.size(); i++)
			{
				// Get class name
				QString className = primClass->DynamicChildren[i].ClassName.c_str();

				// Get icon
				QIcon icon(QString("./old_ico/%1.ico").arg(className));

				// Create and add action in popur menu
				QAction *action = popurMenu->addAction(icon, QString("Add %1").arg(className));
				addSignalMapper->setMapping(action, i);
				connect(action, SIGNAL(triggered()), addSignalMapper, SLOT(map()));
			}
		}

		// What class is it ?
		if (primClass && primClass->GeneratedChildren.size())
		{
			popurMenu->addSeparator();

			// For each child, add a create method
			for (size_t i = 0; i < primClass->GeneratedChildren.size(); i++)
			{
				// Get class name
				QString childName = primClass->GeneratedChildren[i].ClassName.c_str();

				// Create and add action in popur menu
				QAction *action = popurMenu->addAction(QString("Generate %1").arg(childName));
				generateSignalMapper->setMapping(action, i);
				connect(generateSignalMapper, SIGNAL(triggered()), addSignalMapper, SLOT(map()));
			}
		}
		/*
				// What class is it ?
				if (primClass)
				{
					// Look for files
					std::vector<std::string> filenames;

					// Filenames
					buildFilenameVector (*Selection.front (),  filenames);

					// File names ?
					if (!filenames.empty ())
					{
							// Add separator
							popurMenu->addSeparator();

							// Found ?
							for (uint i = 0; i < filenames.size(); i++)
							{
								// Add a menu entry
								pMenu->AppendMenu (MF_STRING, ID_EDIT_OPEN_FILE_BEGIN+i, ("Open "+NLMISC::CFile::getFilename (filenames[i])).c_str ());
							}
					}
				}
		*/
	}

	popurMenu->exec(event->globalPos());
	delete popurMenu;
	delete addSignalMapper;
	delete generateSignalMapper;
	delete openSignalMapper;
	event->accept();
}

} /* namespace WorldEditor */
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
#include "world_editor_actions.h"
#include "world_editor_constants.h"
#include "world_editor_misc.h"
#include "primitive_item.h"
#include "world_editor_scene.h"
#include "world_editor_scene_item.h"

// Lanscape Editor plugin
#include "../landscape_editor/builder_zone_base.h"

// STL includes
#include <string>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/ligo/primitive_utils.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_class.h>
#include <nel/misc/file.h>

// Qt includes
#include <QModelIndex>
#include <QPersistentModelIndex>

namespace WorldEditor
{

void addNewGraphicsItems(const QModelIndex &primIndex, PrimitivesTreeModel *model, WorldEditorScene *scene)
{
	PrimitiveNode *node = static_cast<PrimitiveNode *>(primIndex.internalPointer());

	float cellSize = Utils::ligoConfig()->CellSize;
	if (node != 0)
	{
		NLLIGO::IPrimitive *primitive = node->primitive();
		NLLIGO::CPrimVector *vec = 0;
		AbstractWorldItem *item = 0;
		switch (node->primitiveClass()->Type)
		{
		case NLLIGO::CPrimitiveClass::Point:
		{
			vec = primitive->getPrimVector();
			NLLIGO::CPrimPoint *primPoint = static_cast<NLLIGO::CPrimPoint *>(primitive);

			// Draw arrow ?
			bool showArrow = node->primitiveClass()->ShowArrow;

			// Have a radius ?
			std::string strRadius;
			qreal radius = 0;
			if (primitive->getPropertyByName ("radius", strRadius))
				radius = atof(strRadius.c_str());

			item = scene->addWorldItemPoint(QPointF(vec->x, -vec->y + cellSize),
											primPoint->Angle, radius, showArrow);
			break;
		}
		case NLLIGO::CPrimitiveClass::Path:
		{
			QPolygonF polygon;
			vec = primitive->getPrimVector();
			int sizeVec = primitive->getNumVector();

			for (int i = 0; i < sizeVec; ++i)
			{
				polygon << QPointF(vec->x, -vec->y + cellSize);
				++vec;
			}

			item = scene->addWorldItemPath(polygon);
			break;
		}
		case NLLIGO::CPrimitiveClass::Zone:
		{
			QPolygonF polygon;
			vec = primitive->getPrimVector();
			int sizeVec = primitive->getNumVector();

			for (int i = 0; i < sizeVec; ++i)
			{
				polygon << QPointF(vec->x, -vec->y + cellSize);
				++vec;
			}
			item = scene->addWorldItemZone(polygon);
			break;
		}
		}

		if (item != 0)
		{
			// Get color from world_editor_classes.xml
			NLMISC::CRGBA color = Utils::ligoConfig()->getPrimitiveColor(*primitive);

			/*
			// Configurations (from world_editor_primitive_configuration.xml)
			const std::vector<NLLIGO::CPrimitiveConfigurations> &configurations = Utils::ligoConfig()->getPrimitiveConfiguration();

			// Look for the configuration
			sint search = 0;
			bool colorFound = false;
			while ((search = theApp.getActiveConfiguration (*primitive, search)) != -1)
			{
				// Configuration activated ?
				if (theApp.Configurations[search].Activated)
				{
					colorFound = true;
					mainColor = configurations[search].Color;
					break;
				}
				search++;
			}

			// try to get the primitive color ?
			//if (!colorFound)*/
			primitive->getPropertyByName ("Color", color);

			item->setColor(QColor(color.R, color.G, color.B));
		}

		QVariant variantNode;
		variantNode.setValue<Node *>(node);
		item->setData(Constants::WORLD_EDITOR_NODE, variantNode);

		QVariant graphicsData;
		graphicsData.setValue<AbstractWorldItem *>(item);
		node->setData(Constants::GRAPHICS_DATA_QT4_2D, graphicsData);
	}

	int count = model->rowCount(primIndex);
	for (int i = 0; i < count; ++i)
	{
		addNewGraphicsItems(primIndex.child(i, 0), model, scene);
	}
}

void removeGraphicsItems(const QModelIndex &primIndex, PrimitivesTreeModel *model, WorldEditorScene *scene)
{
	PrimitiveNode *node = static_cast<PrimitiveNode *>(primIndex.internalPointer());

	if (node != 0)
	{
		switch (node->primitiveClass()->Type)
		{
		case NLLIGO::CPrimitiveClass::Point:
		case NLLIGO::CPrimitiveClass::Path:
		case NLLIGO::CPrimitiveClass::Zone:
		{
			QGraphicsItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
			if (item != 0)
				scene->removeWorldItem(item);
			break;
		}
		}
	}

	int count = model->rowCount(primIndex);
	for (int i = 0; i < count; ++i)
	{
		removeGraphicsItems(primIndex.child(i, 0), model, scene);
	}
}

QList<Path> graphicsItemsToPaths(const QList<QGraphicsItem *> &items, PrimitivesTreeModel *model)
{
	QList<Path> result;
	Q_FOREACH(QGraphicsItem *item, items)
	{
		Node *node = qvariant_cast<Node *>(item->data(Constants::WORLD_EDITOR_NODE));
		result.push_back(model->pathFromNode(node));
	}
	return result;
}

/*
QList<GraphicsItem *> pathsToGraphicsItems(const QList<Path> &items, PrimitivesTreeModel *model)
{
	QList<GraphicsItem *> result;
}
*/

CreateWorldCommand::CreateWorldCommand(const QString &fileName, PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_fileName(fileName),
	  m_model(model)
{
	setText("Create new world");
}

CreateWorldCommand::~CreateWorldCommand()
{
}

void CreateWorldCommand::undo()
{
	m_model->deleteWorldEditNode();
}

void CreateWorldCommand::redo()
{
	m_model->createWorldEditNode(m_fileName);
}

LoadLandscapeCommand::LoadLandscapeCommand(const QString &fileName, PrimitivesTreeModel *model,
		LandscapeEditor::ZoneBuilderBase *zoneBuilder, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_id(-1),
	  m_fileName(fileName),
	  m_model(model),
	  m_zoneBuilder(zoneBuilder)
{
	setText("Load land file");
}

LoadLandscapeCommand::~LoadLandscapeCommand()
{
}

void LoadLandscapeCommand::undo()
{
	m_zoneBuilder->deleteZoneRegion(m_id);
	m_model->deleteNode(landIndex);
}

void LoadLandscapeCommand::redo()
{
	if (m_id == -1)
		m_id = m_zoneBuilder->loadZoneRegion(m_fileName);
	else
		m_zoneBuilder->loadZoneRegion(m_fileName, m_id);

	landIndex = m_model->createLandscapeNode(m_fileName);
}

CreateRootPrimitiveCommand::CreateRootPrimitiveCommand(const QString &fileName, PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_fileName(fileName),
	  m_model(model)
{
	setText("Create new primitive");
}

CreateRootPrimitiveCommand::~CreateRootPrimitiveCommand()
{
}

void CreateRootPrimitiveCommand::undo()
{
	QModelIndex index = m_model->pathToIndex(m_rootPrimIndex);

	RootPrimitiveNode *node = static_cast<RootPrimitiveNode *>(index.internalPointer());

	delete node->primitives();

	m_model->deleteNode(m_rootPrimIndex);
}

void CreateRootPrimitiveCommand::redo()
{
	NLLIGO::CPrimitives *newRootPrim = new NLLIGO::CPrimitives();
	m_rootPrimIndex = m_model->createRootPrimitiveNode(m_fileName, newRootPrim);
}


LoadRootPrimitiveCommand::LoadRootPrimitiveCommand(const QString &fileName, WorldEditorScene *scene,
		PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_fileName(fileName),
	  m_scene(scene),
	  m_model(model)
{
	setText("Load primitive file");
}

LoadRootPrimitiveCommand::~LoadRootPrimitiveCommand()
{
}

void LoadRootPrimitiveCommand::undo()
{
	QModelIndex index = m_model->pathToIndex(m_rootPrimIndex);

	removeGraphicsItems(index, m_model, m_scene);

	RootPrimitiveNode *node = static_cast<RootPrimitiveNode *>(index.internalPointer());

	delete node->primitives();

	m_model->deleteNode(m_rootPrimIndex);
}

void LoadRootPrimitiveCommand::redo()
{
	NLLIGO::CPrimitives *primitives = new NLLIGO::CPrimitives();

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = primitives;

	NLLIGO::loadXmlPrimitiveFile(*primitives, m_fileName.toStdString(), *NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig);

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;


	// Initialize default values
	Utils::recursiveUpdateDefaultValues(primitives->RootNode);

	// Check property types
	if (Utils::recursiveUpdateDefaultValues(primitives->RootNode))
	{
		nlwarning("In file (%s) : Some primitives have been modified to initialise their default values\nor to change their properties type.", m_fileName.toStdString().c_str());
	}

	m_rootPrimIndex = m_model->createRootPrimitiveNode(m_fileName, primitives);

	addNewGraphicsItems(m_model->pathToIndex(m_rootPrimIndex), m_model, m_scene);
}

AddPrimitiveByClassCommand::AddPrimitiveByClassCommand(const QString &className, const Path &parentIndex,
		PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_className(className),
	  m_parentIndex(parentIndex),
	  m_model(model)
{
	setText(QString("Add %1").arg(m_className));
}

AddPrimitiveByClassCommand::~AddPrimitiveByClassCommand()
{
}

void AddPrimitiveByClassCommand::undo()
{
	QModelIndex index = m_model->pathToIndex(m_newPrimIndex);
	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = node->rootPrimitiveNode()->primitives();

	Utils::deletePrimitive(node->primitive());

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

	m_model->deleteNode(m_newPrimIndex);
}

void AddPrimitiveByClassCommand::redo()
{
	QModelIndex parentIndex = m_model->pathToIndex(m_parentIndex);
	PrimitiveNode *parentNode = static_cast<PrimitiveNode *>(parentIndex.internalPointer());
	const NLLIGO::CPrimitiveClass *primClass = parentNode->primitiveClass();

	float delta = 10;
	int id = 0;
	while (primClass->DynamicChildren[id].ClassName != m_className.toStdString())
		++id;

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = parentNode->rootPrimitiveNode()->primitives();

	QString namePrimititve = QString("%1_%2").arg(m_className).arg(parentNode->childCount());
	NLLIGO::IPrimitive *newPrimitive = Utils::createPrimitive(m_className.toStdString().c_str(), namePrimititve.toStdString().c_str(),
									   NLMISC::CVector(), delta, primClass->DynamicChildren[id].Parameters, parentNode->primitive());

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

	m_newPrimIndex = m_model->createPrimitiveNode(newPrimitive, m_parentIndex);
}

MoveWorldItemsCommand::MoveWorldItemsCommand(const QList<QGraphicsItem *> &items, const QPointF &offset,
		PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_listPaths(graphicsItemsToPaths(items, model)),
	  m_offset(offset),
	  m_model(model),
	  m_firstRun(true)
{
	setText("Move item(s)");
}

MoveWorldItemsCommand::~MoveWorldItemsCommand()
{
}

void MoveWorldItemsCommand::undo()
{
	for (int i = 0; i < m_listPaths.count(); ++i)
	{
		Node *node = m_model->pathToNode(m_listPaths.at(i));
		AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
		item->moveBy(-m_offset.x(), -m_offset.y());
	}
}

void MoveWorldItemsCommand::redo()
{
	if (!m_firstRun)
	{
		for (int i = 0; i < m_listPaths.count(); ++i)
		{
			Node *node = m_model->pathToNode(m_listPaths.at(i));
			AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
			item->moveBy(m_offset.x(), m_offset.y());
		}
	}
	m_firstRun = false;
}

RotateWorldItemsCommand::RotateWorldItemsCommand(const QList<QGraphicsItem *> &items, const qreal angle,
		const QPointF &pivot, PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_listPaths(graphicsItemsToPaths(items, model)),
	  m_angle(angle),
	  m_pivot(pivot),
	  m_model(model),
	  m_firstRun(true)
{
	setText(QString("Rotate item(s) %1").arg(m_angle));
}

RotateWorldItemsCommand::~RotateWorldItemsCommand()
{
}

void RotateWorldItemsCommand::undo()
{
	for (int i = 0; i < m_listPaths.count(); ++i)
	{
		Node *node = m_model->pathToNode(m_listPaths.at(i));
		AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
		item->rotateOn(m_pivot, -m_angle);
	}
}

void RotateWorldItemsCommand::redo()
{
	if (!m_firstRun)
	{
		for (int i = 0; i < m_listPaths.count(); ++i)
		{
			Node *node = m_model->pathToNode(m_listPaths.at(i));
			AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
			item->rotateOn(m_pivot, m_angle);
		}
	}
	m_firstRun = false;
}

ScaleWorldItemsCommand::ScaleWorldItemsCommand(const QList<QGraphicsItem *> &items, const QPointF &factor,
		const QPointF &pivot, PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_listPaths(graphicsItemsToPaths(items, model)),
	  m_factor(factor),
	  m_pivot(pivot),
	  m_model(model),
	  m_firstRun(true)
{
	setText("Scale item(s)");
}

ScaleWorldItemsCommand::~ScaleWorldItemsCommand()
{
}

void ScaleWorldItemsCommand::undo()
{
	QPointF m_invertFactor(1 / m_factor.x(), 1 / m_factor.y());
	for (int i = 0; i < m_listPaths.count(); ++i)
	{
		Node *node = m_model->pathToNode(m_listPaths.at(i));
		AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
		item->scaleOn(m_pivot, m_invertFactor);
	}
}

void ScaleWorldItemsCommand::redo()
{
	if (!m_firstRun)
	{
		for (int i = 0; i < m_listPaths.count(); ++i)
		{
			Node *node = m_model->pathToNode(m_listPaths.at(i));
			AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
			item->scaleOn(m_pivot, m_factor);
		}
	}
	m_firstRun = false;
}

TurnWorldItemsCommand::TurnWorldItemsCommand(const QList<QGraphicsItem *> &items, const qreal angle,
		PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_listPaths(graphicsItemsToPaths(items, model)),
	  m_angle(angle),
	  m_model(model),
	  m_firstRun(true)
{
	setText(QString("Turn item(s) %1").arg(m_angle));
}

TurnWorldItemsCommand::~TurnWorldItemsCommand()
{
}

void TurnWorldItemsCommand::undo()
{
	for (int i = 0; i < m_listPaths.count(); ++i)
	{
		Node *node = m_model->pathToNode(m_listPaths.at(i));
		AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
		item->turnOn(-m_angle);
	}
}

void TurnWorldItemsCommand::redo()
{
	if (!m_firstRun)
	{
		for (int i = 0; i < m_listPaths.count(); ++i)
		{
			Node *node = m_model->pathToNode(m_listPaths.at(i));
			AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
			item->turnOn(m_angle);
		}
	}
	m_firstRun = false;
}

} /* namespace WorldEditor */

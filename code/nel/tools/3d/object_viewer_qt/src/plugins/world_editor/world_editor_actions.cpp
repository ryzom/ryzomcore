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
#include "world_editor_misc.h"
#include "primitive_item.h"

// Lanscape Editor plugin
#include "../landscape_editor/builder_zone_base.h"

// STL includes
#include <string>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/ligo/primitive_utils.h>
#include <nel/ligo/primitive.h>
#include <nel/misc/file.h>

// Qt includes
#include <QModelIndex>

namespace WorldEditor
{

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


LoadRootPrimitiveCommand::LoadRootPrimitiveCommand(const QString &fileName, PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_fileName(fileName),
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

} /* namespace WorldEditor */

// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include <nel/misc/mem_stream.h>
#include <nel/misc/o_xml.h>

// Qt includes
#include <QModelIndex>
#include <QGraphicsView>
#include <QPersistentModelIndex>
#include <QApplication>
#include <QMessageBox>

// libxml
#include <libxml/xmlstring.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

namespace WorldEditor
{

QGraphicsItem *getGraphicsItem(Node *node)
{
	QGraphicsItem *result = 0;
	if (node->type() == Node::PrimitiveNodeType)
	{
		PrimitiveNode *primitiveNode = static_cast<PrimitiveNode *>(node);
		if (primitiveNode != 0)
		{
			switch (primitiveNode->primitiveClass()->Type)
			{
			case NLLIGO::CPrimitiveClass::Point:
			case NLLIGO::CPrimitiveClass::Path:
			case NLLIGO::CPrimitiveClass::Zone:
			{
				result = qvariant_cast<AbstractWorldItem *>(primitiveNode->data(Constants::GRAPHICS_DATA_QT4_2D));
				break;
			}
			}
		}
	}
	return result;
}

void addNewGraphicsItems(const QModelIndex &primIndex, PrimitivesTreeModel *model, WorldEditorScene *scene)
{
	PrimitiveNode *node = static_cast<PrimitiveNode *>(primIndex.internalPointer());

	float cellSize = Utils::ligoConfig()->CellSize;
	if (node != 0)
	{
		NLLIGO::IPrimitive *primitive = node->primitive();
		NLLIGO::CPrimVector *vec = 0;
		AbstractWorldItem *item = 0;

		// Draw arrow ?
		bool showArrow = node->primitiveClass()->ShowArrow;

		switch (node->primitiveClass()->Type)
		{
		case NLLIGO::CPrimitiveClass::Point:
		{
			vec = primitive->getPrimVector();
			NLLIGO::CPrimPoint *primPoint = static_cast<NLLIGO::CPrimPoint *>(primitive);

			// Have a radius ?
			std::string strRadius;
			qreal radius = 0;
			if (primitive->getPropertyByName ("radius", strRadius))
				radius = atof(strRadius.c_str());
			qreal angle = ((2 * NLMISC::Pi - primPoint->Angle) * 180 / NLMISC::Pi);
			item = scene->addWorldItemPoint(QPointF(vec->x, -vec->y + cellSize),
											angle, radius, showArrow);
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
			item = scene->addWorldItemPath(polygon, showArrow);
			break;
		}
		case NLLIGO::CPrimitiveClass::Zone:
		{
			QPolygonF polygon;
			vec = primitive->getPrimVector();
			int sizeVec = primitive->getNumVector();
			for (int i = 0; i < sizeVec; ++i)
			{
				polygon << QPointF(vec->x, cellSize - vec->y);
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
			primitive->getPropertyByName ("Color", color);
			item->setColor(QColor(color.R, color.G, color.B));

			QVariant variantNode;
			variantNode.setValue<Node *>(node);
			item->setData(Constants::WORLD_EDITOR_NODE, variantNode);

			QVariant graphicsData;
			graphicsData.setValue<AbstractWorldItem *>(item);
			node->setData(Constants::GRAPHICS_DATA_QT4_2D, graphicsData);

			QVariant persistenVariant;
			QPersistentModelIndex *persistentIndex = new QPersistentModelIndex(primIndex);
			persistenVariant.setValue<QPersistentModelIndex *>(persistentIndex);
			item->setData(Constants::NODE_PERISTENT_INDEX, persistenVariant);
		}
	}

	int count = model->rowCount(primIndex);
	for (int i = 0; i < count; ++i)
	{
		addNewGraphicsItems(primIndex.child(i, 0), model, scene);
	}
}

void removeGraphicsItems(const QModelIndex &primIndex, PrimitivesTreeModel *model, WorldEditorScene *scene)
{
	Node *node = static_cast<Node *>(primIndex.internalPointer());

	if (node != 0)
	{
		QGraphicsItem *item = getGraphicsItem(node);
		if (item != 0)
		{
			delete qvariant_cast<QPersistentModelIndex *>(item->data(Constants::NODE_PERISTENT_INDEX));
			scene->removeWorldItem(item);
		}
	}

	int count = model->rowCount(primIndex);
	for (int i = 0; i < count; ++i)
	{
		removeGraphicsItems(primIndex.child(i, 0), model, scene);
	}
}

QList<QPolygonF> polygonsFromItems(const QList<QGraphicsItem *> &items)
{
	QList<QPolygonF> result;
	Q_FOREACH(QGraphicsItem *item, items)
	{
		AbstractWorldItem *worldItem = qgraphicsitem_cast<AbstractWorldItem *>(item);
		result.push_back(worldItem->polygon());
	}
	return result;
}

CreateWorldCommand::CreateWorldCommand(const QString &fileName, PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_fileName(fileName),
	  m_model(model)
{
	setText(QObject::tr("Create new world"));
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


WorldSaver::WorldSaver( PrimitivesTreeModel *model, LandscapeEditor::ZoneBuilderBase *zoneBuilder, const std::string &dataDir, const std::string &context )
{
	m_model = model;
	m_zoneBuilder = zoneBuilder;
	m_dataDir = dataDir;
	m_context = context;
}

void WorldSaver::setLoadedFile(const std::string &fileName, const std::string &content, const std::string &dataDir,
	const std::string &context, const NotLoadedFiles &notLoaded)
{
	m_loadedFileName = fileName;
	m_loadedContent = content;
	m_loadedDataDir = dataDir;
	m_loadedContext = context;
	m_notLoaded = notLoaded;
}

bool WorldSaver::save()
{
	QModelIndex pidx = m_model->index( 0, 0 );
	if( !pidx.isValid() )
	{
		lastError = "No root node.";
		return false;
	}

	try
	{
		m_data.clear();
		std::vector<Node *> nodes;
		int rows = m_model->rowCount(pidx);
		for (int i = 0; i < rows; i++)
		{
			QModelIndex idx = m_model->index(i, 0, pidx);
			Node *node = reinterpret_cast<Node *>(idx.internalPointer());
			if (LandscapeNode *land = dynamic_cast<LandscapeNode *>(node))
			{
				m_data.push_back(std::make_pair(std::string("landscape"), std::string(land->fileName().toUtf8().constData())));
				nodes.push_back(node);
			}
			else if (RootPrimitiveNode *primitive = dynamic_cast<RootPrimitiveNode *>(node))
			{
				m_data.push_back(std::make_pair(std::string("primitive"), std::string(primitive->fileName().toUtf8().constData())));
				nodes.push_back(node);
			}
		}

		// the .worldedit content is made first, so that nothing is written when it cannot be made
		if (!makeWorldEditFile())
			return false;

		for (size_t i = 0; i < nodes.size(); ++i)
		{
			bool ok = (nodes[i]->type() == Node::LandscapeNodeType) ? saveLandscapeNode(nodes[i]) : savePrimitiveNode(nodes[i]);
			if (!ok)
				return false;
		}

		m_currentFile = m_worldEditFileName;
		if (!Utils::writeFileIfChanged(m_worldEditFileName, m_worldEditContent, lastError))
			return false;
	}
	catch (const NLMISC::Exception &e)
	{
		lastError = "Error writing " + m_currentFile + ": " + e.what();
		return false;
	}
	return true;
}

bool WorldSaver::saveLandscapeNode( Node *node )
{
	LandscapeNode *n = dynamic_cast< LandscapeNode* >( node );
	if( n == NULL )
	{
		lastError = "Trying to save a non-landscape node as landscape.";
		return false;
	}

	LandscapeEditor::ZoneRegionObject *z = m_zoneBuilder->zoneRegion( n->id() );
	if( z == NULL )
	{
		lastError = "The specified zoneregion doesn't exist.";
		return false;
	}

	m_currentFile = n->fileName().toUtf8().constData();
	z->setFileName(m_currentFile);

	// the land file as ZoneRegionObject::save writes it
	NLMISC::CMemStream stream;
	{
		NLMISC::COXml xml;
		xml.init(&stream);
		z->ligoZoneRegion().serial(xml);
	}
	const std::string content((const char *)stream.buffer(), stream.length());

	if (!Utils::writeFileIfChanged(m_currentFile, content, lastError))
		return false;
	z->setModified(false);
	return true;
}

bool WorldSaver::savePrimitiveNode( Node *node )
{
	RootPrimitiveNode *n = dynamic_cast< RootPrimitiveNode* >( node );
	if( n == NULL )
	{
		lastError = "Trying to save a non-primitive node as primitive.";
		return false;
	}

	m_currentFile = n->fileName().toUtf8().constData();
	if (!Utils::savePrimitivesFile(*(n->primitives()), m_currentFile, lastError))
		return false;
	n->setData(Constants::PRIMITIVE_IS_MODIFIED, false);
	return true;
}

bool WorldSaver::makeWorldEditFile()
{
	QModelIndex idx = m_model->index( 0, 0 );
	Node *node = reinterpret_cast< Node* >( idx.internalPointer() );

	WorldEditNode *n = dynamic_cast< WorldEditNode* >( node );
	if( n == NULL )
	{
		lastError = "Not a worldedit node.";
		return false;
	}

	m_worldEditFileName = n->data(0).toString().toUtf8().constData();
	if (m_worldEditFileName.empty())
	{
		lastError = "The world has no file name, nothing was written.";
		return false;
	}
	m_currentFile = m_worldEditFileName;

	NLMISC::CMemStream stream;
	{
		NLMISC::COXml xml;
		xml.init(&stream);

		// the file the world was loaded from is updated, a new world is made from the model
		if (!m_loadedContent.empty() && m_worldEditFileName == m_loadedFileName)
		{
			xmlDocPtr updatedDoc = updateLoadedFile(m_worldEditFileName);
			if (updatedDoc == NULL)
				return false;
			xmlDocSetRootElement(xml.getDocument(), xmlDocCopyNode(xmlDocGetRootElement(updatedDoc), xml.getDocument(), 1));
			xmlFreeDoc(updatedDoc);
		}
		else
		{
			xmlNodePtr rootNode = xmlNewDocNode(xml.getDocument(), NULL, (const xmlChar *)"NEL_WORLD_EDITOR_PROJECT", NULL);
			xmlDocSetRootElement(xml.getDocument(), rootNode);
			xmlNodePtr node = xmlNewChild(rootNode, NULL, (const xmlChar *)"VERSION", NULL);
			xmlNodePtr text = xmlNewText((const xmlChar *)NLMISC::toString(WORLD_EDITOR_FILE_VERSION).c_str());
			xmlAddChild(node, text);
			node = xmlNewChild(rootNode, NULL, (const xmlChar *)"DATA_DIRECTORY", NULL);
			xmlSetProp(node, (const xmlChar *)"VALUE", (const xmlChar *)m_dataDir.c_str());
			node = xmlNewChild(rootNode, NULL, (const xmlChar *)"CONTEXT", NULL);
			xmlSetProp(node, (const xmlChar *)"VALUE", (const xmlChar *)m_context.c_str());

			const std::string folder = NLMISC::CFile::getPath(m_worldEditFileName);
			for (size_t i = 0; i < m_data.size(); ++i)
				addDatabaseElement(rootNode, m_data[i], folder);
		}

		xml.flush();
	}
	m_worldEditContent = std::string((const char *)stream.buffer(), stream.length());
	return true;
}

void WorldSaver::addDatabaseElement(struct _xmlNode *rootNode, const std::pair<std::string, std::string> &entry, const std::string &folder)
{
	// as the MFC editor does, the name is relative to the folder of the .worldedit, or absolute when the
	// two share no folder below the root
	std::string name = entry.second;
	NLMISC::CPath::makePathRelative(folder, name);

	xmlNodePtr node = xmlNewChild(rootNode, NULL, (const xmlChar *)"DATABASE_ELEMENT", NULL);
	xmlSetProp(node, (const xmlChar *)"FILENAME", (const xmlChar *)name.c_str());
	xmlSetProp(node, (const xmlChar *)"TYPE", (const xmlChar *)(entry.first == "landscape" ? "landscape" : "primitive"));
}

struct _xmlDoc *WorldSaver::updateLoadedFile(const std::string &fn)
{
	// start from the file as it was loaded, so that what this editor does not handle stays as it was
	xmlDocPtr doc = xmlReadMemory(m_loadedContent.data(), (int)m_loadedContent.size(), fn.c_str(), NULL, XML_PARSE_NOBLANKS);
	xmlNodePtr rootNode = doc ? xmlDocGetRootElement(doc) : NULL;
	if (rootNode == NULL)
	{
		if (doc)
			xmlFreeDoc(doc);
		lastError = "The world editor file " + fn + " as it was loaded cannot be read again, nothing was written.";
		return NULL;
	}

	const std::string folder = NLMISC::CFile::getPath(fn);
	std::vector<bool> written(m_data.size(), false);
	bool hasDataDir = false;
	bool hasContext = false;

	xmlNodePtr child = rootNode->children;
	while (child != NULL)
	{
		xmlNodePtr next = child->next;
		if (child->type == XML_ELEMENT_NODE)
		{
			const std::string name = (const char *)child->name;
			if (name == "DATA_DIRECTORY")
			{
				hasDataDir = true;
				if (m_dataDir != m_loadedDataDir)
					xmlSetProp(child, (const xmlChar *)"VALUE", (const xmlChar *)m_dataDir.c_str());
			}
			else if (name == "CONTEXT")
			{
				hasContext = true;
				if (m_context != m_loadedContext)
					xmlSetProp(child, (const xmlChar *)"VALUE", (const xmlChar *)m_context.c_str());
			}
			else if (name == "DATABASE_ELEMENT")
			{
				std::string type;
				NLMISC::CIXml::getPropertyString(type, child, "TYPE");

				// an element of a type the loader does not read is kept as it is
				if (type == "landscape" || type == "primitive")
				{
					std::string fileName;
					NLMISC::CIXml::getPropertyString(fileName, child, "FILENAME");
					const std::string path = Utils::worldEditFilePath(fileName, folder);

					// an element keeps its name when its file is in the model, matched first so that
					// each file of the model takes one element, or else when its file is not loaded
					bool keep = false;
					for (size_t i = 0; i < m_data.size() && !keep; ++i)
					{
						if (!written[i] && m_data[i].second == path)
						{
							written[i] = true;
							keep = true;
						}
					}
					if (!keep)
						keep = m_notLoaded.count(path) != 0;

					// the others were unloaded in the editor
					if (!keep)
					{
						xmlUnlinkNode(child);
						xmlFreeNode(child);
					}
				}
			}
		}
		child = next;
	}

	if (!hasDataDir && m_dataDir != m_loadedDataDir)
	{
		xmlNodePtr node = xmlNewChild(rootNode, NULL, (const xmlChar *)"DATA_DIRECTORY", NULL);
		xmlSetProp(node, (const xmlChar *)"VALUE", (const xmlChar *)m_dataDir.c_str());
	}
	if (!hasContext && m_context != m_loadedContext)
	{
		xmlNodePtr node = xmlNewChild(rootNode, NULL, (const xmlChar *)"CONTEXT", NULL);
		xmlSetProp(node, (const xmlChar *)"VALUE", (const xmlChar *)m_context.c_str());
	}

	// the files of the model that the file did not name are added
	for (size_t i = 0; i < m_data.size(); ++i)
	{
		if (!written[i])
			addDatabaseElement(rootNode, m_data[i], folder);
	}

	return doc;
}

// Keeps the files that are not loaded in step with a load: true when it changed them
static bool updateNotLoaded(NotLoadedFiles *notLoaded, const QString &fileName, bool loaded)
{
	if (notLoaded == NULL)
		return false;
	const std::string name = fileName.toUtf8().constData();
	if (loaded)
		return notLoaded->erase(name) != 0;
	return notLoaded->insert(name).second;
}

// Undoes what updateNotLoaded changed
static void restoreNotLoaded(NotLoadedFiles *notLoaded, const QString &fileName, bool loaded, bool changed)
{
	if (notLoaded == NULL || !changed)
		return;
	const std::string name = fileName.toUtf8().constData();
	if (loaded)
		notLoaded->insert(name);
	else
		notLoaded->erase(name);
}

LoadLandscapeCommand::LoadLandscapeCommand(const QString &fileName, PrimitivesTreeModel *model,
		LandscapeEditor::ZoneBuilderBase *zoneBuilder, NotLoadedFiles *notLoaded, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_id(-1),
	  m_fileName(fileName),
	  m_model(model),
	  m_zoneBuilder(zoneBuilder),
	  m_notLoaded(notLoaded),
	  m_loaded(false),
	  m_notLoadedChanged(false),
	  m_firstRedo(true)
{
	setText(QObject::tr("Load land file"));
}

LoadLandscapeCommand::~LoadLandscapeCommand()
{
}

void LoadLandscapeCommand::undo()
{
	restoreNotLoaded(m_notLoaded, m_fileName, m_loaded, m_notLoadedChanged);
	m_notLoadedChanged = false;

	if (!m_loaded)
		return;

	m_zoneBuilder->deleteZoneRegion(m_id);
	m_model->deleteNode(landIndex);
}

void LoadLandscapeCommand::redo()
{
	// the first load gives the land a new id, the next ones keep it
	m_error.clear();
	int id = m_zoneBuilder->loadZoneRegion(m_fileName, m_id, &m_error);
	m_loaded = (id != -1);
	m_notLoadedChanged = updateNotLoaded(m_notLoaded, m_fileName, m_loaded);
	bool firstRedo = m_firstRedo;
	m_firstRedo = false;
	if (!m_loaded)
	{
		// a land that did not load gets no node, so that saving does not write it back empty; the code
		// that pushed the command says so the first time
		nlwarning("The land file %s could not be loaded", m_fileName.toUtf8().constData());
		if (!firstRedo)
		{
			QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Error opening land file"),
				QObject::tr("%1 is not loaded: %2.").arg(m_fileName, m_error));
		}
		return;
	}

	m_id = id;
	landIndex = m_model->createLandscapeNode(m_fileName, m_id);
}


UnloadLandscapeCommand::UnloadLandscapeCommand(const QModelIndex &index, PrimitivesTreeModel *model,
		LandscapeEditor::ZoneBuilderBase *zoneBuilder, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_model(model),
	  m_zoneBuilder(zoneBuilder)
{
	setText(QObject::tr("Unload land file"));
	m_path = m_model->pathFromIndex(index);
}

UnloadLandscapeCommand::~UnloadLandscapeCommand()
{
}

void UnloadLandscapeCommand::undo()
{
	m_zoneBuilder->loadZoneRegion(m_fileName, m_id);

	m_model->createLandscapeNode(m_fileName, m_id, m_path.back().first);
}

void UnloadLandscapeCommand::redo()
{
	QModelIndex index = m_model->pathToIndex(m_path);
	LandscapeNode *node = static_cast<LandscapeNode *>(index.internalPointer());

	m_id = node->id();
	m_fileName = node->fileName();

	m_zoneBuilder->deleteZoneRegion(m_id);
	m_model->deleteNode(m_path);
}

CreateRootPrimitiveCommand::CreateRootPrimitiveCommand(const QString &fileName, PrimitivesTreeModel *model, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_fileName(fileName),
	  m_model(model)
{
	setText(QObject::tr("Create new primitive"));
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
	m_rootPrimIndex = m_model->createRootPrimitiveNode("", newRootPrim);
}


LoadRootPrimitiveCommand::LoadRootPrimitiveCommand(const QString &fileName, WorldEditorScene *scene,
		PrimitivesTreeModel *model, QTreeView *view, NotLoadedFiles *notLoaded, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_fileName(fileName),
	  m_scene(scene),
	  m_model(model),
	  m_view(view),
	  m_notLoaded(notLoaded),
	  m_loaded(false),
	  m_notLoadedChanged(false),
	  m_firstRedo(true)
{
	setText(QObject::tr("Load primitive file"));
}

LoadRootPrimitiveCommand::~LoadRootPrimitiveCommand()
{
}

void LoadRootPrimitiveCommand::undo()
{
	restoreNotLoaded(m_notLoaded, m_fileName, m_loaded, m_notLoadedChanged);
	m_notLoadedChanged = false;

	if (!m_loaded)
		return;

	// Disable edit points mode
	m_scene->setEnabledEditPoints(false);

	m_view->selectionModel()->clearSelection();

	QModelIndex index = m_model->pathToIndex(m_rootPrimIndex);

	removeGraphicsItems(index, m_model, m_scene);

	RootPrimitiveNode *node = static_cast<RootPrimitiveNode *>(index.internalPointer());

	delete node->primitives();

	m_model->deleteNode(m_rootPrimIndex);
}

void LoadRootPrimitiveCommand::redo()
{
	m_scene->setEnabledEditPoints(false);

	NLLIGO::CPrimitives *primitives = new NLLIGO::CPrimitives();

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = primitives;

	m_loaded = NLLIGO::loadXmlPrimitiveFile(*primitives, m_fileName.toUtf8().constData(), *Utils::ligoConfig());

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

	m_notLoadedChanged = updateNotLoaded(m_notLoaded, m_fileName, m_loaded);
	bool firstRedo = m_firstRedo;
	m_firstRedo = false;
	if (!m_loaded)
	{
		// a primitive file that did not load gets no node, so that saving does not write it back empty;
		// the code that pushed the command says so the first time
		nlwarning("The primitive file %s could not be loaded", m_fileName.toUtf8().constData());
		delete primitives;
		if (!firstRedo)
		{
			QMessageBox::warning(QApplication::activeWindow(), QObject::tr("Error opening primitive file"),
				QObject::tr("%1 is not loaded: it cannot be read, the log says why.").arg(m_fileName));
		}
		return;
	}


	// Initialize default values
	Utils::recursiveUpdateDefaultValues(primitives->RootNode);

	// Check property types
	if (Utils::recursiveUpdateDefaultValues(primitives->RootNode))
	{
		nlwarning("In file (%s) : Some primitives have been modified to initialise their default values\nor to change their properties type.", m_fileName.toUtf8().constData());
	}

	m_rootPrimIndex = m_model->createRootPrimitiveNode(m_fileName, primitives);

	addNewGraphicsItems(m_model->pathToIndex(m_rootPrimIndex), m_model, m_scene);
}


UnloadRootPrimitiveCommand::UnloadRootPrimitiveCommand(const QModelIndex &index, WorldEditorScene *scene,
		PrimitivesTreeModel *model, QTreeView *view, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_scene(scene),
	  m_model(model),
	  m_view(view)
{
	setText(QObject::tr("Unload primitive file"));
	m_path = m_model->pathFromIndex(index);
}

UnloadRootPrimitiveCommand::~UnloadRootPrimitiveCommand()
{
}

void UnloadRootPrimitiveCommand::undo()
{
	// Disable edit points mode
	m_scene->setEnabledEditPoints(false);

	m_path = m_model->createRootPrimitiveNode(m_fileName, m_primitives, m_path.back().first);

	addNewGraphicsItems(m_model->pathToIndex(m_path), m_model, m_scene);
}

void UnloadRootPrimitiveCommand::redo()
{
	m_scene->setEnabledEditPoints(false);

	m_view->selectionModel()->clearSelection();
	QModelIndex index = m_model->pathToIndex(m_path);
	RootPrimitiveNode *node = static_cast<RootPrimitiveNode *>(index.internalPointer());
	m_fileName = node->fileName();
	m_primitives = node->primitives();

	removeGraphicsItems(index, m_model, m_scene);

	m_model->deleteNode(m_path);
}

AddPrimitiveByClassCommand::AddPrimitiveByClassCommand(const QString &className, const Path &parentIndex,
		WorldEditorScene *scene, PrimitivesTreeModel *model, QTreeView *view, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_className(className),
	  m_parentIndex(parentIndex),
	  m_scene(scene),
	  m_model(model),
	  m_view(view)
{
	setText(QObject::tr("Add %1").arg(m_className));

	QGraphicsView *graphicsView = m_scene->views().first();

	// TODO: returns incorrect position when zoom in
	QRectF visibleArea = graphicsView->mapToScene(view->rect()).boundingRect();
	m_delta = visibleArea.height() / 10.0;
	m_initPos = visibleArea.center();
}

AddPrimitiveByClassCommand::~AddPrimitiveByClassCommand()
{
}

void AddPrimitiveByClassCommand::undo()
{
	m_scene->setEnabledEditPoints(false);

	m_view->selectionModel()->clearSelection();

	QModelIndex index = m_model->pathToIndex(m_newPrimIndex);
	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = node->rootPrimitiveNode()->primitives();

	removeGraphicsItems(index, m_model, m_scene);

	Utils::deletePrimitive(node->primitive());

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

	m_model->deleteNode(m_newPrimIndex);
}

void AddPrimitiveByClassCommand::redo()
{
	m_scene->setEnabledEditPoints(false);

	QModelIndex parentIndex = m_model->pathToIndex(m_parentIndex);
	PrimitiveNode *parentNode = static_cast<PrimitiveNode *>(parentIndex.internalPointer());
	const NLLIGO::CPrimitiveClass *primClass = parentNode->primitiveClass();

	std::string className = m_className.toUtf8().constData();

	int id = 0;
	while (primClass->DynamicChildren[id].ClassName != className)
		++id;

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = parentNode->rootPrimitiveNode()->primitives();

	QString namePrimititve = QString("%1_%2").arg(m_className).arg(parentNode->childCount());
	NLLIGO::IPrimitive *newPrimitive = Utils::createPrimitive(m_className.toUtf8().constData(), namePrimititve.toUtf8().constData(),
									   NLMISC::CVector(m_initPos.x(), -m_initPos.y(), 0.0), m_delta, primClass->DynamicChildren[id].Parameters, parentNode->primitive());

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

	m_newPrimIndex = m_model->createPrimitiveNode(newPrimitive, m_parentIndex);

	addNewGraphicsItems(m_model->pathToIndex(m_newPrimIndex), m_model, m_scene);
}

DeletePrimitiveCommand::DeletePrimitiveCommand(const QModelIndex &index, PrimitivesTreeModel *model,
		WorldEditorScene *scene, QTreeView *view, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_scene(scene),
	  m_model(model),
	  m_view(view)
{
	setText(QObject::tr("Delete primitive"));

	// Save path to primitive
	m_path = m_model->pathFromIndex(index);
	m_parentPath = m_model->pathFromIndex(index.parent());

	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());

	NLLIGO::IPrimitive *primitive = node->primitive();

	// Backup primitive
	m_oldPrimitive = primitive->copy();

	// Backup position primitive
	primitive->getParent()->getChildId(m_posPrimitive, primitive);
}

DeletePrimitiveCommand::~DeletePrimitiveCommand()
{
	delete m_oldPrimitive;
}

void DeletePrimitiveCommand::undo()
{
	m_scene->setEnabledEditPoints(false);
	m_view->selectionModel()->clearSelection();

	QModelIndex parentIndex = m_model->pathToIndex(m_parentPath);
	PrimitiveNode *parentNode = static_cast<PrimitiveNode *>(parentIndex.internalPointer());

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = parentNode->rootPrimitiveNode()->primitives();

	NLLIGO::IPrimitive *newPrimitive = m_oldPrimitive->copy();
	if (!parentNode->primitive()->insertChild(newPrimitive, m_posPrimitive))
		nlerror("Primitive can't insert, m_posPrimitive is not a valid.");

	// Insert primitive node in tree model
	Path newPath = m_model->createPrimitiveNode(newPrimitive, m_parentPath, m_path.back().first);

	// Scan graphics model
	addNewGraphicsItems(m_model->pathToIndex(newPath), m_model, m_scene);

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;
}

void DeletePrimitiveCommand::redo()
{
	m_scene->setEnabledEditPoints(false);
	m_view->selectionModel()->clearSelection();

	QModelIndex index = m_model->pathToIndex(m_path);
	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());
	NLLIGO::IPrimitive *primitive = node->primitive();

	// Removes all graphics items
	removeGraphicsItems(index, m_model, m_scene);

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = node->rootPrimitiveNode()->primitives();

	// Delete primitive
	Utils::deletePrimitive(primitive);

	// unset the context
	NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

	// Remove primitive from tree model
	m_model->deleteNode(m_path);
}

AbstractWorldItemCommand::AbstractWorldItemCommand(const QList<QGraphicsItem *> &items,
		WorldEditorScene *scene,
		PrimitivesTreeModel *model,
		QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_listPaths(graphicsItemsToPaths(items, model)),
	  m_model(model),
	  m_scene(scene),
	  m_firstRun(true)
{
}

AbstractWorldItemCommand::~AbstractWorldItemCommand()
{
}

void AbstractWorldItemCommand::undo()
{
	bool pointsMode = m_scene->isEnabledEditPoints();
	m_scene->setEnabledEditPoints(false);
	for (int i = 0; i < m_listPaths.count(); ++i)
	{
		Node *node = m_model->pathToNode(m_listPaths.at(i));
		AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
		undoChangeItem(i, item);
		updatePrimitiveData(item);
	}
	m_scene->setEnabledEditPoints(pointsMode);
}

void AbstractWorldItemCommand::redo()
{
	if (!m_firstRun)
	{
		bool pointsMode = m_scene->isEnabledEditPoints();
		m_scene->setEnabledEditPoints(false);
		for (int i = 0; i < m_listPaths.count(); ++i)
		{
			Node *node = m_model->pathToNode(m_listPaths.at(i));
			AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
			redoChangeItem(i, item);
			updatePrimitiveData(item);
		}
		m_scene->setEnabledEditPoints(pointsMode);
	}
	else
	{
		for (int i = 0; i < m_listPaths.count(); ++i)
		{
			Node *node = m_model->pathToNode(m_listPaths.at(i));
			AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
			updatePrimitiveData(item);
		}
	}

	m_firstRun = false;
}

void AbstractWorldItemCommand::updatePrimitiveData(AbstractWorldItem *item)
{
	float cellSize = Utils::ligoConfig()->CellSize;
	Node *node = qvariant_cast<Node *>(item->data(Constants::WORLD_EDITOR_NODE));
	PrimitiveNode *primitiveNode = static_cast<PrimitiveNode *>(node);
	if (primitiveNode != 0)
	{
		NLLIGO::IPrimitive *primitive = primitiveNode->primitive();

		std::vector<NLLIGO::CPrimVector> vPoints;
		QPolygonF polygon = item->polygon();
		polygon.translate(item->pos());

		for (int i = 0; i < polygon.size(); ++i)
		{
			NLMISC::CVector vec(polygon.at(i).x(), cellSize - polygon.at(i).y(), 0.0);
			vPoints.push_back(NLLIGO::CPrimVector(vec));
		}

		switch (primitiveNode->primitiveClass()->Type)
		{
		case NLLIGO::CPrimitiveClass::Point:
		{
			qreal angle = static_cast<WorldItemPoint *>(item)->angle();
			angle = 2 * NLMISC::Pi - (angle * NLMISC::Pi / 180.0);
			NLLIGO::CPrimPoint *point = static_cast<NLLIGO::CPrimPoint *>(primitive);
			point->Point = vPoints.front();
			point->Angle = angle;
			break;
		}
		case NLLIGO::CPrimitiveClass::Path:
		{
			NLLIGO::CPrimPath *path = static_cast<NLLIGO::CPrimPath *>(primitive);
			path->VPoints = vPoints;
			break;
		}
		case NLLIGO::CPrimitiveClass::Zone:
		{
			NLLIGO::CPrimZone *zone = static_cast<NLLIGO::CPrimZone *>(primitive);
			zone->VPoints = vPoints;
			break;
		}
		}
	}
}

QList<Path> AbstractWorldItemCommand::graphicsItemsToPaths(const QList<QGraphicsItem *> &items, PrimitivesTreeModel *model)
{
	QList<Path> result;
	Q_FOREACH(QGraphicsItem *item, items)
	{
		Node *node = qvariant_cast<Node *>(item->data(Constants::WORLD_EDITOR_NODE));
		result.push_back(model->pathFromNode(node));
	}
	return result;
}

MoveWorldItemsCommand::MoveWorldItemsCommand(const QList<QGraphicsItem *> &items, const QPointF &offset,
		WorldEditorScene *scene, PrimitivesTreeModel *model, QUndoCommand *parent)
	: AbstractWorldItemCommand(items, scene, model, parent),
	  m_offset(offset)
{
	setText(QObject::tr("Move item(s)"));
}

MoveWorldItemsCommand::~MoveWorldItemsCommand()
{
}

void MoveWorldItemsCommand::undoChangeItem(int i, AbstractWorldItem *item)
{
	item->moveBy(-m_offset.x(), -m_offset.y());
}

void MoveWorldItemsCommand::redoChangeItem(int i, AbstractWorldItem *item)
{
	item->moveBy(m_offset.x(), m_offset.y());
}

RotateWorldItemsCommand::RotateWorldItemsCommand(const QList<QGraphicsItem *> &items, const qreal angle,
		const QPointF &pivot, WorldEditorScene *scene, PrimitivesTreeModel *model, QUndoCommand *parent)
	: AbstractWorldItemCommand(items, scene, model, parent),
	  m_angle(angle),
	  m_pivot(pivot)
{
	setText(QObject::tr("Rotate item(s)"));
}

RotateWorldItemsCommand::~RotateWorldItemsCommand()
{
}

void RotateWorldItemsCommand::undoChangeItem(int i, AbstractWorldItem *item)
{
	item->rotateOn(m_pivot, -m_angle);
}

void RotateWorldItemsCommand::redoChangeItem(int i, AbstractWorldItem *item)
{
	item->rotateOn(m_pivot, m_angle);
}

ScaleWorldItemsCommand::ScaleWorldItemsCommand(const QList<QGraphicsItem *> &items, const QPointF &factor,
		const QPointF &pivot, WorldEditorScene *scene, PrimitivesTreeModel *model, QUndoCommand *parent)
	: AbstractWorldItemCommand(items, scene, model, parent),
	  m_factor(factor),
	  m_pivot(pivot)
{
	setText(QObject::tr("Scale item(s)"));
}

ScaleWorldItemsCommand::~ScaleWorldItemsCommand()
{
}

void ScaleWorldItemsCommand::undoChangeItem(int i, AbstractWorldItem *item)
{
	QPointF m_invertFactor(1 / m_factor.x(), 1 / m_factor.y());
	item->scaleOn(m_pivot, m_invertFactor);
}

void ScaleWorldItemsCommand::redoChangeItem(int i, AbstractWorldItem *item)
{
	item->scaleOn(m_pivot, m_factor);
}

TurnWorldItemsCommand::TurnWorldItemsCommand(const QList<QGraphicsItem *> &items, const qreal angle,
		WorldEditorScene *scene, PrimitivesTreeModel *model, QUndoCommand *parent)
	: AbstractWorldItemCommand(items, scene, model, parent),
	  m_angle(angle)
{
	setText(QObject::tr("Turn item(s)"));
}

TurnWorldItemsCommand::~TurnWorldItemsCommand()
{
}

void TurnWorldItemsCommand::undoChangeItem(int i, AbstractWorldItem *item)
{
	item->turnOn(-m_angle);
}

void TurnWorldItemsCommand::redoChangeItem(int i, AbstractWorldItem *item)
{
	item->turnOn(m_angle);
}

ShapeWorldItemsCommand::ShapeWorldItemsCommand(const QList<QGraphicsItem *> &items, const QList<QPolygonF> &polygons,
		WorldEditorScene *scene, PrimitivesTreeModel *model,
		QUndoCommand *parent)
	: AbstractWorldItemCommand(items, scene, model, parent),
	  m_redoPolygons(polygons),
	  m_undoPolygons(polygonsFromItems(items))
{
	setText(QObject::tr("Change shape"));
}

ShapeWorldItemsCommand::~ShapeWorldItemsCommand()
{
}

void ShapeWorldItemsCommand::undoChangeItem(int i, AbstractWorldItem *item)
{
	item->setPolygon(m_redoPolygons.at(i));
}

void ShapeWorldItemsCommand::redoChangeItem(int i, AbstractWorldItem *item)
{
	item->setPolygon(m_undoPolygons.at(i));
}

} /* namespace WorldEditor */

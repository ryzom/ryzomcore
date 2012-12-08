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
#include "particle_tree_model.h"

// Qt includes

// NeL includes
#include "nel/3d/particle_system.h"

// Project includes
#include "object_viewer_constants.h"
#include "modules.h"

namespace NLQT
{

CParticleTreeItem::CParticleTreeItem(const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent):
	_itemData(data), _itemIconType(typeItem), _parentItem(parent)
{

}

CParticleTreeItem::CParticleTreeItem(CParticleWorkspace *ws, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent):
	_itemData(data), _itemIconType(typeItem), _parentItem(parent)
{
	nlassert(ws);
	_WS = ws;
}

CParticleTreeItem::CParticleTreeItem(NL3D::CPSLocated *loc, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent):
	_itemData(data), _itemIconType(typeItem), _parentItem(parent)
{
	nlassert(loc);
	_Loc = loc;
}

CParticleTreeItem::CParticleTreeItem(NL3D::CPSLocated *loc, uint32 index, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent):
	_itemData(data), _itemIconType(typeItem), _parentItem(parent)
{
	nlassert(loc);
	_Loc = loc;
	_LocatedInstanceIndex = index;
}

CParticleTreeItem::CParticleTreeItem(CWorkspaceNode *node, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent):
	_itemData(data), _itemIconType(typeItem), _parentItem(parent)
{
	_PS = node;
}

CParticleTreeItem::CParticleTreeItem(NL3D::CPSLocatedBindable *lb, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent):
	_itemData(data), _itemIconType(typeItem), _parentItem(parent)
{
	_Bind = lb;
}

CParticleTreeItem::~CParticleTreeItem()
{
	qDeleteAll(_childItems);
}

void CParticleTreeItem::appendChild(CParticleTreeItem *child)
{
	_childItems.append(child);
}

CParticleTreeItem *CParticleTreeItem::child(int row)
{
	return _childItems.value(row);
}

int CParticleTreeItem::childCount() const
{
	return _childItems.count();
}

int CParticleTreeItem::columnCount() const
{
	return _itemData.count();
}

int CParticleTreeItem::itemType() const
{
	return _itemIconType;
}

QVariant CParticleTreeItem::data(int column) const
{
	return _itemData.value(column);
}

CParticleTreeItem *CParticleTreeItem::parent()
{
	return _parentItem;
}

int CParticleTreeItem::row() const
{
	if (_parentItem)
		return _parentItem->_childItems.indexOf(const_cast<CParticleTreeItem *>(this));
	return 0;
}

bool CParticleTreeItem::replace(const QList<QVariant> &data)
{
	_itemData = data;
	return true;
}

bool CParticleTreeItem::deleteChild(int row)
{
	delete _childItems.takeAt(row);
	return true;
}

CParticleWorkspace *CParticleTreeItem::getPW() const
{
	return _WS;
}

NL3D::CPSLocated *CParticleTreeItem::getLoc() const
{
	return _Loc;
}

NL3D::CPSLocatedBindable *CParticleTreeItem::getBind() const
{
	return _Bind;
}

CWorkspaceNode *CParticleTreeItem::getNode() const
{
	return _PS;
}

uint32 CParticleTreeItem::getLocatedInstanceIndex() const
{
	return _LocatedInstanceIndex;
}

void CParticleTreeItem::setLocatedInstanceIndex(uint32 index)
{
	_LocatedInstanceIndex = index;
}

CParticleTreeModel::CParticleTreeModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	QList<QVariant> rootData;
	rootData << "Name";
	_rootItem = new CParticleTreeItem(rootData, ItemType::Root);
}

CParticleTreeModel::~CParticleTreeModel()
{
	delete _rootItem;
}

int CParticleTreeModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<CParticleTreeItem *>(parent.internalPointer())->columnCount();
	else
		return _rootItem->columnCount();
}

QVariant CParticleTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		CParticleTreeItem *item = static_cast<CParticleTreeItem *>(index.internalPointer());
		if (item->itemType() == ItemType::ParticleSystem)
		{
			if (item->getNode()->isModified())
				return "*" + item->data(index.column()).toString();
		}
		if (item->itemType() == ItemType::Workspace)
		{
			if (item->getPW()->isModified())
				return "*" + item->data(index.column()).toString();
		}
		return item->data(index.column());
	}
	if (role == Qt::FontRole)
	{
		CParticleTreeItem *item = static_cast<CParticleTreeItem *>(index.internalPointer());
		if (Modules::psEdit().getActiveNode() != NULL)
		{
			if (item->data(0) == QString(Modules::psEdit().getActiveNode()->getFilename().c_str()))
				return QFont("SansSerif", 9, QFont::Bold);
		}
		return QFont("SansSerif", 9, QFont::Normal);
	}
	if (role == Qt::DecorationRole)
		return qVariantFromValue(getIcon(index));

	return QVariant();
}

Qt::ItemFlags CParticleTreeModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags itmFlags = Qt::NoItemFlags;

	if (!index.isValid())
		return itmFlags;

	CParticleTreeItem *item = static_cast<CParticleTreeItem *>(index.internalPointer());
	switch (item->itemType())
	{
	case ItemType::Particle:
	case ItemType::Emitter:
	case ItemType::Force:
	case ItemType::Light:
	case ItemType::Sound:
	case ItemType::Located:
	case ItemType::CollisionZone:
		itmFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
		break;
	case ItemType::LocatedInstance:
		if (Modules::psEdit().isRunning())
			itmFlags = Qt::NoItemFlags;
		else
			itmFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		break;
	default:
		itmFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		break;
	}

	//CParticleWorkspaceDialog *pwsd = qobject_cast<CParticleWorkspaceDialog *>(QObject::parent());
	//pwsd->updateTreeView();

	return itmFlags;
}

QVariant CParticleTreeModel::headerData(int section, Qt::Orientation orientation,
										int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return _rootItem->data(section);

	return QVariant();
}

QModelIndex CParticleTreeModel::index(int row, int column, const QModelIndex &parent)
const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	CParticleTreeItem *parentItem;

	if (!parent.isValid())
		parentItem = _rootItem;
	else
		parentItem = static_cast<CParticleTreeItem *>(parent.internalPointer());

	CParticleTreeItem *childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QModelIndex CParticleTreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	CParticleTreeItem *childItem = static_cast<CParticleTreeItem *>(index.internalPointer());
	CParticleTreeItem *parentItem = childItem->parent();

	if (parentItem == _rootItem)
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

int CParticleTreeModel::rowCount(const QModelIndex &parent) const
{
	CParticleTreeItem *parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = _rootItem;
	else
		parentItem = static_cast<CParticleTreeItem *>(parent.internalPointer());

	return parentItem->childCount();
}

bool CParticleTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole)
	{
		CParticleTreeItem *item = static_cast<CParticleTreeItem *>(index.internalPointer());
		QList<QVariant> listData;
		listData << value;
		item->replace(listData);
		std::string name = value.toString().toUtf8().constData();
		if (item->itemType() == ItemType::Located)
			item->getLoc()->setName(name);
		else
			item->getBind()->setName(name);
		Q_EMIT dataChanged(index, index);
		return true;
	}
	return false;
}

QIcon CParticleTreeModel::getIcon(const QModelIndex &index) const
{
	CParticleTreeItem *item = static_cast<CParticleTreeItem *>(index.internalPointer());
	switch (item->itemType())
	{
	case ItemType::Root:
		break;
	case ItemType::Workspace:
		return QIcon(Constants::ICON_WORKSPACE_ITEM_SMALL);
	case ItemType::ParticleSystem:
		return QIcon(Constants::ICON_PARTICLE_SYSTEM_SMALL);
	case ItemType::Particle:
		return QIcon(Constants::ICON_PARTICLE_ITEM_SMALL);
	case ItemType::Emitter:
		return QIcon(Constants::ICON_EMITTER_ITEM_SMALL);
	case ItemType::Force:
		return QIcon(Constants::ICON_FORCE_ITEM_SMALL);
	case ItemType::Light:
		return QIcon(Constants::ICON_LIGHT_ITEM_SMALL);
	case ItemType::Sound:
		return QIcon(Constants::ICON_SOUND_ITEM_SMALL);
	case ItemType::Located:
		return QIcon(Constants::ICON_LOCATED_ITEM_SMALL);
	case ItemType::CollisionZone:
		return QIcon(Constants::ICON_COLLISION_ZONE_ITEM_SMALL);
	case ItemType::LocatedInstance:
		return QIcon(Constants::ICON_INSTANCE_ITEM_SMALL);
	case ItemType::ParticleSystemNotLoaded:
		return QIcon(Constants::ICON_PARTICLE_SYSTEM_CLOSE_SMALL);
	}
	return QIcon();
}

bool CParticleTreeModel::insertRows(CWorkspaceNode *node, int position, const QModelIndex &parent)
{
	CParticleTreeItem *item = static_cast<CParticleTreeItem *>(parent.internalPointer());

	beginInsertRows(parent, position, position);
	setupModelFromPS(node, item);
	endInsertRows();

	if (node->isLoaded())
	{
		QModelIndex indexPS = index(item->childCount() - 1, 0, parent);
		for (uint k = 0; k < node->getPSPointer()->getNbProcess(); ++k)
		{
			NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(node->getPSPointer()->getProcess(k));
			insertRows(loc, k, indexPS);
		}
	}
	return true;
}

bool CParticleTreeModel::insertRows(NL3D::CPSLocated *loc, int position, const QModelIndex &parent)
{
	CParticleTreeItem *item = static_cast<CParticleTreeItem *>(parent.internalPointer());

	beginInsertRows(parent, position, position);
	createItemFromLocated(loc, item);
	endInsertRows();

	QModelIndex indexLocated = index(item->childCount() - 1, 0, parent);
	for (uint l = 0; l < loc->getNbBoundObjects(); ++l)
		insertRow(loc->getBoundObject(l), l, indexLocated);

	for (uint k = 0; k < loc->getSize(); ++k)
		insertRow(loc, k, k + loc->getNbBoundObjects(), indexLocated);

	return true;
}

bool CParticleTreeModel::insertRow(NL3D::CPSLocated *loc, uint32 index, int position, const QModelIndex &parent)
{
	beginInsertRows(parent, position, position);
	createItemFromLocatedInstance(loc, index, static_cast<CParticleTreeItem *>(parent.internalPointer()));
	endInsertRows();
	return true;
}

bool CParticleTreeModel::insertRow(NL3D::CPSLocatedBindable *lb, int position, const QModelIndex &parent)
{
	beginInsertRows(parent, position, position);
	createItemFromLocatedBindable(lb, static_cast<CParticleTreeItem *>(parent.internalPointer()));
	endInsertRows();
	return true;
}

bool CParticleTreeModel::removeRows(int position, const QModelIndex &parent)
{
	CParticleTreeItem *item = static_cast<CParticleTreeItem *>(parent.internalPointer())->child(position);
	while (item->childCount() != 0)
		removeRows(0, parent.child(position, 0));

	beginRemoveRows(parent, position, position);
	static_cast<CParticleTreeItem *>(parent.internalPointer())->deleteChild(position);
	endRemoveRows();
	return false;
}

CWorkspaceNode *CParticleTreeModel::getOwnerNode(CParticleTreeItem *item) const
{
	CWorkspaceNode *node = NULL;
	switch (item->itemType())
	{
	case ItemType::ParticleSystem:
		node = item->getNode();
		break;
	case ItemType::Located:
		node = item->parent()->getNode();
		break;
	case ItemType::LocatedInstance:
	case ItemType::Particle:
	case ItemType::Emitter:
	case ItemType::Force:
	case ItemType::Light:
	case ItemType::Sound:
	case ItemType::CollisionZone:
		node = item->parent()->parent()->getNode();
		break;
	}
	return node;
}

void CParticleTreeModel::rebuildLocatedInstance(const QModelIndex &parent)
{
	CParticleTreeItem *item = static_cast<CParticleTreeItem *>(parent.internalPointer());
	int k = 0;
	for (int i = 0; i < item->childCount(); ++i)
	{
		if (item->child(i)->itemType() == ItemType::LocatedInstance)
		{
			item->child(i)->setLocatedInstanceIndex(k);
			++k;
		}
	}
}

void CParticleTreeModel::setupModelFromWorkSpace()
{
	beginResetModel();
	delete _rootItem;

	QList<QVariant> rootData;
	rootData << "Name";
	_rootItem = new CParticleTreeItem(rootData, ItemType::Root);

	QList<QVariant> workspaceData;
	CParticleWorkspace *workspace = Modules::psEdit().getParticleWorkspace();

	if (workspace == NULL)
	{
		endResetModel();
		return;
	}

	workspaceData << workspace->getFilename().c_str();
	CParticleTreeItem *parent = new CParticleTreeItem(workspace ,workspaceData, ItemType::Workspace, _rootItem);
	_rootItem->appendChild(parent);
	endResetModel();

	QModelIndex rootIndex = index(0, 0);
	uint numNode = workspace->getNumNode();
	for (uint i = 0; i < numNode; ++i)
		insertRows(workspace->getNode(i), i, rootIndex);
}

void CParticleTreeModel::setupModelFromPS(CWorkspaceNode *node, CParticleTreeItem *parent)
{
	QList<QVariant> particleSystemData;
	particleSystemData << node->getFilename().c_str();
	CParticleTreeItem *child;
	if (node->isLoaded())
	{
		child = new CParticleTreeItem(node, particleSystemData, ItemType::ParticleSystem, parent);
		parent->appendChild(child);
	}
	else
	{
		child = new CParticleTreeItem(node, particleSystemData, ItemType::ParticleSystemNotLoaded, parent);
		parent->appendChild(child);
	}
}

void CParticleTreeModel::createItemFromLocated(NL3D::CPSLocated *loc, CParticleTreeItem *parent)
{
	QList<QVariant> locatedData;
	locatedData << QString(loc->getName().c_str());
	CParticleTreeItem *child = new CParticleTreeItem(loc, locatedData, ItemType::Located, parent);
	parent->appendChild(child);
}

void CParticleTreeModel::createItemFromLocatedInstance(NL3D::CPSLocated *loc, uint32 index, CParticleTreeItem *parent)
{
	QList<QVariant> locatedData;
	locatedData << QString("instance");
	CParticleTreeItem *child = new CParticleTreeItem(loc, index, locatedData, ItemType::LocatedInstance, parent);
	parent->appendChild(child);
}

void CParticleTreeModel::createItemFromLocatedBindable(NL3D::CPSLocatedBindable *lb, CParticleTreeItem *parent)
{
	QList<QVariant> forceData;
	forceData << lb->getName().c_str();
	parent->appendChild(new CParticleTreeItem(lb, forceData, lb->getType(), parent));
}

} /* namespace NLQT */
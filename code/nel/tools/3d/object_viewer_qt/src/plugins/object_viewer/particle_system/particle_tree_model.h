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

#ifndef PARTICLE_TREE_MODEL_H
#define PARTICLE_TREE_MODEL_H

// Qt includes
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtGui/QIcon>

// NeL includes
#include "nel/misc/types_nl.h"
#include "nel/3d/ps_located.h"

// Projects includes
#include "particle_node.h"

namespace NLQT
{

struct ItemType
{
	enum List
	{
		Force = 0,
		Particle,
		Emitter,
		Light,
		CollisionZone,
		Sound,
		Root,
		Workspace,
		ParticleSystem,
		Located,
		LocatedInstance,
		ParticleSystemNotLoaded
	};
};

/**
@class CParticleTreeItem
@brief Basic elements tree model particles workspace.
Contains pointer to items particle workspace and type icons.
*/
class CParticleTreeItem
{
public:
	CParticleTreeItem(const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent = 0);

	CParticleTreeItem(CParticleWorkspace *ws, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent );

	CParticleTreeItem(NL3D::CPSLocated *loc, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent);

	CParticleTreeItem(NL3D::CPSLocated *loc, uint32 index, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent);

	CParticleTreeItem(CWorkspaceNode *node, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent);

	CParticleTreeItem(NL3D::CPSLocatedBindable *lb, const QList<QVariant> &data, const int typeItem, CParticleTreeItem *parent);

	~CParticleTreeItem();

	void appendChild(CParticleTreeItem *child);

	CParticleTreeItem *child(int row);
	int childCount() const;
	int columnCount() const;
	int itemType() const;
	QVariant data(int column) const;
	int row() const;
	CParticleTreeItem *parent();
	bool replace(const QList<QVariant> &data);
	bool deleteChild(int row);
	CParticleWorkspace *getPW() const;
	NL3D::CPSLocated *getLoc() const;
	NL3D::CPSLocatedBindable *getBind() const;
	CWorkspaceNode *getNode() const;
	uint32 getLocatedInstanceIndex() const;
	void setLocatedInstanceIndex(uint32 index);

private:
	union
	{
		CParticleWorkspace *_WS;
		NL3D::CPSLocated *_Loc;
		NL3D::CPSLocatedBindable *_Bind;
		CWorkspaceNode *_PS;
	};

	// for the located instance type, this is the index of the instance
	uint32 _LocatedInstanceIndex;

	QList<CParticleTreeItem *> _childItems;
	QList<QVariant> _itemData;
	int _itemIconType;
	CParticleTreeItem *_parentItem;
};

/**
@class CParticleTreeModel
@brief Tree model particles workspace.
*/
class CParticleTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	CParticleTreeModel(QObject *parent = 0);
	~CParticleTreeModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	QIcon getIcon(const QModelIndex &index) const;

	/// Insert WorkspaceNode in model and add all its sub-item.
	bool insertRows(CWorkspaceNode *node, int position, const QModelIndex &parent = QModelIndex());

	/// Insert Located item in model and add all its sub-item.
	bool insertRows(NL3D::CPSLocated *loc, int position, const QModelIndex &parent = QModelIndex());

	/// Insert Located item in model.
	bool insertRow(NL3D::CPSLocated *loc, uint32 index, int position, const QModelIndex &parent = QModelIndex());

	/// Insert LocatedBindable item in model.
	bool insertRow(NL3D::CPSLocatedBindable *lb, int position, const QModelIndex &parent = QModelIndex());

	/// Deletes a tree item and all its children.
	bool removeRows(int position, const QModelIndex &parent = QModelIndex());

	/// Get the parent node in the workspace for the given element in the tree
	CWorkspaceNode *getOwnerNode(CParticleTreeItem *item) const;

	/// Rebuild the located instance in the tree (after loading for example)
	void rebuildLocatedInstance(const QModelIndex &parent);

	/// Build the whole tree from a workspace
	void setupModelFromWorkSpace();

private:
	/// Build a portion of the tree using the given particle system
	void setupModelFromPS(CWorkspaceNode *node, CParticleTreeItem *parent);

	/// Add item from the given located
	void createItemFromLocated(NL3D::CPSLocated *loc, CParticleTreeItem *parent);

	/// Add item from the given located instance
	void createItemFromLocatedInstance(NL3D::CPSLocated *loc, uint32 index, CParticleTreeItem *parent);

	/// Add item from the given located bindable
	void createItemFromLocatedBindable(NL3D::CPSLocatedBindable *lb, CParticleTreeItem *parent);

	CParticleTreeItem *_rootItem;
};

} /* namespace NLQT */

#endif // PARTICLE_TREE_MODEL_H

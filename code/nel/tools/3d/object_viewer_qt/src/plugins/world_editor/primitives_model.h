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


#ifndef PRIMITIVES_MODEL_H
#define PRIMITIVES_MODEL_H

// NeL includes
#include <nel/misc/vector.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_class.h>
#include <nel/ligo/ligo_config.h>

// Qt includes
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace WorldEditor
{

class BaseTreeItem;
class PrimitiveItem;

/**
@class PrimitivesTreeModel
@brief
@details
*/
class PrimitivesTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	PrimitivesTreeModel(QObject *parent = 0);
	~PrimitivesTreeModel();

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
					  const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	// Get primitive
	NLLIGO::IPrimitive *primitive(const QModelIndex &index);

	// Get primitive class
	const NLLIGO::CPrimitiveClass *primitiveClass(const QModelIndex &index);

	// Load primitive from file
	void loadPrimitive(const QString &fileName);

	// Create new primitive and add in tree model
	void newPrimitiveWithoutUndo(const QString &className, uint id, const QModelIndex &parent);

	void deletePrimitiveWithoutUndo(const QModelIndex &index);

	NLLIGO::CLigoConfig *ligoConfig() const;

private:
	// Add root primitive in tree model and add all its sub-items.
	void addRootPrimitive(const QString &name, NLLIGO::CPrimitives *primitives);

	void scanPrimitive(NLLIGO::IPrimitive *prim, const QModelIndex &parentIndex);
	void scanPrimitive(NLLIGO::IPrimitive *prim, BaseTreeItem *parent = 0);

	void removeRows(int position, const QModelIndex &parent);

	BaseTreeItem *m_rootItem;
};

} /* namespace WorldEditor */

#endif // PRIMITIVES_MODEL_H

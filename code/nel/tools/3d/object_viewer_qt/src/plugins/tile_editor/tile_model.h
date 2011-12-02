// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef TILE_MODEL_H
#define TILE_MODEL_H

#include "nel/misc/types_nl.h"

#include <QAbstractListModel>

class TileItem;

class TileModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TileModel::TileModel(const QStringList &headers, QObject *parent);
    ~TileModel();

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column, const QModelIndex &parent= QModelIndex()) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	// Tile Model specific functions
	void appendRow(const QList<TileItem*> &items);
	void appendRow(TileItem *item);

private:
	TileItem *getItem(const QModelIndex &index) const;

	//QList<TileItem*> m_tiles;
	//int m_activeEditChannel;
	TileItem *rootItem;
};

#endif // TILE_128_MODEL_H

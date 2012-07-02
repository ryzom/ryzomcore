// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include <QtGui/QtGui>
#include "tiles_model.h"
#include "tile_widget.h"


bool caseInsensitiveLessThan(const TileModel &t1, const TileModel &t2)
{
	return t1.getIndex() < t2.getIndex();
}


TileModel::TileModel():pixmapSide(128),tileLabel(QObject::tr("Right-Click to select Bitmap")), index(-1)
{
}

TileModel::TileModel(const QPixmap &pixmap, QString tileLabel, int index):pixmap(pixmap), tileLabel(tileLabel), index(index)
{
	pixmapSide = pixmap.width();
}

TileModel::TileModel(int pixmapSide, QString tileLabel, int index):pixmapSide(pixmapSide), index(index)
{
	if (!tileLabel.isEmpty())
		this->tileLabel = tileLabel;
	else
		this->tileLabel = QObject::tr("Right-Click to select Bitmap");
}




tiles_model::tiles_model(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant tiles_model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole || role == Qt::UserRole)
	{
		CTile_Widget* wiwi = new CTile_Widget;
		wiwi->initWidget( tiles.value(index.row()).getPixmap(), tiles.value(index.row()).getPixmapSide(), tiles.value(index.row()).getTileLabel() );
		QPixmap pixpix = QPixmap::grabWidget(wiwi, wiwi->contentsRect());
		delete wiwi;
		return pixpix;
	}
    else if (role == Qt::UserRole + 1)
	{
        return tiles.value(index.row()).getIndex();
	}

    return QVariant();
}

void tiles_model::sort ( int column, Qt::SortOrder order)
{
	qSort(tiles.begin(), tiles.end(), caseInsensitiveLessThan);

}


void tiles_model::addTile(const TileModel &tile)
{

    int row;
    if (int(2.0*qrand()/(RAND_MAX+1.0)) == 1)
        row = 0;
    else
        row = tiles.size();

    beginInsertRows(QModelIndex(), row, row);

	tiles.insert(row, tile);
   
	endInsertRows();
}


Qt::ItemFlags tiles_model::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    return Qt::ItemIsDropEnabled;
}

bool tiles_model::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;

    if(!tiles.size())
        return false;

    if (row >= tiles.size() || row + count <= 0)
        return false;

    int beginRow = qMax(0, row);
    int endRow = qMin(row + count - 1, tiles.size() - 1);

    beginRemoveRows(parent, beginRow, endRow);
    while (beginRow <= endRow) {
        tiles.removeAt(beginRow);
        ++beginRow;
    }

    endRemoveRows();

    return true;
}


int tiles_model::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return tiles.size();
}


void tiles_model::removeAllTiles()
{
    if(tiles.size())
    {
        beginRemoveRows(QModelIndex(), 0, tiles.size() - 1);
        tiles.clear();
        endRemoveRows();
    }
}
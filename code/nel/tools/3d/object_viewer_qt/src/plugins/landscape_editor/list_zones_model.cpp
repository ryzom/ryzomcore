// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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
#include "list_zones_model.h"
#include "builder_zone.h"

// NeL includes
#include <nel/misc/debug.h>

// STL includes
#include <string>
#include <vector>

// Qt includes
#include <QApplication>
#include <QtGui/QProgressDialog>

namespace LandscapeEditor
{

ListZonesModel::ListZonesModel(int scaleRatio, QObject *parent)
	: QAbstractListModel(parent),
	  m_scaleRatio(scaleRatio)
{

}
ListZonesModel::~ListZonesModel()
{
	resetModel();
}

int ListZonesModel::rowCount(const QModelIndex & /* parent */) const
{
	return m_listNames.count();
}

int ListZonesModel::columnCount(const QModelIndex & /* parent */) const
{
	return 1;
}

QVariant ListZonesModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	switch (role)
	{
	case Qt::TextAlignmentRole:
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	case Qt::DisplayRole:
		return m_listNames.at(index.row());
	case Qt::DecorationRole:
	{
		QPixmap *pixmap = getPixmap(m_listNames.at(index.row()));
		return qVariantFromValue(*pixmap);
	}
	default:
		return QVariant();
	}
}

QVariant ListZonesModel::headerData(int section, Qt::Orientation, int role) const
{
	return QVariant();
}

void ListZonesModel::setScaleRatio(int scaleRatio)
{
	m_scaleRatio = scaleRatio;
}

void ListZonesModel::setListZones(QStringList &listZones)
{
	beginResetModel();
	m_listNames.clear();
	m_listNames = listZones;
	endResetModel();
}

void ListZonesModel::resetModel()
{
	beginResetModel();
	QStringList listNames(m_pixmapMap.keys());
	Q_FOREACH(QString name, listNames)
	{
		QPixmap *pixmap = m_pixmapMap.value(name);
		delete pixmap;
	}
	m_pixmapMap.clear();
	m_listNames.clear();
	endResetModel();
}

void ListZonesModel::rebuildModel(PixmapDatabase *pixmapDatabase)
{
	resetModel();

	beginResetModel();
	QStringList listNames;
	listNames = pixmapDatabase->listPixmaps();

	Q_FOREACH(QString name, listNames)
	{
		QPixmap *pixmap = pixmapDatabase->pixmap(name);
		QPixmap *smallPixmap = new QPixmap(pixmap->scaled(pixmap->width() / m_scaleRatio, pixmap->height() / m_scaleRatio));
		m_pixmapMap.insert(name, smallPixmap);
	}
	endResetModel();
}

QPixmap *ListZonesModel::getPixmap(const QString &zoneName) const
{
	QPixmap *result = 0;
	if (!m_pixmapMap.contains(zoneName))
		nlwarning("QPixmap %s not found", zoneName.toUtf8().constData());
	else
		result = m_pixmapMap.value(zoneName);
	return result;
}

} /* namespace LandscapeEditor */

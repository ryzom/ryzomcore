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

#ifndef LIST_ZONES_MODEL_H
#define LIST_ZONES_MODEL_H

// Project includes

// NeL includes
#include <nel/ligo/zone_bank.h>

// Qt includes
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtGui/QPixmap>
#include <QAbstractListModel>

namespace LandscapeEditor
{
class PixmapDatabase;

/**
@class ListZonesModel
@brief ListZonesModel is used for managed list bricks by ListZonesWidget
@details ListZonesModel contains the small images for QListView
*/
class ListZonesModel : public QAbstractListModel
{
	Q_OBJECT
public:
	ListZonesModel(int scaleRatio = 4, QObject *parent = 0);
	~ListZonesModel();

	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation,
						int role) const;

	/// Set size for small pixmaps
	/// Value should be set before calling rebuildModel
	void setScaleRatio(int scaleRatio);

	/// Delete all small images and reset model
	void resetModel();

	/// Set current list zones which will be available in QListView
	void setListZones(QStringList &listZones);

	/// Build own pixmaps database(all images are scaled: width/scaleRatio, height/scaleRatio) from pixmapDatabase
	void rebuildModel(PixmapDatabase *pixmapDatabase);

private:
	/// Get pixmap
	/// @return QPixmap* if the image is in the database ; otherwise returns 0.
	QPixmap *getPixmap(const QString &zoneName) const;

	int m_scaleRatio;
	QMap<QString, QPixmap *> m_pixmapMap;
	QStringList m_listNames;
};

} /* namespace LandscapeEditor */

#endif // LIST_ZONES_MODEL_H

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

// NeL includes
#include <nel/misc/debug.h>

// STL includes
#include <string>
#include <vector>

// Qt includes
#include <QApplication>
#include <QSize>
#include <QtGui/QProgressDialog>

namespace LandscapeEditor
{

ListZonesModel::ListZonesModel(int pixmapSize, QObject *parent)
	: QAbstractListModel(parent),
	  m_pixmapSize(pixmapSize)
{

}
ListZonesModel::~ListZonesModel()
{
	resetModel();
}

int ListZonesModel::rowCount(const QModelIndex & /* parent */) const
{
	return m_pixmapNameList.count();
}

int ListZonesModel::columnCount(const QModelIndex & /* parent */) const
{
	return 1;
}

QVariant ListZonesModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextAlignmentRole)
	{
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else if (role == Qt::DisplayRole)
	{
		return m_pixmapNameList.at(index.row());
	}
	else if (role == Qt::DecorationRole)
	{
		QPixmap *pixmap = getSmallPixmap(m_pixmapNameList.at(index.row()));
		return qVariantFromValue(*pixmap);
	}
	return QVariant();
}

QVariant ListZonesModel::headerData(int section,
									Qt::Orientation /* orientation */,
									int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	return QVariant();
}

void ListZonesModel::setSmallPixmapSize(int pixmapSize)
{
	m_pixmapSize = pixmapSize;
}

void ListZonesModel::setListZones(QStringList &listZones)
{
	beginResetModel();
	m_pixmapNameList.clear();
	m_pixmapNameList = listZones;
	endResetModel();
}

void ListZonesModel::resetModel()
{
	beginResetModel();
	Q_FOREACH(QString name, m_pixmapNameList)
	{
		QPixmap *pixmap = m_pixmapMap.value(name);
		delete pixmap;
		QPixmap *smallPixmap = m_smallPixmapMap.value(name);
		delete smallPixmap;
	}
	m_pixmapMap.clear();
	m_pixmapNameList.clear();
	m_smallPixmapMap.clear();
	endResetModel();
}

bool ListZonesModel::rebuildModel(const QString &zonePath, NLLIGO::CZoneBank &zoneBank)
{
	beginResetModel();
	m_zonePath = zonePath;

	QProgressDialog *progressDialog = new QProgressDialog();
	progressDialog->show();

	std::vector<std::string> zoneNames;
	zoneBank.getCategoryValues ("zone", zoneNames);
	progressDialog->setRange(0, zoneNames.size());
	for (uint i = 0; i < zoneNames.size(); ++i)
	{
		QApplication::processEvents();
		progressDialog->setValue(i);

		NLLIGO::CZoneBankElement *zoneBankItem = zoneBank.getElementByZoneName (zoneNames[i]);

		// Read the texture file
		QString zonePixmapName(zoneNames[i].c_str());
		uint8 sizeX = zoneBankItem->getSizeX();
		uint8 sizeY = zoneBankItem->getSizeY();
		const std::vector<bool> &rMask = zoneBankItem->getMask();

		QPixmap *pixmap = new QPixmap(zonePath + zonePixmapName + ".png");
		if (pixmap->isNull())
		{
			// Generate filled pixmap
		}
		QPixmap *smallPixmap = new QPixmap(pixmap->scaled(m_pixmapSize * sizeX, m_pixmapSize * sizeY));

		m_pixmapMap.insert(zonePixmapName, pixmap);
		m_smallPixmapMap.insert(zonePixmapName, smallPixmap);

	}
	endResetModel();
	delete progressDialog;
	return true;
}

QPixmap *ListZonesModel::getPixmap(const QString &zoneName) const
{
	QPixmap *result = 0;
	if (!m_pixmapMap.contains(zoneName))
		nlwarning("QPixmap %s not found", zoneName.toStdString().c_str());
	else
		result = m_pixmapMap.value(zoneName);
	return result;
}

QPixmap *ListZonesModel::getSmallPixmap(const QString &zoneName) const
{
	QPixmap *result = 0;
	if (!m_pixmapMap.contains(zoneName))
		nlwarning("QPixmap %s not found", zoneName.toStdString().c_str());
	else
		result = m_smallPixmapMap.value(zoneName);
	return result;
}


} /* namespace LandscapeEditor */

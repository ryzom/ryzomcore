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
#include "zone_list_model.h"

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

ZoneListModel::ZoneListModel(int pixmapSize, QObject *parent)
	: QAbstractListModel(parent),
	  m_pixmapSize(pixmapSize)
{

}
ZoneListModel::~ZoneListModel()
{
	resetModel();
}

int ZoneListModel::rowCount(const QModelIndex & /* parent */) const
{
	return m_pixmapMap.count();
}

int ZoneListModel::columnCount(const QModelIndex & /* parent */) const
{
	return 1;
}

QVariant ZoneListModel::data(const QModelIndex &index, int role) const
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

QVariant ZoneListModel::headerData(int section,
								   Qt::Orientation /* orientation */,
								   int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();
	return QVariant();
}

void ZoneListModel::setSmallPixmapSize(int pixmapSize)
{
	m_pixmapSize = pixmapSize;
}

void ZoneListModel::resetModel()
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

bool ZoneListModel::rebuildModel(const QString &zonePath, NLLIGO::CZoneBank &zoneBank)
{
	beginResetModel();
	m_zonePath = zonePath;

	QProgressDialog *progressDialog = new QProgressDialog();

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
		QPixmap *smallPixmap = new QPixmap(pixmap->scaled(m_pixmapSize * sizeX, m_pixmapSize * sizeY));

		m_pixmapMap.insert(zonePixmapName, pixmap);
		m_smallPixmapMap.insert(zonePixmapName, smallPixmap);

	}
	m_pixmapNameList = m_pixmapMap.keys();
	endResetModel();
	delete progressDialog;
	return false;
}

QPixmap *ZoneListModel::getPixmap(const QString &zoneName) const
{
	QPixmap *result = 0;
	if (!m_pixmapMap.contains(zoneName))
		nlwarning("QPixmap %s not found", zoneName.toStdString().c_str());
	else
		result = m_pixmapMap.value(zoneName);
	return result;
}

QPixmap *ZoneListModel::getSmallPixmap(const QString &zoneName) const
{
	QPixmap *result = 0;
	if (!m_pixmapMap.contains(zoneName))
		nlwarning("QPixmap %s not found", zoneName.toStdString().c_str());
	else
		result = m_smallPixmapMap.value(zoneName);
	return result;
}


} /* namespace LandscapeEditor */

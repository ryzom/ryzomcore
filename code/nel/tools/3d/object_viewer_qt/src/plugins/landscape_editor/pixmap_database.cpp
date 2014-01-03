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

// Project includes
#include "pixmap_database.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/ligo/zone_region.h>

// STL includes
#include <vector>
#include <string>

// Qt includes
#include <QtCore/QDir>
#include <QtGui/QPainter>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QProgressDialog>

namespace LandscapeEditor
{

PixmapDatabase::PixmapDatabase(int textureSize)
	: m_textureSize(textureSize),
	  m_errorPixmap(0)
{
	// Create pixmap for case if pixmap and LIGO files not found
	m_errorPixmap = new QPixmap(QSize(m_textureSize, m_textureSize));
	QPainter painter(m_errorPixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.fillRect(m_errorPixmap->rect(), QBrush(QColor(Qt::black)));
	painter.setFont(QFont("Helvetica [Cronyx]", 14));
	painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
	painter.drawText(m_errorPixmap->rect(), Qt::AlignCenter | Qt::TextWordWrap,
					 QObject::tr("Pixmap and LIGO files not found."));
	painter.end();
}

PixmapDatabase::~PixmapDatabase()
{
	delete m_errorPixmap;
	reset();
}

bool PixmapDatabase::loadPixmaps(const QString &zonePath, NLLIGO::CZoneBank &zoneBank, bool displayProgress)
{
	QProgressDialog *progressDialog;
	std::vector<std::string> listNames;
	zoneBank.getCategoryValues ("zone", listNames);
	if (displayProgress)
	{
		progressDialog = new QProgressDialog(QObject::tr("Loading ligo zones."), QObject::tr("Cancel"), 0, listNames.size());
		progressDialog->show();
	}

	for (uint i = 0; i < listNames.size(); ++i)
	{
		QApplication::processEvents();

		if (displayProgress)
			progressDialog->setValue(i);

		NLLIGO::CZoneBankElement *zoneBankItem = zoneBank.getElementByZoneName (listNames[i]);

		// Read the texture file
		QString zonePixmapName(listNames[i].c_str());
		uint8 sizeX = zoneBankItem->getSizeX();
		uint8 sizeY = zoneBankItem->getSizeY();

		QPixmap *pixmap = new QPixmap(zonePath + zonePixmapName + ".png");
		if (pixmap->isNull())
		{
			// Generate filled pixmap if could not load pixmap
			QPixmap *emptyPixmap = new QPixmap(QSize(sizeX * m_textureSize, sizeY * m_textureSize));
			QPainter painter(emptyPixmap);
			painter.setRenderHint(QPainter::Antialiasing, true);
			painter.fillRect(emptyPixmap->rect(), QBrush(QColor(Qt::black)));
			painter.setFont(QFont("Helvetica [Cronyx]", 18));
			painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
			painter.drawText(emptyPixmap->rect(), Qt::AlignCenter, QObject::tr("Pixmap not found"));
			painter.end();
			delete pixmap;
			m_pixmapMap.insert(zonePixmapName, emptyPixmap);
			nlwarning(QString("not found " + zonePath + zonePixmapName + ".png").toStdString().c_str());
		}
		// All pixmaps must be have same size
		else if (pixmap->width() != sizeX * m_textureSize)
		{
			QPixmap *scaledPixmap = new QPixmap(pixmap->scaled(sizeX * m_textureSize, sizeY * m_textureSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
			delete pixmap;
			m_pixmapMap.insert(zonePixmapName, scaledPixmap);
		}
		else
			m_pixmapMap.insert(zonePixmapName, pixmap);
	}

	QPixmap *pixmap = new QPixmap(zonePath + "_unused_.png");
	QPixmap *scaledPixmap = new QPixmap(pixmap->scaled(m_textureSize, m_textureSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
	delete pixmap;
	m_pixmapMap.insert(QString(STRING_UNUSED), scaledPixmap);

	if (displayProgress)
		delete progressDialog;

	return true;
}

void PixmapDatabase::reset()
{
	QStringList listNames(m_pixmapMap.keys());
	Q_FOREACH(QString name, listNames)
	{
		QPixmap *pixmap = m_pixmapMap.value(name);
		delete pixmap;
	}
	m_pixmapMap.clear();
}

QStringList PixmapDatabase::listPixmaps() const
{
	return m_pixmapMap.keys();
}

QPixmap *PixmapDatabase::pixmap(const QString &zoneName) const
{
	QPixmap *result = m_errorPixmap;
	if (!m_pixmapMap.contains(zoneName))
		nlwarning("QPixmap %s not found", zoneName.toStdString().c_str());
	else
		result = m_pixmapMap.value(zoneName);
	return result;
}

int PixmapDatabase::textureSize() const
{
	return m_textureSize;
}

} /* namespace LandscapeEditor */

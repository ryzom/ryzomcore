// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
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
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtGui/QPainter>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QProgressDialog>

namespace LandscapeEditor
{

namespace
{

// The zone names in the .ligozone files are lower case, the matching bitmaps are not
// (zone "converted-168_ew" -> file "converted-168_EW.png"). On Windows this goes
// unnoticed; on Linux not a single tile would find its image. So build a mapping from
// the lower case name to the actual file name, once per directory.
QString resolvePixmapFile(const QString &dirPath, const QString &baseName)
{
	const QString direct = dirPath + baseName + ".png";
	if (QFile::exists(direct))
		return direct;

	static QString cachedDir;
	static QHash<QString, QString> cachedNames;
	if (cachedDir != dirPath)
	{
		cachedDir = dirPath;
		cachedNames.clear();
		QDir dir(dirPath);
		Q_FOREACH (const QString &entry, dir.entryList(QStringList() << "*.png", QDir::Files))
			cachedNames.insert(entry.toLower(), entry);
	}

	QHash<QString, QString>::const_iterator it =
		cachedNames.constFind((baseName + ".png").toLower());
	if (it == cachedNames.constEnd())
		return direct;
	return dirPath + it.value();
}

} // anonymous namespace


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

		QPixmap *pixmap = new QPixmap(resolvePixmapFile(zonePath, zonePixmapName));
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
			nlwarning(QString("not found " + resolvePixmapFile(zonePath, zonePixmapName)).toUtf8().constData());
		}
		// All pixmaps must be have same size. Check both edges: a bitmap with the right
		// width but a wrong height stayed unscaled, and the scene derives the zone size
		// back from the pixmap, so it would cover the wrong number of cells.
		else if ((pixmap->width() != sizeX * m_textureSize) ||
				 (pixmap->height() != sizeY * m_textureSize))
		{
			QPixmap *scaledPixmap = new QPixmap(pixmap->scaled(sizeX * m_textureSize, sizeY * m_textureSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
			delete pixmap;
			m_pixmapMap.insert(zonePixmapName, scaledPixmap);
		}
		else
			m_pixmapMap.insert(zonePixmapName, pixmap);
	}

	// Placeholder for zones marked as unused. The file _unused_.png is missing from the
	// data sets; without a replacement an empty QPixmap ends up here and the affected
	// tiles are not drawn at all - the map shows holes with no hint what they are. So
	// generate one when in doubt.
	QPixmap *unusedPixmap = new QPixmap(resolvePixmapFile(zonePath, "_unused_"));
	if (unusedPixmap->isNull())
	{
		delete unusedPixmap;
		unusedPixmap = new QPixmap(QSize(m_textureSize, m_textureSize));
		unusedPixmap->fill(QColor(70, 70, 70));
		QPainter painter(unusedPixmap);
		painter.setPen(QPen(QColor(110, 110, 110), 1, Qt::DashLine));
		painter.drawRect(0, 0, m_textureSize - 1, m_textureSize - 1);
		painter.end();
	}
	else
	{
		QPixmap *scaled = new QPixmap(unusedPixmap->scaled(m_textureSize, m_textureSize,
		                              Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		delete unusedPixmap;
		unusedPixmap = scaled;
	}
	m_pixmapMap.insert(QString(STRING_UNUSED), unusedPixmap);

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
		nlwarning("QPixmap %s not found", zoneName.toUtf8().constData());
	else
		result = m_pixmapMap.value(zoneName);
	return result;
}

int PixmapDatabase::textureSize() const
{
	return m_textureSize;
}

} /* namespace LandscapeEditor */

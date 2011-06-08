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
#include "builder_zone.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QDir>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QProgressDialog>

namespace LandscapeEditor
{
const int PixmapScale = 256;

PixmapDatabase::PixmapDatabase()
{
}

PixmapDatabase::~PixmapDatabase()
{
	reset();
}

bool PixmapDatabase::loadPixmaps(const QString &zonePath, NLLIGO::CZoneBank &zoneBank)
{
	QProgressDialog *progressDialog = new QProgressDialog();
	progressDialog->show();

	std::vector<std::string> listNames;
	zoneBank.getCategoryValues ("zone", listNames);
	progressDialog->setRange(0, listNames.size());
	for (uint i = 0; i < listNames.size(); ++i)
	{
		QApplication::processEvents();
		progressDialog->setValue(i);

		NLLIGO::CZoneBankElement *zoneBankItem = zoneBank.getElementByZoneName (listNames[i]);

		// Read the texture file
		QString zonePixmapName(listNames[i].c_str());
		uint8 sizeX = zoneBankItem->getSizeX();
		uint8 sizeY = zoneBankItem->getSizeY();

		QPixmap *pixmap = new QPixmap(zonePath + zonePixmapName + ".png");
		if (pixmap->isNull())
		{
			// Generate filled pixmap
		}
		// All pixmaps must be have same size
		if (pixmap->width() != sizeX * PixmapScale)
		{
			QPixmap *scaledPixmap = new QPixmap(pixmap->scaled(sizeX * PixmapScale, sizeY * PixmapScale));
			delete pixmap;
			m_pixmapMap.insert(zonePixmapName, scaledPixmap);
		}
		else
			m_pixmapMap.insert(zonePixmapName, pixmap);
	}
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
	QPixmap *result = 0;
	if (!m_pixmapMap.contains(zoneName))
		nlwarning("QPixmap %s not found", zoneName.toStdString().c_str());
	else
		result = m_pixmapMap.value(zoneName);
	return result;
}

ZoneBuilder::ZoneBuilder()
	: m_pixmapDatabase(0)
{
	m_pixmapDatabase = new PixmapDatabase();
	m_lastPathName = "";
}

ZoneBuilder::~ZoneBuilder()
{
	delete m_pixmapDatabase;
}

bool ZoneBuilder::init(const QString &pathName, bool makeAZone)
{
	bool bRet = true;
	if (pathName != m_lastPathName)
	{
		m_lastPathName = pathName;
		QString zoneBankPath = pathName;
		zoneBankPath += "/zoneligos/";

		// Init the ZoneBank
		m_zoneBank.reset ();
		if (!initZoneBank (zoneBankPath))
		{
			m_zoneBank.reset ();
			return false;
		}
		// Construct the DataBase from the ZoneBank
		QString zoneBitmapPath = pathName;
		zoneBitmapPath += "/zonebitmaps/";
		m_pixmapDatabase->reset();
		if (!m_pixmapDatabase->loadPixmaps(zoneBitmapPath, m_zoneBank))
		{
			m_zoneBank.reset();
			return false;
		}
	}
	if ((makeAZone) && (bRet))
		newZone();
	return bRet;
}

bool ZoneBuilder::initZoneBank (const QString &pathName)
{
	QDir *dir = new QDir(pathName);
	QStringList filters;
	filters << "*.ligozone";

	// Find all ligozone files in dir
	QStringList listFiles = dir->entryList(filters, QDir::Files);

	std::string error;
	Q_FOREACH(QString file, listFiles)
	{
		//nlinfo(file.toStdString().c_str());
		if (!m_zoneBank.addElement((pathName + file).toStdString(), error))
			QMessageBox::critical(0, QObject::tr("Landscape editor"), QString(error.c_str()), QMessageBox::Ok);
	}
	delete dir;
	return true;
}

PixmapDatabase *ZoneBuilder::pixmapDatabase() const
{
	return m_pixmapDatabase;
}

QString ZoneBuilder::dataPath() const
{
	return m_lastPathName;
}

void ZoneBuilder::newZone (bool bDisplay)
{
}

} /* namespace LandscapeEditor */

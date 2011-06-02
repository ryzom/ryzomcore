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
#include "zone_list_model.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QDir>
#include <QtGui/QMessageBox>

namespace LandscapeEditor
{

ZoneBuilder::ZoneBuilder()
	: m_zoneListModel(0)
{
	m_zoneListModel = new ZoneListModel();
	m_lastPathName = "";
}

ZoneBuilder::~ZoneBuilder()
{
	delete m_zoneListModel;
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
		m_zoneListModel->resetModel();
		if (!m_zoneListModel->rebuildModel(zoneBitmapPath, m_zoneBank))
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
	return true;
}

ZoneListModel *ZoneBuilder::zoneModel() const
{
	return m_zoneListModel;
}

void ZoneBuilder::newZone (bool bDisplay)
{
}

} /* namespace LandscapeEditor */

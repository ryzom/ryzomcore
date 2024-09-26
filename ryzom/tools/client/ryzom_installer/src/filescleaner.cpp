// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "filescleaner.h"
#include "operation.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CFilesCleaner::CFilesCleaner(IOperationProgressListener *listener):m_listener(listener)
{
}

CFilesCleaner::~CFilesCleaner()
{
}

void CFilesCleaner::setDirectory(const QString &src)
{
	m_directory = src;
}

bool CFilesCleaner::exec()
{
	if (m_directory.isEmpty()) return false;

	if (m_listener) m_listener->operationPrepare();

	QDir dir(m_directory);

	// directory doesn't exist
	if (!dir.exists()) return false;

	if (!dir.cd("data")) return false;

	QStringList filter;
	filter << "*.string_cache";

	// certificate should be in gamedev.bnp now
	filter << "*.pem";

	if (dir.exists("packedsheets.bnp"))
	{
		filter << "*.packed_sheets";
		filter << "*.packed";
	}

	// only .ref files should be there
	filter << "exedll*.bnp";

	// temporary files
	QStringList files = dir.entryList(filter, QDir::Files);

	// temporary directories
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

	// fonts directory is not needed anymore if fonts.bnp exists
	if (!dir.exists("fonts.bnp")) dirs.removeAll("fonts");

	if (m_listener)
	{
		m_listener->operationInit(0, files.size() + dirs.size());
		m_listener->operationStart();
	}

	int filesCount = 0;

	foreach(const QString &file, files)
	{
		dir.remove(file);

		if (m_listener) m_listener->operationProgress(filesCount, file);

		++filesCount;
	}

	foreach(const QString &d, dirs)
	{
		if (dir.cd(d))
		{
			dir.removeRecursively();
			dir.cdUp();
		}

		if (m_listener) m_listener->operationProgress(filesCount, d);

		++filesCount;
	}

	if (m_listener) m_listener->operationSuccess(files.size() + dirs.size());

	return true;
}

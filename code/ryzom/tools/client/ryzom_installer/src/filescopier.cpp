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
#include "filescopier.h"
#include "utils.h"
#include "operation.h"

#include "nel/misc/path.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CFilesCopier::CFilesCopier(IOperationProgressListener *listener):m_listener(listener)
{
}

CFilesCopier::~CFilesCopier()
{
}

void CFilesCopier::setSourceDirectory(const QString &src)
{
	m_sourceDirectory = src;
}

void CFilesCopier::setDestinationDirectory(const QString &dst)
{
	m_destinationDirectory = dst;
}

void CFilesCopier::setIncludeFilter(const QStringList &filter)
{
	m_includeFilter = filter;
}

void CFilesCopier::addFile(const QString &filename)
{
	m_files << filename;
}

bool CFilesCopier::exec()
{
	if (m_sourceDirectory.isEmpty() || m_destinationDirectory.isEmpty()) return false;

	if (m_listener) m_listener->operationPrepare();

	QDir().mkpath(m_destinationDirectory);

	FilesToCopy files;

	// create the list of files to copy
	CFilesCopier::getFilesList(files);

	// copy them
	return copyFiles(files);
}

void CFilesCopier::getFile(const QFileInfo &fileInfo, const QDir &srcDir, FilesToCopy &files) const
{
	// full path to file
	QString fullPath = fileInfo.absoluteFilePath();

	QString relativePath = srcDir.relativeFilePath(fullPath);

	QFileInfo relativeFileInfo(relativePath);

	// correct absolute path
	if (relativeFileInfo.isAbsolute())
	{
		relativePath = relativeFileInfo.fileName();
	}

	// full path where to copy file
	QString dstPath = m_destinationDirectory + "/" + relativePath;

	if (fileInfo.isDir())
	{
		// create directory
		QDir().mkpath(dstPath);

		QDir subDir(fullPath);

		// get list of all files in directory
		QFileInfoList entries = subDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);

		// proces seach file recursively
		foreach(const QFileInfo &entry, entries)
		{
			getFile(entry, srcDir, files);
		}
	}
	else
	{
		// add the file to list with all useful information
		FileToCopy file;
		file.filename = fileInfo.fileName();
		file.src = fileInfo.filePath();
		file.dst = dstPath;
		file.size = fileInfo.size();
		file.date = fileInfo.lastModified().toTime_t();
		file.permissions = fileInfo.permissions();

		files << file;
	}
}

void CFilesCopier::getFilesList(FilesToCopy &files) const
{
	QDir srcDir(m_sourceDirectory);

	// only copy all files from filter
	QFileInfoList entries = srcDir.entryInfoList(m_includeFilter);

	foreach(const QFileInfo &entry, entries)
	{
		getFile(entry, srcDir, files);
	}

	// copy additional files
	foreach(const QString &fullpath, m_files)
	{
		QFileInfo fileInfo(fullpath);

		getFile(fileInfo, srcDir, files);
	}
}

bool CFilesCopier::copyFiles(const FilesToCopy &files)
{
	qint64 totalSize = 0;

	foreach(const FileToCopy &file, files)
	{
		totalSize += file.size;
	}

	if (m_listener)
	{
		m_listener->operationInit(0, totalSize);
		m_listener->operationStart();
	}

	qint64 processedSize = 0;

	foreach(const FileToCopy &file, files)
	{
		if (m_listener && m_listener->operationShouldStop())
		{
			m_listener->operationStop();
			return true;
		}

		if (m_listener) m_listener->operationProgress(processedSize, file.filename);

		QFileInfo dstFileInfo(file.dst);

		if (dstFileInfo.size() != file.size || dstFileInfo.lastModified().toTime_t() != file.date)
		{
			// force deleting previous file since it was incomplete
			QFile::remove(file.dst);

			if (!QFile::copy(file.src, file.dst))
			{
				if (m_listener) m_listener->operationFail(QApplication::tr("Unable to copy file %1 to %2").arg(file.src).arg(file.dst));
				return false;
			}

			if (!QFile::setPermissions(file.dst, file.permissions))
			{
				nlwarning("Unable to change permissions of %s", Q2C(file.dst));
			}

			if (!NLMISC::CFile::setFileModificationDate(qToUtf8(file.dst), file.date))
			{
				nlwarning("Unable to change date of %s", Q2C(file.dst));
			}
		}

		processedSize += file.size;
	}

	if (m_listener)
	{
		m_listener->operationSuccess(totalSize);
	}

	return true;
}

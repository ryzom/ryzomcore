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

#ifndef FILESCOPIER_H
#define FILESCOPIER_H

class IOperationProgressListener;

/**
 * Files copier
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CFilesCopier
{
public:
	CFilesCopier(IOperationProgressListener *listener);
	virtual ~CFilesCopier();

	void setSourceDirectory(const QString &src);
	void setDesinationDirectory(const QString &src);

	void setIncludeFilter(const QStringList &filter);

	void addFile(const QString &file);

	bool exec();

protected:

	struct FileToCopy
	{
		QString filename;
		QString src;
		QString dst;
		qint64 size;
		uint date;
	};

	typedef QList<FileToCopy> FilesToCopy;

	void getFilesList(FilesToCopy &files);
	bool copyFiles(const FilesToCopy &files);

	IOperationProgressListener *m_listener;

	QString m_sourceDirectory;
	QString m_destinationDirectory;

	QStringList m_includeFilter;
	QStringList m_files;
};

#endif

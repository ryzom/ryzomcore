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

#ifndef ARCHIVE_H
#define ARCHIVE_H

/**
 * Files copy, decompression, extraction
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CArchive : public QObject
{
	Q_OBJECT

public:
	CArchive(QObject *parent = NULL);
	virtual ~CArchive();

	bool extract(const QString &filename, const QString &dest);
	bool copyServerFiles(const QString &src, const QString &dst);
	bool copyProfileFiles(const QString &src, const QString &dst);
	bool cleanServerFiles(const QString &directory);

	void stop();
	bool mustStop();

signals:
	// emitted when requesting real URL
	void extractPrepare();

	// emitted when we got the initial (local) and total (remote) size of file
	void extractInit(qint64 current, qint64 total);

	// emitted when we begin to download
	void extractStart();

	// emitted when the download stopped
	void extractStop();

	// emitted when extracting
	void extractProgress(qint64 current, const QString &filename);

	// emitted when the whole file is downloaded
	void extractSuccess(qint64 total);

	// emitted when an error occurs
	void extractFail(const QString &error);

protected:

	struct FileToCopy
	{
		QString filename;
		QString src;
		QString dst;
		qint64 size;
		QDateTime date;
	};

	typedef QList<FileToCopy> FilesToCopy;

	bool extract7z();
	bool extractZip();
	bool extractBnp();

	bool progress(const std::string &filename, uint32 currentFile, uint32 totalFiles);

	bool copyServerFiles();
	bool copyProfileFiles();
	bool copyFiles(const FilesToCopy &files);

	static void getFilesList(const QString &srcDir, const QString &dstDir, const QStringList &filter, FilesToCopy &files);

	QString m_filename;
	QString m_dest;

	QMutex m_mutex;

	bool m_mustStop;
};

#endif

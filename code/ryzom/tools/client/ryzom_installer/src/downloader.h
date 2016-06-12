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

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

class IOperationProgressListener;

/**
 * Files downloader, please note that only one file can be downloaded at once.
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CDownloader : public QObject
{
	Q_OBJECT

public:
	CDownloader(QObject *parent, IOperationProgressListener *listener);
	virtual ~CDownloader();

	bool getHtmlPageContent(const QString &url);

	bool prepareFile(const QString &url, const QString &fullPath);
	bool getFile();

	bool supportsResume() const { return m_supportsAcceptRanges && m_supportsContentRange; }

	bool isDownloading() const { return m_file != NULL; }

signals:
	void htmlPageContent(const QString &html);

private slots:
	void onTimeout();
	void onHtmlPageFinished();
	void onHeadFinished();
	void onDownloadFinished();
	void onError(QNetworkReply::NetworkError error);
	void onDownloadProgress(qint64 current, qint64 total);
	void onDownloadRead();

protected:
	IOperationProgressListener *m_listener;

	void startTimer();
	void stopTimer();

	bool openFile();
	void closeFile();

	void getFileHead();
	void downloadFile();

	bool checkDownloadedFile();

	QNetworkAccessManager *m_manager;
	QNetworkReply *m_reply;
	QTimer *m_timer;

	QString m_url;
	QString m_referer;
	QString m_fullPath;

	qint64 m_offset;
	qint64 m_size;
	QDateTime m_lastModified;

	bool m_supportsAcceptRanges;
	bool m_supportsContentRange;

	bool m_downloadAfterHead;

	QFile *m_file;
};

#endif

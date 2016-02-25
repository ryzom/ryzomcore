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
#include "downloader.h"

#include "nel/misc/system_info.h"
#include "nel/misc/path.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CDownloader::CDownloader(QObject *parent):QObject(parent), m_manager(NULL), m_reply(NULL), m_timer(NULL),
	m_offset(0), m_size(0), m_supportsAcceptRanges(false), m_supportsContentRange(false),
	m_downloadAfterHead(false), m_aborted(false), m_file(NULL)
{
	m_manager = new QNetworkAccessManager(this);
	m_timer = new QTimer(this);

	connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

CDownloader::~CDownloader()
{
	stopTimer();
	closeFile();
}

bool CDownloader::getHtmlPageContent(const QString &url)
{
	if (url.isEmpty()) return false;

	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::UserAgentHeader, "Ryzom Installer/1.0");

	QNetworkReply *reply = m_manager->get(request);

	connect(reply, SIGNAL(finished()), SLOT(onHtmlPageFinished()));

	return true;
}

bool CDownloader::prepareFile(const QString &url, const QString &fullPath)
{
	if (url.isEmpty()) return false;

	m_downloadAfterHead = false;

	emit downloadPrepare();

	m_fullPath = fullPath;
	m_url = url;

	getFileHead();

	return true;
}

bool CDownloader::getFile()
{
	if (m_fullPath.isEmpty() || m_url.isEmpty())
	{
		qDebug() << "You forget to call prepareFile before";

		return false;
	}

	m_downloadAfterHead = true;

	getFileHead();

	return true;
}

bool CDownloader::stop()
{
	if (!m_reply) return false;

	m_reply->abort();

	return true;
}

void CDownloader::startTimer()
{
	stopTimer();

	m_timer->setInterval(5000);
	m_timer->setSingleShot(true);
	m_timer->start();
}

void CDownloader::stopTimer()
{
	if (m_timer->isActive()) m_timer->stop();
}

bool CDownloader::openFile()
{
	closeFile();

	m_file = new QFile(m_fullPath);

	if (m_file->open(QFile::Append)) return true;

	closeFile();

	return false;
}

void CDownloader::closeFile()
{
	if (m_file)
	{
		m_file->close();

		delete m_file;
		m_file = NULL;
	}
}

void CDownloader::getFileHead()
{
	if (m_supportsAcceptRanges)
	{
		QFileInfo fileInfo(m_fullPath);

		if (fileInfo.exists())
		{
			m_offset = fileInfo.size();
		}
		else
		{
			m_offset = 0;
		}

		// continue if offset less than size
		if (m_offset >= m_size)
		{
			if (checkDownloadedFile())
			{
				// file is already downloaded
				emit downloadSuccess(m_size);
			}
			else
			{
				// or has wrong size
				emit downloadFail(tr("File (%1B) is larger than expected (%2B)").arg(m_offset).arg(m_size));
			}

			return;
		}
	}

	QNetworkRequest request(m_url);
	request.setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.8; rv:24.0) Gecko/20100101 Firefox/24.0");

	if (m_supportsAcceptRanges)
	{
		request.setRawHeader("Range", QString("bytes=%1-").arg(m_offset).toLatin1());
	}

	m_reply = m_manager->head(request);

	connect(m_reply, SIGNAL(finished()), SLOT(onHeadFinished()));
	connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onError(QNetworkReply::NetworkError)));

	startTimer();
}

void CDownloader::downloadFile()
{
	qint64 freeSpace = NLMISC::CSystemInfo::availableHDSpace(m_fullPath.toUtf8().constData());

	if (freeSpace < m_size - m_offset)
	{
		// we have not enough free disk space to continue download
		emit downloadFail(tr("You only have %1 bytes left on device, but %2 bytes are required.").arg(freeSpace).arg(m_size - m_offset));
		return;
	}

	if (!openFile())
	{
		emit downloadFail(tr("Unable to write file"));
		return;
	}

	QNetworkRequest request(m_url);
	request.setHeader(QNetworkRequest::UserAgentHeader, "Opera/9.80 (Windows NT 6.2; Win64; x64) Presto/2.12.388 Version/12.17");

	if (supportsResume())
	{
		request.setRawHeader("Range", QString("bytes=%1-%2").arg(m_offset).arg(m_size-1).toLatin1());
	}

	m_reply = m_manager->get(request);

	connect(m_reply, SIGNAL(finished()), SLOT(onDownloadFinished()));
	connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onError(QNetworkReply::NetworkError)));
	connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(onDownloadProgress(qint64, qint64)));
	connect(m_reply, SIGNAL(readyRead()), SLOT(onDownloadRead()));

	emit downloadStart();

	startTimer();
}

bool CDownloader::checkDownloadedFile()
{
	QFileInfo file(m_fullPath);

	return file.size() == m_size && file.lastModified().toUTC() == m_lastModified;
}

void CDownloader::onTimeout()
{
	qDebug() << "Timeout";

	emit downloadFail(tr("Timeout"));
}

void CDownloader::onHtmlPageFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QString html = QString::fromUtf8(reply->readAll());

	reply->deleteLater();

	emit htmlPageContent(html);
}

void CDownloader::onHeadFinished()
{
	stopTimer();

	int status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	QString redirection = m_reply->header(QNetworkRequest::LocationHeader).toString();

	m_size = m_reply->header(QNetworkRequest::ContentLengthHeader).toInt();
	m_lastModified = m_reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toUTC();

	QString acceptRanges = QString::fromLatin1(m_reply->rawHeader("Accept-Ranges"));
	QString contentRange = QString::fromLatin1(m_reply->rawHeader("Content-Range"));

	m_reply->deleteLater();
	m_reply = NULL;

	// redirection
	if (status == 302)
	{
		if (redirection.isEmpty())
		{
			emit downloadFail(tr("Redirection URL is not defined"));
			return;
		}

		// redirection on another server, recheck resume
		m_supportsAcceptRanges = false;
		m_supportsContentRange = false;

		m_referer = m_url;

		// update real URL
		m_url = redirection;

		getFileHead();

		return;
	}

	// we requested without range
	else if (status == 200)
	{
		// update size
		emit downloadInit(0, m_size);

		if (!m_supportsAcceptRanges && acceptRanges == "bytes")
		{
			// server supports resume, part 1
			m_supportsAcceptRanges = true;

			// request range
			getFileHead();
			return;
		}

		// server doesn't support resume or
		// we requested range, but server always returns 200
		// download from the beginning
	}
	
	// we requested with a range
	else if (status == 206)
	{
		// server supports resume
		QRegExp regexp("^bytes ([0-9]+)-([0-9]+)/([0-9]+)$");

		if (m_supportsAcceptRanges && regexp.exactMatch(contentRange))
		{
			m_supportsContentRange = true;
			m_offset = regexp.cap(1).toLongLong();

			// when resuming, Content-Length is the size of missing parts to download
			m_size = regexp.cap(3).toLongLong();

			// update offset and size
			emit downloadInit(m_offset, m_size);
		}
		else
		{
			qDebug() << "Unable to parse";
		}
	}

	// other status
	else
	{
		emit downloadFail(tr("Wrong status code: %1").arg(status));
		return;
	}

	if (m_downloadAfterHead)
	{
		if (checkDownloadedFile())
		{
			qDebug() << "same date and size";
		}
		else
		{
			downloadFile();
		}
	}
}

void CDownloader::onDownloadFinished()
{
	m_reply->deleteLater();
	m_reply = NULL;

	closeFile();

	if (m_aborted)
	{
		m_aborted = false;
		emit downloadStop();
	}
	else
	{
		bool ok = NLMISC::CFile::setFileModificationDate(m_fullPath.toUtf8().constData(), m_lastModified.toTime_t());

		emit downloadSuccess(m_size);
	}
}

void CDownloader::onError(QNetworkReply::NetworkError error)
{
	if (error == QNetworkReply::OperationCanceledError)
	{
		m_aborted = true;
	}
	else
	{
		emit downloadFail(tr("Network error: %1").arg(error));
	}
}

void CDownloader::onDownloadProgress(qint64 current, qint64 total)
{
	stopTimer();

	emit downloadProgress(m_offset + current);
}

void CDownloader::onDownloadRead()
{
	if (m_file) m_file->write(m_reply->readAll());
}

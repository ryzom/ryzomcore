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
#include "operation.h"
#include "downloader.h"
#include "utils.h"

#include "nel/misc/system_info.h"
#include "nel/misc/path.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CDownloader::CDownloader(QObject *parent, IOperationProgressListener *listener):QObject(parent), m_listener(listener), m_manager(NULL), m_timer(NULL),
	m_offset(0), m_size(0), m_supportsAcceptRanges(false), m_supportsContentRange(false),
	m_downloadAfterHead(false), m_file(NULL)
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
	request.setHeader(QNetworkRequest::UserAgentHeader, QString("Ryzom Installer/%1").arg(QApplication::applicationVersion()));

	QNetworkReply *reply = m_manager->get(request);

	connect(reply, SIGNAL(finished()), SLOT(onHtmlPageFinished()));

	return true;
}

bool CDownloader::prepareFile(const QString &url, const QString &fullPath)
{
	if (url.isEmpty()) return false;

	m_downloadAfterHead = false;

	if (m_listener) m_listener->operationPrepare();

	m_fullPath = fullPath;
	m_url = url;

	getFileHead();

	return true;
}

bool CDownloader::getFile()
{
	if (m_fullPath.isEmpty() || m_url.isEmpty())
	{
		nlwarning("You forget to call prepareFile before");

		return false;
	}

	m_downloadAfterHead = true;

	getFileHead();

	return true;
}

void CDownloader::startTimer()
{
	stopTimer();

	m_timer->setInterval(30000);
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
				if (m_listener) m_listener->operationSuccess(m_size);

				emit downloadDone();
			}
			else
			{
				// or has wrong size
				if (m_listener) m_listener->operationFail(tr("File is larger (%1B) than expected (%2B)").arg(m_offset).arg(m_size));
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

	QNetworkReply *reply = m_manager->head(request);

	connect(reply, SIGNAL(finished()), SLOT(onHeadFinished()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onError(QNetworkReply::NetworkError)));

	startTimer();
}

void CDownloader::downloadFile()
{
	qint64 freeSpace = NLMISC::CSystemInfo::availableHDSpace(m_fullPath.toUtf8().constData());

	if (freeSpace == 0)
	{
		if (m_listener)
		{
			QString error = qFromUtf8(NLMISC::formatErrorMessage(NLMISC::getLastError()));
			m_listener->operationFail(tr("Error '%1' occured when trying to check free disk space on %2.").arg(error).arg(m_fullPath));
		}
		return;
	}

	if (freeSpace < m_size - m_offset)
	{
		// we have not enough free disk space to continue download
		if (m_listener) m_listener->operationFail(tr("You only have %1 bytes left on the device, but %2 bytes are needed.").arg(freeSpace).arg(m_size - m_offset));
		return;
	}

	if (!openFile())
	{
		if (m_listener) m_listener->operationFail(tr("Unable to write file"));
		return;
	}

	QNetworkRequest request(m_url);
	request.setHeader(QNetworkRequest::UserAgentHeader, "Opera/9.80 (Windows NT 6.2; Win64; x64) Presto/2.12.388 Version/12.17");

	if (supportsResume())
	{
		request.setRawHeader("Range", QString("bytes=%1-%2").arg(m_offset).arg(m_size-1).toLatin1());
	}

	QNetworkReply *reply = m_manager->get(request);

	connect(reply, SIGNAL(finished()), SLOT(onDownloadFinished()));
	connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onError(QNetworkReply::NetworkError)));
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), SLOT(onDownloadProgress(qint64, qint64)));
	connect(reply, SIGNAL(readyRead()), SLOT(onDownloadRead()));

	if (m_listener) m_listener->operationStart();

	startTimer();
}

bool CDownloader::checkDownloadedFile()
{
	QFileInfo file(m_fullPath);

	return file.size() == m_size && file.lastModified().toUTC() == m_lastModified;
}

void CDownloader::onTimeout()
{
	nlwarning("Timeout");

	if (m_listener) m_listener->operationFail(tr("Timeout"));
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

	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QString url = reply->url().toString();

	QString redirection = reply->header(QNetworkRequest::LocationHeader).toString();

	m_size = reply->header(QNetworkRequest::ContentLengthHeader).toInt();
	m_lastModified = reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toUTC();

	QString acceptRanges = QString::fromLatin1(reply->rawHeader("Accept-Ranges"));
	QString contentRange = QString::fromLatin1(reply->rawHeader("Content-Range"));

	reply->deleteLater();

	nlinfo("HTTP status code %d on HEAD for %s", status, Q2C(url));

	if (!redirection.isEmpty())
	{
		nlinfo("Redirected to %s", Q2C(redirection));
	}

	// redirection
	if (status == 302 || status == 307)
	{
		if (redirection.isEmpty())
		{
			nlwarning("No redirection defined");

			if (m_listener) m_listener->operationFail(tr("Redirection URL is not defined"));
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
		if (m_listener) m_listener->operationInit(0, m_size);

		if (!m_supportsAcceptRanges && acceptRanges == "bytes")
		{
			nlinfo("Server supports resume for %s", Q2C(url));

			// server supports resume, part 1
			m_supportsAcceptRanges = true;

			// request range
			getFileHead();
			return;
		}

		// server doesn't support resume or
		// we requested range, but server always returns 200
		// download from the beginning
		nlwarning("Server doesn't support resume, download %s from the beginning", Q2C(url));
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
			if (m_listener) m_listener->operationInit(m_offset, m_size);

			nlinfo("Server supports resume for %s: offset %" NL_I64 "d, size %" NL_I64 "d", Q2C(url), m_offset, m_size);
		}
		else
		{
			nlwarning("Unable to parse %s", Q2C(contentRange));
		}
	}

	// error when download is not yet ready
	else if (status == 307)
	{
		if (m_listener) m_listener->operationFail(tr("File is not available, please retry later (status code: %1)").arg(status));
		return;
	}

	// other status
	else
	{
		if (m_listener) m_listener->operationFail(tr("Incorrect status code: %1").arg(status));
		return;
	}

	if (m_downloadAfterHead)
	{
		if (checkDownloadedFile())
		{
			nlwarning("Same date and size");
		}
		else
		{
			downloadFile();
		}
	}
	else
	{
		emit downloadPrepared();
	}
}

void CDownloader::onDownloadFinished()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QString url = reply->url().toString();

	reply->deleteLater();

	nlwarning("Download finished with HTTP status code %d when downloading %s", status, Q2C(url));

	closeFile();

	if (m_listener && m_listener->operationShouldStop())
	{
		m_listener->operationStop();
	}
	else
	{
		if (QFileInfo(m_fullPath).size() == m_size)
		{
			bool ok = NLMISC::CFile::setFileModificationDate(m_fullPath.toUtf8().constData(), m_lastModified.toTime_t());

			if (m_listener) m_listener->operationSuccess(m_size);

			emit downloadDone();
		}
		else if (status == 206)
		{
			if (m_listener) m_listener->operationContinue();
		}
		else
		{
			if (m_listener) m_listener->operationFail(tr("HTTP error: %1").arg(status));
		}
	}
}

void CDownloader::onError(QNetworkReply::NetworkError error)
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	nlwarning("Network error %s (%d) when downloading %s", Q2C(reply->errorString()), error, Q2C(m_url));

	if (!m_listener) return;

	if (error == QNetworkReply::OperationCanceledError)
	{
		 m_listener->operationStop();
	}
}

void CDownloader::onDownloadProgress(qint64 current, qint64 total)
{
	stopTimer();

	if (!m_listener) return;

	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	m_listener->operationProgress(m_offset + current, m_url);

	// abort download
	if (m_listener->operationShouldStop() && reply) reply->abort();
}

void CDownloader::onDownloadRead()
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

	if (m_file && reply) m_file->write(reply->readAll());
}

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
#include "configfile.h"

#include "nel/misc/system_info.h"
#include "nel/misc/path.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CDownloader::CDownloader(QObject *parent, IOperationProgressListener *listener):QObject(parent), m_listener(listener), m_manager(NULL), m_timer(NULL),
	m_offset(0), m_size(0), m_sizeKnown(false), m_getStatus(0), m_supportsAcceptRanges(false), m_supportsContentRange(false),
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

	// what was learnt about the previous file does not apply to this one
	m_referer.clear();
	m_offset = 0;
	m_size = 0;
	m_sizeKnown = false;
	m_lastModified = QDateTime();
	m_contentType.clear();
	m_supportsAcceptRanges = false;
	m_supportsContentRange = false;

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

	// append to the part already downloaded only when the server sends the rest of it,
	// otherwise the whole file comes again and replaces it
	QIODevice::OpenMode mode = (supportsResume() && m_offset > 0) ? QIODevice::OpenMode(QFile::Append) : (QFile::WriteOnly | QFile::Truncate);

	// unbuffered, so that a write the disk cannot take fails in write() and not unseen when the file is closed
	mode |= QIODevice::Unbuffered;

	if (m_file->open(mode)) return true;

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
		const bool partExists = fileInfo.exists();
		m_offset = partExists ? fileInfo.size() : 0;

		// without the size of the whole file nothing can be said about the part, it is kept
		if (partExists && !m_sizeKnown)
		{
			nlwarning("The server gave no size for %s, the part %s of %" NL_I64 "d bytes is kept", Q2C(m_url), Q2C(m_fullPath), m_offset);
		}
		// continue if offset less than size
		else if (partExists && (m_offset >= m_size))
		{
			if (checkDownloadedFile())
			{
				// file is already downloaded
				if (m_listener) m_listener->operationSuccess(m_size);

				emit downloadDone();

				return;
			}

			if (refusesToReplacePart()) return;

			// a second disagreement in this run ends the step instead of downloading the file again and again
			int &deletions = m_partDeletions[m_fullPath];
			if (++deletions > 1)
			{
				const QString partDate = fileInfo.lastModified().toUTC().toString(Qt::ISODate);
				const QString serverDate = m_lastModified.toString(Qt::ISODate);
				nlwarning("%s disagrees again with %s: the part has %" NL_I64 "d bytes dated %s, the server announces %" NL_I64 "d bytes dated %s", Q2C(m_fullPath), Q2C(m_url), m_offset,
				    Q2C(partDate), m_size, Q2C(serverDate));
				if (m_listener)
					m_listener->operationFail(tr("%1 disagrees again with the file on %2: the part has %3 bytes dated %4, the server announces %5 bytes dated %6")
					        .arg(m_fullPath, QUrl(m_url).host(), QString::number(m_offset), partDate, QString::number(m_size), serverDate));
				return;
			}

			// a part that is larger than the file on the server, or as large but with another date, cannot be resumed
			if (m_offset > m_size)
			{
				nlwarning("%s is larger (%" NL_I64 "d bytes) than the file on the server (%" NL_I64 "d bytes), downloading it again", Q2C(m_fullPath), m_offset, m_size);
			}
			else
			{
				nlwarning("%s has the size of the file on the server but the date %s instead of %s, downloading it again", Q2C(m_fullPath),
				    Q2C(QFileInfo(m_fullPath).lastModified().toUTC().toString(Qt::ISODate)), Q2C(m_lastModified.toString(Qt::ISODate)));
			}

			if (!QFile::remove(m_fullPath))
			{
				if (m_listener) m_listener->operationFail(tr("Unable to delete %1, which does not match the file on the server").arg(m_fullPath));
				return;
			}

			m_offset = 0;
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
	bool ignoreFreeDiskSpaceChecks =  CConfigFile::getInstance()->ignoreFreeDiskSpaceChecks();
	qint64 freeSpace = NLMISC::CSystemInfo::availableHDSpace(m_fullPath.toUtf8().constData());

	if (!ignoreFreeDiskSpaceChecks && freeSpace == 0)
	{
		if (m_listener)
		{
			QString error = qFromUtf8(NLMISC::formatErrorMessage(NLMISC::getLastError()));
			m_listener->operationFail(tr("Error '%1' occurred when trying to check free disk space on %2.").arg(error).arg(m_fullPath));
		}
		return;
	}

	if (!ignoreFreeDiskSpaceChecks && freeSpace < m_size - m_offset)
	{
		// we have not enough free disk space to continue download
		if (m_listener) m_listener->operationFail(tr("You only have %1 bytes left on the device, but %2 bytes are needed.").arg(freeSpace).arg(m_size - m_offset));
		return;
	}

	// the part is about to be replaced unless the rest of it is requested
	if (refusesToReplacePart()) return;

	if (!openFile())
	{
		if (m_listener) m_listener->operationFail(tr("Unable to write file"));
		return;
	}

	m_getStatus = 0;
	m_getFailure.clear();

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

	return m_sizeKnown && file.exists() && file.size() == m_size && file.lastModified().toUTC() == m_lastModified;
}

bool CDownloader::refusesToReplacePart()
{
	// an answer of another kind than the first one for the file, such as an error or maintenance page, does not replace a larger part
	const qint64 partSize = QFileInfo(m_fullPath).size();
	if (!m_sizeKnown || (partSize <= m_size)) return false;

	const QString firstContentType = m_firstContentTypes.value(m_fullPath);
	if (m_contentType == firstContentType) return false;

	const QString host = QUrl(m_url).host();
	nlwarning("%s answers for %s with %" NL_I64 "d bytes of '%s' where it first answered with '%s', the part %s of %" NL_I64 "d bytes is kept", Q2C(host), Q2C(m_url), m_size,
	    Q2C(m_contentType), Q2C(firstContentType), Q2C(m_fullPath), partSize);
	if (m_listener)
		m_listener->operationFail(tr("%1 now answers for %2 with %3 bytes of '%4' where it first answered with '%5', the part %6 of %7 bytes is kept")
		        .arg(host, m_url, QString::number(m_size), m_contentType, firstContentType, m_fullPath, QString::number(partSize)));
	return true;
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

	// only the Content-Length of a 200 is the size of the file, a 206 announces the length of its range
	const QVariant contentLength = reply->header(QNetworkRequest::ContentLengthHeader);
	const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();

	// an answer without a date, such as a 416, keeps the one an earlier answer for the file gave
	const QDateTime lastModified = reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toUTC();
	if (lastModified.isValid()) m_lastModified = lastModified;

	QString acceptRanges = QString::fromLatin1(reply->rawHeader("Accept-Ranges"));
	QString contentRange = QString::fromLatin1(reply->rawHeader("Content-Range"));

	reply->deleteLater();

	nlinfo("HTTP status code %d on HEAD for %s", status, Q2C(url));

	if (!redirection.isEmpty())
	{
		nlinfo("Redirected to %s", Q2C(redirection));
	}

	// the kind of answer, to tell the file from an error or maintenance page in a later answer
	if ((status == 200) || (status == 206))
	{
		m_contentType = contentType;
		if (!m_firstContentTypes.contains(m_fullPath)) m_firstContentTypes.insert(m_fullPath, contentType);
	}

	// redirection
	if (status >= 300 && status < 400)
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
		m_sizeKnown = contentLength.isValid();
		m_size = m_sizeKnown ? contentLength.toLongLong() : 0;
		if (!m_sizeKnown)
			nlwarning("The server gives no size for %s", Q2C(url));

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

		m_supportsContentRange = false;
		m_offset = 0;
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
			m_sizeKnown = true;

			// update offset and size
			if (m_listener) m_listener->operationInit(m_offset, m_size);

			nlinfo("Server supports resume for %s: offset %" NL_I64 "d, size %" NL_I64 "d", Q2C(url), m_offset, m_size);
		}
		else
		{
			// the size stays what an earlier answer gave, or unknown
			nlwarning("Unable to parse the Content-Range %s for %s, it is not resumed", Q2C(contentRange), Q2C(url));
			m_supportsContentRange = false;
			m_offset = 0;
		}
	}

	// the range starts at or after the end of the file, whose size the answer gives
	else if (status == 416)
	{
		QRegExp regexp("^bytes \\*/([0-9]+)$");

		// with the size known, the part is complete or larger than the file, the rules for such a part apply
		if (m_supportsAcceptRanges && regexp.exactMatch(contentRange) && (m_offset >= regexp.cap(1).toLongLong()))
		{
			m_size = regexp.cap(1).toLongLong();
			m_sizeKnown = true;
			m_supportsContentRange = false;

			nlinfo("The range from byte %" NL_I64 "d of %s is past the end of the file, which has %" NL_I64 "d bytes", m_offset, Q2C(url), m_size);

			getFileHead();
			return;
		}

		nlwarning("%s answers the range from byte %" NL_I64 "d with 416 and the Content-Range '%s'", Q2C(url), m_offset, Q2C(contentRange));
		if (m_listener) m_listener->operationFail(tr("%1 answers the range from byte %2 with 416 and the Content-Range '%3'").arg(url, QString::number(m_offset), contentRange));
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
			nlinfo("%s has the date and size of the file on the server", Q2C(m_fullPath));

			if (m_listener) m_listener->operationSuccess(m_size);

			emit downloadDone();
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
	const bool noError = reply->error() == QNetworkReply::NoError;
	const QVariant contentLength = reply->header(QNetworkRequest::ContentLengthHeader);

	reply->deleteLater();

	nlwarning("Download finished with HTTP status code %d when downloading %s", status, Q2C(url));

	closeFile();

	if (!m_getFailure.isEmpty())
	{
		const QString error = m_getFailure;
		m_getFailure.clear();

		if (m_listener) m_listener->operationFail(error);
		return;
	}

	if (m_listener && m_listener->operationShouldStop())
	{
		m_listener->operationStop();
	}
	else
	{
		// a HEAD without Content-Length leaves the size of the file to the GET, when it gives one
		if (!m_sizeKnown && (status == 200) && contentLength.isValid())
		{
			m_size = contentLength.toLongLong();
			m_sizeKnown = true;
		}

		const qint64 fileSize = QFileInfo(m_fullPath).size();

		// without a known size, a 200 that ends without an error is the whole file
		if (m_sizeKnown ? (fileSize == m_size) : (noError && (status == 200)))
		{
			// the date tells a later run that a part file of this size is complete
			if (!NLMISC::CFile::setFileModificationDate(m_fullPath.toUtf8().constData(), m_lastModified.toTime_t()))
			{
				nlwarning("Unable to set the date of %s to the one of the file on the server", Q2C(m_fullPath));
			}

			if (m_listener) m_listener->operationSuccess(fileSize);

			emit downloadDone();
		}
		else if (status >= 200 && status < 300)
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

	// an abort of the downloader's own is reported as a failure when the reply has finished
	if ((error == QNetworkReply::OperationCanceledError) && m_getFailure.isEmpty())
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

	if (!m_file || !reply) return;

	if (m_getStatus == 0)
	{
		m_getStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		// a server that ignores the range sends the whole file, which replaces the part instead of following it,
		// but an answer that is not the file, such as an error page sent with 200, leaves the part as it is
		if ((m_getStatus == 200) && supportsResume() && (m_offset > 0))
		{
			const QVariant contentLength = reply->header(QNetworkRequest::ContentLengthHeader);
			const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
			const QString firstContentType = m_firstContentTypes.value(m_fullPath);
			const bool wholeFile = m_sizeKnown ? (contentLength.isValid() && (contentLength.toLongLong() == m_size)) : (contentType == firstContentType);
			if (!wholeFile)
			{
				const QString announced = contentLength.isValid() ? tr("%1 bytes").arg(contentLength.toLongLong()) : tr("no length");
				const QString expected = m_sizeKnown ? tr("%1 bytes").arg(m_size) : tr("an unknown length");
				nlwarning("%s answers the range from byte %" NL_I64 "d of %s with status %d, %s of '%s', where the file has %s of '%s': the part %s is kept", Q2C(QUrl(m_url).host()), m_offset,
				    Q2C(m_url), m_getStatus, Q2C(announced), Q2C(contentType), Q2C(expected), Q2C(firstContentType), Q2C(m_fullPath));
				m_getFailure = tr("%1 answers the range from byte %2 of %3 with status %4, %5 of '%6', where the file has %7 of '%8': the part %9 is kept")
				                   .arg(QUrl(m_url).host(), QString::number(m_offset), m_url, QString::number(m_getStatus), announced, contentType, expected, firstContentType, m_fullPath);
				closeFile();
				reply->abort();
				return;
			}

			nlwarning("The server sends the whole of %s instead of the range from byte %" NL_I64 "d, the part %s is downloaded again", Q2C(m_url), m_offset, Q2C(m_fullPath));
			if (!m_file->resize(0) || !m_file->seek(0))
			{
				nlwarning("Unable to empty %s: %s", Q2C(m_fullPath), Q2C(m_file->errorString()));
				m_getFailure = tr("Unable to empty %1 for the whole file the server sends: %2").arg(m_fullPath, m_file->errorString());
				closeFile();
				reply->abort();
				return;
			}
			m_offset = 0;
		}
	}

	// the body of an answer that is neither the file nor its range, such as an error page, stays out of the part
	if ((m_getStatus != 200) && (m_getStatus != 206))
	{
		reply->readAll();
		return;
	}

	// a short or failed write ends the download, the file would be accepted without the missing data
	const QByteArray data = reply->readAll();
	if (m_file->write(data) != data.size())
	{
		nlwarning("Unable to write to %s: %s", Q2C(m_fullPath), Q2C(m_file->errorString()));
		m_getFailure = tr("Unable to write to %1: %2").arg(m_fullPath, m_file->errorString());
		closeFile();
		reply->abort();
	}
}

// Nel MMORPG framework - Error Reporter
//
// Copyright (C) 2015  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "crash_report_socket.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#   include <QUrlQuery>
#endif

class CCrashReportSocketPvt
{
public:
	QNetworkAccessManager mgr;
};

CCrashReportSocket::CCrashReportSocket( QObject *parent ) :
QObject( parent )
{
	m_pvt = new CCrashReportSocketPvt();

	connect( &m_pvt->mgr, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( onFinished( QNetworkReply* ) ) );
}

CCrashReportSocket::~CCrashReportSocket()
{
	delete m_pvt;
}

void CCrashReportSocket::sendReport( const SCrashReportData &data )
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
	QUrlQuery params;
#else
	QUrl params;
#endif
	params.addQueryItem( "report", data.report );
	params.addQueryItem( "descr", data.description );
	params.addQueryItem("email", data.email);

	QUrl url( m_url );
	QNetworkRequest request( url );
	request.setRawHeader( "Connection", "close" );

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
	QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();
#else
	QByteArray postData = params.encodedQuery();
#endif

	m_pvt->mgr.post(request, postData);
}

void CCrashReportSocket::onFinished( QNetworkReply *reply )
{
	if( reply->error() != QNetworkReply::NoError )
		Q_EMIT reportFailed();
	else
		Q_EMIT reportSent();
}


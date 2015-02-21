// Ryzom Core MMORPG framework - Error Reporter
//
// Copyright (C) 2015 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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

#include "rcerror_socket.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace
{
	static const char *BUG_URL = "http://192.168.2.66/dfighter/r.php";
}

class RCErrorSocketPvt
{
public:
	QNetworkAccessManager mgr;
};

RCErrorSocket::RCErrorSocket( QObject *parent ) :
QObject( parent )
{
	m_pvt = new RCErrorSocketPvt();

	connect( &m_pvt->mgr, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( onFinished( QNetworkReply* ) ) );
}

RCErrorSocket::~RCErrorSocket()
{
	delete m_pvt;
}

void RCErrorSocket::sendReport( const RCErrorData &data )
{
	QUrl params;
	params.addQueryItem( "report", data.report );
	params.addQueryItem( "descr", data.description );
	params.addQueryItem( "email", data.email );

	QUrl url( BUG_URL );
	QNetworkRequest request( url );
	request.setRawHeader( "Connection", "close" );

	m_pvt->mgr.post( request, params.encodedQuery() );
}

void RCErrorSocket::onFinished( QNetworkReply *reply )
{
	if( reply->error() != QNetworkReply::NoError )
		Q_EMIT reportFailed();
	else
		Q_EMIT reportSent();
}


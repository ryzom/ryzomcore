// Nel MMORPG framework - Error Reporter
//
// Copyright (C) 2010-2015  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#ifndef RCERROR_SOCKET
#define RCERROR_SOCKET

#include "crash_report_data.h"

#include <QObject>

class CCrashReportSocketPvt;
class QNetworkReply;

class CCrashReportSocket : public QObject
{
	Q_OBJECT

public:	
	CCrashReportSocket( QObject *parent );
	~CCrashReportSocket();

	void setURL( const char *URL ){ m_url = URL; }
	QString url() const{ return m_url; }

	void sendReport( const SCrashReportData &data );

Q_SIGNALS:
	void reportSent();
	void reportFailed();

private Q_SLOTS:
	void onFinished( QNetworkReply *reply );

private:
	CCrashReportSocketPvt *m_pvt;
	QString m_url;
};

#endif


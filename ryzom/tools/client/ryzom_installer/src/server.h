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

#ifndef SERVER_H
#define SERVER_H

#include "operation.h"

class CServer;

typedef QVector<CServer> CServers;

class CServer
{
public:
	CServer()
	{
		dataCompressedSize = 0;
		dataUncompressedSize = 0;
	}

	QString id;
	QString name;
	QString displayUrl;
	QString filesListUrl;
	QString dataDownloadUrl;
	QString dataDownloadFilename;
	qint64 dataCompressedSize;
	qint64 dataUncompressedSize;
	QString clientDownloadUrl;
	QString clientDownloadFilename;
	QString clientFilename;
	QString clientFilenameOld;
	QString configurationFilename;
	QString comments;

	void loadFromSettings(const QSettings &settings);
	void loadFromServers(const CServers &servers);
	void loadFromServer(const CServer &server);
	void saveToSettings(QSettings &settings) const;

	// helpers
	QString getDirectory() const;
	QString getClientFullPath() const;
	QString getConfigurationFullPath() const;
	QString getDefaultClientConfigFullPath() const;
};

extern const CServer NoServer;

#endif

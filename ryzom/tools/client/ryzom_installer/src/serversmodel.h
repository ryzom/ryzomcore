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

#ifndef SERVERSMODEL_H
#define SERVERSMODEL_H

#include "configfile.h"

/**
 * Servers model
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CServersModel : public QAbstractListModel
{
public:
	CServersModel(QObject *parent);
	CServersModel(const CServers &servers, QObject *parent);
	virtual ~CServersModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

	CServers& getServers() { return m_servers; }

	bool save() const;

	int getIndexFromServerID(const QString &serverId) const;
	QString getServerIDFromIndex(int index) const;

private:
	CServers m_servers;
};

#endif

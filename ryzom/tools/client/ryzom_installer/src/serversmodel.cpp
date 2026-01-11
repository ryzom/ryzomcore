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
#include "serversmodel.h"

CServersModel::CServersModel(QObject *parent):QAbstractListModel(parent)
{
	m_servers = CConfigFile::getInstance()->getServers();
}

CServersModel::CServersModel(const CServers &servers, QObject *parent):QAbstractListModel(parent), m_servers(servers)
{
}

CServersModel::~CServersModel()
{
}

int CServersModel::rowCount(const QModelIndex &parent) const
{
	return m_servers.size();
}

QVariant CServersModel::data(const QModelIndex &index, int role) const
{
	if (role != Qt::DisplayRole) return QVariant();

	const CServer &server = m_servers.at(index.row());

	return server.name;
}

bool CServersModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if (row < 0) return false;

	beginRemoveRows(parent, row, row + count - 1);

	m_servers.removeAt(row);

	endRemoveRows();

	return true;
}

bool CServersModel::save() const
{
	CConfigFile::getInstance()->setServers(m_servers);

	return true;
}

int CServersModel::getIndexFromServerID(const QString &serverId) const
{
	for(int i = 0; i < m_servers.size(); ++i)
	{
		if (m_servers[i].id == serverId) return i;
	}

	return -1;
}

QString CServersModel::getServerIDFromIndex(int index) const
{
	if (index < 0 || index >= m_servers.size()) return "";

	return m_servers[index].id;
}

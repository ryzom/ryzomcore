// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef SERVDASH_REGISTRY_H
#define SERVDASH_REGISTRY_H

#include <QString>
#include <QVector>

/// The running dashboards of this user, shared with servdash_cli: a JSON record per dashboard
/// in a per-user directory, with the endpoint its clients connect to. servdash_cli lists them
/// and attaches to them, the MCP servers of both tools serve them all.
namespace Registry {

struct CRecord
{
	/// A stable hash of the ini path, the record's file name.
	QString Id;
	QString IniPath;
	QString Title;
	QString Endpoint;
	qint64 Pid = 0;
	/// Unix seconds.
	qint64 Started = 0;
	/// servdash_cli or servdash_gui.
	QString Tool;
};

QString toolName();
QString directory();
/// The id servdash_cli computes for the same file.
QString idFor(const QString &iniPath);
bool write(const CRecord &record, QString &error);
void remove(const QString &id);
/// Every record as it stands, oldest first; a record whose endpoint is gone may still be there.
QVector<CRecord> read();

} // namespace Registry

#endif // SERVDASH_REGISTRY_H

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

#ifndef SERVDASH_SHARD_LOCK_H
#define SERVDASH_SHARD_LOCK_H

#include <QString>

/// The lock that keeps a shard from being launched twice, shared with servdash_cli: a file
/// next to the ini, `<stem>.servdash.lock`, with three lines, the pid of the dashboard that
/// holds it, its tool name and its host name. A holder whose process is gone is stale and
/// taken over; a live one is reported. The file is created exclusively, so two dashboards
/// starting at once cannot both win.
class CShardLock
{
public:
	CShardLock();
	~CShardLock();

	/// Takes the lock for this process; on failure `holder` says who has it.
	bool acquire(const QString &iniPath, QString &holder);
	void release();

	static QString lockPath(const QString &iniPath);

private:
	QString m_Path;
};

#endif // SERVDASH_SHARD_LOCK_H

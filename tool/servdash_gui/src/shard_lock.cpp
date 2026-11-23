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

#include "shard_lock.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSysInfo>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <errno.h>
#include <signal.h>
#endif

namespace {

const char *const s_ToolName = "servdash_gui";

struct CHolder
{
	qint64 Pid = 0;
	QString Tool;
	QString Host;
};

bool processAlive(qint64 pid)
{
#ifdef Q_OS_WIN
	HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, (DWORD)pid);
	if (handle == nullptr)
		return false;
	DWORD code = 0;
	bool alive = GetExitCodeProcess(handle, &code) && code == STILL_ACTIVE;
	CloseHandle(handle);
	return alive;
#else
	return kill((pid_t)pid, 0) == 0 || errno == EPERM;
#endif
}

bool readHolder(const QString &path, CHolder &holder)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	const QStringList lines = QString::fromUtf8(file.readAll()).split('\n');
	if (lines.isEmpty())
		return false;
	bool ok = false;
	holder.Pid = lines[0].trimmed().toLongLong(&ok);
	if (!ok)
		return false;
	holder.Tool = lines.value(1).trimmed();
	holder.Host = lines.value(2).trimmed();
	return true;
}

QString hostName()
{
	return QSysInfo::machineHostName();
}

/// Host names differ in case between tools and systems (COMPUTERNAME is upper case, the DNS
/// name lower case), so the comparison ignores it.
bool sameHost(const QString &a, const QString &b)
{
	return a.compare(b, Qt::CaseInsensitive) == 0;
}

QString describe(const CHolder &holder)
{
	return QString("%1 (pid %2) on %3").arg(holder.Tool.isEmpty() ? "an unknown dashboard" : holder.Tool).arg(holder.Pid).arg(holder.Host);
}

} // namespace

CShardLock::CShardLock()
{
}

CShardLock::~CShardLock()
{
	release();
}

QString CShardLock::lockPath(const QString &iniPath)
{
	QFileInfo info(iniPath);
	return info.dir().filePath(info.completeBaseName() + ".servdash.lock");
}

bool CShardLock::acquire(const QString &iniPath, QString &holderText)
{
	const QString path = lockPath(iniPath);
	const QString host = hostName();
	for (int attempt = 0; attempt < 2; ++attempt)
	{
		QFile file(path);
		if (file.open(QIODevice::WriteOnly | QIODevice::NewOnly))
		{
			file.write(QString("%1\n%2\n%3\n").arg(QCoreApplication::applicationPid()).arg(s_ToolName, host).toUtf8());
			m_Path = path;
			return true;
		}
		CHolder holder;
		if (readHolder(path, holder) && (!sameHost(holder.Host, host) || processAlive(holder.Pid)))
		{
			holderText = describe(holder);
			return false;
		}
		// stale or unreadable: remove it and try once more
		QFile::remove(path);
	}
	CHolder holder;
	holderText = readHolder(path, holder) ? describe(holder) : QString("an unknown dashboard");
	return false;
}

void CShardLock::release()
{
	if (m_Path.isEmpty())
		return;
	// only our own lock goes
	CHolder holder;
	if (readHolder(m_Path, holder) && holder.Pid == QCoreApplication::applicationPid())
		QFile::remove(m_Path);
	m_Path.clear();
}

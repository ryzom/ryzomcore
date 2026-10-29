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

#include "registry.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

namespace Registry {

QString toolName()
{
	return "servdash_gui";
}

QString directory()
{
#ifdef Q_OS_WIN
	QString base = qEnvironmentVariable("LOCALAPPDATA");
	if (base.isEmpty())
		base = QDir::tempPath();
	return QDir(base).filePath("servdash/sessions");
#else
	QString runtime = qEnvironmentVariable("XDG_RUNTIME_DIR");
	if (!runtime.isEmpty())
		return QDir(runtime).filePath("servdash.sessions");
	return QString("/tmp/servdash.sessions.%1").arg((qulonglong)getuid());
#endif
}

QString idFor(const QString &iniPath)
{
	// the same key as servdash_cli: the canonical native path, lower cased on Windows
	QFileInfo info(iniPath);
	QString key = info.canonicalFilePath();
	if (key.isEmpty())
		key = info.absoluteFilePath();
	key = QDir::toNativeSeparators(key);
#ifdef Q_OS_WIN
	key = key.toLower();
#endif
	const QByteArray bytes = key.toUtf8();
	quint64 hash = Q_UINT64_C(0xcbf29ce484222325);
	for (char c : bytes)
	{
		hash ^= (quint64)(uchar)c;
		hash *= Q_UINT64_C(0x100000001b3);
	}
	return QString("%1").arg(hash, 16, 16, QChar('0'));
}

static QString recordPath(const QString &id)
{
	return QDir(directory()).filePath(id + ".json");
}

bool write(const CRecord &record, QString &error)
{
	QDir dir(directory());
	if (!dir.mkpath("."))
	{
		error = QString("Cannot create '%1'").arg(dir.path());
		return false;
	}
#ifndef Q_OS_WIN
	QFile::setPermissions(dir.path(), QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
#endif
	QJsonObject o;
	o["id"] = record.Id;
	o["ini_path"] = record.IniPath;
	o["title"] = record.Title;
	o["endpoint"] = record.Endpoint;
	o["pid"] = record.Pid;
	o["started"] = record.Started;
	o["tool"] = record.Tool;
	QFile file(recordPath(record.Id));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		error = QString("Cannot write '%1': %2").arg(file.fileName(), file.errorString());
		return false;
	}
	file.write(QJsonDocument(o).toJson(QJsonDocument::Indented));
	return true;
}

void remove(const QString &id)
{
	QFile::remove(recordPath(id));
}

QVector<CRecord> read()
{
	QVector<CRecord> records;
	QDir dir(directory());
	for (const QFileInfo &info : dir.entryInfoList(QStringList() << "*.json", QDir::Files))
	{
		QFile file(info.absoluteFilePath());
		if (!file.open(QIODevice::ReadOnly))
			continue;
		const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
		if (!doc.isObject())
			continue;
		const QJsonObject o = doc.object();
		CRecord record;
		record.Id = o.value("id").toString();
		record.IniPath = o.value("ini_path").toString();
		record.Title = o.value("title").toString();
		record.Endpoint = o.value("endpoint").toString();
		record.Pid = (qint64)o.value("pid").toDouble();
		record.Started = (qint64)o.value("started").toDouble();
		record.Tool = o.value("tool").toString();
		if (record.Tool.isEmpty())
			record.Tool = "servdash_cli";
		if (!record.Id.isEmpty() && !record.Endpoint.isEmpty())
			records.append(record);
	}
	std::sort(records.begin(), records.end(), [](const CRecord &a, const CRecord &b) { return a.Started != b.Started ? a.Started < b.Started : a.Id < b.Id; });
	return records;
}

} // namespace Registry

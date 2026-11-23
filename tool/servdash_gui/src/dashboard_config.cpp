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

#include "dashboard_config.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

namespace {

struct CRawSection
{
	QString Name;
	QVector<QPair<QString, QString>> Keys;

	bool has(const QString &key) const
	{
		for (const QPair<QString, QString> &kv : Keys)
			if (kv.first == key)
				return true;
		return false;
	}

	// the last assignment wins, as with a dictionary store
	QString get(const QString &key, const QString &fallback = QString()) const
	{
		for (int i = Keys.size() - 1; i >= 0; --i)
			if (Keys[i].first == key)
				return Keys[i].second;
		return fallback;
	}
};

QVector<CRawSection> parseRaw(const QString &text)
{
	QVector<CRawSection> sections;
	sections.append(CRawSection());
	const QStringList lines = text.split('\n');
	for (QString line : lines)
	{
		if (line.endsWith('\r'))
			line.chop(1);
		if (line.startsWith(';'))
			continue;
		if (line.startsWith('['))
		{
			int end = line.indexOf(']');
			QString name = end >= 0 ? line.mid(1, end - 1) : line.mid(1);
			bool found = false;
			for (CRawSection &section : sections)
			{
				if (section.Name == name)
				{
					// a repeated header replaces the earlier section, keeping its place
					section.Keys.clear();
					found = true;
					break;
				}
			}
			if (!found)
			{
				CRawSection section;
				section.Name = name;
				sections.append(section);
			}
			continue;
		}
		if (line.isEmpty())
			continue;
		int eq = line.indexOf('=');
		QString key = eq >= 0 ? line.left(eq) : line;
		QString value = eq >= 0 ? line.mid(eq + 1) : QString();
		sections.last().Keys.append(qMakePair(key, value));
	}
	return sections;
}

// Windows separators in a path value, for inis written on Windows and used elsewhere
QString nativePath(const QString &value)
{
#ifdef Q_OS_WIN
	return value;
#else
	QString v = value;
	return v.replace('\\', '/');
#endif
}

QString combine(const QString &workingDirectory, const QString &file)
{
	if (workingDirectory.isEmpty())
		return nativePath(file);
	QFileInfo info(nativePath(file));
	if (info.isAbsolute())
		return info.filePath();
	return QDir(workingDirectory).filePath(nativePath(file));
}

} // namespace

QString CSectionConfig::program(const QString &command) const
{
	if (!WorkingDirectory.isEmpty() && command.startsWith('.'))
		return QDir::current().absoluteFilePath(QDir(WorkingDirectory).filePath(nativePath(command)));
	return command;
}

bool CDashboardConfig::read(const QString &path, CDashboardConfig &config, QString &error)
{
	config = CDashboardConfig();
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly))
	{
		error = QString("Cannot read '%1': %2").arg(path, file.errorString());
		return false;
	}
	QString text = QString::fromUtf8(file.readAll());
	if (text.startsWith(QChar(0xFEFF)))
		text.remove(0, 1);
	const QVector<CRawSection> raw = parseRaw(text);
	config.Title = raw[0].get("Title", "Service Dashboard");
	for (int i = 1; i < raw.size(); ++i)
	{
		const CRawSection &r = raw[i];
		CSectionConfig s;
		s.Name = r.Name;
		s.Title = r.get("Title");
		s.LaunchCmd = r.get("LaunchCmd");
		if (s.LaunchCmd.isEmpty())
		{
			error = QString("Section [%1] has no LaunchCmd").arg(r.Name);
			return false;
		}
		s.WorkingDirectory = nativePath(r.get("WorkingDirectory"));
		s.LaunchArgs = r.get("LaunchArgs");
		s.ShutdownCmd = r.get("ShutdownCmd");
		s.ShutdownArgs = r.get("ShutdownArgs");
		s.ShutdownInput = r.get("ShutdownInput");
		s.ReadyPattern = r.get("ReadyPattern");
		if (r.has("Priority"))
		{
			bool ok = false;
			s.Priority = r.get("Priority").trimmed().toInt(&ok);
			if (!ok)
			{
				error = QString("Section [%1] has an invalid Priority '%2'").arg(r.Name, r.get("Priority"));
				return false;
			}
		}
		s.AutoStart = r.get("AutoStart") != "0";
		if (r.has("RestartDelay") && !r.get("RestartDelay").trimmed().isEmpty())
		{
			bool ok = false;
			s.RestartDelay = r.get("RestartDelay").trimmed().toDouble(&ok);
			if (!ok || s.RestartDelay < 0)
			{
				error = QString("Section [%1] has an invalid RestartDelay '%2'").arg(r.Name, r.get("RestartDelay"));
				return false;
			}
		}
		s.Control = r.get("Control") == "1";
		if (r.has("LaunchCtrl"))
		{
			const QStringList parts = r.get("LaunchCtrl").split(',');
			if (parts.size() != 6)
			{
				error = QString("Section [%1] LaunchCtrl needs 6 comma separated values, got %2").arg(r.Name).arg(parts.size());
				return false;
			}
			s.HasLaunchCtrl = true;
			s.LaunchCtrl.CtrlFile = combine(s.WorkingDirectory, parts[0]);
			s.LaunchCtrl.StateFile = combine(s.WorkingDirectory, parts[1]);
			for (int v = 0; v < 4; ++v)
				s.LaunchCtrl.Values[v] = parts[2 + v];
		}
		config.Sections.append(s);
	}
	return true;
}

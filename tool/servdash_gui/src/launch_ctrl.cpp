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

#include "launch_ctrl.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace {

QString siblingWithSuffix(const QString &path, const QString &oldSuffix, const QString &newSuffix)
{
	QFileInfo info(path);
	QString name = info.fileName();
	if (!name.endsWith(oldSuffix))
		return QString();
	name.chop(oldSuffix.size());
	return info.dir().filePath(name + newSuffix);
}

bool writeText(const QString &path, const QString &text)
{
	QFileInfo info(path);
	QDir().mkpath(info.dir().path());
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return false;
	file.write(text.toUtf8());
	return true;
}

} // namespace

CLaunchCtrl::CLaunchCtrl(const CLaunchCtrlConfig &config, bool autoStart, std::function<void(const QString &)> report)
    : m_Config(config)
    , m_Report(std::move(report))
{
	m_DeferredFile = siblingWithSuffix(config.CtrlFile, ".launch_ctrl", ".deferred_launch_ctrl");
	m_StartCountFile = siblingWithSuffix(config.StateFile, ".state", ".start_count");

	// a restart order left by the AES while no dashboard ran is an order like any other
	promoteDeferred();
	bool exists = false;
	QString text = readCtrl(exists);
	if (exists)
	{
		m_AutoLaunch = valueOf(text, Launch) == Launch;
	}
	else
	{
		writeCtrl(autoStart ? Launch : Stop);
		m_AutoLaunch = autoStart;
	}
	writeState(Stopped);
	writeStartCount();
	m_LastSeen = observe();
}

CLaunchCtrl::CSeen CLaunchCtrl::observe() const
{
	CSeen seen;
	QFileInfo info(m_Config.CtrlFile);
	seen.Exists = info.exists();
	if (seen.Exists)
	{
		seen.Text = readCtrl(seen.Exists);
		seen.Modified = info.lastModified();
	}
	return seen;
}

QString CLaunchCtrl::readCtrl(bool &exists) const
{
	QFile file(m_Config.CtrlFile);
	exists = file.open(QIODevice::ReadOnly);
	if (!exists)
		return QString();
	return QString::fromUtf8(file.readAll());
}

int CLaunchCtrl::valueOf(const QString &text, int stateValue) const
{
	// surrounding whitespace is ignored, so `echo LAUNCH > x.launch_ctrl` works as with the shell launcher
	const QString trimmed = text.trimmed();
	for (int i = 0; i < 4; ++i)
		if (m_Config.Values[i] == trimmed)
			return i;
	return stateValue;
}

bool CLaunchCtrl::ctrlIs(int index) const
{
	bool exists = false;
	QString text = readCtrl(exists);
	return exists && text.trimmed() == m_Config.Values[index];
}

void CLaunchCtrl::writeOrReport(const QString &path, const QString &text)
{
	if (!writeText(path, text) && m_Report)
		m_Report(QString("Cannot write '%1'").arg(path));
}

void CLaunchCtrl::writeCtrl(int index)
{
	writeOrReport(m_Config.CtrlFile, m_Config.Values[index]);
	// what we wrote ourselves is not a change to react to
	m_LastSeen = observe();
}

void CLaunchCtrl::writeState(int index)
{
	if (index != Running && index != Stopped)
		return;
	writeOrReport(m_Config.StateFile, m_Config.Values[index]);
}

void CLaunchCtrl::writeStartCount()
{
	if (!m_StartCountFile.isEmpty())
		writeOrReport(m_StartCountFile, QString::number(m_StartCount));
}

void CLaunchCtrl::setValue(int index)
{
	m_Value = index;
	if (index == Running || index == Stopped)
	{
		QFile file(m_Config.StateFile);
		QString current;
		if (file.open(QIODevice::ReadOnly))
			current = QString::fromUtf8(file.readAll());
		if (current != m_Config.Values[index])
			writeState(index);
	}
}

void CLaunchCtrl::countLaunch()
{
	++m_StartCount;
	writeStartCount();
}

void CLaunchCtrl::promoteDeferred()
{
	if (m_DeferredFile.isEmpty() || !QFileInfo::exists(m_DeferredFile))
		return;
	// the current order gives way to the deferred one; a failed move is reported, not hidden
	if (QFileInfo::exists(m_Config.CtrlFile) && !QFile::remove(m_Config.CtrlFile))
	{
		if (m_Report)
			m_Report(QString("Cannot remove '%1'").arg(m_Config.CtrlFile));
		return;
	}
	if (!QFile::rename(m_DeferredFile, m_Config.CtrlFile) && m_Report)
		m_Report(QString("Cannot move '%1' to '%2'").arg(m_DeferredFile, m_Config.CtrlFile));
}

int CLaunchCtrl::poll(int stateValue, bool running)
{
	if (!running)
	{
		// the AES writes the deferred order before it asks the service to quit; it may already
		// be stopped, or the dashboard may have started after the order was written
		promoteDeferred();
	}
	CSeen seen = observe();
	if (seen == m_LastSeen)
		return -1;
	m_LastSeen = seen;
	if (!seen.Exists)
		return -1;
	return valueOf(seen.Text, stateValue);
}

void CLaunchCtrl::afterExit()
{
	if (ctrlIs(Launch) && !QFile::remove(m_Config.CtrlFile) && m_Report)
		m_Report(QString("Cannot remove '%1'").arg(m_Config.CtrlFile));
	promoteDeferred();
	// the next poll reports the resulting order, if any; a STOP or an unchanged file is not a change
	m_LastSeen = CSeen();
	bool exists = false;
	QString text = readCtrl(exists);
	if (exists && valueOf(text, Stopped) != Launch)
		m_LastSeen = observe();
}

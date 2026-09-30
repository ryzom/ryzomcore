// Ryzom Core Studio - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// Project includes
#include "edit_history.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QDateTime>
#include <QFile>
#include <QTextStream>

namespace WorldEditor
{

namespace
{
const char *const HISTORY_SUFFIX = ".history";
const char *const TIME_FORMAT = "yyyy-MM-dd hh:mm:ss";
}

EditHistory::EditHistory(QObject *parent)
	: QObject(parent),
	  m_sessionMarked(false)
{
}

QString EditHistory::historyFileName() const
{
	if (m_projectFile.isEmpty())
		return QString();

	return m_projectFile + QLatin1String(HISTORY_SUFFIX);
}

QStringList EditHistory::previousEntries() const
{
	return m_previousEntries;
}

void EditHistory::setProjectFile(const QString &worldEditFile)
{
	if (worldEditFile == m_projectFile)
		return;

	m_projectFile = worldEditFile;
	m_previousEntries.clear();
	m_sessionMarked = false;

	const QString fileName = historyFileName();
	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream stream(&file);
		while (!stream.atEnd())
		{
			const QString line = stream.readLine();
			if (!line.isEmpty())
				m_previousEntries.append(line);
		}
		file.close();
	}

	Q_EMIT historyLoaded(m_previousEntries, m_projectFile);

	// Anything recorded before the project had a name belongs in the file as well.
	const QStringList pending = m_pending;
	m_pending.clear();
	Q_FOREACH (const QString &line, pending)
		writeLine(line);
}

void EditHistory::writeLine(const QString &line)
{
	const QString fileName = historyFileName();
	if (fileName.isEmpty())
	{
		m_pending.append(line);
		return;
	}

	QFile file(fileName);
	if (!file.open(QIODevice::Append | QIODevice::Text))
	{
		// Worth one warning, not worth interrupting the work over.
		static bool warned = false;
		if (!warned)
		{
			warned = true;
			nlwarning("World Editor: cannot write the history file '%s', "
					  "this session will not be recorded.",
					  fileName.toUtf8().constData());
		}
		return;
	}

	QTextStream stream(&file);
	if (!m_sessionMarked)
	{
		m_sessionMarked = true;
		stream << "\n--- session " << QDateTime::currentDateTime().toString(TIME_FORMAT)
			   << " ---\n";
	}
	stream << line << "\n";
	file.close();
}

void EditHistory::append(const QString &action)
{
	if (action.isEmpty())
		return;

	const QString line = QDateTime::currentDateTime().toString(TIME_FORMAT) +
						 QLatin1String("  ") + action;

	writeLine(line);
	Q_EMIT entryAppended(line);
}

} /* namespace WorldEditor */

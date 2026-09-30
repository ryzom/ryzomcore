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

#ifndef EDIT_HISTORY_H
#define EDIT_HISTORY_H

// Qt includes
#include <QObject>
#include <QString>
#include <QStringList>

namespace WorldEditor
{

/**
@class EditHistory
@brief Records what was done to a project, across sessions.
@details This is deliberately not the undo stack. The commands in there hold pointers to
	the model, the scene and single primitives, so they cannot outlive the session, and
	replaying them against files that may have changed in the meantime would do more harm
	than good. What survives is a plain journal: one line per action, with a time stamp.

	The journal lives next to the project as "<name>.worldedit.history", so it travels
	with the project and stays readable without studio. Lines are written as they happen,
	not on exit, so a crash keeps what came before it.
*/
class EditHistory : public QObject
{
	Q_OBJECT

public:
	explicit EditHistory(QObject *parent = 0);

	/// Point the journal at a project: read back what earlier sessions recorded and
	/// write out whatever was logged while the project still had no file name.
	void setProjectFile(const QString &worldEditFile);

	/// Lines earlier sessions recorded, oldest first.
	QStringList previousEntries() const;

	/// Record one action. Without a project file it is kept until there is one.
	void append(const QString &action);

Q_SIGNALS:
	/// A project was opened: entries is what earlier sessions left behind.
	void historyLoaded(const QStringList &entries, const QString &projectFile);

	/// One action was recorded.
	void entryAppended(const QString &line);

private:
	QString historyFileName() const;
	void writeLine(const QString &line);

	QString m_projectFile;
	QStringList m_previousEntries;

	/// Logged before the project had a name, written out as soon as it has one.
	QStringList m_pending;

	/// Whether this session already wrote its header into the file.
	bool m_sessionMarked;
};

} /* namespace WorldEditor */

#endif // EDIT_HISTORY_H

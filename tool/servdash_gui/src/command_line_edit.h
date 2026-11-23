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

#ifndef SERVDASH_COMMAND_LINE_EDIT_H
#define SERVDASH_COMMAND_LINE_EDIT_H

#include "process_entry.h"

#include <QLineEdit>
#include <QList>

/// The command line of a service console, completing against the command table the service
/// reports. The rest of the first matching name shows in grey after the cursor, with the
/// arguments the command takes, and its help is elided at the right. Tab takes the suggestion
/// (a handler instance such as `moduleManager` completes to `moduleManager.` and its commands
/// follow), Shift+Tab moves to the next match. Once the name is followed by a space, only the
/// help of that command shows.
class CCommandLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	explicit CCommandLineEdit(CProcessEntry *entry, QWidget *parent = nullptr);

protected:
	void paintEvent(QPaintEvent *event) override;
	bool focusNextPrevChild(bool next) override;

private slots:
	void onCommandsChanged();
	void onTextChanged();

private:
	struct CSuggestion
	{
		QList<CProcessEntry::CCommand> Candidates;
		/// The one shown as ghost text and in the hint.
		int Index = 0;
		/// Still typing the command name: the ghost text completes it.
		bool Naming = true;

		const CProcessEntry::CCommand &current() const { return Candidates[Index]; }
	};

	/// The completion of the text against the command table, when there is one.
	bool suggestion(CSuggestion &out) const;

	CProcessEntry *m_Entry;
	/// Which of the completions of the typed word is suggested; Shift+Tab moves it.
	int m_CompletionIndex = 0;
};

#endif // SERVDASH_COMMAND_LINE_EDIT_H

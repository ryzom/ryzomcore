// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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


#ifndef SERVER_ENTRY_DIALOG_H
#define SERVER_ENTRY_DIALOG_H

#include <QDialog>

namespace Ui {
    class ServerEntryDialog;
}

namespace MissionCompiler
{
/**
@class ServerEntryDialog
*/
class ServerEntryDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ServerEntryDialog(QWidget *parent = 0);
	~ServerEntryDialog();

	QString getServerName();
	QString getTextPath();
	QString getPrimPath();

	void setServerName(QString name);
	void setTextPath(QString path);
	void setPrimPath(QString path);

public Q_SLOTS:
	void lookupTextPath();
	void lookupPrimPath();

private:
	Ui::ServerEntryDialog *m_ui;
};

} // namespace MissionCompiler

#endif // SERVER_ENTRY_DIALOG_H

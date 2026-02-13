// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "output_console_dock.h"

#include <QVBoxLayout>
#include <QDateTime>

OutputConsoleDock::OutputConsoleDock(QWidget *parent)
	: QDockWidget(tr("Output Console"), parent)
{
	setupUi();
}

OutputConsoleDock::~OutputConsoleDock()
{
}

void OutputConsoleDock::setupUi()
{
	QWidget *container = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);

	_textEdit = new QTextEdit(container);
	_textEdit->setReadOnly(true);
	_textEdit->setFont(QFont("Courier", 9));

	layout->addWidget(_textEdit);
	setWidget(container);
}

void OutputConsoleDock::outputString(const QString &message)
{
	QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
	_textEdit->append(QString("[%1] %2").arg(timestamp, message));
}

void OutputConsoleDock::clear()
{
	_textEdit->clear();
}

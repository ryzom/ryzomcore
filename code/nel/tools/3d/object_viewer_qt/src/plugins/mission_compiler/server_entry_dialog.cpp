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

// Project includes
#include "server_entry_dialog.h"

#include "ui_server_entry_dialog.h"

// NeL includes

// Qt includes
#include <QFileDialog>

namespace MissionCompiler
{

ServerEntryDialog::ServerEntryDialog(QWidget *parent)
	: QDialog(parent),
	  m_ui(new Ui::ServerEntryDialog)
{
	m_ui->setupUi(this);
	
	connect(m_ui->serverTextPathButton, SIGNAL(clicked()), this, SLOT(lookupTextPath()));
	connect(m_ui->serverPrimPathButton, SIGNAL(clicked()), this, SLOT(lookupPrimPath()));
}

ServerEntryDialog::~ServerEntryDialog()
{
	delete m_ui;
}

QString ServerEntryDialog::getServerName()
{
	return m_ui->serverNameEdit->text();
}

QString ServerEntryDialog::getTextPath()
{
	return m_ui->serverTextPathEdit->text();
}

QString ServerEntryDialog::getPrimPath()
{
	return m_ui->serverPrimPathEdit->text();
}

void ServerEntryDialog::setServerName(QString name)
{
	m_ui->serverNameEdit->setText(name);
}

void ServerEntryDialog::setTextPath(QString path)
{
	m_ui->serverTextPathEdit->setText(path);
}

void ServerEntryDialog::setPrimPath(QString path)
{
	m_ui->serverPrimPathEdit->setText(path);
}

void ServerEntryDialog::lookupTextPath()
{
	QString curPath = m_ui->serverTextPathEdit->text();
	QString path = QFileDialog::getExistingDirectory(this, "", curPath);
	m_ui->serverTextPathEdit->setText(path);
}

void ServerEntryDialog::lookupPrimPath()
{
	QString curPath = m_ui->serverPrimPathEdit->text();
	QString path = QFileDialog::getExistingDirectory(this, "", curPath);
	m_ui->serverPrimPathEdit->setText(path);
}
} /* namespace MissionCompiler */
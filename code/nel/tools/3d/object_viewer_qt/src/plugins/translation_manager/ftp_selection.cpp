// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
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

#include "ftp_selection.h"

#include <QtGui/QMessageBox>
#include <QtNetwork/QFtp>

namespace TranslationManager
{
CFtpSelection::CFtpSelection(QWidget *parent): QDialog(parent)
{
	_ui.setupUi(this);
	connect(_ui.connectButton, SIGNAL(clicked()), this, SLOT(ConnectButtonClicked()));
	connect(_ui.doneButton, SIGNAL(clicked()), this, SLOT(DoneButtonClicked()));
	connect(_ui.cdToParrent, SIGNAL(clicked()), this, SLOT(cdToParent()));
	connect(_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	// file list
	connect(_ui.fileList, SIGNAL(itemActivated(QTreeWidgetItem *,int)),this, SLOT(processItem(QTreeWidgetItem *,int)));
	_ui.fileList->setEnabled(false);
	_ui.fileList->setRootIsDecorated(false);
	_ui.fileList->setHeaderLabels(QStringList() << tr("Name") << tr("Size") << tr("Owner") << tr("Group") << tr("Time"));
	_ui.fileList->header()->setStretchLastSection(false);

	// buttons
	_ui.cdToParrent->setEnabled(false);
	_ui.doneButton->setEnabled(false);

	status = false;
}

// Connection with the FTP Server. We retrieve the file list.
void CFtpSelection::ConnectButtonClicked()
{
	conn = new QFtp(this);
	connect(conn, SIGNAL(commandFinished(int,bool)), this, SLOT(FtpCommandFinished(int,bool)));
	connect(conn, SIGNAL(listInfo(QUrlInfo)), this, SLOT(AddToList(QUrlInfo)));

	setCursor(Qt::WaitCursor);

	QUrl url(_ui.url->text());
	if (!url.isValid() || url.scheme().toLower() != QLatin1String("ftp"))
	{
		conn->connectToHost(_ui.url->text(), 21);
		conn->login();
	}
	else
	{
		conn->connectToHost(url.host(), url.port(21));

		if (!url.userName().isEmpty())
			conn->login(QUrl::fromPercentEncoding(url.userName().toLatin1()), url.password());
		else
			conn->login();
		if (!url.path().isEmpty())
			conn->cd(url.path());
	}
}

// Get the user action.
void CFtpSelection::FtpCommandFinished(int, bool error)
{
	setCursor(Qt::ArrowCursor);

	if (conn->currentCommand() == QFtp::ConnectToHost)
	{
		if (error)
		{
			QMessageBox::information(this, tr("FTP"),
									 tr("Unable to connect to the FTP server "
										"at %1. Please check that the host "
										"name is correct.")
									 .arg(_ui.url->text()));
			return;
		}

		return;
	}

	if (conn->currentCommand() == QFtp::Login)
	{
		conn->list();
	}

	if (conn->currentCommand() == QFtp::Get)
	{
		if(error)
		{
			status = false;
			file->close();
			file->remove();
		}
		else
		{
			file->close();
			status = true;
		}
		_ui.cancelButton->setEnabled(true);
	}

	if (conn->currentCommand() == QFtp::List)
	{
		if (isDirectory.isEmpty())
		{
			_ui.fileList->addTopLevelItem(new QTreeWidgetItem(QStringList() << tr("<empty>")));
			_ui.fileList->setEnabled(false);
		}
	}
}
// Make the file list with directories and files
void CFtpSelection::AddToList(const QUrlInfo &urlInfo)
{
	QTreeWidgetItem *item = new QTreeWidgetItem;
	item->setText(0, urlInfo.name());
	item->setText(1, QString::number(urlInfo.size()));
	item->setText(2, urlInfo.owner());
	item->setText(3, urlInfo.group());
	item->setText(4, urlInfo.lastModified().toString("MMM dd yyyy"));

	QPixmap pixmap(urlInfo.isDir() ? ":/translationManager/images/dir.png" : ":/translationManager/images/file.png");
	item->setIcon(0, pixmap);

	isDirectory[urlInfo.name()] = urlInfo.isDir();
	_ui.fileList->addTopLevelItem(item);
	if (!_ui.fileList->currentItem())
	{
		_ui.fileList->setCurrentItem(_ui.fileList->topLevelItem(0));
		_ui.fileList->setEnabled(true);
	}
}

void CFtpSelection::processItem(QTreeWidgetItem *item, int)
{
	QString name = item->text(0);
	if (isDirectory.value(name))
	{
		_ui.fileList->clear();
		isDirectory.clear();
		currentPath += '/';
		currentPath += name;
		conn->cd(name);
		conn->list();

		setCursor(Qt::WaitCursor);
		return;
	}
	_ui.doneButton->setEnabled(true);
}

// Exit from a directory
void CFtpSelection::cdToParent()
{
	setCursor(Qt::WaitCursor);

	_ui.fileList->clear();
	isDirectory.clear();
	currentPath = currentPath.left(currentPath.lastIndexOf('/'));
	if (currentPath.isEmpty())
	{
		_ui.cdToParrent->setEnabled(false);
		conn->cd("/");
	}
	else
	{
		conn->cd(currentPath);
	}
	conn->list();
}

// Done action
void CFtpSelection::DoneButtonClicked()
{
	QString fileName = _ui.fileList->currentItem()->text(0);

	if (QFile::exists(fileName))
	{
		QMessageBox::information(this, tr("FTP"),
								 tr("There already exists a file called %1 in "
									"the current directory.")
								 .arg(fileName));
		return;
	}

	file = new QFile(fileName);

	setCursor(Qt::WaitCursor);

	if (!file->open(QIODevice::WriteOnly))
	{
		QMessageBox::information(this, tr("FTP"),
								 tr("Unable to save the file %1: %2.")
								 .arg(fileName).arg(file->errorString()));
		delete file;
		return;
	}
	_ui.cancelButton->setEnabled(false);
	conn->get(_ui.fileList->currentItem()->text(0), file);

	reject();
}

}

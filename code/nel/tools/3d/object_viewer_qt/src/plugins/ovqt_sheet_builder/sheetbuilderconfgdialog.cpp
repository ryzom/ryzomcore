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

#include "sheetbuilderconfgdialog.h"
#include "../core/icore.h"

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QSettings>
#include <QCloseEvent>
#include <QFileDialog>

SheetBuilderConfigDialog::SheetBuilderConfigDialog(QWidget *parent)
	: QDialog(parent)
{
	/*
	 * Paths
	 */
	QLabel *lblPaths = new QLabel(tr("Paths:"));
	lstPaths = new QListWidget;
	lstPaths->addItem("");

	QPushButton *btnAddPath = new QPushButton(tr("Add"));
	connect(btnAddPath, SIGNAL(clicked()), SLOT(addPath()));
	QPushButton *btnDeletePath = new QPushButton(tr("Delete"));
	connect(btnDeletePath, SIGNAL(clicked()), SLOT(deletePath()));

	QVBoxLayout *ltButtonsPaths = new QVBoxLayout();
	ltButtonsPaths->addWidget(btnAddPath);
	ltButtonsPaths->addWidget(btnDeletePath);
	ltButtonsPaths->addStretch(1);

	QHBoxLayout *ltPaths = new QHBoxLayout;
	ltPaths->addWidget(lstPaths);
	ltPaths->addLayout(ltButtonsPaths);

	/*
	 * Output file
	 */
	QLabel *lblOutputFile = new QLabel(tr("Output file:"));
	txtOutputFile = new QLineEdit();
	QPushButton *btnBrowse = new QPushButton(tr("Browse..."));
	connect(btnBrowse, SIGNAL(clicked()), SLOT(browseOutput()));

	QHBoxLayout *ltOutput = new QHBoxLayout();
	ltOutput->addWidget(txtOutputFile);
	ltOutput->addWidget(btnBrowse);

	/*
	 * Extensions
	 */
	QLabel *lblExtensions = new QLabel(tr("Allowed extensions:"));
	lstExtensionsAllowed = new QListWidget();

	QPushButton *btnAddExtension = new QPushButton(tr("Add"));
	connect(btnAddExtension, SIGNAL(clicked()), SLOT(addExtension()));
	QPushButton *btnDeleteExtension = new QPushButton(tr("Delete"));
	connect(btnDeleteExtension, SIGNAL(clicked()), SLOT(deleteExtension()));

	QVBoxLayout *ltButtonsExtensions = new QVBoxLayout();
	ltButtonsExtensions->addWidget(btnAddExtension);
	ltButtonsExtensions->addWidget(btnDeleteExtension);
	ltButtonsExtensions->addStretch(1);

	QHBoxLayout *ltExtensions = new QHBoxLayout();
	ltExtensions->addWidget(lstExtensionsAllowed);
	ltExtensions->addLayout(ltButtonsExtensions);

	/*
	 * Buttons
	 */
	QPushButton *btnOk = new QPushButton(tr("OK"));
	connect(btnOk, SIGNAL(clicked()), SLOT(accept()));
	connect(btnOk, SIGNAL(clicked()), SLOT(writeSettings()));

	QPushButton *btnCancel = new QPushButton(tr("Cancel"));
	connect(btnCancel, SIGNAL(clicked()), SLOT(reject()));

	QHBoxLayout *ltButtons = new QHBoxLayout;
	ltButtons->addStretch(1);
	ltButtons->addWidget(btnOk);
	ltButtons->addWidget(btnCancel);

	/*
	 * Main layout
	 */
	QVBoxLayout *ltMain = new QVBoxLayout;
	ltMain->addWidget(lblPaths);
	ltMain->addLayout(ltPaths);
	ltMain->addWidget(lblOutputFile);
	ltMain->addLayout(ltOutput);
	ltMain->addWidget(lblExtensions);
	ltMain->addLayout(ltExtensions);
	ltMain->addLayout(ltButtons);

	setLayout(ltMain);
	setWindowTitle(tr("Sheet builder configuration"));
	resize(500, 450);
	readSettings();
}

void SheetBuilderConfigDialog::addPath()
{
	QString path =
		QFileDialog::getExistingDirectory(this, tr("Choose path"));
	if (!path.isEmpty())
	{
		QListWidgetItem *newItem = new QListWidgetItem;
		newItem->setText(path);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		lstPaths->addItem(newItem);
		lstPaths->setCurrentItem(newItem);
	}
}

void SheetBuilderConfigDialog::addExtension()
{
	QListWidgetItem *newItem = new QListWidgetItem;
	newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	lstExtensionsAllowed->addItem(newItem);
	lstExtensionsAllowed->setCurrentItem(newItem);
}

void SheetBuilderConfigDialog::deletePath()
{
	QListWidgetItem *removeItem = lstPaths->takeItem(lstPaths->currentRow());
	if (!removeItem)
		delete removeItem;
}

void SheetBuilderConfigDialog::deleteExtension()
{
	QListWidgetItem *removeItem
	= lstExtensionsAllowed->takeItem(lstExtensionsAllowed->currentRow());
	if (!removeItem)
		delete removeItem;
}

void SheetBuilderConfigDialog::browseOutput()
{
	QString fileName =
		QFileDialog::getSaveFileName(this, tr("Choose output file"), "");
	if (!fileName.isEmpty())
		txtOutputFile->setText(fileName);
}

void SheetBuilderConfigDialog::readSettings()
{
	QStringList paths;
	QString outputFile;
	QStringList extensions;

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("SheetBuilder");
	paths = settings->value("SheetPaths").toStringList();
	outputFile = settings->value("SheetOutputFile").toString();
	extensions = settings->value("ExtensionsAllowed").toStringList();
	settings->endGroup();

	lstPaths->clear();
	lstExtensionsAllowed->clear();

	QListWidgetItem *newItem;

	Q_FOREACH (QString path, paths)
	{
		newItem = new QListWidgetItem;
		newItem->setText(path);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		lstPaths->addItem(newItem);
	}

	txtOutputFile->setText(outputFile);

	Q_FOREACH (QString extension, extensions)
	{
		newItem = new QListWidgetItem;
		newItem->setText(extension);
		newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		lstExtensionsAllowed->addItem(newItem);
	}
}

void SheetBuilderConfigDialog::writeSettings()
{
	QStringList paths;
	for (int i = 0; i < lstPaths->count(); i++)
		paths.push_back(lstPaths->item(i)->text());

	QString outputFile = txtOutputFile->text();

	QStringList extensions;
	for (int i = 0; i < lstExtensionsAllowed->count(); i++)
		extensions.push_back(lstExtensionsAllowed->item(i)->text());

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("SheetBuilder");
	settings->setValue("SheetPaths", paths);
	settings->setValue("SheetOutputFile", outputFile);
	settings->setValue("ExtensionsAllowed", extensions);
	settings->endGroup();

	// Forced save settings
	settings->sync();
}

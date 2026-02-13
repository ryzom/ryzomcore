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

#include "settings_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	setupUi();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUi()
{
	setWindowTitle(tr("Settings"));
	setMinimumWidth(450);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	QGroupBox *pathGroup = new QGroupBox(tr("Paths"), this);
	QFormLayout *formLayout = new QFormLayout(pathGroup);

	// Root Search Path
	QHBoxLayout *rootPathLayout = new QHBoxLayout();
	_rootPathEdit = new QLineEdit(pathGroup);
	_browseRootBtn = new QPushButton(tr("Browse..."), pathGroup);
	rootPathLayout->addWidget(_rootPathEdit);
	rootPathLayout->addWidget(_browseRootBtn);
	formLayout->addRow(tr("Root Search Path:"), rootPathLayout);

	// Type/DFN Sub Directory
	_typeDfnSubDirEdit = new QLineEdit(pathGroup);
	formLayout->addRow(tr("Type/DFN Sub Directory:"), _typeDfnSubDirEdit);

	// Default Type
	_defaultTypeEdit = new QLineEdit(pathGroup);
	formLayout->addRow(tr("Default Type:"), _defaultTypeEdit);

	// Default DFN
	_defaultDfnEdit = new QLineEdit(pathGroup);
	formLayout->addRow(tr("Default DFN:"), _defaultDfnEdit);

	mainLayout->addWidget(pathGroup);

	QGroupBox *optGroup = new QGroupBox(tr("Options"), this);
	QFormLayout *optLayout = new QFormLayout(optGroup);

	// Remember List Size
	_rememberListSpin = new QSpinBox(optGroup);
	_rememberListSpin->setRange(1, 100);
	_rememberListSpin->setValue(10);
	optLayout->addRow(tr("Remember List Size:"), _rememberListSpin);

	// Max Undo
	_maxUndoSpin = new QSpinBox(optGroup);
	_maxUndoSpin->setRange(1, 1000);
	_maxUndoSpin->setValue(50);
	optLayout->addRow(tr("Max Undo:"), _maxUndoSpin);

	// Start Expanded
	_startExpandedCheck = new QCheckBox(tr("Start Expanded"), optGroup);
	_startExpandedCheck->setChecked(true);
	optLayout->addRow("", _startExpandedCheck);

	// Remember Window State
	_rememberWindowStateCheck = new QCheckBox(tr("Remember Window State"), optGroup);
	_rememberWindowStateCheck->setChecked(false);
	optLayout->addRow("", _rememberWindowStateCheck);

	mainLayout->addWidget(optGroup);

	// Button box
	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	mainLayout->addWidget(buttonBox);

	connect(_browseRootBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseRootPath);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void SettingsDialog::loadSettings()
{
	QSettings settings("NeL", "Georges Editor Qt");
	_rootPathEdit->setText(settings.value("rootSearchPath", "").toString());
	_typeDfnSubDirEdit->setText(settings.value("typeDfnSubDir", "").toString());
	_defaultTypeEdit->setText(settings.value("defaultType", "").toString());
	_defaultDfnEdit->setText(settings.value("defaultDfn", "").toString());
	_rememberListSpin->setValue(settings.value("rememberListSize", 10).toInt());
	_maxUndoSpin->setValue(settings.value("maxUndo", 50).toInt());
	_startExpandedCheck->setChecked(settings.value("startExpanded", true).toBool());
	_rememberWindowStateCheck->setChecked(settings.value("rememberWindowState", false).toBool());
}

void SettingsDialog::saveSettings()
{
	QSettings settings("NeL", "Georges Editor Qt");
	settings.setValue("rootSearchPath", _rootPathEdit->text());
	settings.setValue("typeDfnSubDir", _typeDfnSubDirEdit->text());
	settings.setValue("defaultType", _defaultTypeEdit->text());
	settings.setValue("defaultDfn", _defaultDfnEdit->text());
	settings.setValue("rememberListSize", _rememberListSpin->value());
	settings.setValue("maxUndo", _maxUndoSpin->value());
	settings.setValue("startExpanded", _startExpandedCheck->isChecked());
	settings.setValue("rememberWindowState", _rememberWindowStateCheck->isChecked());
}

QString SettingsDialog::rootSearchPath() const
{
	return _rootPathEdit->text();
}

QString SettingsDialog::typeDfnSubDir() const
{
	return _typeDfnSubDirEdit->text();
}

QString SettingsDialog::defaultType() const
{
	return _defaultTypeEdit->text();
}

QString SettingsDialog::defaultDfn() const
{
	return _defaultDfnEdit->text();
}

int SettingsDialog::rememberListSize() const
{
	return _rememberListSpin->value();
}

int SettingsDialog::maxUndo() const
{
	return _maxUndoSpin->value();
}

bool SettingsDialog::startExpanded() const
{
	return _startExpandedCheck->isChecked();
}

bool SettingsDialog::rememberWindowState() const
{
	return _rememberWindowStateCheck->isChecked();
}

void SettingsDialog::onBrowseRootPath()
{
	QString dir = QFileDialog::getExistingDirectory(this,
		tr("Select Root Search Path"),
		_rootPathEdit->text());
	if (!dir.isEmpty())
		_rootPathEdit->setText(dir);
}

void SettingsDialog::onAccept()
{
	accept();
}

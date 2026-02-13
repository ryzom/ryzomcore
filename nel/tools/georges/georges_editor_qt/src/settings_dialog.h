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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

/**
 * Settings dialog, ported from CSettingsDialog.
 */
class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(QWidget *parent = nullptr);
	~SettingsDialog();

	QString rootSearchPath() const;
	QString typeDfnSubDir() const;
	QString defaultType() const;
	QString defaultDfn() const;
	int rememberListSize() const;
	int maxUndo() const;
	bool startExpanded() const;
	bool rememberWindowState() const;

	void loadSettings();
	void saveSettings();

private slots:
	void onBrowseRootPath();
	void onAccept();

private:
	void setupUi();

	QLineEdit *_rootPathEdit;
	QPushButton *_browseRootBtn;
	QLineEdit *_typeDfnSubDirEdit;
	QLineEdit *_defaultTypeEdit;
	QLineEdit *_defaultDfnEdit;
	QSpinBox *_rememberListSpin;
	QSpinBox *_maxUndoSpin;
	QCheckBox *_startExpandedCheck;
	QCheckBox *_rememberWindowStateCheck;
};

#endif // SETTINGS_DIALOG_H

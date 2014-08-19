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

#ifndef SHEETBUILDERDIALOG_H
#define SHEETBUILDERDIALOG_H

#include <QDialog>

class QCheckBox;
class QTextEdit;

class SheetBuilderDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SheetBuilderDialog(QWidget *parent = 0);
	~SheetBuilderDialog() {}

private Q_SLOTS:
	void buildSheet();
	void detailsShowHide();
	void showConfig();

private:
	void displayInfo(QString str);

	int defHeight;
	int defWidth;
	bool detailsVisible;
	QCheckBox *chckClean;
	QTextEdit *txtOutput;
};

#endif // SHEETBUILDERDIALOG_H

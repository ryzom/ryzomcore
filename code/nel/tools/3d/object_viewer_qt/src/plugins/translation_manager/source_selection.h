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

#ifndef SOURCE_SELECTION_H
#define	SOURCE_SELECTION_H

#include "ui_source_selection.h"

#include <QtCore/QObject>
#include <QtGui/QDialog>
#include <QtCore/QString>
#include <QtGui/QListWidgetItem>

#include <map>

namespace TranslationManager
{

class CSourceDialog : public QDialog
{
	Q_OBJECT

public:
	CSourceDialog(QWidget *parent = 0);
	~CSourceDialog() {}
	void setSourceOptions(std::map<QListWidgetItem *, int> &options);
	QListWidgetItem *selected_item;

private Q_SLOTS:
	void OkButtonClicked();
	void itemDoubleClicked(QListWidgetItem *item);

private:
	Ui::SourceSelectionDialog _ui;
};

}


#endif	/* SOURCE_SELECTION_H */


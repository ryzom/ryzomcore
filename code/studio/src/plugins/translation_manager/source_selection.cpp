// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "source_selection.h"

#include <QtGui/QListWidget>

namespace TranslationManager
{

CSourceDialog::CSourceDialog(QWidget *parent): QDialog(parent)
{
	_ui.setupUi(this);

	connect(_ui.ok_button, SIGNAL(clicked()), this, SLOT(OkButtonClicked()));
	connect(_ui.cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

	_ui.sourceSelectionListWidget->setSortingEnabled(false);
	connect(_ui.sourceSelectionListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
			this, SLOT(itemDoubleClicked(QListWidgetItem *)));
}

// Insert options in the source dialog. Options like: from FTP Server, from Local directory etc.
void CSourceDialog::setSourceOptions(std::map<QListWidgetItem *, int> &options)
{
	std::map<QListWidgetItem *,int>::iterator it;

	for(it = options.begin(); it != options.end(); ++it)
	{
		_ui.sourceSelectionListWidget->addItem((*it).first);
	}
}

void CSourceDialog::OkButtonClicked()
{
	selected_item = _ui.sourceSelectionListWidget->currentItem();
	accept();
}

void CSourceDialog::itemDoubleClicked(QListWidgetItem *item)
{
	selected_item = item;
	accept();
}

}
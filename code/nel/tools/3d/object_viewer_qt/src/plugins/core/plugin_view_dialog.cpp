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

#include "plugin_view_dialog.h"

// Qt includes
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QIcon>
#include <QtGui/QStyle>
#include <QtGui/QTreeWidgetItem>

// Project includes
#include "../../extension_system/iplugin_spec.h"
#include "../../extension_system/iplugin_manager.h"

namespace ExtensionSystem
{

CPluginView::CPluginView(IPluginManager *pluginManager, QWidget *parent)
	: QDialog(parent)
{
	_ui.setupUi(this);
	_pluginManager = pluginManager;

	connect(_pluginManager, SIGNAL(pluginsChanged()), this, SLOT(updateList()));

	updateList();
}

CPluginView::~CPluginView()
{
}

void CPluginView::updateList()
{
	static QIcon okIcon = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
	static QIcon errorIcon = QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);

	QList<QTreeWidgetItem *> items;
	Q_FOREACH (IPluginSpec *spec, _pluginManager->plugins())
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList()
				<< ""
				<< spec->name()
				<< QString("%1").arg(spec->version())
				<< spec->vendor()
				<< QDir::toNativeSeparators(spec->filePath()));
		item->setIcon(0, spec->hasError() ? errorIcon : okIcon);
		items.append(item);
	}

	_ui.pluginTreeWidget->clear();
	if (!items.isEmpty())
		_ui.pluginTreeWidget->addTopLevelItems(items);
}

} /* namespace NLQT */
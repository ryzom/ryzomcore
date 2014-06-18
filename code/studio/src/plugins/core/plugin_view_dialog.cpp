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
#include "core_constants.h"

#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QDir>
#include <QtGui/QIcon>
#include <QtGui/QStyle>
#include <QtGui/QTreeWidgetItem>
#include <QMessageBox>
#include <QFileDialog>

// Project includes
#include "../../extension_system/iplugin_spec.h"
#include "../../extension_system/iplugin_manager.h"

namespace Core
{

PluginView::PluginView(ExtensionSystem::IPluginManager *pluginManager, QWidget *parent)
	: QDialog(parent),
	  m_checkStateColumn(0)
{
	m_ui.setupUi(this);
	m_pluginManager = pluginManager;

	connect(m_pluginManager, SIGNAL(pluginsChanged()), this, SLOT(updateList()));
	connect(this, SIGNAL(accepted()), this, SLOT(updateSettings()));

	connect( m_ui.pluginTreeWidget, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ), this, SLOT( onItemClicked() ) );
	connect( m_ui.unloadButton, SIGNAL( clicked( bool ) ), this, SLOT( onUnloadClicked() ) );
	connect( m_ui.loadButton, SIGNAL( clicked( bool ) ), this, SLOT( onLoadClicked() ) );

	// WhiteList is list of plugins which can not disable.
	m_whiteList << Constants::OVQT_CORE_PLUGIN;
	updateList();
}

PluginView::~PluginView()
{
}

void PluginView::updateList()
{
	static QIcon okIcon = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
	static QIcon errorIcon = QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
	static QIcon notLoadedIcon = QApplication::style()->standardIcon(QStyle::SP_DialogResetButton);

	m_specToItem.clear();
	m_itemToSpec.clear();

	QList<QTreeWidgetItem *> items;
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, m_pluginManager->plugins())
	{
		QTreeWidgetItem *item = new QTreeWidgetItem(QStringList()
				<< spec->name()
				<< QString("%1").arg(spec->version())
				<< spec->vendor()
				<< QDir::toNativeSeparators(spec->filePath()));

		bool ok = !spec->hasError();
		QIcon icon = ok ? okIcon : errorIcon;
		if (ok && (spec->state() != ExtensionSystem::State::Running))
			icon = notLoadedIcon;

		item->setIcon(m_checkStateColumn, icon);

		if (!m_whiteList.contains(spec->name()))
			item->setCheckState(m_checkStateColumn, spec->isEnabled() ? Qt::Checked : Qt::Unchecked);

		items.append(item);
		m_specToItem.insert(spec, item);
		m_itemToSpec.insert(item, spec);
	}

	m_ui.pluginTreeWidget->clear();
	if (!items.isEmpty())
		m_ui.pluginTreeWidget->addTopLevelItems(items);

	m_ui.pluginTreeWidget->resizeColumnToContents(m_checkStateColumn);
}

void PluginView::updateSettings()
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, m_pluginManager->plugins())
	{
		if (m_specToItem.contains(spec) && (!m_whiteList.contains(spec->name())))
		{
			QTreeWidgetItem *item = m_specToItem.value(spec);
			if (item->checkState(m_checkStateColumn) == Qt::Checked)
				spec->setEnabled(true);
			else
				spec->setEnabled(false);
		}
	}
}

void PluginView::onItemClicked()
{
	m_ui.unloadButton->setEnabled( true );
}

void PluginView::onUnloadClicked()
{
	QTreeWidgetItem *item = m_ui.pluginTreeWidget->currentItem();
	if( item == NULL )
	{
		QMessageBox::warning( this,
								tr( "Plugin unload" ),
								tr( "No plugin selected!" ) );
		return;
	}

	QMap< QTreeWidgetItem*, ExtensionSystem::IPluginSpec* >::const_iterator itr
		= m_itemToSpec.find( item );
	if( itr == m_itemToSpec.end() )
		return;

	bool success = m_pluginManager->unloadPlugin( itr.value() );
	if( !success )
	{
		QMessageBox::warning( this,
								tr( "Plugin unload" ),
								tr( "Failed to unload plugin." ) );
	}
}

void PluginView::onLoadClicked()
{
	QString f = QFileDialog::getOpenFileName( this,
												tr( "Loading a plugin" ),
												".",
												"Plugin specifications ( *.xml )" );

	if( f.isEmpty() )
	{
		return;
	}

	QApplication::setOverrideCursor( Qt::WaitCursor );
	bool success = m_pluginManager->loadPlugin( f );
	QApplication::setOverrideCursor( Qt::ArrowCursor );

	if( !success )
	{
		QMessageBox::warning( this,
								tr( "Loading plugin" ),
								tr( "Error loading plugin!" ) );
	}
}


} /* namespace Core */


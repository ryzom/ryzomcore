// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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
#include "log_settings_page.h"
#include "log_plugin.h"
#include "../core/core_constants.h"
#include "../core/icore.h"
#include "../../extension_system/plugin_manager.h"

// NeL includes

// Qt includes
#include <QtGui/QWidget>
#include <QtCore/QSettings>

namespace ExtensionSystem
{
	class IPluginManager;
}

namespace Plugin
{

	class CLogPlugin;

	CLogSettingsPage::CLogSettingsPage(CLogPlugin *logPlugin, QObject *parent)
		: IOptionsPage(parent),
		m_logPlugin(logPlugin),
		m_currentPage(NULL),
		m_error(true),
		m_warning(true),
		m_debug(true),
		m_assert(true),
		m_info(true)
	{
	}

	QString CLogSettingsPage::id() const
	{
		return QLatin1String("log");
	}

	QString CLogSettingsPage::trName() const
	{
		return tr("Log");
	}

	QString CLogSettingsPage::category() const
	{
		return QLatin1String(Core::Constants::SETTINGS_CATEGORY_GENERAL);
	}

	QString CLogSettingsPage::trCategory() const
	{
		return tr(Core::Constants::SETTINGS_TR_CATEGORY_GENERAL);
	}

	QIcon CLogSettingsPage::categoryIcon() const
	{
		return QIcon();
	}

	QWidget *CLogSettingsPage::createPage(QWidget *parent)
	{
		m_currentPage = new QWidget(parent);
		m_ui.setupUi(m_currentPage);

		readSettings();
		m_ui.errorCheck->setChecked(m_error);
		m_ui.warningCheck->setChecked(m_warning);
		m_ui.debugCheck->setChecked(m_debug);
		m_ui.assertCheck->setChecked(m_assert);
		m_ui.infoCheck->setChecked(m_info);

		return m_currentPage;
	}

	void CLogSettingsPage::apply()
	{
		m_error = m_ui.errorCheck->isChecked();
		m_warning = m_ui.warningCheck->isChecked();
		m_debug = m_ui.debugCheck->isChecked();
		m_assert = m_ui.assertCheck->isChecked();
		m_info = m_ui.infoCheck->isChecked();

		writeSettings();
		m_logPlugin->setDisplayers();
	}

	void CLogSettingsPage::readSettings()
	{
		QSettings *settings = Core::ICore::instance()->settings();

		settings->beginGroup(Core::Constants::LOG_SECTION);
		m_error = settings->value(Core::Constants::LOG_ERROR,     true).toBool();
		m_warning = settings->value(Core::Constants::LOG_WARNING, true).toBool();
		m_debug = settings->value(Core::Constants::LOG_DEBUG,     true).toBool();
		m_assert = settings->value(Core::Constants::LOG_ASSERT,   true).toBool();
		m_info = settings->value(Core::Constants::LOG_INFO,       true).toBool();
		settings->endGroup();
	}

	void CLogSettingsPage::writeSettings()
	{
		QSettings *settings = Core::ICore::instance()->settings();

		settings->beginGroup(Core::Constants::LOG_SECTION);
		settings->setValue(Core::Constants::LOG_ERROR,   m_error);
		settings->setValue(Core::Constants::LOG_WARNING, m_warning);
		settings->setValue(Core::Constants::LOG_DEBUG,   m_debug);
		settings->setValue(Core::Constants::LOG_ASSERT,  m_assert);
		settings->setValue(Core::Constants::LOG_INFO,    m_info);
		settings->endGroup();

		settings->sync();
	}

} /* namespace Plugin */
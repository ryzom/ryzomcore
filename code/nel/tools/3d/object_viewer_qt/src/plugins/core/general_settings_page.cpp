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

// Project includes
#include "general_settings_page.h"
#include "core_constants.h"
#include "icore.h"

// NeL includes
#include <nel/misc/path.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QStyleFactory>
#include <QtGui/QStyle>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace Core
{

GeneralSettingsPage::GeneralSettingsPage(QObject *parent)
	: IOptionsPage(parent),
	  m_page(0)
{
	m_originalPalette = QApplication::palette();
}

GeneralSettingsPage::~GeneralSettingsPage()
{
}

QString GeneralSettingsPage::id() const
{
	return QLatin1String("general_settings");
}

QString GeneralSettingsPage::trName() const
{
	return tr("General");
}

QString GeneralSettingsPage::category() const
{
	return QLatin1String(Constants::SETTINGS_CATEGORY_GENERAL);
}

QString GeneralSettingsPage::trCategory() const
{
	return tr(Constants::SETTINGS_TR_CATEGORY_GENERAL);
}

QIcon GeneralSettingsPage::categoryIcon() const
{
	return QIcon();
}

void GeneralSettingsPage::applyGeneralSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();

	settings->beginGroup(Constants::MAIN_WINDOW_SECTION);
	QApplication::setStyle(QStyleFactory::create(settings->value(Constants::QT_STYLE, "").toString()));

	if (settings->value(Constants::QT_PALETTE, true).toBool())
		QApplication::setPalette(QApplication::style()->standardPalette());
	else
		QApplication::setPalette(m_originalPalette);
	settings->endGroup();

	QString levelDesignPrefix;
#if defined(_DEBUG) && defined(NL_OS_WINDOWS)
	levelDesignPrefix = "l:";
#else
	levelDesignPrefix = QString("%1/data_leveldesign").arg(RYZOM_SHARE_PREFIX);
#endif

	// Add primitives path and ligo config file to CPath
	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	QString primitivePath = settings->value(Core::Constants::PRIMITIVES_PATH, QString("%1/primitives").arg(levelDesignPrefix)).toString();
	QString ligoConfigFile = settings->value(Core::Constants::LIGOCONFIG_FILE, QString("%1/leveldesign/world_editor_files/world_editor_classes.xml").arg(levelDesignPrefix)).toString();
	QString leveldesignPath = settings->value(Core::Constants::LEVELDESIGN_PATH, QString("%1/leveldesign").arg(levelDesignPrefix)).toString();
	NLMISC::CPath::addSearchPath(primitivePath.toUtf8().constData(), true, false);
	NLMISC::CPath::display();
	NLMISC::CPath::addSearchFile(ligoConfigFile.toUtf8().constData());
	NLMISC::CPath::addSearchPath(leveldesignPath.toUtf8().constData(), true, false);
	settings->endGroup();
}

QWidget *GeneralSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	readSettings();
	connect(m_ui.languageComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeLanguage(QString)));
	connect(m_ui.pluginsPathButton, SIGNAL(clicked()), this, SLOT(setPluginsPath()));
	connect(m_ui.leveldesignPathButton, SIGNAL(clicked()), this, SLOT(setLevelDesignPath()));
	connect(m_ui.assetsPathButton, SIGNAL(clicked()), this, SLOT(setAssetsPath()));
	connect(m_ui.primitivesPathButton, SIGNAL(clicked()), this, SLOT(setPrimitivesPath()));
	connect(m_ui.ligoConfigFileButton, SIGNAL(clicked()), this, SLOT(setLigoConfigFile()));
	return m_page;
}

void GeneralSettingsPage::apply()
{
	writeSettings();
	applyGeneralSettings();
}

void GeneralSettingsPage::finish()
{
	delete m_page;
	m_page = 0;
}

void GeneralSettingsPage::changeLanguage(const QString &lang)
{
	QMessageBox::information(0, tr("Restart required"),
							 tr("The language change will take effect after a restart of Object Viewer Qt."));
}

void GeneralSettingsPage::setPluginsPath()
{
	QString newPath = QFileDialog::getExistingDirectory(0, tr("Set the plugins path"),
					  m_ui.pluginsPathLineEdit->text());
	if (!newPath.isEmpty())
	{
		m_ui.pluginsPathLineEdit->setText(newPath);
	}
}

void GeneralSettingsPage::setLevelDesignPath()
{
	QString newPath = QFileDialog::getExistingDirectory(0, tr("Set the level design path"),
					  m_ui.leveldesignPathLineEdit->text());
	if (!newPath.isEmpty())
	{
		m_ui.leveldesignPathLineEdit->setText(newPath);
	}
}

void GeneralSettingsPage::setPrimitivesPath()
{
	QString newPath = QFileDialog::getExistingDirectory(0, tr("Set the primitives path"),
					  m_ui.primitivesPathLineEdit->text());
	if (!newPath.isEmpty())
	{
		m_ui.primitivesPathLineEdit->setText(newPath);
	}
}

void GeneralSettingsPage::setLigoConfigFile()
{
	QString newFile = QFileDialog::getOpenFileName(0, tr("Set the ligo config file"),
					  m_ui.ligoConfigFileLineEdit->text());
	if (!newFile.isEmpty())
	{
		m_ui.ligoConfigFileLineEdit->setText(newFile);
	}
}


void GeneralSettingsPage::setAssetsPath()
{
	QString newPath = QFileDialog::getExistingDirectory(0, tr("Set the assets path"),
					  m_ui.assetsPathLineEdit->text());
	if (!newPath.isEmpty())
	{
		m_ui.assetsPathLineEdit->setText(newPath);
	}
}

void GeneralSettingsPage::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();

	m_ui.pluginsPathLineEdit->setText(settings->value(Core::Constants::PLUGINS_PATH, "./plugins").toString());

	settings->beginGroup(Constants::MAIN_WINDOW_SECTION);
	m_ui.styleComboBox->addItems(QStyleFactory::keys());
	QString style = settings->value(Constants::QT_STYLE, "").toString();
	if (style == "")
		m_ui.styleComboBox->setCurrentIndex(0);
	else
		m_ui.styleComboBox->setCurrentIndex(m_ui.styleComboBox->findText(style));
	m_ui.paletteCheckBox->setChecked(settings->value(Constants::QT_PALETTE, true).toBool());
	settings->endGroup();

	QStringList paths;
	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	m_ui.leveldesignPathLineEdit->setText(settings->value(Core::Constants::LEVELDESIGN_PATH, "l:/leveldesign").toString());
	m_ui.assetsPathLineEdit->setText(settings->value(Core::Constants::ASSETS_PATH, "w:/database").toString());
	m_ui.primitivesPathLineEdit->setText(settings->value(Core::Constants::PRIMITIVES_PATH, "l:/primitives").toString());
	m_ui.ligoConfigFileLineEdit->setText(settings->value(Core::Constants::LIGOCONFIG_FILE, "l:/leveldesign/world_editor_files/world_editor_classes.xml").toString());
	settings->endGroup();
}

void GeneralSettingsPage::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();

	settings->setValue(Core::Constants::PLUGINS_PATH, m_ui.pluginsPathLineEdit->text());

	settings->beginGroup(Constants::MAIN_WINDOW_SECTION);
	if (m_ui.styleComboBox->currentIndex() == 0)
		settings->setValue(Constants::QT_STYLE, "");
	else
		settings->setValue(Constants::QT_STYLE, m_ui.styleComboBox->currentText());
	settings->setValue(Constants::QT_PALETTE, m_ui.paletteCheckBox->isChecked());
	settings->endGroup();

	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	settings->setValue(Core::Constants::LEVELDESIGN_PATH, m_ui.leveldesignPathLineEdit->text());
	settings->setValue(Core::Constants::ASSETS_PATH, m_ui.assetsPathLineEdit->text());
	settings->setValue(Core::Constants::PRIMITIVES_PATH, m_ui.primitivesPathLineEdit->text());
	settings->setValue(Core::Constants::LIGOCONFIG_FILE, m_ui.ligoConfigFileLineEdit->text());
	settings->endGroup();
	settings->sync();
}

} /* namespace Core */
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
#include "stdpch.h"
#include "graphics_settings_page.h"
#include "modules.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QStyleFactory>
#include <QtGui/QStyle>

// NeL includes
#include <nel/3d/bloom_effect.h>

namespace NLQT
{

GraphicsSettingsPage::GraphicsSettingsPage(QObject *parent)
	: IOptionsPage(parent),
	  m_page(0)
{
}

QString GraphicsSettingsPage::id() const
{
	return QLatin1String("GraphicsPage");
}

QString GraphicsSettingsPage::trName() const
{
	return tr("Graphics");
}

QString GraphicsSettingsPage::category() const
{
	return QLatin1String("ObjectViewer");
}

QString GraphicsSettingsPage::trCategory() const
{
	return tr("Object Viewer");
}

QWidget *GraphicsSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	Modules::config().setAndCallback("GraphicsDrivers", CConfigCallback(this, &GraphicsSettingsPage::cfcbGraphicsDrivers));
	m_ui.enableBloomCheckBox->setChecked(Modules::objView().getBloomEffect());
	m_ui.squareBloomCheckBox->setChecked(NL3D::CBloomEffect::instance().getSquareBloom());
	m_ui.bloomDensityHorizontalSlider->setValue(NL3D::CBloomEffect::instance().getDensityBloom());
	m_ui.styleComboBox->addItems(QStyleFactory::keys());
	m_ui.styleComboBox->setCurrentIndex(m_ui.styleComboBox->findText(Modules::config().getValue("QtStyle", std::string("")).c_str()));
	m_ui.paletteCheckBox->setChecked(Modules::config().getValue("QtPalette", false));

	connect(m_ui.enableBloomCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnableBloom(bool)));
	connect(m_ui.squareBloomCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnableSquareBloon(bool)));
	connect(m_ui.bloomDensityHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setDensityBloom(int)));

#ifdef NL_OS_UNIX
	m_ui.driverGraphComboBox->setEnabled(false);
#endif
	return m_page;
}

void GraphicsSettingsPage::apply()
{
	// save graphics settings to config file
	Modules::config().getConfigFile().getVar("GraphicsDriver").setAsString(m_ui.driverGraphComboBox->currentText().toStdString());

	Modules::config().getConfigFile().getVar("BloomEffect").setAsInt(m_ui.enableBloomCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("BloomSquare").setAsInt(m_ui.squareBloomCheckBox->isChecked());
	Modules::config().getConfigFile().getVar("BloomDensity").setAsInt(m_ui.bloomDensityHorizontalSlider->value());

	Modules::config().getConfigFile().getVar("QtStyle").setAsString(m_ui.styleComboBox->currentText().toStdString());
	Modules::config().getConfigFile().getVar("QtPalette").setAsInt(m_ui.paletteCheckBox->isChecked());

	QApplication::setStyle(QStyleFactory::create(m_ui.styleComboBox->currentText()));

	if (m_ui.paletteCheckBox->isChecked())
		QApplication::setPalette(QApplication::style()->standardPalette());
	else
		QApplication::setPalette(Modules::mainWin().getOriginalPalette());

	// save config file
	Modules::config().getConfigFile().save();
}
void GraphicsSettingsPage::finish()
{
	Modules::config().dropCallback("GraphicsDrivers");
}

void GraphicsSettingsPage::setEnableBloom(bool state)
{
	Modules::objView().setBloomEffect(state);
}

void GraphicsSettingsPage::setEnableSquareBloon(bool state)
{
	NL3D::CBloomEffect::instance().setSquareBloom(state);
}

void GraphicsSettingsPage::setDensityBloom(int density)
{
	NL3D::CBloomEffect::instance().setDensityBloom(density);
}

void GraphicsSettingsPage::cfcbGraphicsDrivers(NLMISC::CConfigFile::CVar &var)
{
	while (m_ui.driverGraphComboBox->count())
		m_ui.driverGraphComboBox->removeItem(0);

	// load types graphics driver from the config file
	for (uint i = 0; i < var.size(); ++i)
		m_ui.driverGraphComboBox->addItem(var.asString(i).c_str());

	// set graphics driver from the config file
	QString value = Modules::config().getValue("GraphicsDriver", std::string("OpenGL")).c_str();
	QString dn = value.toLower();
	for (sint i = 0; i < m_ui.driverGraphComboBox->count(); ++i)
	{
		if (dn == m_ui.driverGraphComboBox->itemText(i).toLower())
		{
			m_ui.driverGraphComboBox->setCurrentIndex(i);
			return;
		}
	}
}

} /* namespace NLQT */
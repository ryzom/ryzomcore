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
#include "object_viewer_constants.h"
#include "../core/icore.h"
#include "modules.h"

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QWidget>

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
	return QLatin1String(Constants::OBJECT_VIEWER_SECTION);
}

QString GraphicsSettingsPage::trCategory() const
{
	return tr("Object Viewer");
}

QIcon GraphicsSettingsPage::categoryIcon() const
{
	return QIcon();
}

QWidget *GraphicsSettingsPage::createPage(QWidget *parent)
{
	m_page = new QWidget(parent);
	m_ui.setupUi(m_page);

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	QString graphicsDriver = settings->value(Constants::GRAPHICS_DRIVER, "OpenGL").toString();
	m_ui.driverGraphComboBox->setCurrentIndex(m_ui.driverGraphComboBox->findText(graphicsDriver));

	m_ui.enableBloomCheckBox->setChecked(settings->value(Constants::ENABLE_BLOOM, false).toBool());
	m_ui.squareBloomCheckBox->setChecked(NL3D::CBloomEffect::instance().getSquareBloom());
	m_ui.bloomDensityHorizontalSlider->setValue(NL3D::CBloomEffect::instance().getDensityBloom());

	settings->endGroup();

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
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	settings->setValue(Constants::GRAPHICS_DRIVER, m_ui.driverGraphComboBox->currentText());

	// save settings
	settings->setValue(Constants::ENABLE_BLOOM, m_ui.enableBloomCheckBox->isChecked());
	settings->setValue(Constants::ENABLE_SQUARE_BLOOM, m_ui.squareBloomCheckBox->isChecked());
	settings->setValue(Constants::BLOOM_DENSITY, m_ui.bloomDensityHorizontalSlider->value());

	settings->endGroup();
	settings->sync();
}

void GraphicsSettingsPage::finish()
{
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

} /* namespace NLQT */
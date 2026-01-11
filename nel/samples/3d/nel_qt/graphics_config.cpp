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

#include <nel/misc/types_nl.h>
#include "graphics_config.h"

// STL includes

// Qt includes
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QUndoStack>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/i18n.h>

// Project includes
#include "main_window.h"
#include "configuration.h"
#include "internationalization.h"
#include "undo_redo_binders.h"

using namespace std;
using namespace NLMISC;

namespace NLQT {

namespace {

QString nli18n(const char *label)
{
	return QString::fromUtf16(CI18N::get(label).c_str());
}

} /* anonymous namespace */

CGraphicsConfig::CGraphicsConfig(QWidget *parent, CConfiguration *configuration, CInternationalization *internationalization, QUndoStack *undoStack) 
	: QWidget(parent), m_Configuration(configuration), m_Internationalization(internationalization), m_UndoStack(undoStack), 
	m_DriverGroup(NULL), m_Enabled(NULL), m_DriverLabel(NULL), m_Driver(NULL),
	m_FontNameLabel(NULL), m_FontName(NULL), m_Apply(NULL),
	m_RenderGroup(NULL), m_BackgroundColor(NULL), m_FontShadow(NULL), 
	m_ScreenshotGroup(NULL)
{
	nlassert(m_Configuration);
	nlassert(m_Internationalization);
	nlassert(m_UndoStack);
	
	QVBoxLayout *vboxLayout = new QVBoxLayout();

	// Driver
	{
		nlassert(!m_DriverGroup);
		m_DriverGroup = new QGroupBox();
		QVBoxLayout *groupVboxLayout = new QVBoxLayout();

		// Enabled
		{
			nlassert(!m_Enabled);
			m_Enabled = new QCheckBox();
			groupVboxLayout->addWidget(m_Enabled);
		}

		// Driver
		{
			QHBoxLayout *hboxLayout = new QHBoxLayout();
			nlassert(!m_DriverLabel);
			m_DriverLabel = new QLabel();
			nlassert(!m_Driver);
			m_Driver = new QComboBox();
			m_DriverLabel->setBuddy(m_Driver);
			hboxLayout->addWidget(m_DriverLabel);
			hboxLayout->addWidget(m_Driver);
			hboxLayout->setStretch(1, 1);
			groupVboxLayout->addLayout(hboxLayout);
		}
		
		// Font Name
		{
			QHBoxLayout *hboxLayout = new QHBoxLayout();
			nlassert(!m_FontNameLabel);
			m_FontNameLabel = new QLabel();
			nlassert(!m_FontName);
			m_FontName = new QLineEdit();
			m_FontNameLabel->setBuddy(m_FontName);
			hboxLayout->addWidget(m_FontNameLabel);
			hboxLayout->addWidget(m_FontName);
			hboxLayout->setStretch(1, 1);
			groupVboxLayout->addLayout(hboxLayout);
		}

		// Apply
		{
			nlassert(!m_Apply);
			m_Apply = new QPushButton();
			m_Apply->setDefault(true);
			groupVboxLayout->addWidget(m_Apply);
			connect(m_Apply, SIGNAL(pressed()), this, SLOT(applyPressed()));
		}
		
		m_DriverGroup->setLayout(groupVboxLayout);
		vboxLayout->addWidget(m_DriverGroup);
	}

	// Render
	{
		nlassert(!m_RenderGroup);
		m_RenderGroup = new QGroupBox();
		QVBoxLayout *groupVboxLayout = new QVBoxLayout();

		// Background Color
		{
			m_BackgroundColor = new QtColorPicker();
			m_BackgroundColor->setStandardColors();
			groupVboxLayout->addWidget(m_BackgroundColor);
			connect(m_BackgroundColor, SIGNAL(colorChanged(const QColor &)), this, SLOT(uicbBackgroundColor(const QColor &)));
		}
				
		// Font Shadow
		{
			nlassert(!m_FontShadow);
			m_FontShadow = new QCheckBox();
			groupVboxLayout->addWidget(m_FontShadow);
			connect(m_FontShadow, SIGNAL(toggled(bool)), this, SLOT(uicbFontShadow(bool)));
		}
		
		m_RenderGroup->setLayout(groupVboxLayout);
		vboxLayout->addWidget(m_RenderGroup);
	}

	// Screenshots
	{
		nlassert(!m_ScreenshotGroup);
		m_ScreenshotGroup = new QGroupBox();
		QVBoxLayout *groupVboxLayout = new QVBoxLayout();
		
		m_ScreenshotGroup->setLayout(groupVboxLayout);
		vboxLayout->addWidget(m_ScreenshotGroup);		
	}

	vboxLayout->addStretch();
	setLayout(vboxLayout);
	
	// setup config file callbacks and initialize values
	m_Configuration->setAndCallback("GraphicsEnabled", CConfigCallback(this, &CGraphicsConfig::cfcbGraphicsEnabled));
	m_Configuration->setCallback("GraphicsDriver", CConfigCallback(this, &CGraphicsConfig::cfcbGraphicsDriver));
	m_Configuration->setAndCallback("GraphicsDrivers", CConfigCallback(this, &CGraphicsConfig::cfcbGraphicsDrivers));
	m_Configuration->setAndCallback("FontName", CConfigCallback(this, &CGraphicsConfig::cfcbFontName));
	m_Configuration->setAndCallback("BackgroundColor", CConfigCallback(this, &CGraphicsConfig::cfcbBackgroundColor));		
	m_Configuration->setAndCallback("FontShadow", CConfigCallback(this, &CGraphicsConfig::cfcbFontShadow));		
	
	// setup translation callback and initialize translation
	m_Internationalization->enableCallback(CEmptyCallback(this, &CGraphicsConfig::incbTranslate));
	incbTranslate();

	// setup undo/redo automation
	CUndoRedoBinderButton *undoRedoEnabled = new CUndoRedoBinderButton(m_Enabled, undoStack);
	CUndoRedoBinderComboBox *undoRedoDriver = new CUndoRedoBinderComboBox(m_Driver, undoStack);
	CUndoRedoBinderLineEdit *undoRedoFontName = new CUndoRedoBinderLineEdit(m_FontName, undoStack);
	CUndoRedoBinderColorPicker *undoRedoBackgroundColor = new CUndoRedoBinderColorPicker(m_BackgroundColor, undoStack);
	CUndoRedoBinderButton *undoRedoFontShadow = new CUndoRedoBinderButton(m_FontShadow, undoStack);
}

CGraphicsConfig::~CGraphicsConfig()
{
	m_Internationalization->disableCallback(CEmptyCallback(this, &CGraphicsConfig::incbTranslate));
	
	m_Configuration->dropCallback("FontShadow");
	m_Configuration->dropCallback("BackgroundColor");
	m_Configuration->dropCallback("FontName");
	m_Configuration->dropCallback("GraphicsDrivers");
	m_Configuration->dropCallback("GraphicsDriver");
	m_Configuration->dropCallback("GraphicsEnabled");
}

void CGraphicsConfig::incbTranslate()
{
	m_DriverGroup->setTitle(nli18n("GraphicsConfigDriverGroup"));
	m_Enabled->setText(nli18n("GraphicsConfigEnabled"));
	m_DriverLabel->setText(nli18n("GraphicsConfigDriver"));
	m_FontNameLabel->setText(nli18n("GraphicsConfigFontName"));
	m_Apply->setText(nli18n("GraphicsConfigApply"));

	m_RenderGroup->setTitle(nli18n("GraphicsConfigRenderGroup"));
	m_BackgroundColor->setText(nli18n("GraphicsConfigBackgroundColor"));
	m_FontShadow->setText(nli18n("GraphicsConfigFontShadow"));

	m_ScreenshotGroup->setTitle(nli18n("GraphicsConfigScreenshotGroup"));
}

void CGraphicsConfig::cfcbGraphicsEnabled(NLMISC::CConfigFile::CVar &var)
{
	m_Enabled->setChecked(var.asBool());
}

void CGraphicsConfig::cfcbGraphicsDrivers(NLMISC::CConfigFile::CVar &var)
{
	while (m_Driver->count())
		m_Driver->removeItem(0);
	for (uint i = 0; i < var.size(); ++i)
		m_Driver->addItem(var.asString(i).c_str());
	cfcbGraphicsDriver(m_Configuration->getConfigFile().getVar("GraphicsDriver"));
	m_UndoStack->clear();
}

void CGraphicsConfig::cfcbGraphicsDriver(NLMISC::CConfigFile::CVar &var)
{
	QString value = var.asString().c_str();
	QString dn = value.toLower();
	for (sint i = 0; i < m_Driver->count(); ++i)
	{
		if (dn == m_Driver->itemText(i).toLower())
		{
			m_Driver->setCurrentIndex(i);
			return;
		}
	}
	nlwarning("Unknown GraphicsDriver specified in config, skipping value.");
}

void CGraphicsConfig::cfcbFontName(NLMISC::CConfigFile::CVar &var)
{
	m_FontName->setText(var.asString().c_str());
}

void CGraphicsConfig::cfcbBackgroundColor(NLMISC::CConfigFile::CVar &var)
{
	m_BackgroundColor->setCurrentColor(QColor(var.asInt(0), var.asInt(1), var.asInt(2)));
	emit onBackgroundColor(getBackgroundColor());
}

void CGraphicsConfig::cfcbFontShadow(NLMISC::CConfigFile::CVar &var)
{
	m_FontShadow->setChecked(var.asBool());
	emit onFontShadow(getFontShadow());
}

void CGraphicsConfig::applyPressed()
{
	m_Configuration->getConfigFile().getVar("GraphicsEnabled").setAsInt(getGraphicsEnabled() ? 1 : 0);
	m_Configuration->getConfigFile().getVar("GraphicsDriver").setAsString(getGraphicsDriver());
	m_Configuration->getConfigFile().getVar("FontName").setAsString(getFontName());
	emit applyGraphicsConfig();
}

void CGraphicsConfig::uicbBackgroundColor(const QColor &backgroundColor)
{
	m_Configuration->getConfigFile().getVar("BackgroundColor").setAsInt(backgroundColor.red(), 0);
	m_Configuration->getConfigFile().getVar("BackgroundColor").setAsInt(backgroundColor.green(), 1);
	m_Configuration->getConfigFile().getVar("BackgroundColor").setAsInt(backgroundColor.blue(), 2);
	emit onBackgroundColor(getBackgroundColor());
}

void CGraphicsConfig::uicbFontShadow(bool checked)
{
	m_Configuration->getConfigFile().getVar("FontShadow").setAsInt(checked ? 1 : 0);
	emit onFontShadow(checked);
}

} /* namespace NLQT */

/* end of file */

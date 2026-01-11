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

#ifndef NLQT_GRAPHICS_CONFIG_H
#define NLQT_GRAPHICS_CONFIG_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/misc/rgba.h>

// Project includes
#include "qtcolorpicker.h"

class QUndoStack;

namespace NLQT {
	class CMainWindow;
	class CConfiguration;
	class CInternationalization;

/**
 * CGraphicsConfig
 * \brief CGraphicsConfig
 * \date 2010-02-07 10:15GMT
 * \author Jan Boon (Kaetemi)
 */
class CGraphicsConfig : public QWidget
{
	Q_OBJECT

public:
	CGraphicsConfig(QWidget *parent, CConfiguration *configuration, CInternationalization *internationalization, QUndoStack *undoStack);
	virtual ~CGraphicsConfig();
	
	void incbTranslate();

	inline bool getGraphicsEnabled() const { return m_Enabled->isChecked(); }
	inline std::string getGraphicsDriver() const { std::string v = std::string(m_Driver->currentText().toAscii()); return v; }
	inline std::string getFontName() const { std::string v = std::string(m_FontName->text().toAscii()); return v; }
	
	inline NLMISC::CRGBA getBackgroundColor() const { QColor c = m_BackgroundColor->currentColor(); NLMISC::CRGBA v(c.red(), c.green(), c.blue()); return v; }
	inline bool getFontShadow() const { return m_FontShadow->isChecked(); }

	inline std::string getScreenshotName() const { return "nel_qt"; }
	inline bool getScreenshotJPG() const { return true; }
	inline bool getScreenshotPNG() const { return true; }
	inline bool getScreenshotTGA() const { return false; }
	inline std::string getScreenshotPath() const { return "screenshots"; }

private slots:
	void applyPressed();
	void uicbBackgroundColor(const QColor &backgroundColor);
	void uicbFontShadow(bool checked);

signals:
	/// GraphicsEnabled, GraphicsDriver, FontName
	void applyGraphicsConfig();
	void onBackgroundColor(NLMISC::CRGBA backgroundColor);
	void onFontShadow(bool fontShadow);

private:
	void cfcbGraphicsEnabled(NLMISC::CConfigFile::CVar &var);
	void cfcbGraphicsDrivers(NLMISC::CConfigFile::CVar &var);
	void cfcbGraphicsDriver(NLMISC::CConfigFile::CVar &var);
	void cfcbFontName(NLMISC::CConfigFile::CVar &var);
	void cfcbBackgroundColor(NLMISC::CConfigFile::CVar &var);
	void cfcbFontShadow(NLMISC::CConfigFile::CVar &var);

private:
	CConfiguration *m_Configuration;
	CInternationalization *m_Internationalization;
	QUndoStack *m_UndoStack;

	QGroupBox *m_DriverGroup;
	QCheckBox *m_Enabled;
	QLabel *m_DriverLabel;
	QComboBox *m_Driver;
	QLabel *m_FontNameLabel;
	QLineEdit *m_FontName;
	QPushButton *m_Apply;

	QGroupBox *m_RenderGroup;
	QtColorPicker *m_BackgroundColor;
	QCheckBox *m_FontShadow;

	QGroupBox *m_ScreenshotGroup;

private:
	CGraphicsConfig(const CGraphicsConfig &);
	CGraphicsConfig &operator=(const CGraphicsConfig &);
	
}; /* class CGraphicsConfig */

} /* namespace NLQT */

#endif /* #ifndef NLQT_GRAPHICS_CONFIG_H */

/* end of file */

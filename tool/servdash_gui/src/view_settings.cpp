// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "view_settings.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPalette>
#include <QRegularExpression>
#include <QSettings>
#include <QStyleHints>

namespace {

/// Width of the shortened function name: three characters, an ellipsis, five characters.
const int s_FunctionWidth = 9;

QSettings settings()
{
	return QSettings("Ryzom Core", "servdash_gui");
}

} // namespace

CViewSettings &CViewSettings::instance()
{
	static CViewSettings s_Instance;
	return s_Instance;
}

CViewSettings::CViewSettings()
{
	QSettings store = settings();
	m_CompactPrefix = store.value("view/compactPrefix", true).toBool();
	m_Wrap = store.value("view/wrap", false).toBool();
	m_Timestamps = store.value("view/timestamps", true).toBool();
	m_Theme = (TTheme)qBound(0, store.value("view/theme", 0).toInt(), 2);
}

bool CViewSettings::themeSwitchAvailable()
{
	return true;
}

void CViewSettings::setCompactPrefix(bool compact)
{
	if (m_CompactPrefix == compact)
		return;
	m_CompactPrefix = compact;
	settings().setValue("view/compactPrefix", compact);
	emit changed();
}

void CViewSettings::setWrap(bool wrap)
{
	if (m_Wrap == wrap)
		return;
	m_Wrap = wrap;
	settings().setValue("view/wrap", wrap);
	emit changed();
}

void CViewSettings::setTimestamps(bool show)
{
	if (m_Timestamps == show)
		return;
	m_Timestamps = show;
	settings().setValue("view/timestamps", show);
	emit changed();
}

void CViewSettings::setTheme(TTheme theme)
{
	if (m_Theme == theme)
		return;
	m_Theme = theme;
	settings().setValue("view/theme", (int)theme);
	applyTheme();
	emit changed();
}

void CViewSettings::applyTheme()
{
	// The palette makes the theme, so it works without a platform theme (the offscreen
	// platform, a bare Linux session). The system palette is the one the application started
	// with. The colour scheme hint goes along where Qt has it, which also switches the
	// Windows title bar.
	static const QPalette s_SystemPalette = QApplication::palette();
	QPalette palette = s_SystemPalette;
	if (m_Theme == Light)
	{
		// the light Fusion palette by hand: the style's own follows the platform's colour scheme,
		// so on a system in dark mode it would be dark as well
		palette = QPalette(QColor(239, 239, 239));
		palette.setColor(QPalette::WindowText, Qt::black);
		palette.setColor(QPalette::Base, Qt::white);
		palette.setColor(QPalette::AlternateBase, QColor(247, 247, 247));
		palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
		palette.setColor(QPalette::ToolTipText, Qt::black);
		palette.setColor(QPalette::Text, Qt::black);
		palette.setColor(QPalette::ButtonText, Qt::black);
		palette.setColor(QPalette::BrightText, Qt::white);
		palette.setColor(QPalette::Link, QColor(0, 0, 255));
		palette.setColor(QPalette::Highlight, QColor(48, 140, 198));
		palette.setColor(QPalette::HighlightedText, Qt::white);
		palette.setColor(QPalette::PlaceholderText, QColor(128, 128, 128));
		palette.setColor(QPalette::Disabled, QPalette::Text, QColor(190, 190, 190));
		palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(190, 190, 190));
		palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(190, 190, 190));
	}
	else if (m_Theme == Dark)
	{
		// the usual dark Fusion palette; the shades of the frames and buttons follow from the window colour
		palette = QPalette(QColor(53, 53, 53));
		palette.setColor(QPalette::WindowText, Qt::white);
		palette.setColor(QPalette::Base, QColor(35, 35, 35));
		palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
		palette.setColor(QPalette::ToolTipBase, QColor(25, 25, 25));
		palette.setColor(QPalette::ToolTipText, Qt::white);
		palette.setColor(QPalette::Text, Qt::white);
		palette.setColor(QPalette::ButtonText, Qt::white);
		palette.setColor(QPalette::BrightText, Qt::red);
		palette.setColor(QPalette::Link, QColor(42, 130, 218));
		palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
		palette.setColor(QPalette::HighlightedText, Qt::white);
		palette.setColor(QPalette::PlaceholderText, QColor(140, 140, 140));
		palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
		palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
		palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
		palette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
	}
	QApplication::setPalette(palette);
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
	Qt::ColorScheme scheme = Qt::ColorScheme::Unknown;
	if (m_Theme == Light)
		scheme = Qt::ColorScheme::Light;
	else if (m_Theme == Dark)
		scheme = Qt::ColorScheme::Dark;
	QGuiApplication::styleHints()->setColorScheme(scheme);
#endif
}

QString CViewSettings::compactPrefix(const QString &line)
{
	// `INF  b8c  service.cpp  879 NLNET::IService::main: ` as CWindowDisplayer writes it, or
	// `b8c: ` as the command log writes it; the thread id is hex, the function has no spaces
	static const QRegularExpression s_Prefix("^(?:([A-Z]{3}) +)?[0-9a-fA-F]{1,8}(?: +\\S+ +\\d+(?: +(\\S+))?)?: ");
	QRegularExpressionMatch match = s_Prefix.match(line);
	if (!match.hasMatch())
		return line;
	const QString type = match.captured(1);
	const QString function = match.captured(2);
	const QString rest = line.mid(match.capturedLength());
	if (function.isEmpty())
		return rest;
	// the function keeps its first three and last five characters, in a fixed width so the
	// messages line up
	QString shortName = function;
	if (shortName.size() > s_FunctionWidth)
		shortName = function.left(3) + QChar(0x2026) + function.right(5);
	shortName = shortName.leftJustified(s_FunctionWidth);
	return (type.isEmpty() ? QString() : type + " ") + shortName + ": " + rest;
}

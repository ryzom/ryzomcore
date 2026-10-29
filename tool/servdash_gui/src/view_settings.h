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

#ifndef SERVDASH_VIEW_SETTINGS_H
#define SERVDASH_VIEW_SETTINGS_H

#include <QObject>
#include <QString>

/// How the logs are shown, kept across runs: whether the NeL log header (type, thread, file,
/// line, function) is hidden, whether long lines wrap, and the colour theme. One instance,
/// shared by every pane; a change is signalled so the panes redraw.
class CViewSettings : public QObject
{
	Q_OBJECT

public:
	enum TTheme
	{
		System = 0,
		Light = 1,
		Dark = 2
	};

	static CViewSettings &instance();

	bool hidePrefix() const { return m_HidePrefix; }
	bool wrap() const { return m_Wrap; }
	TTheme theme() const { return m_Theme; }
	/// Whether the theme can be chosen here; it always can, the palette makes it.
	static bool themeSwitchAvailable();

	void setHidePrefix(bool hide);
	void setWrap(bool wrap);
	void setTheme(TTheme theme);
	/// Applies the stored theme to the application; called once at startup.
	void applyTheme();

	/// A log line without the NeL header: `DBG 9ea8 file.cpp 162 CClass::method: ` or the
	/// `9ea8: ` of a command log line. Lines without one come back as they are.
	static QString stripPrefix(const QString &line);

signals:
	void changed();

private:
	CViewSettings();

	bool m_HidePrefix = false;
	bool m_Wrap = false;
	TTheme m_Theme = System;
};

#endif // SERVDASH_VIEW_SETTINGS_H

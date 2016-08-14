// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef PROFILE_H
#define PROFILE_H

#include "operation.h"

class CProfile
{
public:
	CProfile()
	{
		desktopShortcut = false;
		menuShortcut = false;
	}

	QString id;
	QString name;
	QString server;
	QString executable;
	QString arguments;
	QString comments;
	bool desktopShortcut;
	bool menuShortcut;

	// helpers
	QString getDirectory() const;
	QString getClientFullPath() const;
	QString getClientDesktopShortcutFullPath() const;
	QString getClientMenuShortcutFullPath() const;

	void createShortcuts() const;
	void deleteShortcuts() const;
	void updateShortcuts() const;
};

extern const CProfile NoProfile;

typedef QVector<CProfile> CProfiles;

#endif

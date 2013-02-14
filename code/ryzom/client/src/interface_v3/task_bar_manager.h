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



#ifndef NL_TASK_BAR_MANAGER_H
#define NL_TASK_BAR_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "dbctrl_sheet.h"
#include "nel/gui/group_container.h"


// ***************************************************************************
class	CMacroCmd;

// ***************************************************************************
#define	TBM_SHORTCUT_DB				"ui:interface:taskbar:shortcuts:shortcut_bar"
#define	TBM_EMPTY_SHORTCUT_DB		"UI:SHORTCUTS:EMPTY"
#define	TBM_NUM_BARS				10
#define	TBM_NUM_SHORTCUT_PER_BAR	10


// ***************************************************************************
/**
 * DEPRECATED. Still here for serial() compatibility (icfg)
 * Management of the TaskBar
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CTaskBarManager
{
public:

	static	CTaskBarManager *getInstance();

	// release singleton
	static void releaseInstance();

	// serial
	void	serial(NLMISC::IStream &f);

private:

	static	CTaskBarManager	*_Instance;

	struct	CShortcutInfo
	{
		CCtrlSheetInfo::TSheetType	SheetType;
		std::string					DBSheet;
		sint32						MacroId;

		CShortcutInfo()
		{
			MacroId= -1;
			SheetType= CCtrlSheetInfo::SheetType_SBrick;
		}

		void		serial(NLMISC::IStream &f);
	};

	/// Constructor
	CTaskBarManager();
};

// ***************************************************************************
/**
 * Management of the TaskBar (windows)
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date October 2003
 */
class CGroupContainerWindows : public CGroupContainer
{

public:
	CGroupContainerWindows(const TCtorParam &param)
		: CGroupContainer(param)
	{
		_ShowDesktops = true;
	}

	virtual bool wantSerialConfig() const { return true; }

	virtual void serialConfig(NLMISC::IStream &f);

	void update(bool updatePos=false);

	bool getShowDesktops () { return _ShowDesktops; }

	void setShowDesktops (bool newVal) { _ShowDesktops = newVal; update(true); }

private:
	bool _ShowDesktops;
};


#endif // NL_TASK_BAR_MANAGER_H

/* End of task_bar_manager.h */

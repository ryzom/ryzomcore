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



#ifndef NL_INTERFACE_CONFIG_H
#define NL_INTERFACE_CONFIG_H

#include "nel/misc/stream.h"

namespace NLGUI
{
	class CGroupContainer;
}

namespace NLMISC{
	class CCDBNodeLeaf;
}

/**
 * interface config
 * class used to managed an interface configuration
 * \author Matthieu 'TRAP' Besson
 * \author Nevrax France
 * \date 2003
 */
class CInterfaceConfig
{
public:
	// Elements saved from the container
	struct SCont
	{
		std::string		Id;
		uint8			ContainerMode; // 0 - Normal, 1 - Small (save just Opened, ActiveSavable and Active)
		bool			Popuped;
		bool			Opened;
		sint32			X;
		sint32			Y;
		sint32			W;
		sint32			H;
		sint32			MinW;
		sint32			MaxW;
		bool			ActiveSavable;
		bool			Active;
		sint32			ScrollPos;
		bool			Locked;
		bool			Movable;
		// alpha
		uint8			BgAlpha;
		uint8			ContentAlpha;
		uint8			RolloverAlphaContent;
		uint8			RolloverAlphaContainer;
		bool			UseGlobalAlpha;
		// popup state
		sint32          PopupX;
		sint32          PopupY;
		sint32          PopupW;
		sint32          PopupH;
		sint32			PopupMaxH;
		// backuped position
		bool			BackupedPositionValid;
		sint32			BackupX;
		sint32			BackupY;
		// touch flag
		bool			TouchFlag;

		//
		// ------------------------------
		void serial (NLMISC::IStream &f);
		// ------------------------------
		void setFrom ( NLGUI::CGroupContainer *pGC);
		void setTo ( NLGUI::CGroupContainer *pGC);
	};

	// Image of a desktop
	class CDesktopImage
	{
	public:
		std::vector<SCont>	GCImages;				// Image of each group container in the desktop
		sint				Version;				// Stream version for extra datas
		NLMISC::CMemStream	ExtraDatas;				// TODO : replace it with some polymorphic scheme
		//
		CDesktopImage();
		void serial(NLMISC::IStream &s);
		// Build a virtual desktop image from the current desktop
		void fromCurrentDesktop();
		// Set current desktop from this desktop image
		void toCurrentDesktop();
		// Update image of the given group container (added to the list if it does not exist)
		void updateGroupContainerImage( NLGUI::CGroupContainer &gc);
		// Remove a group container from the image
		void removeGroupContainerImage(const std::string &groupName);
	private:
		void read(NLMISC::IStream &s);
		void write(NLMISC::IStream &s);
	};

	// Elements saved from database
	struct SDBLeaf
	{
		std::string		Name;
		sint64			Value;
		sint64			OldValue;
		// ------------------------------
		void serial (NLMISC::IStream &f);
		// ------------------------------
		void setFrom (NLMISC::CCDBNodeLeaf *pNL);
		void setTo (NLMISC::CCDBNodeLeaf *pNL);
	};

	void dataBaseToStream (NLMISC::IStream &f);

	// Write to stream (should support seek functionnality)
	void streamToDataBase (NLMISC::IStream &f, uint32 uiDbSaveVersion);


};





#endif // NL_INTERFACE_CONFIG_H

/* End of interface_config.h */

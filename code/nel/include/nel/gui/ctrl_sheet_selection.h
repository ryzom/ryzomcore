// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#ifndef CL_SHEET_CTRL_SELECTION_H
#define CL_SHEET_CTRL_SELECTION_H

namespace NLGUI
{
	class IActionHandler;

	/** Infos about a selection group
	  */
	class CSheetSelectionGroup
	{
	public:
		CSheetSelectionGroup(std::string name) : _Name(name), _Active(false), _TextureIndex(-1), _Color(NLMISC::CRGBA::White), _GlobalColorEnabled(true) {}
		void                setTexture(const std::string &texName);
		sint32              getTextureIndex() const { return _TextureIndex; }
		sint32              getTextureWidth() const { return _TextureWidth; }
		sint32              getTextureHeight() const { return _TextureHeight; }
		void                setColor(NLMISC::CRGBA color) { _Color = color; }
		NLMISC::CRGBA       getColor() const { return _Color; }
		void                setActive(bool active) { _Active = active; }
		bool                isActive() const { return _Active; }
		const std::string  &getName() const { return _Name; }
		void                enableGlobalColor(bool enabled) { _GlobalColorEnabled = enabled; }
		bool				isGlobalColorEnabled() const { return _GlobalColorEnabled; }
	private:
		std::string	   _Name;
		bool           _Active;
		sint32         _TextureIndex; // index for the selection texture
		sint32         _TextureWidth;
		sint32         _TextureHeight;
		NLMISC::CRGBA  _Color;        // color that modulate the texture of selection
		bool		   _GlobalColorEnabled;
	};

	/** Class to manage selection of sheet.
	  * Sheet are managed by groups, identified by their ID.
	  */
	class CCtrlSheetSelection
	{
    public:
		// Add a group, and returns its index, or -1 if already created.
		sint addGroup(const std::string &name);
		// Get a group by its name (must exist)
		CSheetSelectionGroup *getGroup(const std::string &name);
		const CSheetSelectionGroup *getGroup(const std::string &name) const;
		// Get a group by its index
		CSheetSelectionGroup *getGroup(uint index);
		const CSheetSelectionGroup *getGroup(uint index) const;
		// Get the index of a group from its name, return -1 if not a group
		sint getGroupIndex(const std::string &name) const;
		// Deactivate all groups
		void deactivateAll();
		// delete all groups
		void deleteGroups();
	private:
		//
		typedef std::vector<CSheetSelectionGroup> TGroupVect;
		typedef std::map<std::string, uint>   TGroupNameToIndex;
	private:
		TGroupVect			_Groups;
		TGroupNameToIndex   _GroupNameToIndex;
	};

}

#endif

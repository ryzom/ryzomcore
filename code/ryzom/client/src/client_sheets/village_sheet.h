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



#ifndef RY_VILLAGE_SHEET_H
#define RY_VILLAGE_SHEET_H


// ig in a village
struct CVillageIG
{
	std::string IgName;
	std::string ParentName;
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial(IgName, ParentName);
	}
	/// Build the sheet from an external script.
	void build(const NLGEORGES::UFormElm &item);
};

/** Sheet of a village
  *  NB : it doesn't derives from CEntitySheet, because its instances are aggragated in a CContinentSheet
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */

struct CVillageSheet
{
	std::string Zone;
	float		Altitude;
	float       ForceLoadDist;
	float		LoadDist;
	float		UnloadDist;
	float		CenterX;
	float		CenterY;
	uint32		Width;
	uint32		Height;
	float		Rotation;
	std::string Name;
	std::vector<CVillageIG> IGs;

	// ctor
	CVillageSheet();

	/// Build the sheet from an external script.
	void build(const NLGEORGES::UFormElm &item);

	/// Serialize character sheet into binary data file.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

};

#endif

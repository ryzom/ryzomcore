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

#ifndef NL_ZONE_BANK_H
#define NL_ZONE_BANK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include <string>
#include <vector>

namespace NLLIGO
{

// Categories that MUST exist
// (zone)
// (size)
// (material) or (transname + transtype + transnum)

// All categories are string but some categories are particular string:
// zone is any string
// size is two number separated by a 'x'
// material is any string
// transname is two string (of materials separated by a '-')
// transnum is a number
// transtype is a string among (flat, cornera, cornerb)

// ***************************************************************************

/// No category of the type given found
#define STRING_NO_CAT_TYPE	"< NOCATTYPE >"

// ***************************************************************************

class CZoneBankElement
{

	// Category stuff
	// The key is the Type of the category	(Ex: "material", "size", ...)
	// The second element is the value		(Ex: "Grass", "2x2", ...)
	std::map<std::string, std::string>	_CategoriesMap;

	// In this list the category type and value must be unique and 2 categories MUST
	// appears : "zone" (The zone name) and "size" (*x* (ex:4x4 3x1 etc...))
	// Some categories used in WorldEditor : "material", "transition"
	uint8					_SizeX, _SizeY;
	std::vector<bool>		_Mask;

	static std::string		_NoCatTypeFound; // = STRING_NO_CAT_TYPE

public:

	CZoneBankElement ();

	// Set the mask of the zone bank element
	void setMask (const std::vector<bool> &mask, uint8 sizeX, uint8 sizeY);

	void addCategory (const std::string &CatType, const std::string &CatValue);
	const std::string &getName (); // Return the value of the "zone" category
	const std::string &getSize ();
	uint8 getSizeX () { return _SizeX; }
	uint8 getSizeY () { return _SizeY; }
	const std::vector<bool> &getMask () { return _Mask; }

	/// Return the CatValue or STRING_NO_CAT_TYPE if no category of that type found
	const std::string &getCategory(const std::string &CatType);

	/// Convert size in the categories to _SizeX, _SizeY
	void convertSize ();
	void serial (NLMISC::IStream &f);

	friend class CZoneBank;
};

// ***************************************************************************

class CZoneBank
{

	std::map<std::string,CZoneBankElement>	_ElementsMap;

	std::vector<CZoneBankElement*>	_Selection;

public:

	// Debug stuff beg
	// ---------------
	void debugInit(const std::string &sPath);
	void debugSaveInit(CZoneBankElement &zbeTmp, const std::string &fileName);
	// ---------------
	// Debug stuff end

	void reset ();
	/// Initialize the zone bank with all files present in the path given (note pathName must not end with '\\')
	bool initFromPath (const std::string &pathName, std::string &error);
	/// Load an element in the current directory
	bool addElement (const std::string &elementName, std::string &error);

	void getCategoriesType (std::vector<std::string> &CategoriesType);
	void getCategoryValues (const std::string &CategoryType, std::vector<std::string> &CategoryValues);
	CZoneBankElement *getElementByZoneName (const std::string &ZoneName);

	// Selection
	void resetSelection ();
	void addOrSwitch (const std::string &CategoryType, const std::string &CategoryValue);
	void addAndSwitch (const std::string &CategoryType, const std::string &CategoryValue);
	void getSelection (std::vector<CZoneBankElement*> &SelectedElements);

};

// ***************************************************************************

} // namespace NLLIGO

// ***************************************************************************

#endif // NL_ZONE_BANK_H

/* End of zone_bank.h */

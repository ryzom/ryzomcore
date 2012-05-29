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



#include "nel/misc/types_nl.h"
#include <string>


#ifndef CL_COLOR_SLOT_MANAGER_H
#define CL_COLOR_SLOT_MANAGER_H



namespace NL3D
{
	class UInstance;
}

namespace NLMISC
{
	class CConfigFile;
}

/**	This class allows to setup a model color's when color are encoded in various texture files.
  * It contains a set of slots description. Each slot match a part of a model whose color can be modified.
  * As an example, "skin", "hair" may be slots.
  * Each slot can have a limited number of colors. Each color must have its extension
  * The manager is initialized from a set of textures files.
  * The slot color each texture represents is encoded in the texture name
  * Example:
  * armour_red.dds
  * armour_green.dds
  * ...
  * Once the slots description have been filled, we can change model's colors for a given slot
  * \warning all slot should be created before textures are parsed
  */
class CColorSlotManager
{
public:
	typedef std::vector<std::string> TStringVect;
	typedef std::pair<uint, uint>	 TIntCouple;
	typedef std::vector<TIntCouple>	 TIntCoupleVect;
	typedef std::vector<uint>		 TUIntVect;
	enum { NotFound = (uint) -1 };
public:
	/// ctor
	CColorSlotManager();
	/** Add a slot containing file extensions
	  * \return the slot id
	  */
	uint				    addSlot(const TStringVect &slotDescs);

	/** Init slots from a config file. This config file is the same that the one that is in the 'panoply_maker' tool
	  * \param startSlot id of the first slot that has been added
	  * \param numSlots
	  * \return true is sucess
	  */
	bool					addSlotsFromConfigFile(NLMISC::CConfigFile &cf, uint &startSlot, uint &numSlots);

	/**	Search if an extension is in a slot
	  * \return a pair (slot ID, extension index) or (NotFound, 0)
	  */
	std::pair<uint, uint>	findFileExtensionInSlot(const std::string &ext) const;
	/// Set a separator for slot extensions. By default it is an underscore '_'
	void					setExtensionSeparator(const char *separator) { _Separator = separator; }
	/** Add a texture name. Its name will be parsed to see which slot colors it contains.
	  * \warning The necessary slots should have been added, otherwise parsing will fail
	  * \return True if the texture has been added successfully
	  */
	bool					addTexture(const char *texName);
	/** Add all texture from the given path. You can also provides a list of valid extensions for the files, though
	  * a default one is provided (TGA, DDS)
	  * \param path the path textures should be retrieved from.
	  * \param validExtensions an array of the valid bitmap files extensions.
	  * \param numExtensions number of texture extensions in the array
	  * \return true if all texture where added successfully
	  */
	bool					addTexturesFromPath(const std::string &path,
												const char *validExtensions[] = _DefaultTextureExtension,
												uint numExtensions = _NumDefaultTextureExtensions
											   );
	/** Modify a slot of an instance.
	  * \warning on the first call, all slot are set to their default value (0)
	  * \param instance the instance to change
	  * \param slotID The slot to change
	  * \param the value to assign to that slot
	  * \return true if all slots where changed successfully
	  * NB : If you must activate a texture slot on an instance, you should do it before calling this method.
	  */
	bool					setInstanceSlot(NL3D::UInstance instance, uint slotID, uint value) const;
	/** Modify several slots of an instance. This is usually faster than several calls to setInstanceSlot (single slot version)
	  * \warning on the first call, all slot are set to their default value (0)
	  * \param instance the instance to change
	  * \param slotIDs An array of pairs (slotID, slotValue)
	  * \param numValues Number of values in the array
	  * \return true if all slots where changed successfully
	  * NB : If you must activate a texture slot on an instance, you should do it before calling this method.
	  */
	bool					setInstanceSlot(NL3D::UInstance instance, TIntCouple *slotIDs, uint numValues) const;
	/// Reset all textures names
	void					resetTextures();
	/// Reset all slots
	void					resetSlots();
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
	typedef std::vector<TStringVect>					TSlotVect;
	typedef std::map<std::string, TUIntVect >   TTex2Slots; // from a texture name, tells which slots it can use
private:
	TSlotVect		_Slots;
	TTex2Slots      _TexMap;
	std::string	    _Separator;
private:
	static			const char	   *_DefaultTextureExtension[];
	static			const uint	   _NumDefaultTextureExtensions;
private:
	/** Parse a texture name and fills a vector with the ids of the slots it contains (if one is provided)
	  * It also remove the extensions from the fileName
	  * If it fails, the ids vector if left unmodified
	  * \param texNameWithoutExtensions A string that will be fill with the tex name with no extensions if the call succeed (and if not NULL)
	  * \param slotsId a vector that will be filled with couples (slotID, extensionID)
	  * \return true if the filename is correct (must contain at least one slot)
	  */
	bool					parseTexName(const char *texName, std::string *texNameWithoutExtensions = NULL,
										 TIntCoupleVect *slotsId = NULL) const;
	/** Change a texture name with the given parameters.
	  * \return true if the name changed
	  */
	bool					changeTexName(std::string &name, TIntCouple *slotIDs, uint numValues, bool &everyThingOk) const;
};

////////////
// GLOBAL //
////////////
/// Manage slot and color for instances.
extern CColorSlotManager ColorSlotManager;
/// Initialize the Color slot manager for the client.
void initColorSlotManager();


#endif

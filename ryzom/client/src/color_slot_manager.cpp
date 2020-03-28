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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"

#include "color_slot_manager.h"

#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/misc/config_file.h"
#include "nel/misc/file.h"

#include "nel/3d/u_instance_material.h"

#include <algorithm>

using namespace std;

////////////
// GLOBAL //
////////////

H_AUTO_DECL(RZ_ColorSlotManager)

NLMISC::CConfigFile ColorSlotConfigFile;

/// Manage slot and color for instances.
CColorSlotManager ColorSlotManager;

// convert an array of char * to a vector of strings
static void stringArrayToVector(const char *strArray[], uint size, CColorSlotManager::TStringVect &dest)
{
	dest.resize(size);
	std::copy(strArray, strArray + size, dest.begin());
}

//-----------------------------------------------
// initColorSlotManager :
// Initialize the Color slot manager for the client.
//-----------------------------------------------
void initColorSlotManager()
{
	char tmpBuff[300];
	uint startSlot;

	static const char *skins[] = { "FY", "MA", "TR", "ZO" };
	static const char *user[] = { "U1", "U2", "U3", "U4", "U5", "U6", "U7", "U8" };
	static const char *eyes[] = { "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8" };
	static const char *hair[] = { "H1", "H2", "H3", "H4", "H5", "H6" };

	CColorSlotManager::TStringVect sv;
	// skin
	stringArrayToVector(skins, sizeof(skins) / sizeof(skins[0]), sv);
	startSlot = ColorSlotManager.addSlot(sv);
	// user
	stringArrayToVector(user, sizeof(user) / sizeof(user[0]), sv);
	ColorSlotManager.addSlot(sv);
	// hair
	stringArrayToVector(hair, sizeof(hair) / sizeof(hair[0]), sv);
	ColorSlotManager.addSlot(sv);
	// eyes
	stringArrayToVector(eyes, sizeof(eyes) / sizeof(eyes[0]), sv);
	ColorSlotManager.addSlot(sv);


	string filename = NLMISC::CPath::lookup("panoply_files.txt", false, true);
	if (filename.empty())
	{
		nlwarning("Couldn't find the color slot manager texture list");
		return;
	}
	NLMISC::CIFile cfg;
	if(cfg.open(filename, false))
	{
		while(!cfg.eof())
		{
			cfg.getline(tmpBuff, 300);
			ColorSlotManager.addTexture(tmpBuff);
		}

		// Close the file.
		cfg.close();
	}
}// initColorSlotManager //


const char *CColorSlotManager::_DefaultTextureExtension[] = { "tga", "dds" };
const uint CColorSlotManager::_NumDefaultTextureExtensions = sizeof(CColorSlotManager::_DefaultTextureExtension) / sizeof(const char *);


//=======================================================================
CColorSlotManager::TIntCouple CColorSlotManager::findFileExtensionInSlot(const std::string &ext) const
{
	H_AUTO_USE(RZ_ColorSlotManager)
	for(TSlotVect::const_iterator it = _Slots.begin(); it != _Slots.end(); ++it)
	{
		TStringVect::const_iterator extIt = std::find(it->begin(), it->end(), ext);
		if (extIt != it->end())
		{
			return std::make_pair((uint)(it - _Slots.begin()), (uint)(extIt - it->begin()));
		}
	}
	return TIntCouple((uint) NotFound, 0);
}

//=======================================================================
CColorSlotManager::CColorSlotManager()	: _Separator("_")
{
}

//=======================================================================
uint CColorSlotManager::addSlot(const TStringVect &slotDescs)
{
	H_AUTO_USE(RZ_ColorSlotManager)
	_Slots.push_back(slotDescs);
	for(uint k = 0; k < slotDescs.size(); ++k)
	{
		_Slots.back()[k] = NLMISC::toUpper(_Slots.back()[k]);
	}
	return (uint)_Slots.size() - 1;
}


//=======================================================================
bool CColorSlotManager::addTexture(const char *texName)
{
	H_AUTO_USE(RZ_ColorSlotManager)
	static std::string texNameNoExtension;
	static TIntCoupleVect slotIDs;
	bool result = parseTexName(texName, &texNameNoExtension, &slotIDs);
	if (!result) return false;
	// see if not already in the map
	TTex2Slots::iterator it = _TexMap.find(texNameNoExtension);
	if (it == _TexMap.end())
	{
		// insert in map
		TUIntVect &slots = _TexMap[texNameNoExtension];
		slots.resize(slotIDs.size());
		for(uint k = 0; k < slotIDs.size(); ++k)
		{
			slots[k] = slotIDs[k].first;
		}
	}
	else
	{
		TUIntVect &slots = it->second;
		// make sure the slots are identical
		if (slots.size() != slotIDs.size())
		{
			nlwarning("%s number of slots is not constant", texName);
		}
		for(uint k = 0; k < slots.size(); ++k)
		{
			if (slots[k] != slotIDs[k].first)
			{
				nlwarning("Slots for %s are inconsistents", texName);
				return false;
			}
		}
	}
	return true;
}

//=======================================================================
bool CColorSlotManager::addTexturesFromPath(const std::string &path, const char *validExtensions[] /*=_DefaultTextureExtension*/, uint numExtensions /*=_NumDefaultTextureExtensions*/)
{
	H_AUTO_USE(RZ_ColorSlotManager)
	if (!validExtensions || numExtensions == 0)
	{
		nlwarning("CColorSlotManager::addTexturesFromPath : no file extensions provided");
		return false;
	}
	if (path.empty())
	{
		nlwarning("CColorSlotManager::addTexturesFromPath : empty path provided");
		return false;
	}
	if (!NLMISC::CFile::isExists(path))
	{
		nlwarning(("CColorSlotManager::addTexturesFromPath : path not found : " + path).c_str());
		return false;
	}
	std::vector<std::string> files;
	NLMISC::CPath::getPathContent(path, true, false, true, files);
	if (files.empty())
	{
		 nlwarning(("CColorSlotManager::addTexturesFromPath : empty path :" + path).c_str());
		 return false;
	}
	bool everythingOk = true;
	bool fileFound = false;
	std::string fileExt;
	for(uint k = 0; k < files.size(); ++k)
	{
		fileExt = NLMISC::CFile::getExtension(files[k]);
		bool extFound = false;
		for(uint l = 0; l < numExtensions && !extFound; ++l)
		{
			if (NLMISC::nlstricmp(fileExt.c_str(), validExtensions[l]) == 0)
			{
				fileFound = true;
				if (!addTexture(files[k].c_str()))
				{
					everythingOk = false;
				}
				break;
			}
		}
	}
	if (!fileFound)
	{
		nlwarning(("CColorSlotManager::addTexturesFromPath : the path " + path + " contains no texture files").c_str());
	}
	return everythingOk;
}



//=======================================================================
void CColorSlotManager::resetTextures()
{
	H_AUTO_USE(RZ_ColorSlotManager)
	_TexMap.clear();
}


//=======================================================================
void CColorSlotManager::resetSlots()
{
	H_AUTO_USE(RZ_ColorSlotManager)
	_Slots.clear();
}

//=======================================================================
bool CColorSlotManager::parseTexName(const char *texName, std::string *texNameWithoutExtensions,
									 TIntCoupleVect *destSlotsId /*=NULL*/) const
{
	H_AUTO_USE(RZ_ColorSlotManager)
	/** If we got a name such as "armour_red_shiny.tga", and if shiny and red are extensions, we parse
	  * shiny ('_' is considered has a separator), and find the slot that has 'shiny' in it (lets says slot 0),
	  * then 'red' is parsed (let says it is in the slot 1).
	  * The slotId vector will be { 0, 1 } at the end
	  * If no slots where found, the parsing is considered to have failed.
	  * If an extension is duplicated then it fails, too.
	  */
	typedef std::string::size_type TStrPos;
	static  std::string nameToParse;
	static  std::string currentExt;
	static  TIntCoupleVect slotsId;
	nameToParse = NLMISC::toUpper(NLMISC::CFile::getFilenameWithoutExtension(texName));

	TStrPos currPos = nameToParse.length();

	slotsId.clear();
	for(;;)
	{
		// search a separator
		TStrPos extPos = nameToParse.find_last_of(_Separator, currPos);
		if (extPos == std::string::npos) break;
		// get the extension and see if it is in a slot
		currentExt = nameToParse.substr(extPos + _Separator.length(), currPos - extPos - _Separator.length() + 1);
		TIntCouple slotId = findFileExtensionInSlot(currentExt);
		if (slotId.first == NotFound)
		{
			break;
		}
		for(uint k = 0; k < slotsId.size(); ++k)
		{
			if (slotsId[k].first == slotId.first)
			{
				nlwarning((std::string("Duplicated extension in texture name ") + texName).c_str());
				return false;
			}
		}
		slotsId.push_back(slotId);
		if (extPos == 0) break;
		currPos = extPos - 1;
	}

	if (slotsId.empty()) return false;
	if (destSlotsId)
	{
		std::reverse(slotsId.begin(), slotsId.end());
		*destSlotsId = slotsId;	// commit changes
	}
	if (texNameWithoutExtensions)
	{
		texNameWithoutExtensions->resize(currPos + 1);
		std::copy(nameToParse.begin(), nameToParse.begin() + currPos + 1, texNameWithoutExtensions->begin());
	}
	return true;
}




//=======================================================================
bool CColorSlotManager::addSlotsFromConfigFile(NLMISC::CConfigFile &cf, uint &startSlotDest, uint &numSlots)
{
	H_AUTO_USE(RZ_ColorSlotManager)
	NLMISC::CConfigFile::CVar *mask_extensions;
	// get list of masks / slots
	try
	{
		mask_extensions = &cf.getVar("mask_extensions");
	}
	catch (const NLMISC::EUnknownVar &)
	{
		return false;
	}

	uint startSlot = (uint)_Slots.size();

	_Slots.resize(_Slots.size() + mask_extensions->size());
	/// For each kind of mask, build a slot
	for (uint k = 0; k < (uint) mask_extensions->size(); ++k)
	{
		try
		{
			NLMISC::CConfigFile::CVar &extensions = cf.getVar(mask_extensions->asString(k) +"_color_id");
			_Slots[k + startSlot].resize(extensions.size());
			for(sint l = 0; l < (sint) extensions.size(); ++l)
			{
				_Slots[k + startSlot][l] = extensions.asString(l);
			}
		}
		catch (const NLMISC::EUnknownVar &)
		{
			_Slots.resize(startSlot);
			nlwarning(("CColorSlotManager::addSlotsFromConfigFile : invalid config file, variable not found :" + mask_extensions->asString(k) + "_color_id").c_str());
			return false;
		}
	}

	try
	{
		_Separator = cf.getVar("default_separator").asString();
	}
	catch (const NLMISC::EUnknownVar &)
	{
		_Separator = "_";
	}

	startSlotDest = startSlot;
	numSlots	  = (uint)_Slots.size() - startSlot;
	return true;
}


//=======================================================================
bool CColorSlotManager::changeTexName(std::string &texName, TIntCouple *slotIDs, uint numValues, bool &everythingOk) const
{
	H_AUTO_USE(RZ_ColorSlotManager)
	static std::string texNameNoExt;
	static std::string texNameBuild;
	static std::string texExt;
	static TIntCoupleVect srcSlotIDs;

	everythingOk = true;
	texNameNoExt = NLMISC::toUpper(NLMISC::CFile::getFilenameWithoutExtension(texName));
	texExt       = NLMISC::CFile::getExtension(texName);
	TTex2Slots::const_iterator texIt = _TexMap.find(texNameNoExt);
	if (texIt != _TexMap.end())
	{
		/** Texture hasn't been setup yet.
		  * We add all the extensions
		  */
		for(uint m = 0; m < texIt->second.size(); ++m)
		{
			uint n;
			// search if there's a matching slot in the list
			for(n = 0; n < numValues; ++n)
			{
				if (texIt->second[m] == slotIDs[n].first)
				{
					if (slotIDs[n].second < _Slots[slotIDs[n].first].size())
					{
						texNameNoExt += _Separator + _Slots[slotIDs[n].first][slotIDs[n].second];
					}
					else
					{
						nlwarning("Color slot : out of range value found");
						// Use previous extension
						texNameNoExt += _Separator + _Slots[srcSlotIDs[m].first][srcSlotIDs[m].second];
						everythingOk = false;
					}
					break;
				}
			}
			if (n == numValues) // not found
			{
				// Use default extension
				texNameNoExt += _Separator + _Slots[texIt->second[m]][0];
			}
		}
		texName = texNameNoExt += "." + texExt;
		return true;
	}
	else // Extensions are already present in the tex name, or simply, there are no slots for this texture
	{
		/** The texture has been setup yet, or it isn't a texture that has slots.
		  * So we just try to parse it, and we change slots
		  */
		  if (parseTexName(texNameNoExt.c_str(), &texNameBuild, &srcSlotIDs))
		  {
				// ok, texture has slots
				for(uint m = 0; m < srcSlotIDs.size(); ++m)
				{
					uint n;
					// search if there's a matching slot in the list
					for(n = 0; n < numValues; ++n)
					{
						if (srcSlotIDs[m].first == slotIDs[n].first)
						{
							if (slotIDs[n].second < _Slots[slotIDs[n].first].size())
							{
								texNameBuild += _Separator + _Slots[slotIDs[n].first][slotIDs[n].second];
							}
							else
							{
								nlwarning("Color slot : out of range value found");
								// Use previous extension
								texNameBuild += _Separator + _Slots[srcSlotIDs[m].first][srcSlotIDs[m].second];
								everythingOk = false;
							}
							break;
						}
					}
					if (n == numValues) // not found
					{
						// Use previous extension
						texNameBuild += _Separator + _Slots[srcSlotIDs[m].first][srcSlotIDs[m].second];
					}
				}
				texName = texNameBuild += "." + texExt;
				return true;
		  }
		  else
		  {
			  // this texture has no slots
			  return false;
		  }
	}
}


//=======================================================================
bool CColorSlotManager::setInstanceSlot(NL3D::UInstance instance,TIntCouple *slotIDs, uint numValues) const
{
	H_AUTO_USE(RZ_ColorSlotManager)
	nlassert(!instance.empty());
	bool everythingOk = true;
	bool everythingOkForCurrent;
	uint numMat = instance.getNumMaterials();
	static std::string texName;
	for(uint k = 0; k < numMat; ++k)
	{
		NL3D::UInstanceMaterial mat = instance.getMaterial(k);
		sint numTex = mat.getLastTextureStage();
		for(sint l = 0; l <= numTex; ++l)
		{
			if (mat.isTextureFile(l)) // one texture from a file ?
			{
				// if the texture hasn't been setup yet, we should find it in the map
				texName      = mat.getTextureFileName(l);
				if (changeTexName(texName, slotIDs, numValues, everythingOkForCurrent))
				{
					mat.setTextureFileName(texName, l);
				}
				if (!everythingOkForCurrent)
				{
					everythingOk = false;
				}
			}
		}
	}
	return everythingOk;
}


//=======================================================================
bool CColorSlotManager::setInstanceSlot(NL3D::UInstance instance, uint slotID, uint value) const
{
	H_AUTO_USE(RZ_ColorSlotManager)
	TIntCouple valuePair(slotID, value);
	return setInstanceSlot(instance, &valuePair, 1);
}

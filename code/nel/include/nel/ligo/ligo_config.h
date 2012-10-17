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

#ifndef NL_LIGO_CONFIG_H
#define NL_LIGO_CONFIG_H

#include "nel/misc/types_nl.h"
#include "primitive_class.h"
#include "primitive_configuration.h"

#define DEFAULT_PRIMITIVE_COLOR (CRGBA (128, 0, 0, 128))

namespace NLLIGO
{

class IPrimitive;
class CPrimitive;

/**
 *  Ligo config file
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CLigoConfig
{
public:

	CLigoConfig();

	virtual ~CLigoConfig() { }

	/** Load the config file. Can throw some exceptions if file doesn't exist or is malformed.
	  *
	  * This file will try to open the file ligo class description file (XML) using the LigoClass as file name.
	  * It will try first to load directly the file and then to lookup the file in NLMISC::CPath.
	  */
	bool readConfigFile (const char *fileName, bool parsePrimitiveComboContent);

	/**
	  * This file will read the file ligo class description file (XML) using the LigoClass as file name.
	  * It will try first to load directly the file and then to lookup the file in NLMISC::CPath.
	  */
	bool readPrimitiveClass (const char *fileName, bool parsePrimitiveComboContent);

	bool reloadIndexFile(const std::string &indexFileName = std::string());


	/// Reset the primitive configurations
	void resetPrimitiveConfiguration ();

	/// \name Public value

	/// Size of a cell of the ligoscape in meter
	float	CellSize;

	/// Snap precision for vertex position checking in meter
	float	Snap;

	/// Zone snap shot resolution
	uint	ZoneSnapShotRes;

	/// The ligo class file
	std::string	PrimitiveClassFilename;

	/// \name Primitive class accessors

	/// Get the dynamic bit size for alias
	uint32				getDynamicAliasSize() const;
	/// Get the dynamic bit mask for alias
	uint32				getDynamicAliasMask() const;
	/// Get the static bit size for alias
	uint32				getStaticAliasSize() const;
	/// Get the static bit mask for alias
	uint32				getStaticAliasMask() const;
	/// Build an alias given a static and dynamic part
	uint32				buildAlias(uint32 staticPart, uint32 dynamicPart, bool warnIfOverload = true) const;
	/// register filename to static alias translation
	void				registerFileToStaticAliasTranslation(const std::string &fileName, uint32 staticPart);
	/// get the static alias mapping (or 0 if no mapping defined)
	virtual uint32				getFileStaticAliasMapping(const std::string &fileName) const;
	/// get the filename for a static alias (or empty string for 0)
	const std::string	&getFileNameForStaticAlias(uint32 staticAlias) const;
	/// Check if a file is already mapped
	bool				isFileStaticAliasMapped(const std::string &fileName) const;
	/// Build a standard human readable alias string
	std::string			aliasToString(uint32 fullAlias);
	/// Read a standard human readable alias string
	uint32				aliasFromString(std::string fullAlias);


	// Get a primitive class
	const CPrimitiveClass		*getPrimitiveClass (const NLLIGO::IPrimitive &primitive) const;

	// Get a primitive class
	const CPrimitiveClass		*getPrimitiveClass (const char *className) const;

	// Get the primitive color
	NLMISC::CRGBA				getPrimitiveColor (const NLLIGO::IPrimitive &primitive);

	// Is the primitive deletable ?
	bool isStaticChild (const NLLIGO::IPrimitive &primitive);

	// Is the primitive linked to its brother primitive ?
	bool isPrimitiveLinked (const NLLIGO::IPrimitive &primitive);

	// Return the next primitive linked to 'primitive', or NULL
	const NLLIGO::IPrimitive *getLinkedPrimitive (const NLLIGO::IPrimitive &primitive) const;

	// Return the previous primitive linked to 'primitive', or NULL
	const NLLIGO::IPrimitive *getPreviousLinkedPrimitive (const NLLIGO::IPrimitive &primitive) const;

	// Is the primitive deletable ?
	bool isPrimitiveDeletable (const NLLIGO::IPrimitive &primitive);

	// Is the child primitive can be a child of the parent primitive ?
	bool canBeChild (const NLLIGO::IPrimitive &child, const NLLIGO::IPrimitive &parent);

	// Is the primitive a root primitive ?
	bool canBeRoot (const NLLIGO::IPrimitive &primitive);

	// Read a property from an XML file
	bool getPropertyString (std::string &result, const char *filename, xmlNodePtr xmlNode, const char *propName);

	// Output error message
	void syntaxError (const char *filename, xmlNodePtr xmlNode, const char *format, ...);
	virtual void errorMessage (const char *format, ... );

	// Access to the config string
	const std::vector<std::string> &getContextString () const;

	// Access the primitive configuration
	const std::vector<CPrimitiveConfigurations> &getPrimitiveConfiguration() const
	{
		return _PrimitiveConfigurations;
	}

	// Update the DynamicAlias bit count that was previously defined in the config file.
	// The _StaticAliasFileMapping is updated in order that full alias stay the same.
	// All previous DynamicAlias must fit in the new DynamicAliasBitCount
	void updateDynamicAliasBitCount(uint32 newDynamicAliasBitCount);

private:

	// Init primitive class manager
	bool		initPrimitiveClass (const char *filename);

	// The primitive class manager
	std::map<std::string, CPrimitiveClass>	_PrimitiveClasses;

	// The context strings
	std::vector<std::string>	_Contexts;

	// The file context look up
	std::map<std::string, std::string>	_ContextFilesLookup;

	// The primitive configurations
	std::vector<CPrimitiveConfigurations>	_PrimitiveConfigurations;

	// Dynamic alias bit count
	uint32				_DynamicAliasBitCount;

	/// Name of the index file
	std::string			_IndexFileName;
	// Static alias part file mapping : filename -> staticAliasPart
	std::map<std::string, uint32>		_StaticAliasFileMapping;
};

}

#endif // NL_LIGO_CONFIG_H

/* End of ligo_config.h */

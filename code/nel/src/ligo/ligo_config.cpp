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

#include "stdligo.h"
#include "nel/ligo/ligo_config.h"

#include "nel/ligo/primitive.h"
#include "nel/misc/config_file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include <cstdlib>

using namespace std;
using namespace NLMISC;

namespace NLLIGO
{

// ***************************************************************************
CLigoConfig::CLigoConfig()
: _DynamicAliasBitCount(32)
{
}

// ***************************************************************************

bool CLigoConfig::readConfigFile (const char *fileName, bool parsePrimitiveComboContent)
{
	// The CF
	CConfigFile cf;

	// Load and parse the file
	cf.load (fileName);

	// Read the parameters
	CConfigFile::CVar &cell_size = cf.getVar ("cell_size");
	CellSize = cell_size.asFloat ();
	CConfigFile::CVar &snap = cf.getVar ("snap");
	Snap = snap.asFloat ();
	CConfigFile::CVar &snapShot = cf.getVar ("zone_snapeshot_res");
	ZoneSnapShotRes = (uint)snapShot.asInt ();
	CConfigFile::CVar &primitiveClassFilename = cf.getVar ("primitive_class_filename");
	PrimitiveClassFilename= primitiveClassFilename.asString ();

	// Clear the previous classes
	_Contexts.clear();
	_Contexts.push_back ("default");
	_PrimitiveClasses.clear();
	_PrimitiveConfigurations.clear();

	// Read the primitive class name
	if (!PrimitiveClassFilename.empty())
	{
		return readPrimitiveClass (PrimitiveClassFilename.c_str(), parsePrimitiveComboContent);
	}
	return true;
}

// ***************************************************************************

bool CLigoConfig::readPrimitiveClass (const char *_fileName, bool parsePrimitiveComboContent)
{
	// File exist ?
	string filename = _fileName;
	filename = CPath::lookup (_fileName, false, false, false);
	if (filename.empty())
		filename = _fileName;

	// The context strings
	set<string> contextStrings;

	// Read the document
	CIFile file;
	if (file.open (filename))
	{
		try
		{
			// XML stream
			CIXml xml;
			xml.init (file);

			// Get the root node
			xmlNodePtr root = xml.getRootNode ();
			nlassert (root);

			// Check the header
			if (strcmp ((const char*)root->name, "NEL_LIGO_PRIMITIVE_CLASS") == 0)
			{
				// ALIAS_DYNAMIC_BITS
				xmlNodePtr aliasBits = CIXml::getFirstChildNode (root, "ALIAS_DYNAMIC_BITS");
				if (aliasBits)
				{
					string bits;
					if (getPropertyString (bits, filename.c_str(), aliasBits, "BIT_COUNT"))
					{
						uint32 uBits;
						NLMISC::fromString(bits, uBits);
						_DynamicAliasBitCount = std::min((uint32)32, uBits);
					}
					else
						return false;

				}
				else
				{
					// by default, set the size of dynamic alias to 32 bits
					_DynamicAliasBitCount = 32;
				}
				// ALIAS_STATIC_FILE_ID
				xmlNodePtr indexFileNameNode = CIXml::getFirstChildNode (root, "ALIAS_STATIC_FILE_ID");
				if (indexFileNameNode)
				{
					string indexFileName;
					if (getPropertyString (indexFileName, filename.c_str(), indexFileNameNode, "FILE_NAME"))
					{
						if (CPath::lookup(indexFileName, false, false, true).empty())
						{
							// try to append the class file path
							indexFileName = CFile::getPath(_fileName)+indexFileName;
						}
						// load the configuration file
						reloadIndexFile(indexFileName);
					}
					else
						nlwarning("Can't find XML element <FILE_NAME>, no file index available for alias" );
				}
				else
				{
					// by default, set the size of dynamic alias to 32 bits
					_DynamicAliasBitCount = 32;
				}


				// Get the first primitive description
				xmlNodePtr primitive = CIXml::getFirstChildNode (root, "PRIMITIVE");
				if (primitive)
				{
					do
					{
						// Get the primitive name
						std::string name;
						if (getPropertyString (name, filename.c_str(), primitive, "CLASS_NAME"))
						{
							// Add the primitive
							pair<std::map<std::string, CPrimitiveClass>::iterator, bool> insertResult =
								_PrimitiveClasses.insert (std::map<std::string, CPrimitiveClass>::value_type (name, CPrimitiveClass ()));
							if (insertResult.second)
							{
								if (!insertResult.first->second.read (primitive, filename.c_str(), name.c_str (), contextStrings, _ContextFilesLookup, *this, parsePrimitiveComboContent))
									return false;
							}
							else
							{
								syntaxError (filename.c_str(), root, "Class (%s) already defined", name.c_str ());
							}
						}
						else
							return false;

						primitive = CIXml::getNextChildNode (primitive, "PRIMITIVE");
					}
					while (primitive);
				}

				// Add the context strings
				{
					set<string>::iterator ite = contextStrings.begin ();
					while (ite != contextStrings.end ())
					{
						if (*ite != "default")
							_Contexts.push_back (*ite);
						ite++;
					}
				}

				// Get the first primitive configuration
				_PrimitiveConfigurations.reserve (_PrimitiveConfigurations.size()+CIXml::countChildren (root, "CONFIGURATION"));
				xmlNodePtr configuration = CIXml::getFirstChildNode (root, "CONFIGURATION");
				if (configuration)
				{
					do
					{
						// Get the configuration name
						std::string name;
						if (getPropertyString (name, filename.c_str(), configuration, "NAME"))
						{
							// Add the configuration
							_PrimitiveConfigurations.resize (_PrimitiveConfigurations.size()+1);
							if (!_PrimitiveConfigurations.back().read (configuration, filename.c_str(), name.c_str (), *this))
								return false;
						}
						else
							return false;

						configuration = CIXml::getNextChildNode (configuration, "CONFIGURATION");
					}
					while (configuration);
				}

				// Ok
				return true;
			}
			else
			{
				syntaxError (filename.c_str(), root, "Wrong root node, should be NEL_LIGO_PRIMITIVE_CLASS");
			}
		}
		catch (const Exception &e)
		{
			errorMessage ("File read error (%s):%s", filename.c_str(), e.what ());
		}
	}
	else
	{
		errorMessage ("Can't open the file %s for reading.", filename.c_str());
	}
	return false;
}

// ***************************************************************************

bool CLigoConfig::reloadIndexFile(const std::string &indexFileName)
{
	if (_IndexFileName.empty() && indexFileName.empty())
	{
		nlwarning("CLigoConfig::reloadIndexFile: no file name specified and index file not previously loaded, can't load anything");
		return false;
	}

	if (!_IndexFileName.empty() && !indexFileName.empty() && _IndexFileName != indexFileName)
	{
		nlwarning("CLigoConfig::reloadIndexFile: index file already loaded as '%s', can't load another file '%s'!",
			_IndexFileName.c_str(),
			indexFileName.c_str());
		return false;
	}

	if (_IndexFileName.empty())
		_IndexFileName = indexFileName;

	// load the configuration file
	CConfigFile cf;
	string pathName = CPath::lookup(_IndexFileName, false);

	if (pathName.empty())
	{
		nlwarning("Can't find index file '%s' in search path, no file index available for alias", indexFileName.c_str());
		return false;
	}
	cf.load(pathName);

	// get the variable
	CConfigFile::CVar *files = cf.getVarPtr("Files");
	if (files != NULL)
	{
		for (uint i=0; i<files->size()/2; ++i)
		{
			string fileName;
			uint32 index;

			fileName = files->asString(i*2);
			index = files->asInt(i*2+1);

			if (isFileStaticAliasMapped(fileName))
			{
				// check that the mapping as not changed
				if (getFileStaticAliasMapping(fileName) != index)
				{
					nlwarning("CLigoConfig::reloadIndexFile: the mapping for the file '%s' as changed from %u to %u in the config file, the change is ignored",
						fileName.c_str(),
						index,
						getFileStaticAliasMapping(fileName));
				}
			}
			else
			{
				registerFileToStaticAliasTranslation(fileName, index);
			}
		}
	}

	return true;
}

// ***************************************************************************

NLMISC::CRGBA CLigoConfig::getPrimitiveColor (const NLLIGO::IPrimitive &primitive)
{
	// Get the class
	string className;
	if (primitive.getPropertyByName ("class", className))
	{
		// Get the class
		std::map<std::string, CPrimitiveClass>::iterator ite = _PrimitiveClasses.find (className);
		if (ite != _PrimitiveClasses.end ())
		{
			return ite->second.Color;
		}
	}
	return DEFAULT_PRIMITIVE_COLOR;
}

// ***************************************************************************

bool CLigoConfig::isPrimitiveLinked (const NLLIGO::IPrimitive &primitive)
{
	// Get the class
	string className;
	if (primitive.getPropertyByName ("class", className))
	{
		// Get the class
		std::map<std::string, CPrimitiveClass>::iterator ite = _PrimitiveClasses.find (className);
		if (ite != _PrimitiveClasses.end ())
		{
			return ite->second.LinkBrothers;
		}
	}
	return false;
}

// ***************************************************************************

const NLLIGO::IPrimitive *CLigoConfig::getLinkedPrimitive (const NLLIGO::IPrimitive &primitive) const
{
	// Get the parent
	const IPrimitive *parent = primitive.getParent ();
	if (parent)
	{
		uint childId;
		if (parent->getChildId (childId, &primitive))
		{
			// Test the next primitive

			// Get the primitive class
			string className;
			if (primitive.getPropertyByName ("class", className))
			{
				// Get the class
				std::map<std::string, CPrimitiveClass>::const_iterator ite = _PrimitiveClasses.find (className);
				if (ite != _PrimitiveClasses.end ())
				{
					if (ite->second.LinkBrothers)
					{
						// Add the next child
						const IPrimitive *brother;
						if (parent->getChild (brother, childId+1))
							return brother;
					}
				}
			}
		}
	}
	return NULL;
}

// ***************************************************************************

const NLLIGO::IPrimitive *CLigoConfig::getPreviousLinkedPrimitive (const NLLIGO::IPrimitive &primitive) const
{
	// Get the parent
	const IPrimitive *parent = primitive.getParent ();
	if (parent)
	{
		uint childId;
		if (parent->getChildId (childId, &primitive))
		{
			// Test the previous primitive
			if (childId > 0)
			{
				const IPrimitive *brother;
				if (parent->getChild (brother, childId-1) && brother)
				{
					// Get the primitive class
					string className;
					if (brother->getPropertyByName ("class", className))
					{
						// Get the class
						std::map<std::string, CPrimitiveClass>::const_iterator ite = _PrimitiveClasses.find (className);
						if (ite != _PrimitiveClasses.end ())
						{
							if (ite->second.LinkBrothers)
							{
								// Return the previous child
								return brother;
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

// ***************************************************************************

bool CLigoConfig::isPrimitiveDeletable (const NLLIGO::IPrimitive &primitive)
{
	// If it is a static child, it can't be deleted.
	if (isStaticChild (primitive))
		return false;

	// Get the class
	string className;
	if (primitive.getPropertyByName ("class", className))
	{
		// Get the class
		std::map<std::string, CPrimitiveClass>::iterator ite = _PrimitiveClasses.find (className);
		if (ite != _PrimitiveClasses.end ())
		{
			return ite->second.Deletable;
		}
	}
	return false;
}

// ***************************************************************************

bool CLigoConfig::canBeChild (const NLLIGO::IPrimitive &child, const NLLIGO::IPrimitive &parent)
{
	// Get the child class
	string childClassName;
	if (child.getPropertyByName ("class", childClassName))
	{
		// Get the parent class
		const CPrimitiveClass *parentClass = getPrimitiveClass (parent);
		if (parentClass)
		{
			// Search for the child class
			uint i;
			for (i=0; i<parentClass->DynamicChildren.size (); i++)
			{
				// The same ?
				if (parentClass->DynamicChildren[i].ClassName == childClassName)
					break;
			}

			if (i<parentClass->DynamicChildren.size ())
				return true;

			for (i=0; i<parentClass->GeneratedChildren.size (); i++)
			{
				// The same ?
				if (parentClass->GeneratedChildren[i].ClassName == childClassName)
					break;
			}

			return (i<parentClass->GeneratedChildren.size ());
		}
		else
			return true;
	}
	else
	{
		// Only if it is a root node or parent class doesn't exist
		string parentClassName;
		return ( (parent.getParent () == NULL) || (!parent.getPropertyByName ("class", parentClassName) ) );
	}
}

// ***************************************************************************

bool CLigoConfig::canBeRoot (const NLLIGO::IPrimitive &child)
{
	// Get the child class
	string childClassName;
	if (child.getPropertyByName ("class", childClassName))
	{
		// Get the parent class
		const CPrimitiveClass *parentClass = getPrimitiveClass ("root");
		if (parentClass)
		{
			// Search for the child class
			uint i;
			for (i=0; i<parentClass->DynamicChildren.size (); i++)
			{
				// The same ?
				if (parentClass->DynamicChildren[i].ClassName == childClassName)
					break;
			}

			return (i<parentClass->DynamicChildren.size ());
		}
		else
			return true;
	}
	else
	{
		// Root class doesn't exist
		return ( !getPrimitiveClass ("root") );
	}
}

// ***************************************************************************

bool CLigoConfig::getPropertyString (std::string &result, const char *filename, xmlNodePtr xmlNode, const char *propName)
{
	// Call the CIXml version
	if (!CIXml::getPropertyString (result, xmlNode, propName))
	{
		// Output a formated error
		syntaxError (filename, xmlNode, "Missing XML node property (%s)", propName);
		return false;
	}
	return true;
}

// ***************************************************************************

void CLigoConfig::syntaxError (const char *filename, xmlNodePtr xmlNode, const char *format, ...)
{
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	errorMessage ("(%s), node (%s), line (%p) :\n%s", filename, xmlNode->name, xmlNode->content, buffer);
}

// ***************************************************************************

void CLigoConfig::errorMessage (const char *format, ... )
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	nlwarning (buffer);
}

// ***************************************************************************

const std::vector<std::string> &CLigoConfig::getContextString () const
{
	return _Contexts;
}

// ***************************************************************************

const CPrimitiveClass *CLigoConfig::getPrimitiveClass (const IPrimitive &primitive) const
{
	const CPrimitiveClass *primClass = NULL;

	// Get property class
	string className;
	if (primitive.getPropertyByName ("class", className))
	{
		std::map<std::string, CPrimitiveClass>::const_iterator ite = _PrimitiveClasses.find (className);
		if (ite != _PrimitiveClasses.end ())
		{
			primClass = &(ite->second);
		}
	}

	// Not found ?
	if (!primClass)
	{
		// Root ?
		if (!primitive.getParent ())
		{
			std::map<std::string, CPrimitiveClass>::const_iterator ite = _PrimitiveClasses.find ("root");
			if (ite != _PrimitiveClasses.end ())
			{
				primClass = &(ite->second);
			}
		}
	}
	return primClass;
}

// ***************************************************************************

const CPrimitiveClass *CLigoConfig::getPrimitiveClass (const char *className) const
{
	std::map<std::string, CPrimitiveClass>::const_iterator ite = _PrimitiveClasses.find (className);
	if (ite != _PrimitiveClasses.end ())
	{
		return &(ite->second);
	}
	return NULL;
}

// ***************************************************************************

void CLigoConfig::resetPrimitiveConfiguration ()
{
	_PrimitiveConfigurations.clear ();
}

// ***************************************************************************

bool CLigoConfig::isStaticChild (const NLLIGO::IPrimitive &primitive)
{
	// Has a parent ?
	const IPrimitive *parent = primitive.getParent ();
	if (parent)
	{
		// Get the classes
		const CPrimitiveClass *parentClass = getPrimitiveClass (*parent);
		string className;
		string name;
		if (parentClass && primitive.getPropertyByName ("class", className) && primitive.getPropertyByName ("name", name))
		{
			// Does it belong to the static children ?
			uint i;
			for (i=0; i<parentClass->StaticChildren.size(); i++)
			{
				if (parentClass->StaticChildren[i].Name == name &&
					parentClass->StaticChildren[i].ClassName == className)
				{
					// Found
					return true;
				}
			}
		}
	}
	return false;
}

// ***************************************************************************

/// Get the dynamic bit size for alias
uint32 CLigoConfig::getDynamicAliasSize() const
{
	return _DynamicAliasBitCount;
}

// ***************************************************************************
/// Get the dynamic bit mask for alias
uint32 CLigoConfig::getDynamicAliasMask() const
{
	// this 'strange' test because VC fail to generate a correct shift if
	// _DynamicAliasBitCount is 32 bits.
	// The generated code lead to no shift at all and return 0 instead of 0xffffffff
	if (_DynamicAliasBitCount >= 32)
		return 0xffffffff;
	else
		return (1U<<_DynamicAliasBitCount)-1;
}

// ***************************************************************************
/// Get the static bit size for alias
uint32 CLigoConfig::getStaticAliasSize() const
{
	return 32-_DynamicAliasBitCount;
}

// ***************************************************************************
/// Get the static bit mask for alias
uint32 CLigoConfig::getStaticAliasMask() const
{
	// the opposite of the dynamic mask
	return ~getDynamicAliasMask();
}

// ***************************************************************************
/// Build an alias given a static and dynamic part
uint32 CLigoConfig::buildAlias(uint32 staticPart, uint32 dynamicPart, bool warnIfOverload) const
{
	if (warnIfOverload)
	{
		if (staticPart != (staticPart & (getStaticAliasMask()>>getDynamicAliasSize())))
		{
			nlwarning("CLigoConfig::buildAlias: staticPart 0x%x is outside the mask 0x%x",
				staticPart,
				getStaticAliasMask()>>getDynamicAliasSize());
		}
		if (dynamicPart != (dynamicPart & getDynamicAliasMask()))
		{
			nlwarning("CLigoConfig::buildAlias: dynamicPart 0x%x is outside the mask 0x%x",
				dynamicPart,
				getDynamicAliasMask());
		}
	}

	return dynamicPart | (staticPart << _DynamicAliasBitCount);
}

// ***************************************************************************

void CLigoConfig::registerFileToStaticAliasTranslation(const std::string &fileName, uint32 staticPart)
{
	// check the existing mapping
	std::map<std::string, uint32>::iterator first(_StaticAliasFileMapping.begin()), last(_StaticAliasFileMapping.end());
	for (; first != last; ++first)
	{
		if (first->second == staticPart)
		{
			nlassertex(false, ("While registering static alias %u to file '%s', the alias is already assigned to file '%s'",
				staticPart,
				fileName.c_str(),
				first->first.c_str()));
		}
	}
	if ((staticPart<<getDynamicAliasSize()) != ((staticPart<<getDynamicAliasSize()) & getStaticAliasMask()))
	{
		nlwarning("CLigoConfig::registerStaticAliasTranslation: staticPart 0x%x(%u) is outside the mask 0x%x, the staticPart will be clipped to 0x%x(%u)",
			staticPart,
			staticPart,
			getStaticAliasMask(),
			staticPart & getStaticAliasMask(),
			staticPart & getStaticAliasMask());

		staticPart = (staticPart & getStaticAliasMask());
	}


	_StaticAliasFileMapping[fileName] = staticPart;
}

const std::string &CLigoConfig::getFileNameForStaticAlias(uint32 staticAlias) const
{
	std::map<std::string, uint32>::const_iterator first(_StaticAliasFileMapping.begin()), last(_StaticAliasFileMapping.end());

	for (; first != last; ++first)
	{
		if (first->second == staticAlias)
			return first->first;
	}
	static string emptyString;

	return emptyString;
}


// ***************************************************************************

uint32 CLigoConfig::getFileStaticAliasMapping(const std::string &fileName) const
{
	std::map<std::string, uint32>::const_iterator it(_StaticAliasFileMapping.find(fileName));

	if (it != _StaticAliasFileMapping.end())
	{
		return it->second;
	}
	else
		// no mapping defined.
		return 0;
}

// ***************************************************************************

bool CLigoConfig::isFileStaticAliasMapped(const std::string &fileName) const
{
	std::map<std::string, uint32>::const_iterator it(_StaticAliasFileMapping.find(fileName));

	if (it != _StaticAliasFileMapping.end())
	{
		return true;
	}
	else
		// no mapping defined.
		return false;
}


std::string CLigoConfig::aliasToString(uint32 fullAlias)
{
	uint32 staticPart;
	uint32 dynPart;

	staticPart = (fullAlias & getStaticAliasMask())>>getDynamicAliasSize();
	dynPart = fullAlias & getDynamicAliasMask();

	return toString("(A:%u:%u)", staticPart, dynPart);

}

uint32 CLigoConfig::aliasFromString(std::string fullAlias)
{
	uint32 staticPart;
	uint32 dynPart;
	sscanf(fullAlias.c_str(), "(A:%u:%u)", &staticPart, &dynPart);

	return ((staticPart<<getDynamicAliasSize()) & getStaticAliasMask()) | (dynPart & getDynamicAliasMask());

}


void CLigoConfig::updateDynamicAliasBitCount(uint32 newDynamicAliasBitCount)
{
	sint32 diff = _DynamicAliasBitCount - newDynamicAliasBitCount;

	if (diff <= 0)
	{
		nlwarning("New bit count must be less than previous");
		nlassert(0);

	}

	std::map<std::string, uint32>::iterator first(_StaticAliasFileMapping.begin()), last(_StaticAliasFileMapping.end());
	for ( ; first != last; ++first)
	{
		first->second = first->second  << diff;
	}
	_DynamicAliasBitCount = newDynamicAliasBitCount;
}


}



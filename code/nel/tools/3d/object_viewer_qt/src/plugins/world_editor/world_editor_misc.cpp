// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// Project includes
#include "world_editor_misc.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/o_xml.h>
#include <nel/ligo/primitive_utils.h>
#include <nel/ligo/ligo_config.h>


// Qt includes

namespace WorldEditor
{
namespace Utils
{

void syntaxError(const char *filename, xmlNodePtr xmlNode, const char *format, ...)
{
	char buffer[1024];

	if (format)
	{
		va_list args;
		va_start( args, format );
		sint ret = vsnprintf( buffer, 1024, format, args );
		va_end( args );
	}
	else
	{
		strcpy(buffer, "Unknown error");
	}

	nlerror("(%s), node (%s), line (%s) :\n%s", filename, xmlNode->name, xmlNode->content, buffer);
}

bool getPropertyString(std::string &result, const char *filename, xmlNodePtr xmlNode, const char *propName)
{
	// Call the CIXml version
	if (!NLMISC::CIXml::getPropertyString(result, xmlNode, propName))
	{
		// Output a formated error
		syntaxError(filename, xmlNode, "Missing XML node property (%s)", propName);
		return false;
	}
	return true;
}

uint32 getUniqueId()
{
	// Wait 1 ms
	sint64 time = NLMISC::CTime::getLocalTime ();
	sint64 time2;
	while ((time2 = NLMISC::CTime::getLocalTime ()) == time)
	{
	}

	return (uint32)time2;
}

bool loadWorldEditFile(const std::string &fileName, WorldEditList &worldEditList)
{
	bool result = false;

	// Load the document
	NLMISC::CIFile file;
	if (file.open(fileName))
	{
		try
		{
			// Load the document in XML
			NLMISC::CIXml xml;
			xml.init(file);

			// Get root node
			xmlNodePtr rootNode = xml.getRootNode();
			if (rootNode)
			{
				// Good header ?
				if (strcmp((const char *)(rootNode->name), "NEL_WORLD_EDITOR_PROJECT") == 0)
				{
					int version = -1;

					// Read the parameters
					xmlNodePtr node = NLMISC::CIXml::getFirstChildNode(rootNode, "VERSION");
					if (node)
					{
						std::string versionString;
						if (NLMISC::CIXml::getContentString (versionString, node))
							version = atoi(versionString.c_str ());
					}

					if (version == -1)
						syntaxError(fileName.c_str(), rootNode, "No version node");
					else
					{
						// Old format,
						if (version <= 1)
						{
							syntaxError(fileName.c_str(), rootNode, "Old version node");
						}
						else
						{
							// Read it
							if (version > WORLD_EDITOR_FILE_VERSION)
							{
								syntaxError(fileName.c_str(), node, "Unknown file version");
							}
							else
							{
								// Read data directory
								node = NLMISC::CIXml::getFirstChildNode(rootNode, "DATA_DIRECTORY");
								if (node)
								{
									std::string dataDir;
									NLMISC::CIXml::getPropertyString(dataDir, node, "VALUE");
									worldEditList.push_back(WorldEditItem(DataDirectoryType, dataDir));
								}

								// Read data directory
								node = NLMISC::CIXml::getFirstChildNode(rootNode, "CONTEXT");
								if (node)
								{
									std::string context;
									NLMISC::CIXml::getPropertyString(context, node, "VALUE");
									worldEditList.push_back(WorldEditItem(ContextType, context));
								}

								// Read the database element
								node = NLMISC::CIXml::getFirstChildNode(rootNode, "DATABASE_ELEMENT");
								if (node)
								{
									do
									{
										// Get the type
										std::string type;
										if (getPropertyString(type, fileName.c_str(), node, "TYPE"))
										{
											// Read the filename
											std::string filenameChild;
											if (getPropertyString(filenameChild, fileName.c_str(), node, "FILENAME"))
											{
												// Is it a landscape ?
												if (type == "landscape")
												{
													worldEditList.push_back(WorldEditItem(LandscapeType, filenameChild));

													// Get the primitives
													xmlNodePtr primitives = NLMISC::CIXml::getFirstChildNode(node, "PRIMITIVES");
													if (primitives)
													{
														NLLIGO::CPrimitives ligoPrimitives;

														// Read it
														ligoPrimitives.read(primitives, fileName.c_str(), *ligoConfig());
														//_DataHierarchy.back ().Primitives.read (primitives, filename, theApp.Config);

														// Set the filename
														//_DataHierarchy.back ().Filename = filenameChild;
													}
												}
												else
												{
													worldEditList.push_back(WorldEditItem(PrimitiveType, filenameChild));
												}

											}
										}
									}
									while (node = NLMISC::CIXml::getNextChildNode(node, "DATABASE_ELEMENT"));
								}

								// Done
								result = true;
							}
						}
					}
				}
				else
				{
					// Error
					syntaxError(fileName.c_str(), rootNode, "Unknown file header : %s", rootNode->name);
				}
			}
		}
		catch (NLMISC::Exception &e)
		{
			nlerror("Error reading file %s : %s", fileName.c_str(), e.what());
		}
	}
	else
		nlerror("Can't open the file %s for reading.", fileName.c_str());

	return result;
}

NLLIGO::IPrimitive *getRootPrimitive(NLLIGO::IPrimitive *primitive)
{
	nlassert(primitive);

	if (primitive->getParent() == NULL)
		return primitive;
	else
		return getRootPrimitive(primitive->getParent());
}

void initPrimitiveParameters(const NLLIGO::CPrimitiveClass &primClass, NLLIGO::IPrimitive &primitive,
							 const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters)
{
	// Other parameters
	for (uint p = 0; p < initParameters.size(); ++p)
	{
		// The property
		const NLLIGO::CPrimitiveClass::CInitParameters &parameter = initParameters[p];

		// Look for it in the class
		uint cp;
		for (cp = 0; cp < primClass.Parameters.size(); ++cp)
		{
			// Good one ?
			if (primClass.Parameters[cp].Name == initParameters[p].Name)
				break;
		}

		// The primitive type
		NLLIGO::CPrimitiveClass::CParameter::TType type;

		// Found ?
		if (cp < primClass.Parameters.size())
			type = primClass.Parameters[cp].Type;

		if (initParameters[p].Name == "name")
			type = NLLIGO::CPrimitiveClass::CParameter::String;

		if (cp < primClass.Parameters.size () || (initParameters[p].Name == "name"))
		{
			// Default value ?
			if (!parameter.DefaultValue.empty())
			{
				// Type of property
				switch (type)
				{
				case NLLIGO::CPrimitiveClass::CParameter::Boolean:
				case NLLIGO::CPrimitiveClass::CParameter::ConstString:
				case NLLIGO::CPrimitiveClass::CParameter::String:
				{
					// Some feedback
					if (parameter.DefaultValue.size() > 1)
						nlerror("Warning: parameter (%s) in class name (%s) has more than 1 default value (%d).",
								parameter.Name.c_str(), primClass.Name.c_str(), parameter.DefaultValue.size());

					if ((cp < primClass.Parameters.size() && !primClass.Parameters[cp].Visible)
							|| parameter.DefaultValue[0].GenID)
					{
						// Remove this property
						primitive.removePropertyByName(parameter.Name.c_str());

						// Add this property
						primitive.addPropertyByName(parameter.Name.c_str(),
													new NLLIGO::CPropertyString((parameter.DefaultValue[0].GenID ? NLMISC::toString(getUniqueId()) : "").c_str ()));
					}
					break;
				}
				case NLLIGO::CPrimitiveClass::CParameter::ConstStringArray:
				case NLLIGO::CPrimitiveClass::CParameter::StringArray:
				{
					bool Visible = false;
					if (cp < primClass.Parameters.size() && !primClass.Parameters[cp].Visible)
						Visible = true;
					for (size_t i = 0; i < parameter.DefaultValue.size(); ++i)
					{
						// Generate a unique id ?
						if (parameter.DefaultValue[i].GenID)
							Visible = true;
					}
					if (Visible)
					{
						// Remove this property
						primitive.removePropertyByName (parameter.Name.c_str());

						// Add this property
						NLLIGO::CPropertyStringArray *str = new NLLIGO::CPropertyStringArray();
						str->StringArray.resize (parameter.DefaultValue.size());
						for (size_t i = 0; i < parameter.DefaultValue.size(); ++i)
						{
							// Generate a unique id ?
							if (parameter.DefaultValue[i].GenID)
								str->StringArray[i] = NLMISC::toString(getUniqueId());
							else
								str->StringArray[i] = "";
						}
						primitive.addPropertyByName(parameter.Name.c_str(), str);
					}
					break;
				}
				}
			}
		}
		else
		{
			// Some feedback
			nlerror("Warning: parameter (%s) doesn't exist in class (%s).",
					initParameters[p].Name.c_str(), primClass.Name.c_str());
		}
	}
}

NLLIGO::IPrimitive *createPrimitive(const char *className, const char *primName,
									const NLMISC::CVector &initPos, float deltaPos,
									const std::vector<NLLIGO::CPrimitiveClass::CInitParameters> &initParameters,
									NLLIGO::IPrimitive *parent)
{
	// Get the prim class
	const NLLIGO::CPrimitiveClass *primClass = ligoConfig()->getPrimitiveClass(className);
	if (primClass)
	{
		// Create the base primitive
		NLLIGO::IPrimitive *primitive = NULL;
		switch (primClass->Type)
		{
		case NLLIGO::CPrimitiveClass::Node:
			primitive = new NLLIGO::CPrimNode;
			break;
		case NLLIGO::CPrimitiveClass::Point:
		{
			NLLIGO::CPrimPoint *point = new NLLIGO::CPrimPoint;
			primitive = point;
			point->Point.CVector::operator = (initPos);
		}
		break;
		case NLLIGO::CPrimitiveClass::Path:
		{
			NLLIGO::CPrimPath *path = new NLLIGO::CPrimPath;
			primitive = path;
			path->VPoints.push_back(NLLIGO::CPrimVector(initPos));
			NLMISC::CVector secondPos = NLMISC::CVector(initPos.x + deltaPos, initPos.y, 0.0);
			path->VPoints.push_back(NLLIGO::CPrimVector(secondPos));
			break;
		}
		case NLLIGO::CPrimitiveClass::Zone:
		{
			NLLIGO::CPrimZone *zone = new NLLIGO::CPrimZone;
			primitive = zone;
			zone->VPoints.push_back(NLLIGO::CPrimVector(initPos));
			NLMISC::CVector secondPos = NLMISC::CVector(initPos.x + deltaPos, initPos.y, 0.0);
			zone->VPoints.push_back(NLLIGO::CPrimVector(secondPos));
			secondPos.y = initPos.y + deltaPos;
			zone->VPoints.push_back(NLLIGO::CPrimVector(secondPos));
			break;
		}
		case NLLIGO::CPrimitiveClass::Alias:
			primitive = new NLLIGO::CPrimAlias;
			break;
		case NLLIGO::CPrimitiveClass::Bitmap:
			primitive = new NLLIGO::CPrimNode;
			break;
		}
		nlassert(primitive);

		// Add properties
		primitive->addPropertyByName("class", new NLLIGO::CPropertyString(className));
		primitive->addPropertyByName("name", new NLLIGO::CPropertyString(primName, primName[0] == 0));

		// Init with default parameters
		std::vector<NLLIGO::CPrimitiveClass::CInitParameters> tempParam;
		tempParam.reserve(primClass->Parameters.size());
		for (size_t i = 0; i < primClass->Parameters.size(); i++)
			tempParam.push_back (primClass->Parameters[i]);
		initPrimitiveParameters (*primClass, *primitive, tempParam);

		// Init with option parameters
		initPrimitiveParameters(*primClass, *primitive, initParameters);

		parent->insertChild(primitive);
		/*
					// Insert the primitive
					insertPrimitive (locator, primitive);
		*/
		// The new pos
		NLMISC::CVector newPos = initPos;
		newPos.x += deltaPos;

		// Create static children
		uint c;
		for (c = 0; c < primClass->StaticChildren.size(); c++)
		{
			// The child ref
			const NLLIGO::CPrimitiveClass::CChild &child = primClass->StaticChildren[c];

			// Create the child
			const NLLIGO::IPrimitive *childPrim = createPrimitive(child.ClassName.c_str(), child.Name.c_str(),
												  newPos, deltaPos, primClass->StaticChildren[c].Parameters, primitive);

			// The new pos
			newPos.y += deltaPos;
		}

		// Canceled ?
		if (c < primClass->StaticChildren.size())
		{
			deletePrimitive(primitive);
			return NULL;
		}

		if (primitive)
		{
			if (!primClass->AutoInit)
			{
				// TODO
			}

			// Eval the default name property
			std::string name;
			if (!primitive->getPropertyByName ("name", name) || name.empty())
			{
				const NLLIGO::CPrimitiveClass *primClass = ligoConfig()->getPrimitiveClass(*primitive);
				if (primClass)
				{
					for (size_t i = 0; i < primClass->Parameters.size(); ++i)
					{
						if (primClass->Parameters[i].Name == "name")
						{
							std::string result;
							primClass->Parameters[i].getDefaultValue(result, *primitive, *primClass, NULL);
							if (!result.empty())
							{
								primitive->removePropertyByName("name");
								primitive->addPropertyByName("name", new NLLIGO::CPropertyString(result.c_str(), true));
							}
						}
					}
				}
			}

			primitive->initDefaultValues(*ligoConfig());
		}
		return primitive;
	}
	else
		nlerror("Unknown primitive class name : %s", className);

	return 0;
}

void deletePrimitive(NLLIGO::IPrimitive *primitive)
{
	// Get the parent
	NLLIGO::IPrimitive *parent = primitive->getParent();
	nlassert(parent);

	// Get the child id
	uint childId;
	nlverify(parent->getChildId(childId, primitive));

	// Delete the child
	nlverify(parent->removeChild(childId));
}

bool updateDefaultValues(NLLIGO::IPrimitive *primitive)
{
	bool modified = false;

	// Get the prim class
	const NLLIGO::CPrimitiveClass *primClass = ligoConfig()->getPrimitiveClass(*primitive);
	nlassert(primClass);

	if (primClass)
	{
		// For each parameters
		for (uint i = 0; i < primClass->Parameters.size(); i++)
		{
			// First check the primitive property has to good type
			NLLIGO::IProperty *prop;
			if (primitive->getPropertyByName(primClass->Parameters[i].Name.c_str(), prop))
			{
				// String to array ?
				NLLIGO::CPropertyString *propString = dynamic_cast<NLLIGO::CPropertyString *>(prop);
				const bool classStringArray = primClass->Parameters[i].Type == NLLIGO::CPrimitiveClass::CParameter::StringArray ||
											  primClass->Parameters[i].Type == NLLIGO::CPrimitiveClass::CParameter::ConstStringArray;
				if (propString && classStringArray)
				{
					// Build an array string
					std::vector<std::string> strings;
					if (!propString->String.empty())
						strings.push_back(propString->String);
					prop = new NLLIGO::CPropertyStringArray(strings);
					primitive->removePropertyByName(primClass->Parameters[i].Name.c_str());
					primitive->addPropertyByName(primClass->Parameters[i].Name.c_str(), prop);
					modified = true;
				}

				// Array to string ?
				NLLIGO::CPropertyStringArray *propStringArray = dynamic_cast<NLLIGO::CPropertyStringArray *>(prop);
				if (propStringArray && !classStringArray)
				{
					// Build an array string
					std::string str;
					if (!propStringArray->StringArray.empty())
						str = propStringArray->StringArray[0];
					prop = new NLLIGO::CPropertyString(str);
					primitive->removePropertyByName(primClass->Parameters[i].Name.c_str());
					primitive->addPropertyByName(primClass->Parameters[i].Name.c_str(), prop);
					modified = true;
				}
			}

			// String or string array ?
			if (primClass->Parameters[i].Type == NLLIGO::CPrimitiveClass::CParameter::String)
			{
				// Default value available ?
				if (!primClass->Parameters[i].DefaultValue.empty ())
				{
					// Unique Id ?
					if (primClass->Parameters[i].DefaultValue[0].GenID)
					{
						// The doesn't exist ?
						std::string result;
						if (!primitive->getPropertyByName(primClass->Parameters[i].Name.c_str(), result))
						{
							// Add it !
							primitive->addPropertyByName(primClass->Parameters[i].Name.c_str(), new NLLIGO::CPropertyString(NLMISC::toString(getUniqueId()).c_str()));
							modified = true;
						}
					}
					// Hidden ?
					else if (!primClass->Parameters[i].Visible)
					{
						// The doesn't exist ?
						std::string result;
						if (!primitive->getPropertyByName (primClass->Parameters[i].Name.c_str (), result))
						{
							// Add it !
							primitive->addPropertyByName (primClass->Parameters[i].Name.c_str (), new NLLIGO::CPropertyString (""));
							modified = true;
						}
					}
				}
			}
			else if ((primClass->Parameters[i].Type == NLLIGO::CPrimitiveClass::CParameter::StringArray) ||
					 (primClass->Parameters[i].Type == NLLIGO::CPrimitiveClass::CParameter::ConstStringArray))
			{
				for (uint j = 0; j < primClass->Parameters[i].DefaultValue.size(); j++)
				{
					// Unique Id ?
					if (primClass->Parameters[i].DefaultValue[j].GenID)
					{
						// The doesn't exist ?
						std::vector<std::string> result;
						std::vector<std::string> *resultPtr = NULL;
						if (!primitive->getPropertyByName(primClass->Parameters[i].Name.c_str(), resultPtr) ||
								(resultPtr->size() <= j))
						{
							// Copy
							if (resultPtr)
								result = *resultPtr;

							// Resize
							if (result.size() <= j)
								result.resize(j + 1);

							// Resize to it
							primitive->removePropertyByName(primClass->Parameters[i].Name.c_str());

							// Set the value
							result[j] = NLMISC::toString(getUniqueId());

							// Add the new property array
							primitive->addPropertyByName(primClass->Parameters[i].Name.c_str(), new NLLIGO::CPropertyStringArray(result));
							modified = true;
						}
					}
					// Hidden ?
					else if (!primClass->Parameters[i].Visible)
					{
						// The doesn't exist ?
						std::vector<std::string> result;
						std::vector<std::string> *resultPtr = NULL;
						if (!primitive->getPropertyByName(primClass->Parameters[i].Name.c_str(), resultPtr) || (resultPtr->size () <= j))
						{
							// Copy
							if (resultPtr)
								result = *resultPtr;

							// Resize
							if (result.size() <= j)
								result.resize(j + 1);

							// Resize to it
							primitive->removePropertyByName(primClass->Parameters[i].Name.c_str());

							// Set the value
							result[j] = "";

							// Add the new property array
							primitive->addPropertyByName(primClass->Parameters[i].Name.c_str(), new NLLIGO::CPropertyStringArray(result));
							modified = true;
						}
					}
				}
			}
			else
			{
				// Default value available ?
				if (!primClass->Parameters[i].DefaultValue.empty ())
				{
					// Hidden ?
					if (!primClass->Parameters[i].Visible)
					{
						// The doesn't exist ?
						std::string result;
						if (!primitive->getPropertyByName(primClass->Parameters[i].Name.c_str(), result))
						{
							// Add it !
							primitive->addPropertyByName(primClass->Parameters[i].Name.c_str(), new NLLIGO::CPropertyString(""));
							modified = true;
						}
					}
				}
			}
		}
	}
	return modified;
}

bool recursiveUpdateDefaultValues(NLLIGO::IPrimitive *primitive)
{
	bool modified = updateDefaultValues(primitive);

	const uint count = primitive->getNumChildren();
	for (uint i = 0; i < count; ++i)
	{
		// Get the child
		NLLIGO::IPrimitive *child;
		nlverify(primitive->getChild(child, i));
		modified |= recursiveUpdateDefaultValues(child);
	}

	return modified;
}

NLLIGO::CLigoConfig	*ligoConfig()
{
	return NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig;
}

} /* namespace Utils */
} /* namespace WorldEditor */

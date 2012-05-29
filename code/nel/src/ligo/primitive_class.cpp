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
#include "nel/ligo/primitive_class.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

// ***************************************************************************

CPrimitiveClass::CPrimitiveClass()
{
}

// ***************************************************************************

bool ReadFloat (const char *propName, float &result, xmlNodePtr xmlNode)
{
	string value;
	if (CIXml::getPropertyString (value, xmlNode, propName))
	{
		result = (float)atof (value.c_str ());
		return true;
	}
	return false;
}

// ***************************************************************************

bool ReadInt (const char *propName, int &result, xmlNodePtr xmlNode)
{
	string value;
	if (CIXml::getPropertyString (value, xmlNode, propName))
	{
		result = atoi (value.c_str ());
		return true;
	}
	return false;
}

// ***************************************************************************

bool ReadBool (const char *propName, bool &result, xmlNodePtr xmlNode, const char *filename, CLigoConfig &config)
{
	string str;
	if (CIXml::getPropertyString (str, xmlNode, propName))
	{
		if (str == "true")
			result = true;
		else if (str == "false")
			result = false;
		else
		{
			config.syntaxError (filename, xmlNode, "Unknown (%s) parameter (%s), should be false or true", propName, str.c_str ());
			return false;
		}
		return true;
	}
	return false;
}

// ***************************************************************************

bool ReadColor (CRGBA &color, xmlNodePtr node)
{
	// Read the color
	float r = DEFAULT_PRIMITIVE_COLOR.R;
	float g = DEFAULT_PRIMITIVE_COLOR.G;
	float b = DEFAULT_PRIMITIVE_COLOR.B;
	float a = DEFAULT_PRIMITIVE_COLOR.A;

	// Read the value
	if (!ReadFloat ("R", r, node))
		return false;
	if (!ReadFloat ("G", g, node))
		return false;
	if (!ReadFloat ("B", b, node))
		return false;
	if (!ReadFloat ("A", a, node))
		a = 255;

	// Clamp
	clamp (r, 0.f, 255.f);
	clamp (g, 0.f, 255.f);
	clamp (b, 0.f, 255.f);
	clamp (a, 0.f, 255.f);

	// Set
	color.set((uint8)r, (uint8)g, (uint8)b, (uint8)a);
	return true;
}

// ***************************************************************************

bool ReadChild (CPrimitiveClass::CChild &child, xmlNodePtr childNode, const char *filename, bool _static, CLigoConfig &config)
{
	// Read the class name
	if (!config.getPropertyString (child.ClassName, filename, childNode, "CLASS_NAME"))
		goto failed;

	// Read the name
	if (!_static || config.getPropertyString (child.Name, filename, childNode, "NAME"))
	{
		// Read the parameters
		child.Parameters.reserve (CIXml::countChildren (childNode, "PARAMETER"));
		for (	xmlNodePtr childParamNode = CIXml::getFirstChildNode (childNode, "PARAMETER");
				childParamNode != NULL;
				childParamNode = CIXml::getNextChildNode (childParamNode, "PARAMETER"))
		{
			// Add a static child
			child.Parameters.push_back (CPrimitiveClass::CInitParameters ());

			// Child ref
			CPrimitiveClass::CInitParameters &childParam = child.Parameters.back ();

			// Read the class name
			if (!config.getPropertyString (childParam.Name, filename, childParamNode, "NAME"))
				goto failed;

			// Read the parameters
			uint defaultId = 0;
			childParam.DefaultValue.resize (CIXml::countChildren (childParamNode, "DEFAULT_VALUE"));
			for (	xmlNodePtr childParamValueNode = CIXml::getFirstChildNode (childParamNode, "DEFAULT_VALUE");
					childParamValueNode != NULL;
					childParamValueNode = CIXml::getNextChildNode (childParamValueNode, "DEFAULT_VALUE"))
			{
				// Gen id flag
				childParam.DefaultValue[defaultId].GenID = false;

				// Read the gen id flag
				string value;
				if (CIXml::getPropertyString (value, childParamValueNode, "GEN_ID") && (value != "false"))
				{
					childParam.DefaultValue[defaultId].GenID = true;
				}
				else
				{
					if (!config.getPropertyString (value, filename, childParamValueNode, "VALUE"))
						goto failed;

					childParam.DefaultValue[defaultId].Name = value;
				}
				defaultId++;
			}
		}

		// Ok
		return true;
	}
failed:
	return false;
}

// ***************************************************************************

bool CPrimitiveClass::read (xmlNodePtr primitiveNode,
							const char *filename,
							const char *className,
							std::set<std::string> &contextStrings,
							std::map<std::string, std::string> &contextFilesLookup,
							CLigoConfig &config,
							bool parsePrimitiveComboContent)
{
	//	init default parameters
	AutoInit = false;
	Deletable = true;
	FileExtension = "";
	FileType = "";
	Collision = false;
	LinkBrothers = false;
	ShowArrow = true;
	Numberize = true;
	Visible = true;

	// read parent class properties
	string parentClass;
	if (CIXml::getPropertyString (parentClass, primitiveNode, "PARENT_CLASS"))
	{
		const CPrimitiveClass *parent = config.getPrimitiveClass(parentClass.c_str());

		if (parent == NULL)
		{
			config.syntaxError (filename, primitiveNode, "Can't find parent class (%s) for class (%s)", parentClass.c_str (), className);
			return false;
		}

		// copy all the properties
		*this = *parent;
	}

	// The name
	Name = className;

	// Read the type
	std::string type;
	if (!config.getPropertyString (type, filename, primitiveNode, "TYPE"))
		goto failed;

	// Good type ?
	if (type == "node")
		Type = Node;
	else if (type == "point")
		Type = Point;
	else if (type == "path")
		Type = Path;
	else if (type == "zone")
		Type = Zone;
	else if (type == "bitmap")
		Type = Bitmap;
	else if (type == "alias")
		Type = Alias;
	else
	{
		config.syntaxError (filename, primitiveNode, "Unknown primitive type (%s)", type.c_str ());
		goto failed;
	}

	// Read the color
	ReadColor (Color, primitiveNode);

	// Autoinit
	ReadBool ("AUTO_INIT", AutoInit, primitiveNode, filename, config);

	// Deletable
	ReadBool ("DELETABLE", Deletable, primitiveNode, filename, config);

	// File extension
	CIXml::getPropertyString (FileExtension, primitiveNode, "FILE_EXTENSION");

	// File type
	CIXml::getPropertyString (FileType, primitiveNode, "FILE_TYPE");

	// Collision
	ReadBool ("COLLISION", Collision, primitiveNode, filename, config);

	// LinkBrothers
	ReadBool ("LINK_BROTHERS", LinkBrothers, primitiveNode, filename, config);

	// ShowArrow
	ReadBool ("SHOW_ARROW", ShowArrow, primitiveNode, filename, config);

	// Numberize when copy the primitive
	ReadBool ("NUMBERIZE", Numberize, primitiveNode, filename, config);

	// Visible ?
	ReadBool ("VISIBLE", Visible, primitiveNode, filename, config);

	// Read the parameters
	for (	xmlNodePtr paramNode = CIXml::getFirstChildNode (primitiveNode, "PARAMETER");
			paramNode != NULL;
			paramNode = CIXml::getNextChildNode (paramNode, "PARAMETER"))
	{
		// Read the property name
		if (!config.getPropertyString (type, filename, paramNode, "NAME"))
			goto failed;

		// look if the parameter is not already defined by the parent class
		uint i=0;
		while (i<Parameters.size())
		{
			if (Parameters[i].Name == type)
			{
				// the param already exist, remove parent param
				Parameters.erase(Parameters.begin() + i);
				continue;
			}
			++i;
		}

		// Add a parameter
		Parameters.push_back (CParameter ());

		// The parameter ref
		CParameter &parameter = Parameters.back ();

		// Set the name
		parameter.Name = type;

		// Read the type
		if (!config.getPropertyString (type, filename, paramNode, "TYPE"))
			goto failed;

		// Good type ?
		if (type == "boolean")
			parameter.Type = CParameter::Boolean;
		else if (type == "const_string")
			parameter.Type = CParameter::ConstString;
		else if (type == "string")
			parameter.Type = CParameter::String;
		else if (type == "string_array")
			parameter.Type = CParameter::StringArray;
		else if (type == "const_string_array")
			parameter.Type = CParameter::ConstStringArray;
		else
		{
			config.syntaxError (filename, paramNode, "Unknown primitive parameter type (%s)", type.c_str ());
			goto failed;
		}

		// Visible
		parameter.Visible = true;
		ReadBool ("VISIBLE", parameter.Visible, paramNode, filename, config);

		// Filename
		parameter.Filename = false;
		ReadBool ("FILENAME", parameter.Filename, paramNode, filename, config);

		// Lookup
		parameter.Lookup = false;
		ReadBool ("LOOKUP", parameter.Lookup, paramNode, filename, config);

		// Read only primitive
		parameter.ReadOnly = false;
		ReadBool ("READ_ONLY", parameter.ReadOnly, paramNode, filename, config);

		// Deletable
		parameter.Editable = false;
		ReadBool ("EDITABLE", parameter.Editable, paramNode, filename, config);

		// sort combo box entries
		parameter.SortEntries = false;
		ReadBool ("SORT_ENTRIES", parameter.SortEntries, paramNode, filename, config);

		// Display horizontal scroller in multi-line edit box
		parameter.DisplayHS = false;
		ReadBool ("SHOW_HS", parameter.DisplayHS, paramNode, filename, config);

		// Lookup
		parameter.WidgetHeight = 100;
		int temp;
		if (ReadInt ("WIDGET_HEIGHT", temp, paramNode))
			parameter.WidgetHeight = (uint)temp;

		// Read the file extension
		parameter.FileExtension = "";
		CIXml::getPropertyString (parameter.FileExtension, paramNode, "FILE_EXTENSION");
		parameter.FileExtension = toLower(parameter.FileExtension);

		// Autonaming preference
		parameter.Autoname = "";
		CIXml::getPropertyString (parameter.Autoname, paramNode, "AUTONAME");

		// Read the file extension
		parameter.Folder = "";
		CIXml::getPropertyString (parameter.Folder, paramNode, "FOLDER");
		parameter.Folder = toLower(parameter.Folder);

		// Read the combo values
		for (	xmlNodePtr comboValueNode = CIXml::getFirstChildNode (paramNode, "COMBO_VALUES");
				comboValueNode != NULL;
				comboValueNode = CIXml::getNextChildNode (comboValueNode, "COMBO_VALUES"))
		{
			// Read the context
			if (!config.getPropertyString (type, filename, comboValueNode, "CONTEXT_NAME"))
				goto failed;

			// Add this context
			contextStrings.insert (type);

			// Add a combo value
			pair<std::map<std::string, CParameter::CConstStringValue>::iterator, bool> insertResult =
				parameter.ComboValues.insert (std::map<std::string, CParameter::CConstStringValue>::value_type (type, CParameter::CConstStringValue ()));

			// The combo value ref
			CParameter::CConstStringValue &comboValue = insertResult.first->second;

			// Read the values
			for (	xmlNodePtr comboValueValueNode = CIXml::getFirstChildNode (comboValueNode, "CONTEXT_VALUE");
					comboValueValueNode != NULL;
					comboValueValueNode = CIXml::getNextChildNode (comboValueValueNode, "CONTEXT_VALUE"))
			{
				// Read the value
				if (!config.getPropertyString (type, filename, comboValueValueNode, "VALUE"))
					goto failed;

				comboValue.Values.push_back (type);
			}
		}

		// Read the combo files
		for (	xmlNodePtr comboValueNode = CIXml::getFirstChildNode (paramNode, "COMBO_FILES");
				comboValueNode != NULL;
				comboValueNode = CIXml::getNextChildNode (comboValueNode, "COMBO_FILES"))
		{
			// Read the context
			if (!config.getPropertyString (type, filename, comboValueNode, "CONTEXT_NAME"))
				goto failed;

			// Read the path to search
			string path;
			if	(CIXml::getPropertyString (path, comboValueNode, "PATH"))
			{
				if (!parsePrimitiveComboContent)
					continue;

				// Look for files in the path
				std::vector<std::string> files;
				CPath::getPathContent (path, true, false, true, files);

				// Not empty ?
				if (files.empty ())
					continue;

				// Add this context
				contextStrings.insert (type);

				// For each file
				for (uint i=0; i<files.size (); i++)
				{
					// Good extension ?
					if (toLower(NLMISC::CFile::getExtension (files[i])) != parameter.FileExtension)
						continue;

					// Add a combo value
					pair<std::map<std::string, CParameter::CConstStringValue>::iterator, bool> insertResult =
						parameter.ComboValues.insert (std::map<std::string, CParameter::CConstStringValue>::value_type (type, CParameter::CConstStringValue ()));

					// The combo value ref
					CParameter::CConstStringValue &comboValue = insertResult.first->second;

					// Get the filename without extension
					string nameWithoutExt = toLower(NLMISC::CFile::getFilenameWithoutExtension (files[i]));

					// Add the values
					comboValue.Values.push_back (nameWithoutExt);

					// Add the value for lookup
					contextFilesLookup.insert (map<string, string>::value_type (nameWithoutExt, files[i]));
				}
			}
			else
			{
				string	primpath;
				if	(!config.getPropertyString (primpath, filename, comboValueNode, "PRIM_PATH"))
					goto failed;

				// Add this context
				contextStrings.insert (type);

				// Add a combo value
				pair<std::map<std::string, CParameter::CConstStringValue>::iterator, bool> insertResult =
					parameter.ComboValues.insert (std::map<std::string, CParameter::CConstStringValue>::value_type (type, CParameter::CConstStringValue ()));

				// The combo value ref
				CParameter::CConstStringValue &comboValue = insertResult.first->second;

				comboValue.PrimitivePath.push_back(primpath);
			}
		}

		// Read parameters default values
		uint defaultId = 0;
		parameter.DefaultValue.resize (CIXml::countChildren (paramNode, "DEFAULT_VALUE"));
		for (	xmlNodePtr defaultValueNode = CIXml::getFirstChildNode (paramNode, "DEFAULT_VALUE");
				defaultValueNode != NULL;
				defaultValueNode = CIXml::getNextChildNode (defaultValueNode, "DEFAULT_VALUE"))
		{
			// Gen id flag
			parameter.DefaultValue[defaultId].GenID = false;

			// Read the gen id flag
			string value;
			if (CIXml::getPropertyString (value, defaultValueNode, "GEN_ID") && (value != "false"))
			{
				parameter.DefaultValue[defaultId].GenID = true;
			}
			else
			{
				if (!config.getPropertyString (value, filename, defaultValueNode, "VALUE"))
					goto failed;
				parameter.DefaultValue[defaultId].Name = value;
			}
			defaultId++;
		}
	}

	// Read static children
	StaticChildren.reserve (StaticChildren.size() + CIXml::countChildren (primitiveNode, "STATIC_CHILD"));
	for (	xmlNodePtr childrenNode = CIXml::getFirstChildNode (primitiveNode, "STATIC_CHILD");
			childrenNode != NULL;
			childrenNode = CIXml::getNextChildNode (childrenNode, "STATIC_CHILD"))
	{
		// Add a static child
		StaticChildren.push_back (CChild ());

		// Child ref
		CChild &child = StaticChildren.back ();

		// Read the child
		if (!ReadChild (child, childrenNode, filename, true, config))
			goto failed;
	}

	// Read dynamic children
	DynamicChildren.reserve (DynamicChildren.size() + CIXml::countChildren (primitiveNode, "DYNAMIC_CHILD"));
	for (	xmlNodePtr childrenNode = CIXml::getFirstChildNode (primitiveNode, "DYNAMIC_CHILD");
			childrenNode != NULL;
			childrenNode = CIXml::getNextChildNode (childrenNode, "DYNAMIC_CHILD"))
	{
		// Add a static child
		DynamicChildren.push_back (CChild ());

		// Child ref
		CChild &child = DynamicChildren.back ();

		// Read the child
		if (!ReadChild (child, childrenNode, filename, false, config))
			goto failed;
	}

	// Read generated children
	GeneratedChildren.reserve (GeneratedChildren.size() + CIXml::countChildren (primitiveNode, "GENERATED_CHILD"));
	for (	xmlNodePtr childrenNode = CIXml::getFirstChildNode (primitiveNode, "GENERATED_CHILD");
			childrenNode != NULL;
			childrenNode = CIXml::getNextChildNode (childrenNode, "GENERATED_CHILD"))
	{
		// Add a static child
		GeneratedChildren.push_back (CChild ());

		// Child ref
		CChild &child = GeneratedChildren.back ();

		// Read the child
		if (!ReadChild (child, childrenNode, filename, false, config))
			goto failed;
	}

	return true;
failed:
	return false;
}

// ***************************************************************************

CPrimitiveClass::CParameter::CParameter (const NLLIGO::IProperty &property, const char *propertyName)
{
	Name = propertyName;
	Filename = false;
	Visible = true;
	Type = (typeid (property) == typeid (CPropertyString)) ? CPrimitiveClass::CParameter::String : CPrimitiveClass::CParameter::StringArray;
}

// ***************************************************************************
// CPrimitiveClass::CParameter
// ***************************************************************************

bool CPrimitiveClass::CParameter::operator== (const CParameter &other) const
{
	return (Type == other.Type) &&
		(Name == other.Name) &&
		(Visible == other.Visible) &&
		(Filename == other.Filename) &&
		(ComboValues == other.ComboValues) &&
		(DefaultValue == other.DefaultValue);
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::operator< (const CParameter &other) const
{
	return (Name < other.Name) ? true : (Name > other.Name) ? false :
		(Type < other.Type) ? true : (Type > other.Type) ? false :
		(Visible < other.Visible) ? true : (Visible > other.Visible) ? false :
		(Filename < other.Filename) ? true : (Filename > other.Filename) ? false :
		(ComboValues < other.ComboValues) ? true : (ComboValues > other.ComboValues) ? false :
		(DefaultValue < other.DefaultValue) ? true : (DefaultValue > other.DefaultValue) ? false :
		false;
}

// ***************************************************************************
// CPrimitiveClass::CParameter::CConstStringValue
// ***************************************************************************

bool CPrimitiveClass::CParameter::CConstStringValue::operator== (const CConstStringValue &other) const
{
	return Values == other.Values;
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::CConstStringValue::operator< (const CConstStringValue &other) const
{
	return Values < other.Values;
}


void	CPrimitiveClass::CParameter::CConstStringValue::appendFilePath(std::vector<std::string> &pathList)	const
{
	pathList.insert(pathList.end(), Values.begin(), Values.end());
}

void	CPrimitiveClass::CParameter::CConstStringValue::appendPrimPath(std::vector<std::string> &pathList, const	std::vector<const IPrimitive*>	&relativePrimPaths)	const
{
	std::set<std::string>	relativePrimPathString;
	for	(std::vector<const IPrimitive*>::const_iterator it=relativePrimPaths.begin(), itEnd=relativePrimPaths.end(); it!=itEnd;++it)
	{
		const	uint	nbChilds=(*it)->getNumChildren();
		for (uint childIndex=0;childIndex<nbChilds;childIndex++)
		{
			const	IPrimitive*child=NULL;
			if	(	!(*it)->getChild(child,childIndex)
				||	!child	)
				continue;
			std::string	str;
			if	(child->getPropertyByName("name", str))
				relativePrimPathString.insert(str);
		}

	}
	pathList.insert(pathList.end(), relativePrimPathString.begin(), relativePrimPathString.end());
}

void	CPrimitiveClass::CParameter::CConstStringValue::getPrimitivesForPrimPath	(std::vector<const IPrimitive*>	&relativePrimPaths, const	std::vector<const IPrimitive*>	&startPrimPath)	const
{
	for	(uint i=0; i<PrimitivePath.size (); i++)
	{
		set<const IPrimitive*>	relativePrimPath;
		for (uint locIndex=0;locIndex<startPrimPath.size();locIndex++)
		{
			const	IPrimitive *const	cursor=startPrimPath[locIndex]->getPrimitive(PrimitivePath[i]);
			if	(cursor)
				relativePrimPath.insert(cursor);
		}
		if	(relativePrimPath.size()==1)
			relativePrimPaths.push_back(*relativePrimPath.begin());
	}

}

// ***************************************************************************

bool CPrimitiveClass::CParameter::translateAutoname (std::string &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass) const
{
	result = "";
	string::size_type strBegin = 0;
	string::size_type strEnd = 0;
	while (strBegin != Autoname.size())
	{
		strEnd = Autoname.find ('$', strBegin);
		if (strEnd == string::npos)
		{
			strEnd = Autoname.size();
			result += Autoname.substr (strBegin, strEnd-strBegin);
		}
		else
		{
			// Copy the remaining string
			result += Autoname.substr (strBegin, strEnd-strBegin);
			if (strEnd != Autoname.size())
			{
				strBegin = strEnd+1;
				strEnd = Autoname.find ('$', strBegin);
				if (strEnd == string::npos)
					strEnd = Autoname.size();
				else
				{
					string keyWord = Autoname.substr (strBegin, strEnd-strBegin);

					// Loop for the parameter
					uint i;
					for (i=0; i<primitiveClass.Parameters.size (); i++)
					{
						if (primitiveClass.Parameters[i].Name == keyWord)
						{
							// Get its string value
							string str;
							const IProperty *prop;
							if (primitive.getPropertyByName (keyWord.c_str(), prop))
							{
								// The property has been found ?
								if (prop)
								{
									// Array or string ?
									const CPropertyString *_string = dynamic_cast<const CPropertyString *>(prop);

									// Is a string ?
									if (_string)
									{
										if (!(_string->String.empty()))
										{
											result += _string->String;
											break;
										}
									}
									else
									{
										// Try an array
										const CPropertyStringArray *array = dynamic_cast<const CPropertyStringArray *>(prop);

										// Is an array ?
										if (array)
										{
											if (!(array->StringArray.empty()))
											{
												uint i;
												for (i=0; i<array->StringArray.size()-1; i++)
													result += array->StringArray[i] + "\n";
												result += array->StringArray[i];
												break;
											}
										}
									}
								}
							}

							// Get its default value
							std::string result2;
							if (primitiveClass.Parameters[i].getDefaultValue (result2, primitive, primitiveClass))
							{
								result += result2;
								break;
							}
						}
					}
					strEnd++;
				}

			}
		}
		strBegin = strEnd;
	}
	return true;
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::getDefaultValue (std::string &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass, std::string *fromWhere) const
{
	result = "";
	if (!Autoname.empty())
	{
		if (fromWhere)
			*fromWhere = "Autoname value : "+Autoname;
		return translateAutoname (result, primitive, primitiveClass);
	}
	else
	{
		if (fromWhere)
			*fromWhere = "Default value";
		if (!DefaultValue.empty())
			result = DefaultValue[0].Name;
	}
	return true;
}

// ***************************************************************************

bool CPrimitiveClass::CParameter::getDefaultValue (std::vector<std::string> &result, const IPrimitive &primitive, const CPrimitiveClass &primitiveClass, std::string * /* fromWhere */) const
{
	if (!Autoname.empty())
	{
		string temp;
		if (translateAutoname (temp, primitive, primitiveClass))
		{
			result.clear ();
			if (!temp.empty())
			{
				string tmp;
				uint i;
				for (i=0; i<temp.size(); i++)
				{
					if (temp[i] == '\n')
					{
						result.push_back (tmp);
						tmp.clear();
					}
					else
					{
						tmp.push_back(temp[i]);
					}
				}
				if (!tmp.empty())
					result.push_back (tmp);
			}
			return true;
		}
		else
			return false;
	}
	else
	{
		uint i;
		result.resize (DefaultValue.size());
		for (i=0; i<DefaultValue.size(); i++)
			result[i] = DefaultValue[i].Name;
	}
	return true;
}

// ***************************************************************************



// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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
#include <nel/ligo/ligo_config.h>

// Qt includes

namespace WorldEditor
{

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

		// Name ?
		if (initParameters[p].Name == "name")
			type = NLLIGO::CPrimitiveClass::CParameter::String;

		// Continue ?
		if (cp<primClass.Parameters.size () || (initParameters[p].Name == "name"))
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
					{
						Visible = true;
					}
					for (size_t i = 0; i < parameter.DefaultValue.size(); ++i)
					{
						// Generate a unique id ?
						if (parameter.DefaultValue[i].GenID)
						{
							Visible = true;
						}
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
							{
								str->StringArray[i] = NLMISC::toString(getUniqueId());
							}
							else
							{
								str->StringArray[i] = "";
							}
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
	const NLLIGO::CPrimitiveClass *primClass = NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig->getPrimitiveClass(className);
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
			primitive = new NLLIGO::CPrimPath;
			break;
		case NLLIGO::CPrimitiveClass::Zone:
			primitive = new NLLIGO::CPrimZone;
			break;
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

		// Prim file ?
		if (primClass->Type == NLLIGO::CPrimitiveClass::Bitmap)
		{
			// Create a dialog file
			//CFileDialogEx dialog (BASE_REGISTRY_KEY, "image", TRUE, primClass->FileExtension.c_str (), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
			//	(primClass->FileType+" (*."+primClass->FileExtension+")|*."+primClass->FileExtension+"|All Files (*.*)|*.*||").c_str (), getMainFrame ());
			//if (dialog.DoModal() == IDOK)
			//{
			// Save filename
			//	static_cast<CPrimBitmap*>(primitive)->init(dialog.GetPathName ());
			//}
		}

		// Continue ?
		if (primitive)
		{
			// Auto init ?
			if (!primClass->AutoInit)
			{
				// Make a vector of locator
				//CDatabaseLocatorPointer locatorPtr;
				//getLocator (locatorPtr, locator);
				std::list<NLLIGO::IPrimitive*> locators;
				//locators.push_back (const_cast<IPrimitive*> (locatorPtr.Primitive));

				// Yes, go
				//CDialogProperties dialogProperty (locators, getMainFrame ());
				//dialogProperty.DoModal ();
			}

			// Eval the default name property
			std::string name;
			if (!primitive->getPropertyByName ("name", name) || name.empty())
			{
				const NLLIGO::CPrimitiveClass *primClass = NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig->getPrimitiveClass(*primitive);
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

			// Init primitive default values
			primitive->initDefaultValues(*NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig);
		}

		// Done
		return primitive;
	}
	else
	{
		nlerror("Unknown primitive class name : %s", className);
	}
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
	// Modified
	bool modified = false;

	// Get the prim class
	const NLLIGO::CPrimitiveClass *primClass = NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig->getPrimitiveClass(*primitive);
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

} /* namespace WorldEditor */

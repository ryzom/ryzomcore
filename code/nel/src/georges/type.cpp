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


#include "stdgeorges.h"

#include "nel/misc/i_xml.h"
#include "nel/misc/eval_num_expr.h"
#include "nel/misc/path.h"
#include "nel/georges/u_type.h"

#include "nel/georges/form.h"
#include "nel/georges/form_elm.h"
#include "nel/georges/form_loader.h"
#include "nel/georges/type.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace std;

namespace NLGEORGES
{

// ***************************************************************************

void warning (bool exception, const char *format, ... );

// ***************************************************************************

CType::CType ()
{
	Type = UnsignedInt;
	UIType = Edit;
}

// ***************************************************************************

CType::~CType ()
{
  //	int toto = 0;
}

// ***************************************************************************

void CType::write (xmlDocPtr doc) const
{
	// Create the first node
	xmlNodePtr node = xmlNewDocNode (doc, NULL, (const xmlChar*)"TYPE", NULL);
	xmlDocSetRootElement (doc, node);

	// Type
	xmlSetProp (node, (const xmlChar*)"Type", (const xmlChar*)TypeNames[Type]);
	xmlSetProp (node, (const xmlChar*)"UI", (const xmlChar*)UITypeNames[UIType]);

	// Default valid
	if (!Default.empty())
	{
		xmlSetProp (node, (const xmlChar*)"Default", (const xmlChar*)Default.c_str());
	}

	// Min valid
	if (!Min.empty())
	{
		xmlSetProp (node, (const xmlChar*)"Min", (const xmlChar*)Min.c_str());
	}

	// Max valid
	if (!Max.empty())
	{
		xmlSetProp (node, (const xmlChar*)"Max", (const xmlChar*)Max.c_str());
	}

	// Increment valid
	if (!Increment.empty())
	{
		xmlSetProp (node, (const xmlChar*)"Increment", (const xmlChar*)Increment.c_str());
	}

	// Definition
	uint def = 0;
	for (def = 0; def<Definitions.size(); def++)
	{
		xmlNodePtr defNode = xmlNewChild ( node, NULL, (const xmlChar*)"DEFINITION", NULL);
		xmlSetProp (defNode, (const xmlChar*)"Label", (const xmlChar*)Definitions[def].Label.c_str());
		xmlSetProp (defNode, (const xmlChar*)"Value", (const xmlChar*)Definitions[def].Value.c_str());
	}

	// Header
	Header.write (node);
}

// ***************************************************************************

void CType::read (xmlNodePtr root)
{
	// Check node name
	if ( ((const char*)root->name == NULL) || (strcmp ((const char*)root->name, "TYPE") != 0) )
	{
		// Throw exception
		warning2 (true, "read", "XML Syntax error in block line %d, node (%s) should be TYPE.",
			(sint)root->line, root->name);
	}

	// Read the type
	const char *value = (const char*)xmlGetProp (root, (xmlChar*)"Type");
	if (value)
	{
		// Lookup type
		uint type;
		for (type=0; type<TypeCount; type++)
		{
			if (strcmp (value, TypeNames[type]) == 0)
				break;
		}

		// Type found ?
		if (type!=TypeCount)
			Type = (TType)type;
		else
		{
			// Make an error message
			string valueStr = value;

			// Delete the value
			xmlFree ((void*)value);

			// Throw exception
			warning2 (true, "read", "XML Syntax error in TYPE block line %d, the Type value is unknown (%s).",
				(sint)root->line, valueStr.c_str ());
		}

		// Delete the value
		xmlFree ((void*)value);
	}
	else
	{
		// Throw exception
		warning2 (true, "read", "XML Syntax error in TYPE block line %d, the Type argument was not found.",
			(sint)root->line);
	}

	// Read the UI
	value = (const char*)xmlGetProp (root, (xmlChar*)"UI");
	if (value)
	{
		// Lookup type
		uint type;
		for (type=0; type<UITypeCount; type++)
		{
			if (strcmp (value, UITypeNames[type]) == 0)
				break;
		}

		// Type found ?
		if (type!=UITypeCount)
			UIType = (TUI)type;
		else
			UIType = Edit;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		UIType = Edit;

	// Read Default
	value = (const char*)xmlGetProp (root, (xmlChar*)"Default");
	if (value)
	{
		Default = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		Default.clear();

	// Read Min
	value = (const char*)xmlGetProp (root, (xmlChar*)"Min");
	if (value)
	{
		Min = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		Min.clear();

	// Read Max
	value = (const char*)xmlGetProp (root, (xmlChar*)"Max");
	if (value)
	{
		Max = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		Max.clear();

	// Read Increment
	value = (const char*)xmlGetProp (root, (xmlChar*)"Increment");
	if (value)
	{
		Increment = value;

		// Delete the value
		xmlFree ((void*)value);
	}
	else
		Increment.clear();

	// Read the definitions
	uint childrenCount = CIXml::countChildren (root, "DEFINITION");

	// Resize the array
	Definitions.resize (childrenCount);
	uint child=0;
	xmlNodePtr childPtr = CIXml::getFirstChildNode (root, "DEFINITION");
	while (child < childrenCount)
	{
		// Should not be NULL
		nlassert (childPtr);

		// Read Default
		const char *label = (const char*)xmlGetProp (childPtr, (xmlChar*)"Label");
		if (label)
		{
			// Read Default
			value = (const char*)xmlGetProp (childPtr, (xmlChar*)"Value");
			if (value)
			{
				Definitions[child].Label = label;
				Definitions[child].Value = value;

				// Delete the value
				xmlFree ((void*)value);
			}
			else
			{
				// Delete the value
				xmlFree ((void*)label);

				// Throw exception
				warning2 (true, "read", "XML Syntax error in DEFINITION block line %d, the Value argument was not found.",
					(sint)childPtr->line);
			}

			// Delete the value
			xmlFree ((void*)label);
		}
		else
		{
			// Throw exception
			warning2 (true, "read", "XML Syntax error in DEFINITION block line %d, the Label argument was not found.",
				(sint)childPtr->line);
		}

		// One more
		child++;

		childPtr = CIXml::getNextChildNode (childPtr, "DEFINITION");;
	}

	// Read the header
	Header.read (root);
}

// ***************************************************************************

const char *CType::TypeNames[TypeCount]=
{
	"UnsignedInt",
	"SignedInt",
	"Double",
	"String",
	"Color",
};

// ***************************************************************************

const char *CType::UITypeNames[UITypeCount]=
{
	"Edit",
	"EditSpin",
	"NonEditableCombo",
	"FileBrowser",
	"BigEdit",
	"ColorEdit",
};

// ***************************************************************************

const char *CType::getTypeName (TType type)
{
	return TypeNames[type];
}

// ***************************************************************************

const char *CType::getUIName (TUI type)
{
	return UITypeNames[type];
}

// ***************************************************************************

class CMyEvalNumExpr : public CEvalNumExpr
{
public:
	CMyEvalNumExpr (const CForm *form , const CType *type )
	{
		Type = type;
		Form = form;
	}

	virtual CEvalNumExpr::TReturnState evalValue (const char *value, double &result, uint32 round)
	{
		// If a form is available
		if (Form)
		{
			// Ask for the filename ?
			if (strcmp (value, "$filename") == 0)
			{
				// Get the filename
				const string filename = CFile::getFilenameWithoutExtension (Form->getFilename ());

				// While the filename as a number
				sint i;
				for (i=(sint)filename.size ()-1; i>=0; i--)
				{
					if ((filename[i]<'0') || (filename[i]>'9'))
						break;
				}

				// Number found..
				if ((i >= 0) && (i<((sint)filename.size ()-1)))
				{
					i++;
					// Set the result
					NLMISC::fromString(filename.substr(i), result);
				}
				else
				{
					// If the filename doesn't contain a number, returns 0
					result = 0;
				}
				return CEvalNumExpr::NoError;
			}
			else
			{
				// check if the value is a label defined in the ".typ" file
				for (uint i =0; i < Type->Definitions.size(); i++)
				{
					if ( !nlstricmp( Type->Definitions[i].Label.c_str(),value ) )
					{
						CMyEvalNumExpr expr(Form, Type);
						sint index;
						return expr.evalExpression (Type->Definitions[i].Value.c_str(), result,&index,round+1);
					}
				}

				// try to get a Form value

				// Is an exist request ?
				bool requestExist = false;
				if (*value == '#')
				{
					value++;
					requestExist = true;
				}

				// The parent Dfn
				const CFormDfn *parentDfn;
				const CFormDfn *nodeDfn;
				const CType *nodeType;
				CFormElm *node;
				uint parentIndex;
				bool array;
				bool parentVDfnArray;
				UFormDfn::TEntryType type;
				// Search for the node
				if (((const CFormElm&)Form->getRootNode ()).getNodeByName (value, &parentDfn, parentIndex, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, false, round+1))
				{
					// End, return the current index
					if (type == UFormDfn::EntryType)
					{
						// The atom
						const CFormElmAtom *atom = node ? safe_cast<const CFormElmAtom*> (node) : NULL;

						// Evale
						nlassert (nodeType);
						string res;
						if (nodeType->getValue (res, Form, atom, *parentDfn, parentIndex, UFormElm::Eval, NULL, round+1, value))
						{
							// Request exist ?
							if (requestExist)
							{
								// Doesn't exist
								result = res.empty ()?0:1;
								return CEvalNumExpr::NoError;
							}
							else if (((const CFormElm&)Form->getRootNode ()).convertValue (result, res))
							{
								return CEvalNumExpr::NoError;
							}
						}

						// Request exist ?
						if (requestExist)
						{
							// Doesn't exist
							result = 0;
							return CEvalNumExpr::NoError;
						}
					}
				}
			}
		}
		return CEvalNumExpr::evalValue (value, result, round+1);
	}

	// The working form
	const CForm		*Form;
	// the type of the field containing the expression
	const CType		*Type;
};

// ***************************************************************************

#define NL_TOKEN_STRING 0
#define NL_TOKEN_DOUBLE_QUOTE 1
#define NL_TOKEN_OPEN_BRACKET 2
#define NL_TOKEN_NAME 3
#define NL_TOKEN_END 4

uint getNextToken (const char *startString, string &token, uint &offset)
{
	if (startString[offset] == 0)
		return NL_TOKEN_END;
	if (startString[offset] == '"')
	{
		offset++;
		return NL_TOKEN_DOUBLE_QUOTE;
	}
	if (startString[offset] == '{')
	{
		offset++;
		return NL_TOKEN_OPEN_BRACKET;
	}
	if ( (startString[offset] == '$') && (strncmp (startString+offset+1, "filename", 8) == 0) )
	{
		offset += 9;
		return NL_TOKEN_NAME;
	}
	token.clear();
	while (startString[offset])
	{
		if (startString[offset] == '\\')
		{
			if (startString[offset+1])
			{
				token += startString[offset+1];
				offset++;
			}
			else
			{
				offset++;
				break;
			}
		}
		else if (startString[offset] == '"')
			break;
		else if (startString[offset] == '{')
			break;
		else if ( (startString[offset] == '$') && (strncmp (startString+offset+1, "filename", 8) == 0) )
			break;
		else
			token += startString[offset];
		offset++;
	}
	return NL_TOKEN_STRING;
}

// ***************************************************************************

uint findSpecialCharacter (const char *special, char c, uint startOffset)
{
	uint offset = startOffset;
	while (special[offset])
	{
		if (special[offset] == '\\')
		{
			if (special[offset+1])
				offset++;
			else
				break;
		}
		else
		{
			if (special[offset] == c)
				return offset;
		}
		offset++;
	}
	return 0xffffffff;
}

// ***************************************************************************

void buildError (char *msg, uint offset)
{
	msg[0] = 0;
	if (offset<512)
	{
		uint i;
		for (i=0; i<offset; i++)
			msg[i] = '-';
		msg[i] = '^';
		msg[i+1] = 0;
	}
}

// ***************************************************************************

bool CType::getValue (string &result, const CForm *form, const CFormElmAtom *node, const CFormDfn &parentDfn, uint parentIndex, UFormElm::TEval evaluate, uint32 *where, uint32 round, const std::string &formName) const
{
	if (round > NLGEORGES_MAX_RECURSION)
	{
		// Turn around..
		warning2 (false, "getDefinition", "Recurcive call on the same DFN, look for loop inheritances.");
		return false;
	}

	// Node exist ?
	if (node && !node->Value.empty())
	{
		if (where)
			*where = (node->Form == form) ? CFormElm::ValueForm : CFormElm::ValueParentForm;
		result = node->Value;
	}
	// Have a default dfn value ?
	else
	{
		const string &defDfn = parentDfn.Entries[parentIndex].Default;
		if (!defDfn.empty ())
		{
			if (where)
				*where = CFormElm::ValueDefaultDfn;
			result = defDfn;
		}
		else
		{
			if (where)
				*where = CFormElm::ValueDefaultType;
			result = Default;
		}
	}

	// evaluate the value ?
	if (evaluate == UFormElm::Formula)
	{
		// Evaluate predefinition
		uint i;
		uint predefCount = (uint)Definitions.size ();
		for (i=0; i<predefCount; i++)
		{
			// Ref on the value
			const CType::CDefinition &def = Definitions[i];

			// This predefinition ?
			if (def.Label == result)
			{
				result = def.Value;
				break;
			}
		}
	}
	else if (evaluate == UFormElm::Eval)
	{
		// Evaluate numerical expression
		if ((Type == Double) || (Type == SignedInt) || (Type == UnsignedInt) || (Type == UnsignedInt))
		{
			// Evaluate predefinition
			uint i;
			uint predefCount = (uint)Definitions.size ();
			for (i=0; i<predefCount; i++)
			{
				// Ref on the value
				const CType::CDefinition &def = Definitions[i];

				// This predefinition ?
				if (def.Label == result)
				{
					result = def.Value;
					break;
				}
			}

			double value;
			CMyEvalNumExpr expr (form,this);
			int offset;
			CEvalNumExpr::TReturnState error = expr.evalExpression (result.c_str (), value, &offset, round+1);
			if (error == CEvalNumExpr::NoError)
			{
				// To string
				result = toString (value);
			}
			else
			{
				// Build a nice error output in warning
				char msg[512];
				buildError (msg, offset);
				warning (false, formName, form->getFilename ().c_str (), "getValue", "Syntax error in expression: %s\n%s\n%s", expr.getErrorString (error), result.c_str (), msg);
				return false;
			}
		}
		else // For strings
		{
			// Get next text
			uint offset = 0;
			string dest;
			while (offset < result.size ())
			{
				string token;
				uint tokenType = getNextToken (result.c_str (), token, offset);

				// Brackets, {numerical expressions} : numerical expressions to string
				if (tokenType == NL_TOKEN_OPEN_BRACKET)
				{
					// Find the second "
					uint nextEnd = findSpecialCharacter (result.c_str (), '}', offset);
					if (nextEnd == 0xffffffff)
					{
						// Build a nice error output in warning
						char msg[512];
						buildError (msg, (uint)result.size ());
						warning (false, formName, form->getFilename ().c_str (), "getValue", "Missing closing quote\n%s\n%s", result.c_str (), msg);
						return false;
					}
					else
					{
						// Zero padding
						char zeroPadding = 0;

						// Format code ?
						if ( ( (nextEnd - offset) >= 3 ) && ( result[offset] == '$' ) && ( result[offset+1] == 'z' )
							&& ( result[offset+2] <= '9' ) && ( result[offset+2] >= '0'  ) )
						{
							// Save padding
							zeroPadding = result[offset+2] - '0';
							offset += 3;
						}

						// try to get a Form value
						string valueName = result.substr ( offset, nextEnd-offset );

						double value;
						CMyEvalNumExpr expr (form,this);
						int offsetExpr;
						CEvalNumExpr::TReturnState error = expr.evalExpression (valueName.c_str (), value, &offsetExpr, round+1);
						if (error == CEvalNumExpr::NoError)
						{
							// To string
							char format[200];
							char result[200];
							smprintf (format, 200, "%%0%cg", zeroPadding+'0');
							smprintf (result, 200, format, value);
							dest += result;
						}
						else
						{
							// Build a nice error output in warning
							char msg[512];
							buildError (msg, offset+offsetExpr);
							warning (false, formName, form->getFilename ().c_str (), "getValue", "Syntax error in expression: %s\n%s\n%s", expr.getErrorString (error), result.c_str (), msg);
							return false;
						}

						// Next offset
						offset = nextEnd + 1;
					}
				}
				else if (tokenType == NL_TOKEN_DOUBLE_QUOTE)
				{
					// Find the second "
					uint nextEnd = findSpecialCharacter (result.c_str (), '"', offset);
					if (nextEnd == 0xffffffff)
					{
						// Build a nice error output in warning
						char msg[512];
						buildError (msg, (uint)result.size ());
						warning (false, formName, form->getFilename ().c_str (), "getValue", "Missing double quote\n%s\n%s", result.c_str (), msg);
						return false;
					}
					else
					{
						// try to get a Form value
						string valueName = result.substr ( offset, nextEnd-offset );

						// The parent Dfn
						const CFormDfn *parentDfn;
						const CFormDfn *nodeDfn;
						const CType *nodeType;
						CFormElm *node;
						uint parentIndex;
						bool array;
						bool parentVDfnArray;
						UFormDfn::TEntryType type;

						// Search for the node
						if (((const CFormElm&)form->getRootNode ()).getNodeByName (valueName, &parentDfn, parentIndex, &nodeDfn, &nodeType, &node, type, array, parentVDfnArray, false, round+1))
						{
							// End, return the current index
							if (type == UFormDfn::EntryType)
							{
								// The atom
								const CFormElmAtom *atom = node ? safe_cast<const CFormElmAtom*> (node) : NULL;

								// Evale
								nlassert (nodeType);
								string result2;
								if (nodeType->getValue (result2, form, atom, *parentDfn, parentIndex, UFormElm::Eval, NULL, round+1, valueName.c_str ()))
								{
									dest += result2;
								}
							}
							else
							{
								char msg[512];
								buildError (msg, offset);
								warning (false, formName, form->getFilename ().c_str (), "getValue", "Node is not an atom (%s)\n%s\n%s", valueName.c_str (), result.c_str (), msg);
								return false;
							}
						}
						else
							return false;
					}

					// Next offset
					offset = nextEnd + 1;
				}
				else if (tokenType == NL_TOKEN_STRING)
				{
					// Evaluate predefinition
					uint i;
					uint predefCount = (uint)Definitions.size ();
					for (i=0; i<predefCount; i++)
					{
						// Ref on the value
						const CType::CDefinition &def = Definitions[i];

						// This predefinition ?
						if (def.Label == token)
						{
							token = def.Value;
							break;
						}
					}

					// Take the remaining of the string
					dest += token;
				}
				else if (tokenType == NL_TOKEN_NAME)
				{
					dest += form->getFilename ();
				}
			}

			// Final result
			result = dest;
		}
	}

	// Ok
	return true;
}

// ***************************************************************************

bool CType::uiCompatible (TType type, TUI ui)
{
	switch (type)
	{
	case UnsignedInt:
	case SignedInt:
	case Double:
		return (ui == Edit) || (ui == EditSpin) || (ui == NonEditableCombo);
	case String:
		return (ui == Edit) || (ui == NonEditableCombo) || (ui == FileBrowser) || (ui == BigEdit);
	case Color:
		return (ui == ColorEdit);
	default: break;
	}
	return false;
}

// ***************************************************************************

void CType::warning (bool exception, const std::string &formName, const std::string &formFilename, const std::string &function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	NLGEORGES::warning (exception, "(CType::%s) In form (%s) in node (%s) : %s", function.c_str(), formFilename.c_str(), formName.c_str(), buffer);
}

// ***************************************************************************

void CType::warning2 (bool exception, const std::string &function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	NLGEORGES::warning (exception, "(CType::%s) : %s", function.c_str(), buffer);
}

// ***************************************************************************

UType::TType CType::getType () const
{
	return Type;
}

// ***************************************************************************

const string &CType::getDefault () const
{
	return Default;
}

// ***************************************************************************

const string	&CType::getMin () const
{
	return Min;
}

// ***************************************************************************

const string	&CType::getMax () const
{
	return Max;
}

// ***************************************************************************

const string	&CType::getIncrement () const
{
	return Increment;
}

// ***************************************************************************

uint CType::getNumDefinition () const
{
	return (uint)Definitions.size ();
}

// ***************************************************************************

bool CType::getDefinition (uint index, std::string &label, std::string &value) const
{
	if (index < Definitions.size ())
	{
		label = Definitions[index].Label;
		value = Definitions[index].Value;
		return true;
	}
	warning2 (false, "getDefinition", "Index out of bounds (%d >= %d)", index, Definitions.size ());
	return false;
}

// ***************************************************************************

const string	&CType::getComment () const
{
	return Header.Comments;
}

// ***************************************************************************

void CType::getDependencies (std::set<std::string> & /* dependencies */) const
{

}

// ***************************************************************************

} // NLGEORGES

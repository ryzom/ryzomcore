// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdmisc.h"

#include "nel/misc/i_xml.h"

#ifndef NL_DONT_USE_EXTERNAL_CODE

// Include from libxml2
#include <libxml/xmlerror.h>

using namespace std;

#define NLMISC_READ_BUFFER_SIZE 1024

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// *********************************************************

const char SEPARATOR = ' ';

std::string CIXml::_ErrorString;

bool CIXml::_LibXmlIntialized = false;

// ***************************************************************************

#define readnumber(dest,digits) \
	string number_as_string; \
	serialSeparatedBufferIn( number_as_string ); \
	NLMISC::fromString(number_as_string, dest);

// ***************************************************************************

inline void CIXml::flushContentString ()
{
	// String size
	_ContentString.erase ();

	// Reset
	_ContentStringIndex = 0;
}

// ***************************************************************************

CIXml::CIXml () : IStream (true /* Input mode */)
{
	// Not initialized
	_Parser = NULL;
	_CurrentElement = NULL;
	_CurrentNode = NULL;
	_PushBegin = false;
	_AttribPresent = false;
	_TryBinaryMode = false;
	_BinaryStream = NULL;
}

// ***************************************************************************

CIXml::CIXml (bool tryBinaryMode) : IStream (true /* Input mode */)
{
	// Not initialized
	_Parser = NULL;
	_CurrentElement = NULL;
	_CurrentNode = NULL;
	_PushBegin = false;
	_AttribPresent = false;
	_TryBinaryMode = tryBinaryMode;
	_BinaryStream = NULL;
}

// ***************************************************************************

CIXml::~CIXml ()
{
	// Release
	release ();
}

// ***************************************************************************

void CIXml::release ()
{
	// Release the parser
	if (_Parser)
	{
		// Free it
		xmlClearParserCtxt (_Parser);
		xmlFreeParserCtxt (_Parser);

		_Parser = NULL;
	}

	// Not initialized
	_CurrentElement = NULL;
	_CurrentNode = NULL;
	_PushBegin = false;
	_AttribPresent = false;
	_ErrorString.clear();

	resetPtrTable();
}

// ***************************************************************************

void xmlGenericErrorFuncRead (void *ctx, const char *msg, ...)
{
	// Get the error string
	string str;
	NLMISC_CONVERT_VARGS (str, msg, NLMISC::MaxCStringSize);
	CIXml::_ErrorString += str;
}

// ***************************************************************************

bool CIXml::init (IStream &stream)
{
	// Release
	release ();

	initLibXml();

	// Default : XML mode
	_BinaryStream = NULL;

	// Input stream ?
	if (stream.isReading())
	{
		// Set XML mode
		setXMLMode (true);

		// Get current position
		sint32 pos = stream.getPos ();

		// Go to end
		bool seekGood = stream.seek (0, end);
		nlassert (seekGood);

		// Get input stream length
		sint32 length = stream.getPos () - pos;

		// Go to start
		stream.seek (pos, begin);

		// The read buffer
        char buffer[NLMISC_READ_BUFFER_SIZE];

		// Fill the buffer
		stream.serialBuffer ((uint8*)buffer, 4);
		length -= 4;

		// Try binary mode
		if (_TryBinaryMode)
		{
			char header[5];
			header[0] = buffer[0];
			header[1] = buffer[1];
			header[2] = buffer[2];
			header[3] = buffer[3];
			header[4] = '\0';
			toLowerAscii(header);

			// Does it a xml stream ?
			if (strcmp(header, "<?xm"))
			{
				// NO ! Go in binary mode
				_BinaryStream = &stream;

				// Seek back to the start
				stream.seek (pos, begin);

				// Done
				return true;
			}
		}

		_ErrorString.clear();

		// The parser context
        _Parser = xmlCreatePushParserCtxt(NULL, NULL, buffer, 4, NULL);
		nlassert (_Parser);

		// For all the file
        while (length>=NLMISC_READ_BUFFER_SIZE)
		{
			// Fill the buffer
			stream.serialBuffer ((uint8*)buffer, NLMISC_READ_BUFFER_SIZE);

			// Read a buffer
            int res = xmlParseChunk(_Parser, buffer, NLMISC_READ_BUFFER_SIZE, 0);

			// Error code ?
			if (res)
			{
				throw EXmlParsingError (_ErrorString);
			}

			// Length left
			length -= NLMISC_READ_BUFFER_SIZE;
        }

		// Fill the buffer
		stream.serialBuffer ((uint8*)buffer, length);

		// Parse the last buffer
		int res = xmlParseChunk(_Parser, buffer, length, 1);

		// Error code ?
		if (res)
		{
			throw EXmlParsingError (_ErrorString);
		}

		// Ok
		return true;
	}
	else
	{
		nlwarning ("XML: The stream is not an input stream.");
	}

	// Error
	return false;
}

// ***************************************************************************

void CIXml::serialSeparatedBufferIn ( string &value, bool checkSeparator )
{
	nlassert( isReading() );

	// Output stream has been initialized ?
	if ( _Parser )
	{
		// Current node presents ?
		if (_CurrentElement)
		{
			// Write a push attribute ?
			if (_PushBegin)
			{
				// Current attrib is set ?
				if (_AttribPresent)
				{
					// Should have a current element
					nlassert (_CurrentElement);

					// Get the attribute
					xmlChar *attributeValue = xmlGetProp (_CurrentElement, (const xmlChar*)_AttribName.c_str());

					// Attribute is here ?
					if (attributeValue)
					{
						// Copy the value
						value = (const char*)attributeValue;

						// Delete the value
						xmlFree ((void*)attributeValue);
					}
					else
					{
						// Node name must not be NULL
						nlassert (_CurrentElement->name);

						// Make an error message
						char tmp[512];
						smprintf (tmp, 512, "NeL XML Syntax error in block line %d\nAttribute \"%s\" is missing in node \"%s\"",
							(int)_CurrentElement->line, _AttribName.c_str(), _CurrentElement->name);
						throw EXmlParsingError (tmp);
					}

					// The attribute has been used
					_AttribPresent = false;
				}
				else
				{
					// * Error, the stream don't use XML streaming properly
					// * You must take care of this in your last serial call:
					// * - Between xmlPushBegin() and xmlPushEnd(), before each serial, you must set the attribute name with xmlSetAttrib.
					// * - Between xmlPushBegin() and xmlPushEnd(), you must serial only basic objects (numbers and strings).
					nlerror ( "Error, the stream don't use XML streaming properly" );
				}
			}
			else
			{
				// Content length
				uint length = (uint)_ContentString.length();

				// String empty ?
				if (length==0)
				{
					// Try to open the node
					do
					{
						// If no more node, empty string
						if (_CurrentNode == NULL)
						{
							value.clear();
							_ContentStringIndex = 0;
							_ContentString.erase ();
							return;
						}

						// Node with the good name
						if (_CurrentNode->type == XML_TEXT_NODE)
						{
							// Stop searching
							break;
						}
						else
							// Get next
							_CurrentNode = _CurrentNode->next;
					}
					while (_CurrentNode);

					// Not found ?
					if (_CurrentNode != NULL)
					{
						// Read the content
						const char *content = (const char*)xmlNodeGetContent (_CurrentNode);
						if (content)
						{
							_ContentString = content;

							// Delete the value
							xmlFree ((void*)content);
						}
						else
							_ContentString.erase ();

						// Set the current index
						_ContentStringIndex = 0;

						// New length
						length = (uint)_ContentString.length();
					}
				}

				// Keyword in the buffer ?
				if (_ContentStringIndex < length)
				{
					// First index
					uint first = _ContentStringIndex;

					// Have to take care of separators ?
					if (checkSeparator)
					{
						// Scan to the end
						while (_ContentStringIndex < length)
						{
							// Not a separator ?
							if ( (_ContentString[_ContentStringIndex]==SEPARATOR) || (_ContentString[_ContentStringIndex]=='\n') )
							{
								_ContentStringIndex++;
								break;
							}

							// Next char
							_ContentStringIndex++;
						}
					}
					else
					{
						// Copy all the string
						_ContentStringIndex = length;
					}

					// Make a string
					value.assign (_ContentString, first, _ContentStringIndex-first);
				}
				else
				{
					// Should have a name
					nlassert (_CurrentElement->name);

					// Make an error message
					char tmp[512];
					smprintf (tmp, 512, "NeL XML Syntax error in block line %d \nMissing keywords in text child node in the node %s",
						(int)_CurrentElement->line, _CurrentElement->name);
					throw EXmlParsingError (tmp);
				}
			}
		}
		else
		{
			// * Error, no current node present.
			// * Check that your serial is initialy made between a xmlPushBegin and xmlPushEnd calls.
			nlerror ( "Error, the stream don't use XML streaming properly" );
		}
	}
	else
	{
		nlerror ( "Output stream has not been initialized" );
	}
}

// ***************************************************************************

void CIXml::serial(uint8 &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		// Read the number
		readnumber( b, 3 );
	}
}

// ***************************************************************************

void CIXml::serial(sint8 &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 4 );
	}
}

// ***************************************************************************

void CIXml::serial(uint16 &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 5 );
	}
}

// ***************************************************************************

void CIXml::serial(sint16 &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 6 );
	}
}

// ***************************************************************************

inline uint32 atoui( const char *ident)
{
	return (uint32) strtoul (ident, NULL, 10);
}

void CIXml::serial(uint32 &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 10 );
	}
}

// ***************************************************************************

void CIXml::serial(sint32 &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 11 );
	}
}

// ***************************************************************************

void CIXml::serial(uint64 &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 20 );
	}
}

// ***************************************************************************

void CIXml::serial(sint64 &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 20 );
	}
}

// ***************************************************************************

void CIXml::serial(float &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 128 );
	}
}

// ***************************************************************************

void CIXml::serial(double &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		readnumber( b, 128 );
	}
}

// ***************************************************************************

void CIXml::serial(bool &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		serialBit(b);
	}
}

// ***************************************************************************

void CIXml::serialBit(bool &bit)
{
	if (_BinaryStream)
	{
		_BinaryStream->serialBit(bit);
	}
	else
	{
		uint8 u;
		serial (u);
		bit = (u!=0);
	}
}

// ***************************************************************************

#ifndef NL_OS_CYGWIN
void CIXml::serial(char &b)
{
	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		string toto;
		serialSeparatedBufferIn ( toto );

		// Good value ?
		if (toto.length()!=1)
		{
			// Protect error
			if (_Parser)
			{
				// Should have a name
				nlassert (_CurrentElement->name);

				// Make an error message
				char tmp[512];
				smprintf (tmp, 512, "NeL XML Syntax error in block line %d \nValue is not a char in the node named %s",
					(int)_CurrentElement->line, _CurrentElement->name);
				throw EXmlParsingError (tmp);
			}
			else
			{
				nlerror ( "Output stream has not been initialized" );
			}
		}
		else
			b=toto[0];
	}
}
#endif // NL_OS_CYGWIN

// ***************************************************************************

void CIXml::serial(std::string &b)
{
	nlassert( isReading() );

	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		// Attibute ?
		if (_PushBegin)
		{
			// Only serial the string
			serialSeparatedBufferIn ( b, false );
		}
		else
		{
			// Open a string node
			xmlPush ("S");

			// Serial the string
			serialSeparatedBufferIn ( b, false );

			// Close the node
			xmlPop ();
		}
	}
}

// ***************************************************************************

void CIXml::serial(ucstring &b)
{
	nlassert( isReading() );

	if (_BinaryStream)
	{
		_BinaryStream->serial(b);
	}
	else
	{
		// Serial a simple string
		string tmp;

		// Serial this string
		serial (tmp);

		// Return a ucstring
		b.fromUtf8(tmp);
	}
}

// ***************************************************************************

void CIXml::serialBuffer(uint8 *buf, uint len)
{
	if (_BinaryStream)
	{
		_BinaryStream->serialBuffer(buf, len);
	}
	else
	{
		// Open a node
		xmlPush ("BUFFER");

		// Serialize the buffer
		for (uint i=0; i<len; i++)
		{
			xmlPush ("ELM");

			serial (buf[i]);

			xmlPop ();
		}

		// Close the node
		xmlPop ();
	}
}

// ***************************************************************************

bool CIXml::xmlPushBeginInternal (const std::string &nodeName)
{
	nlassert( isReading() );

	if (_BinaryStream)
	{
		return true;
	}
	else
	{
		// Check _Parser
		if ( _Parser )
		{
			// Can make a xmlPushBegin ?
			if ( ! _PushBegin )
			{
				// Current node exist ?
				if (_CurrentNode==NULL)
				{
					// Get the first node
					_CurrentNode = xmlDocGetRootElement (_Parser->myDoc);

					// Has a root node ?
					if (_CurrentNode)
					{
						// Node name must not be NULL
						nlassert (_CurrentNode->name);

						// Node element with the good name ?
						if ( (_CurrentNode->type != XML_ELEMENT_NODE) || ( (const char*)_CurrentNode->name != nodeName) )
						{
							// Make an error message
							char tmp[512];
							smprintf (tmp, 512, "NeL XML Syntax error : root node has the wrong name : \"%s\" should have \"%s\"",
								_CurrentNode->name, nodeName.c_str());
							throw EXmlParsingError (tmp);
						}
					}
					else
					{
						// Make an error message
						char tmp[512];
						smprintf (tmp, 512, "NeL XML Syntax error : no root node found.");
						throw EXmlParsingError (tmp);
					}
				}

				// Try to open the node
				do
				{
					// Node name must not be NULL
					nlassert (_CurrentNode->name);

					// Node with the good name
					if ( (_CurrentNode->type == XML_ELEMENT_NODE) && ( (const char*)_CurrentNode->name == nodeName) )
					{
						// Save current element
						_CurrentElement = _CurrentNode;

						// Stop searching
						break;
					}
					else
						// Get next
						_CurrentNode = _CurrentNode->next;
				}
				while (_CurrentNode);

				// Not found ?
				if (_CurrentNode == NULL)
				{
					// Make an error message
					char tmp[512];
					smprintf (tmp, 512, "NeL XML Syntax error in block line %d \nCan't open the node named %s in node named %s",
						(int)_CurrentElement->line, nodeName.c_str(), _CurrentElement->name);
					throw EXmlParsingError (tmp);
				}

				// Get first child
				_CurrentNode = _CurrentNode->children;

				// Push begun
				_PushBegin = true;

				// Flush current string
				flushContentString ();
			}
			else
			{
				nlerror ( "You must close your xmlPushBegin - xmlPushEnd before calling a new xmlPushBegin.");
				return false;
			}
		}
		else
		{
			nlerror ( "Output stream has not been initialized.");
			return false;
		}

		// Ok
		return true;
	}
}

// ***************************************************************************

bool CIXml::xmlPushEndInternal ()
{
	nlassert( isReading() );

	if (_BinaryStream)
	{
		return true;
	}
	else
	{
		// Check _Parser
		if ( _Parser )
		{
			// Can make a xmlPushEnd ?
			if ( _PushBegin )
			{
				// Push begun
				_PushBegin = false;
			}
			else
			{
				nlerror ( "You must call xmlPushBegin before calling xmlPushEnd.");
				return false;
			}
		}
		else
		{
			nlerror ( "Output stream has not been initialized.");
			return false;
		}

		// Ok
		return true;
	}
}

// ***************************************************************************

bool CIXml::xmlPopInternal ()
{
	nlassert( isReading() );

	if (_BinaryStream)
	{
		return true;
	}
	else
	{
		// Check _Parser
		if ( _Parser )
		{
			// Not in the push mode ?
			if ( ! _PushBegin )
			{
				// Some content to write ?
				flushContentString ();

				// Get parents
				_CurrentNode = _CurrentElement;
				_CurrentElement = _CurrentElement->parent;
				_CurrentNode = _CurrentNode->next;
			}
			else
			{
				nlerror ( "You must call xmlPop after xmlPushEnd.");
				return false;
			}
		}
		else
		{
			nlerror ( "Output stream has not been initialized.");
			return false;
		}

		// Ok
		return true;
	}
}

// ***************************************************************************

bool CIXml::xmlSetAttribInternal (const std::string &attribName)
{
	nlassert( isReading() );

	if (_BinaryStream)
	{
		return true;
	}
	else
	{
		// Check _Parser
		if ( _Parser )
		{
			// Can make a xmlPushEnd ?
			if ( _PushBegin )
			{
				// Set attribute name
				_AttribName = attribName;

				// Attribute name is present
				_AttribPresent = true;
			}
			else
			{
				nlerror ( "You must call xmlSetAttrib between xmlPushBegin and xmlPushEnd calls.");
				return false;
			}
		}
		else
		{
			nlerror ( "Output stream has not been initialized.");
			return false;
		}

		// Ok
		return true;
	}
}

// ***************************************************************************

bool CIXml::xmlBreakLineInternal ()
{
	// Ok
	return true;
}

// ***************************************************************************

bool CIXml::xmlCommentInternal (const std::string &/* comment */)
{
	// Ok
	return true;
}

// ***************************************************************************

xmlNodePtr CIXml::getFirstChildNode (xmlNodePtr parent, const std::string &childName)
{
	xmlNodePtr child = parent->children;
	while (child)
	{
		if (childName == (const char*)child->name)
			return child;
		child = child->next;
	}
	return NULL;
}

// ***************************************************************************

xmlNodePtr CIXml::getNextChildNode (xmlNodePtr last, const std::string &childName)
{
	last = last->next;
	while (last)
	{
		if (childName == (const char*)last->name)
			return last;
		last = last->next;
	}
	return NULL;
}

// ***************************************************************************

xmlNodePtr CIXml::getFirstChildNode (xmlNodePtr parent, sint /* xmlElementType */ type)
{
	xmlNodePtr child = parent->children;
	while (child)
	{
		if (child->type == (xmlElementType)type)
			return child;
		child = child->next;
	}
	return NULL;
}

// ***************************************************************************

xmlNodePtr CIXml::getNextChildNode (xmlNodePtr last, sint /* xmlElementType */ type)
{
	last = last->next;
	while (last)
	{
		if (last->type == (xmlElementType)type)
			return last;
		last = last->next;
	}
	return NULL;
}

// ***************************************************************************

uint CIXml::countChildren (xmlNodePtr node, const std::string &childName)
{
	uint count=0;
	xmlNodePtr child = getFirstChildNode (node, childName);
	while (child)
	{
		count++;
		child = getNextChildNode (child, childName);
	}
	return count;
}

// ***************************************************************************

uint CIXml::countChildren (xmlNodePtr node, sint /* xmlElementType */ type)
{
	uint count=0;
	xmlNodePtr child = getFirstChildNode (node, type);
	while (child)
	{
		count++;
		child = getNextChildNode (child, type);
	}
	return count;
}

// ***************************************************************************

xmlNodePtr CIXml::getRootNode () const
{
	if (_Parser)
		if (_Parser->myDoc)
			return xmlDocGetRootElement (_Parser->myDoc);
	return NULL;
}

// ***************************************************************************

bool CIXml::getPropertyString (std::string &result, xmlNodePtr node, const std::string &property)
{
	// Get the value
	const char *value = (const char*)xmlGetProp (node, (xmlChar*)property.c_str());
	if (value)
	{
		// Active value
		result = value;

		// Delete the value
		xmlFree ((void*)value);

		// Found
		return true;
	}

	return false;
}

// ***************************************************************************

int CIXml::getIntProperty(xmlNodePtr node, const std::string &property, int defaultValue)
{
	std::string s;

	bool b = getPropertyString(s, node, property);

	if (!b)
		return defaultValue;

	// remove leading and trailing spaces
	s = trim(s);

	sint val;
	
	if (!fromString(s, val) || (val == 0 && s != "0"))
	{
		nlwarning("Bad integer value: %s",s.c_str());
		return defaultValue;
	}

	return val;
}

// ***************************************************************************

double CIXml::getFloatProperty(xmlNodePtr node, const std::string &property, float defaultValue)
{
	std::string s;

	bool b = getPropertyString(s, node, property);

	if (!b)
		return defaultValue;

	// remove leading and trailing spaces
	s = trim(s);

	float val;

	if (!fromString(s, val))
	{
		nlwarning("Bad float value: %s", s.c_str());
		return defaultValue;
	}

	return val;
}

// ***************************************************************************

std::string CIXml::getStringProperty(xmlNodePtr node, const std::string &property, const std::string& defaultValue)
{
	std::string s;

	bool b = getPropertyString(s, node, property);

	if (!b)
		return defaultValue;

	return s;
}

// ***************************************************************************

bool CIXml::getContentString (std::string &result, xmlNodePtr node)
{
	const char *valueText = (const char*)xmlNodeGetContent (node);
	if (valueText)
	{
		result = valueText;

		// Delete the value
		xmlFree ((void*)valueText);

		// Found
		return true;
	}
	return false;
}

// ***************************************************************************

void CIXml::initLibXml()
{
	if (_LibXmlIntialized) return;

	_ErrorString.clear();
	
	// Set error handler
	xmlSetGenericErrorFunc	(NULL, xmlGenericErrorFuncRead);

	LIBXML_TEST_VERSION
	
	// an error occured during initialization
	if (!_ErrorString.empty())
	{
		throw EXmlParsingError (_ErrorString);
	}

	// Ask to get debug info
	xmlLineNumbersDefault(1);

	_LibXmlIntialized = true;
}

// ***************************************************************************

void CIXml::releaseLibXml()
{
	if (!_LibXmlIntialized) return;

	xmlCleanupParser();

	_LibXmlIntialized = false;
}

std::string CIXml::getErrorString()
{
	return _ErrorString;
}

} // NLMISC

#endif // NL_DONT_USE_EXTERNAL_CODE

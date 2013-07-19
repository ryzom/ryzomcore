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

#include "stdmisc.h"

#include "nel/misc/o_xml.h"

#ifndef NL_DONT_USE_EXTERNAL_CODE

// Include from libxml2
#include <libxml/xmlerror.h>

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// ***************************************************************************

const char SEPARATOR = ' ';

// ***************************************************************************

#define writenumber(src,format,digits) \
	char number_as_cstring [digits+1]; \
	sprintf( number_as_cstring, format, src ); \
	serialSeparatedBufferOut( number_as_cstring );

// ***************************************************************************
// XML callbacks
// ***************************************************************************

int xmlOutputWriteCallbackForNeL ( void *context, const char *buffer, int len)
{
	// no need to save empty buffer
	if(len == 0) return 0;

	// Get the object
	COXml *object = (COXml*) context;

	// Serialise the buffer
	object->_InternalStream->serialBuffer ((uint8*)buffer, len);

	// Return the value
	return len;
}

// ***************************************************************************

int xmlOutputCloseCallbackForNeL ( void * /* context */ )
{
	// Get the object
	// COXml *object = (COXml*) context;

	// Does nothing
	return 1;
}

// ***************************************************************************

xmlDocPtr COXml::getDocument ()
{
	if (_Document)
		return _Document;

	// Initialise the document
	_Document = xmlNewDoc ((const xmlChar *)_Version.c_str());

	return _Document;
}


// ***************************************************************************

inline void COXml::flushContentString ()
{
	// Current node must exist here
	nlassert (_CurrentNode);

	// String size
	uint size=(uint)_ContentString.length();

	// Some content to write ?
	if (size)
	{
		// Write it in the current node
		xmlNodePtr textNode = xmlNewText ((const xmlChar *)_ContentString.c_str());
		xmlAddChild (_CurrentNode, textNode);

		// Empty the string
		_ContentString.erase ();
	}
}

// ***************************************************************************

COXml::COXml () : IStream (false /* Output mode */)
{
	// Set XML mode
	setXMLMode (true);

	// Set the stream
	_InternalStream = NULL;

	// Set the version
	_Version = "1.0";

	// Initialise the document
	_Document = NULL;

	// Current node
	_CurrentNode = NULL;

	// Content string
	_ContentString = "";

	// Push begin
	_PushBegin = false;
}

// ***************************************************************************

void xmlGenericErrorFuncWrite (void *ctx, const char *msg, ...)
{
	// Get the error string
	string str;
	NLMISC_CONVERT_VARGS (str, msg, NLMISC::MaxCStringSize);
	((COXml*)ctx)->_ErrorString += str;
}

// ***************************************************************************

bool COXml::init (IStream *stream, const char *version)
{
	resetPtrTable();

	// Output stream ?
	if (!stream->isReading())
	{
		// Set error handler
		_ErrorString = "";
		xmlSetGenericErrorFunc	(this, xmlGenericErrorFuncWrite);

		// Set XML mode
		setXMLMode (true);

		// Set the stream
		_InternalStream = stream;

		// Set the version
		_Version = version;

		// Initialise the document
		_Document = NULL;

		// Current node
		_CurrentNode = NULL;

		// Content string
		_ContentString = "";

		// Push begin
		_PushBegin = false;

		// Ok
		return true;
	}
	else
		return false;
}

// ***************************************************************************

COXml::~COXml ()
{
	// Flush document to the internal stream
	flush ();
}

// ***************************************************************************

void COXml::serialSeparatedBufferOut( const char *value )
{
	nlassert( ! isReading() );

	// Output stream has been setuped ?
	if ( _InternalStream )
	{
		// Current node presents ?
		if (_CurrentNode)
		{
			// Write a push attribute ?
			if (_PushBegin)
			{
				// Current attrib is set ?
				if (_AttribPresent)
				{
					// Set the attribute
					xmlSetProp (_CurrentNode, (const xmlChar*)_AttribName.c_str(), (const xmlChar*)value);

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
				// Get the content buffer size
				uint size=(uint)_ContentString.length();

				// Add a separator
				if ((size) && (_ContentString[size-1]!='\n'))
					_ContentString += SEPARATOR;

				// Concat the strings
				_ContentString += value;
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
		nlerror ( "Output stream has not been setuped" );
	}
}

// ***************************************************************************

void COXml::serial(uint8 &b)
{
	// Write the number
	writenumber( (uint16)b,"%hu", 3 );
}

// ***************************************************************************

void COXml::serial(sint8 &b)
{
	writenumber( (sint16)b, "%hd", 4 );
}

// ***************************************************************************

void COXml::serial(uint16 &b)
{
	writenumber( b, "%hu", 5 );
}

// ***************************************************************************

void COXml::serial(sint16 &b)
{
	writenumber( b, "%hd", 6 );
}

// ***************************************************************************

void COXml::serial(uint32 &b)
{
	writenumber( b, "%u", 10 );
}

// ***************************************************************************

void COXml::serial(sint32 &b)
{
	writenumber( b, "%d", 11 );
}

// ***************************************************************************

void COXml::serial(uint64 &b)
{
	writenumber( b, "%"NL_I64"u", 20 );
}

// ***************************************************************************

void COXml::serial(sint64 &b)
{
	writenumber( b, "%"NL_I64"d", 20 );
}

// ***************************************************************************

void COXml::serial(float &b)
{
	writenumber( (double)b, "%f", 128 );
}

// ***************************************************************************

void COXml::serial(double &b)
{
	writenumber( b, "%f", 128 );
}

// ***************************************************************************

void COXml::serial(bool &b)
{
	serialBit(b);
}

// ***************************************************************************

void COXml::serialBit(bool &bit)
{
	uint8 u = (uint8)bit;
	serial( u );
}

// ***************************************************************************

#ifndef NL_OS_CYGWIN
void COXml::serial(char &b)
{
	char tmp[2] = {b , 0};
	serialSeparatedBufferOut( tmp );
}
#endif // NL_OS_CYGWIN

// ***************************************************************************

void COXml::serial(std::string &b)
{
	nlassert( ! isReading() );

	// Attibute ?
	if (_PushBegin)
	{
		// Only serial the string
		serialSeparatedBufferOut( b.c_str() );
	}
	else
	{
		// Open a string node
		xmlPush ("S");

		// Serial the string
		serialSeparatedBufferOut( b.c_str() );

		// Close the node
		xmlPop ();
	}
}

// ***************************************************************************

void COXml::serial(ucstring &b)
{
	nlassert( ! isReading() );

	// convert ucstring to utf-8 std::string
	std::string output = b.toUtf8();

	// Serial this string
	serial (output);
}

// ***************************************************************************

void COXml::serialBuffer(uint8 *buf, uint len)
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

// ***************************************************************************

bool COXml::xmlPushBeginInternal (const char *nodeName)
{
	nlassert( ! isReading() );

	// Check _InternalStream
	if ( _InternalStream )
	{
		// Can make a xmlPushBegin ?
		if ( ! _PushBegin )
		{
			// Current node exist ?
			if (_CurrentNode==NULL)
			{
				// No document ?
				if (_Document == NULL)
				{
					// Initialise the document
					_Document = xmlNewDoc ((const xmlChar *)_Version.c_str());

					// Return NULL if error
					nlassert (_Document);
				}

				// Create the first node
				_CurrentNode=xmlNewDocNode (_Document, NULL, (const xmlChar*)nodeName, NULL);
				xmlDocSetRootElement (_Document, _CurrentNode);

				// Return NULL if error
				nlassert (_CurrentNode);
			}
			else
			{
				// Flush current content string ?
				flushContentString ();

				// Create a new node
				_CurrentNode=xmlNewChild (_CurrentNode, NULL, (const xmlChar*)nodeName, NULL);

				// Return NULL if error
				nlassert (_CurrentNode);
			}

			// Push begun
			_PushBegin = true;
		}
		else
		{
			nlwarning ( "XML: You must close your xmlPushBegin - xmlPushEnd before calling a new xmlPushBegin.");
			return false;
		}
	}
	else
	{
		nlwarning ( "XML: Output stream has not been setuped.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool COXml::xmlPushEndInternal ()
{
	nlassert( ! isReading() );

	// Check _InternalStream
	if ( _InternalStream )
	{
		// Can make a xmlPushEnd ?
		if ( _PushBegin )
		{
			// Push begun
			_PushBegin = false;
		}
		else
		{
			nlwarning ( "XML: You must call xmlPushBegin before calling xmlPushEnd.");
			return false;
		}
	}
	else
	{
		nlwarning ( "XML: Output stream has not been setuped.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool COXml::xmlPopInternal ()
{
	nlassert( ! isReading() );

	// Check _InternalStream
	if ( _InternalStream )
	{
		// Not in the push mode ?
		if ( ! _PushBegin )
		{
			// Some content to write ?
			flushContentString ();

			// Get parent
			_CurrentNode=_CurrentNode->parent;
		}
		else
		{
			nlwarning ( "XML: You must call xmlPop after xmlPushEnd.");
			return false;
		}
	}
	else
	{
		nlwarning ( "XML: Output stream has not been setuped.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool COXml::xmlSetAttribInternal (const char *attribName)
{
	nlassert( ! isReading() );

	// Check _InternalStream
	if ( _InternalStream )
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
			nlwarning ( "XML: You must call xmlSetAttrib between xmlPushBegin and xmlPushEnd calls.");
			return false;
		}
	}
	else
	{
		nlwarning ( "XML: Output stream has not been setuped.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool COXml::xmlBreakLineInternal ()
{
	nlassert( ! isReading() );

	// Check _InternalStream
	if ( _InternalStream )
	{
		// Not in the push mode ?
		if ( ! _PushBegin )
		{
			// Add a break line
			_ContentString += '\n';
		}
		else
		{
			nlwarning ( "XML: You must call xmlNBreakLine after xmlPushEnd.");
			return false;
		}
	}
	else
	{
		nlwarning ( "XML: Output stream has not been setuped.");
		return false;
	}

	// Ok
	return true;
}

// ***************************************************************************

bool COXml::xmlCommentInternal (const char *comment)
{
	nlassert( ! isReading() );

	// Check _InternalStream
	if ( _InternalStream )
	{
		// Not in the push mode ?
		if ( _CurrentNode != NULL)
		{
			// Add a comment node
			xmlNodePtr commentPtr = xmlNewComment ((const xmlChar *)comment);

			// Add the node
			xmlAddChild (_CurrentNode, commentPtr);
		}
		else
		{
			nlwarning ( "XML: You must call xmlCommentInternal between xmlPushBegin and xmlPushEnd.");
			return false;
		}
	}
	else
	{
		nlwarning ( "XML: Output stream has not been setuped.");
		return false;
	}
	// Ok
	return true;
}

// ***************************************************************************

void COXml::flush ()
{
	if (_Document)
	{
		// Generate indentation
		xmlKeepBlanksDefault (0);

		// Create a output context
		xmlOutputBufferPtr outputBuffer = xmlOutputBufferCreateIO  ( xmlOutputWriteCallbackForNeL, xmlOutputCloseCallbackForNeL, this, NULL );

		// Save the file
		int res = xmlSaveFormatFileTo (outputBuffer, _Document, NULL, 1);

		// No error should be returned because, exception should be raised by the internal stream
		nlassert (res!=-1);

		// Free the document
		xmlFreeDoc (_Document);
		_Document = NULL;
	}
}

// ***************************************************************************

bool COXml::isStringValidForProperties (const char *str)
{
	while (*str)
	{
		if (*str == '\n')
			return false;
		str++;
	}
	return true;
}

// ***************************************************************************

const char *COXml::getErrorString () const
{
	return _ErrorString.c_str ();
}

} // NLMISC

#endif // NL_DONT_USE_EXTERNAL_CODE

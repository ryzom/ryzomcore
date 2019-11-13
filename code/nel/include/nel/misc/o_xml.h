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

#ifndef NL_O_XML_H
#define NL_O_XML_H

//#define NL_DONT_USE_EXTERNAL_CODE
#undef NL_DONT_USE_EXTERNAL_CODE

#ifndef NL_DONT_USE_EXTERNAL_CODE

#include "types_nl.h"
#include "stream.h"

// Forward declarations for libxml2
typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;

typedef struct _xmlDoc xmlDoc;
typedef xmlDoc *xmlDocPtr;

namespace NLMISC {


/**
 * Output xml stream
 *
 * This class is an xml formated output stream.
 *
 * This stream use an internal stream to output final xml code.
 \code
	// Check exceptions
	try
	{
		// File stream
		COFile file;

		// Open the file
		file.open ("output.xml");

		// Create the XML stream
		COXml output;

		// Init
		if (output.init (&file, "1.0"))
		{
			// Serial the class
			myClass.serial (output);

			// Flush the stream, write all the output file
			output.flush ();
		}

		// Close the file
		file.close ();
	}
 	catch (const Exception &e)
	{
	}
\endcode
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class COXml : public IStream
{
	friend int xmlOutputWriteCallbackForNeL ( void *context, const char *buffer, int len );
	friend int xmlOutputCloseCallbackForNeL ( void *context );
public:

	/** Stream ctor
	  *
	  */
	COXml ();

	/** Stream initialisation
	  *
	  * \param stream is the stream the class will use to output xml code.
	  * this pointer is held by the class but won't be deleted.
	  * \param version is the version to write in the XML header. Default is 1.0.
	  * \return true if initialisation is successful, false if the stream passed is not an output stream.
	  */
	bool init (IStream *stream, const std::string &version = "1.0");

	/** Return the error string.
	  * if not empty, something wrong appends
	  */
	static std::string getErrorString ();

	/** Default dstor
	  *
	  * Flush the stream.
	  */
	virtual ~COXml ();

	/** Flush the stream.
	  *
	  * You can only flush the stream when all xmlPushBegin - xmlPop have been closed.
	  */
	void flush ();

	/** Get root XML document pointer
	  */
	xmlDocPtr getDocument ();

	/** Return true if the string is valid to be stored in a XML property without modification.
	  */
	static bool		isStringValidForProperties (const std::string &str);

private:

	/// From IStream
	virtual void	serial(uint8 &b);
	virtual void	serial(sint8 &b);
	virtual void	serial(uint16 &b);
	virtual void	serial(sint16 &b);
	virtual void	serial(uint32 &b);
	virtual void	serial(sint32 &b);
	virtual void	serial(uint64 &b);
	virtual void	serial(sint64 &b);
	virtual void	serial(float &b);
	virtual void	serial(double &b);
	virtual void	serial(bool &b);
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b);
#endif
	virtual void	serial(std::string &b);
	virtual void	serial(ucstring &b);
	virtual void	serialBuffer(uint8 *buf, uint len);
	virtual void	serialBit(bool &bit);

	virtual bool	xmlPushBeginInternal (const std::string &nodeName);
	virtual bool	xmlPushEndInternal ();
	virtual bool	xmlPopInternal ();
	virtual bool	xmlSetAttribInternal (const std::string &attribName);
	virtual bool	xmlBreakLineInternal ();
	virtual bool	xmlCommentInternal (const std::string &comment);

	// Internal functions
	void			serialSeparatedBufferOut( const std::string &value );
	inline void		flushContentString ();

	// Push mode
	bool			_PushBegin;

	// Attribute defined
	bool			_AttribPresent;

	// Attribute name
	std::string		_AttribName;

	// The internal stream
	IStream			*_InternalStream;

	// Document pointer
	xmlDocPtr		_Document;

	// Document version
	std::string		_Version;

	// Current nodes
	xmlNodePtr		_CurrentNode;

	// Current content string
	std::string		_ContentString;
};


} // NLMISC

#endif // NL_DONT_USE_EXTERNAL_CODE

#endif // NL_O_XML_H

/* End of o_xml.h */

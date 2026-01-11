// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/thread.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/common.h"

#include "nel/georges/header.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace NLGEORGES
{

// ***************************************************************************

void warning (bool exception, const char *format, ... );

// ***************************************************************************

CFileHeader::CFileHeader ()
{
	MajorVersion = 0;
	MinorVersion = 0;
	State = Modified;
}

// ***************************************************************************

void CFileHeader::write (xmlNodePtr node) const
{
	// Georges version system
	char tmp[512];
	smprintf (tmp, 512, "%d.%d", MajorVersion, MinorVersion);
	xmlSetProp (node, (const xmlChar*)"Version", (const xmlChar*)tmp);

	// State
	if (State == Modified)
		xmlSetProp (node, (const xmlChar*)"State", (const xmlChar*)"modified");
	else
		xmlSetProp (node, (const xmlChar*)"State", (const xmlChar*)"checked");

	// Comments of the form
	if (!Comments.empty ())
	{
		// Create a new node
		xmlNodePtr child = xmlNewChild ( node, NULL, (const xmlChar*)"COMMENTS", NULL);
		xmlNodePtr textNode = xmlNewText ((const xmlChar *)Comments.c_str());
		xmlAddChild (child, textNode);
	}

	// Logs
	if (!Log.empty ())
	{
		// Create a new node
		xmlNodePtr child = xmlNewChild ( node, NULL, (const xmlChar*)"LOG", NULL);
		xmlNodePtr textNode = xmlNewText ((const xmlChar *)Log.c_str());
		xmlAddChild (child, textNode);
	}
}

// ***************************************************************************

void CFileHeader::addLog (const std::string &log)
{
	time_t t;
	time (&t);
	if (!Log.empty())
		Log += "\n";
	Log += ctime(&t);
	Log.resize (Log.size()-1);
	Log += " (";
	Log += IThread::getCurrentThread ()->getUserName ();
	Log += ") ";
	Log += log;
}

// ***************************************************************************

void CFileHeader::setComments (const std::string &comments)
{
	Comments = comments;
}

// ***************************************************************************

void CFileHeader::read (xmlNodePtr root)
{
	// Get the version
	const char *value = (const char*)xmlGetProp (root, (xmlChar*)"Version");
	if (value)
	{
		// Read the version
		if (sscanf (value, "%d.%d", &MajorVersion, &MinorVersion) != 2)
		{
			// Delete the value
			xmlFree ((void*)value);

			// Throw exception
			warning (true, "read", "XML Syntax error in TYPE block line %d, the Version argument is invalid.",
				(sint)root->line);
		}

		// Delete the value
		xmlFree ((void*)value);
	}
	else
	{
		// Set default
		MajorVersion = 0;
		MinorVersion = 0;
	}

	// Get the version
	value = (const char*)xmlGetProp (root, (xmlChar*)"State");
	if (value)
	{
		// Read the version
		if (strcmp (value, "modified") == 0)
		{
			State = Modified;
		}
		else if (strcmp (value, "checked") == 0)
		{
			State = Checked;
		}
		else
		{
			// Delete the value
			xmlFree ((void*)value);

			// Throw exception
			warning (true, "read", "XML Syntax error in TYPE block line %d, the State argument is invalid.",
				(sint)root->line);
		}

		// Delete the value
		xmlFree ((void*)value);
	}
	else
	{
		// Set default
		State = Modified;
	}

	// Look for the comment node
	Comments.clear();
	xmlNodePtr node = CIXml::getFirstChildNode (root, "COMMENTS");
	if (node)
	{
		// Get a text node
		node = CIXml::getFirstChildNode (node, XML_TEXT_NODE);

		if (node)
		{
			// Get content
			const char *comments = (const char*)xmlNodeGetContent (node);
			if (comments)
			{
				Comments = comments;

				// Delete the value
				xmlFree ((void*)comments);
			}
		}
	}

	// Look for the log node
	Log.clear();
	node = CIXml::getFirstChildNode (root, "LOG");
	if (node)
	{
		// Get a text node
		node = CIXml::getFirstChildNode (node, XML_TEXT_NODE);

		if (node)
		{
			// Get content
			const char *log = (const char*)xmlNodeGetContent (node);
			if (log)
			{
				Log = log;

				// Delete the value
				xmlFree ((void*)log);
			}
		}
	}
}

// ***************************************************************************

const char *CFileHeader::getStateString (TState state)
{
	if (state == Modified)
		return "Modified";
	else
		return "Checked";
}

// ***************************************************************************

void CFileHeader::warning (bool exception, const std::string &function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	NLGEORGES::warning (exception, "(CFileHeader::%s) : %s", function.c_str(), buffer);
}

// ***************************************************************************

} // NLGEORGES

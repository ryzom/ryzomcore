// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/misc/o_xml.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"

#include "nel/georges/form.h"
#include "nel/georges/form_loader.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;

namespace NLGEORGES
{

// ***************************************************************************
// Misc
// ***************************************************************************

void warning (bool exception, const char *format, ... )
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	if (exception)
	{
		// Make an error message
		char tmp[1024];
		smprintf (tmp, 1024, "NeL::Georges %s", buffer);
		throw EXmlParsingError (tmp);
	}
	else
	{
		nlwarning ("NeL::Georges %s", buffer);
	}
}

// ***************************************************************************
// UForm
// ***************************************************************************

UForm::~UForm ()
{
}

// ***************************************************************************

UFormElm& CForm::getRootNode ()
{
	return Elements;
}

// ***************************************************************************

const UFormElm& CForm::getRootNode () const
{
	return Elements;
}

// ***************************************************************************
// CForm
// ***************************************************************************

CForm::CForm () : Elements (this, NULL, NULL, 0xffffffff)
{
	uint i;
	for (i=0; i<HeldElementCount; i++)
	{
		HeldElements[i] = new CFormElmStruct (this, NULL, NULL, 0xffffffff);
	}
}

// ***************************************************************************

CForm::~CForm ()
{
	uint i;
	for (i=0; i<HeldElementCount; i++)
	{
		delete HeldElements[i];
	}
}

// ***************************************************************************

void CForm::write (xmlDocPtr doc, const std::string &filename)
{
	// Save the filename
	if (!filename.empty())
		_Filename = CFile::getFilename (filename);

	// Create the first node
	xmlNodePtr node = xmlNewDocNode (doc, NULL, (const xmlChar*)"FORM", NULL);
	xmlDocSetRootElement (doc, node);

	// List of parent
	for (uint parent=0; parent<ParentList.size (); parent++)
	{
		// Parent name not empty ?
		if (!(ParentList[parent].ParentFilename.empty()))
		{
			// Add a parent node
			xmlNodePtr parentNode = xmlNewChild ( node, NULL, (const xmlChar*)"PARENT", NULL );
			xmlSetProp (parentNode, (const xmlChar*)"Filename", (const xmlChar*)ParentList[parent].ParentFilename.c_str());
		}
	}

	// Write elements
	Elements.write (node, this, std::string(), true);

	// Write held elements
	uint i;
	for (i=0; i<HeldElementCount; i++)
	{
		HeldElements[i]->write (node, this, std::string(), true);
	}

	// Header
	Header.write (node);
}

// ***************************************************************************

void CForm::readParent (const char *parent, CFormLoader &loader)
{
	// Load the parent
	CForm *theParent = (CForm*)loader.loadForm (parent);
	if (theParent != NULL)
	{
		// Set the parent
		if (!insertParent (getParentCount (), parent, theParent))
		{
			// Make an error message
			std::string parentName = parent;

			// Delete the value
			xmlFree ((void*)parent);

			// Throw exception
			warning (true, "readParent", "Can't set the parent FORM named (%s). Check if it is the same form or if it use a differnt formDfn.", parentName.c_str ());
		}
	}
	else
	{
		// Make an error message
		std::string parentName = parent;

		// Delete the value
		xmlFree ((void*)parent);

		// Throw exception
		warning (true, "readParent", "Can't load the parent FORM named (%s).", parentName.c_str ());
	}
}

// ***************************************************************************

void CForm::read (xmlNodePtr node, CFormLoader &loader, CFormDfn *dfn, const std::string &filename)
{
	// Save the filename
	_Filename = CFile::getFilename (filename);

	// Reset form
	clean ();

	// Save the dfn
	_Dfn = dfn;

	// Check node name
	if ( ((const char*)node->name == NULL) || (strcmp ((const char*)node->name, "FORM") != 0) )
	{
		// Make an error message
		warning (true, "read", "XML Syntax error in block line %d, node (%s) should be FORM.",
			(sint)node->line, node->name);
	}

	// Get first struct node
	xmlNodePtr child = CIXml::getFirstChildNode (node, "STRUCT");
	if (child == NULL)
	{
		// Throw exception
		warning (true, "read", "Syntax error in block line %d, node (%s) should have a STRUCT child node.",
			(sint)node->line, node->name);
	}

	// Read the struct
	Elements.read (child, loader, dfn, this);

	// Get next struct node
	child = CIXml::getNextChildNode (node, "STRUCT");
	uint index = 0;
	while ( (child != NULL) && (index < HeldElementCount))
	{
		HeldElements[index]->read (child, loader, dfn, this);
		index++;
	}
	while (index < HeldElementCount)
	{
		// Build the Form
		HeldElements[index]->build (dfn);
		index++;
	}

	// Get the old parent parameter
	const char *parent = (const char*)xmlGetProp (node, (xmlChar*)"Parent");
	if (parent)
	{
		// Add a parent, xmlFree is done by readParent
		readParent (parent, loader);
	}

	// Read the new parent nodes
	uint parentCount = CIXml::countChildren (node, "PARENT");

	// Reserve some parents
	ParentList.reserve (ParentList.size () + parentCount);

	// Enum children node
	child = CIXml::getFirstChildNode (node, "PARENT");
	while (child)
	{
		parent = (const char*)xmlGetProp (child, (xmlChar*)"Filename");

		// Add a parent, xmlFree is done by readParent
		readParent (parent, loader);

		// Next node <PARENT>
		child = CIXml::getNextChildNode (child, "PARENT");
	}

	// Read the header
	Header.read (node);
}

// ***************************************************************************

const std::string &CForm::getComment () const
{
	return Header.Comments;
}

// ***************************************************************************

void CForm::write (NLMISC::IStream &stream)
{
	// Xml stream
	COXml xmlStream;
	xmlStream.init (&stream);

	// Write the file
	write (xmlStream.getDocument (), std::string());
}

// ***************************************************************************

bool CForm::insertParent (uint before, const std::string &filename, CForm *parent)
{
	// Set or reset ?
	nlassert (parent);

	// Must have the same DFN
	if (parent->Elements.FormDfn == Elements.FormDfn)
	{
		// Set members
		std::vector<CParent>::iterator ite = ParentList.insert (ParentList.begin() + before, CParent());
		ite->Parent = parent;
		ite->ParentFilename = filename;

		return true;
	}
	else
	{
		// Output an error
		warning (false, "insertParent", "Can't insert parent form (%s) that has not the same DFN.", filename.c_str());
	}

	return false;
}

// ***************************************************************************

void CForm::removeParent (uint parent)
{
	ParentList.erase (ParentList.begin() + parent);
}

// ***************************************************************************

CForm *CForm::getParent (uint parent) const
{
	return ParentList[parent].Parent;
}

// ***************************************************************************

const std::string &CForm::getParentFilename (uint parent) const
{
	return ParentList[parent].ParentFilename;
}

// ***************************************************************************

uint CForm::getParentCount () const
{
	return (uint)ParentList.size ();
}

// ***************************************************************************

void CForm::clean ()
{
	clearParents ();
}

// ***************************************************************************

void CForm::clearParents ()
{
	ParentList.clear ();
}

// ***************************************************************************

const std::string &CForm::getFilename () const
{
	return _Filename;
}

// ***************************************************************************

void CForm::warning (bool exception, const std::string &function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	NLGEORGES::warning (exception, "(CForm::%s) in form (%s) : %s", function.c_str(), _Filename.c_str (), buffer);
}

// ***************************************************************************

void CForm::getDependencies (std::set<std::string> &dependencies) const
{
	// Add me
	if (dependencies.insert (toLowerAscii(CFile::getFilename (_Filename))).second)
	{
		// Add parents
		uint i;
		for (i=0; i<ParentList.size (); i++)
		{
			if (ParentList[i].Parent)
			{
				ParentList[i].Parent->getDependencies (dependencies);
			}
		}

		// Add elements
		Elements.getDependencies (dependencies);
	}
}

// ***************************************************************************

uint CForm::getNumParent () const
{
	return getParentCount();
}

// ***************************************************************************

UForm *CForm::getParentForm (uint parent) const
{
	CForm *form = getParent (parent);
	return form;
}

// ***************************************************************************

} // NLGEORGES


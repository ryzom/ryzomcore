// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include "nel/misc/path.h"

#include "nel/georges/form_dfn.h"
#include "nel/georges/form_loader.h"
#include "nel/georges/form_elm.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace std;

#ifndef NL_OS_WINDOWS
#define stricmp strcasecmp
#endif


namespace NLGEORGES
{

// ***************************************************************************

void warning (bool exception, const char *format, ... );

// ***************************************************************************

void CFormDfn::addEntry( const std::string &name )
{
	CEntry entry;
	entry.setName( name.c_str() );
	Entries.push_back( entry );
}

void CFormDfn::removeEntry( uint idx )
{
	std::vector< CEntry >::iterator itr = Entries.begin() + idx;
	Entries.erase( itr );
}

// ***************************************************************************

void CFormDfn::write (xmlDocPtr doc, const std::string &filename)
{
	// Save filename
	_Filename = CFile::getFilename (filename);

	// Create the first node
	xmlNodePtr node = xmlNewDocNode (doc, NULL, (const xmlChar*)"DFN", NULL);
	xmlDocSetRootElement (doc, node);

	// Write elements
	uint parent;
	for (parent=0; parent<Parents.size(); parent++)
	{
		// Parent name not empty ?
		if (!Parents[parent].ParentFilename.empty ())
		{
			// Parent node
			xmlNodePtr parentNode = xmlNewChild ( node, NULL, (const xmlChar*)"PARENT", NULL);

			// Save parent
			xmlSetProp (parentNode, (const xmlChar*)"Name", (const xmlChar*)Parents[parent].ParentFilename.c_str());
		}
	}

	// Write elements
	uint elm;
	for (elm=0; elm<Entries.size(); elm++)
	{
		// Add a node
		xmlNodePtr elmPtr = xmlNewChild ( node, NULL, (const xmlChar*)"ELEMENT", NULL);
		xmlSetProp (elmPtr, (const xmlChar*)"Name", (const xmlChar*)Entries[elm].Name.c_str());

		// What kind of element
		switch (Entries[elm].TypeElement)
		{
		case UFormDfn::EntryType:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"Type");
			xmlSetProp (elmPtr, (const xmlChar*)"Filename", (const xmlChar*)Entries[elm].Filename.c_str());
			if ((!Entries[elm].FilenameExt.empty ()) && Entries[elm].FilenameExt != "*.*")
				xmlSetProp (elmPtr, (const xmlChar*)"FilenameExt", (const xmlChar*)Entries[elm].FilenameExt.c_str());
			break;
		case UFormDfn::EntryDfn:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"Dfn");
			xmlSetProp (elmPtr, (const xmlChar*)"Filename", (const xmlChar*)Entries[elm].Filename.c_str());
			break;
		case UFormDfn::EntryVirtualDfn:
			xmlSetProp (elmPtr, (const xmlChar*)"Type", (const xmlChar*)"DfnPointer");
			break;
		}

		// Is an array ?
		if (Entries[elm].Array)
			xmlSetProp (elmPtr, (const xmlChar*)"Array", (const xmlChar*)"true");

		// Default value for type
		if ((Entries[elm].TypeElement == UFormDfn::EntryType) && (!Entries[elm].Default.empty ()))
			xmlSetProp (elmPtr, (const xmlChar*)"Default", (const xmlChar*)Entries[elm].Default.c_str ());
	}

	// Header
	Header.write (node);
}

// ***************************************************************************

void CFormDfn::read (xmlNodePtr root, CFormLoader &loader, bool forceLoad, const std::string &filename)
{
	// Save filename
	_Filename = CFile::getFilename (filename);

	// Check node name
	if ( ((const char*)root->name == NULL) || (strcmp ((const char*)root->name, "DFN") != 0) )
	{
		// Throw exception
		warning (true, "read", "XML Syntax error in block line %d, node (%s) should be DFN.", (sint)root->line, root->name);
	}

	// Count the parent
	uint parentCount = CIXml::countChildren (root, "PARENT");
	Parents.resize (parentCount);

	// For each element entry
	uint parentNumber = 0;
	xmlNodePtr parent = CIXml::getFirstChildNode (root, "PARENT");
	while (parentNumber<parentCount)
	{
		// Get the Parent
		const char *parentFilename = (const char*)xmlGetProp (parent, (xmlChar*)"Name");
		if (parentFilename)
		{
			Parents[parentNumber].ParentFilename = parentFilename;

			// Delete the value
			xmlFree ((void*)parentFilename);

			// Load the parent
			Parents[parentNumber].Parent = loader.loadFormDfn (Parents[parentNumber].ParentFilename.c_str (), forceLoad);
			if ((Parents[parentNumber].Parent == NULL) && !forceLoad)
			{
				// Throw exception
				warning (true, "read", "Can't load parent DFN file (%s).", Parents[parentNumber].ParentFilename.c_str ());
			}
		}
		else
		{
			// Throw exception
			warning (true, "read", "XML Syntax error in block (%s) line %d, aguments Name not found.",
				parent->name, (sint)parent->line);
		}

		// Next parent
		parent = CIXml::getNextChildNode (parent, "PARENT");
		parentNumber++;
	}

	// Count the element children
	uint childCount = CIXml::countChildren (root, "ELEMENT");

	// Resize the element table
	Entries.resize (childCount);

	// For each element entry
	uint childNumber = 0;
	xmlNodePtr child = CIXml::getFirstChildNode (root, "ELEMENT");
	while (childNumber<childCount)
	{
		// Checks
		nlassert (child);

		// Get the name
		const char *value = (const char*)xmlGetProp (child, (xmlChar*)"Name");
		if (value)
		{
			// Store the value
			Entries[childNumber].Name = value;

			// Delete the value
			xmlFree ((void*)value);

			// Reset
			Entries[childNumber].Dfn = NULL;
			Entries[childNumber].Type = NULL;
			Entries[childNumber].Default.clear ();

			const char *filename = (const char*)xmlGetProp (child, (xmlChar*)"Filename");

			if ( filename )
			{
				Entries[childNumber].Filename = filename;

				// Delete the value
				xmlFree ((void*)filename);
			}
			else
			{
				Entries[childNumber].Filename.clear ();
			}

			const char *filenameExt = (const char*)xmlGetProp (child, (xmlChar*)"FilenameExt");
			if ( filenameExt )
			{
				Entries[childNumber].FilenameExt = filenameExt;

				// Delete the value
				xmlFree ((void*)filenameExt);
			}
			else
			{
				Entries[childNumber].FilenameExt = "*.*";
			}

			// Read the type
			const char *typeName = (const char*)xmlGetProp (child, (xmlChar*)"Type");
			if (typeName)
			{
				bool type = false;
				bool dfn = false;
				if (stricmp (typeName, "Type") == 0)
				{
					Entries[childNumber].TypeElement = UFormDfn::EntryType;
					type = true;

					// Load the filename
					if (!Entries[childNumber].Filename.empty ())
					{
						Entries[childNumber].Type = loader.loadType (Entries[childNumber].Filename.c_str ());
						if ((Entries[childNumber].Type == NULL) && !forceLoad)
						{
							// Throw exception
							warning (true, "read", "In XML block (%s) line %d, file not found %s.",
								child->name, (sint)child->line, Entries[childNumber].Filename.c_str ());
						}

						// Read the default value
						const char *defaultName = (const char*)xmlGetProp (child, (xmlChar*)"Default");
						if (defaultName)
						{
							Entries[childNumber].Default = defaultName;

							// Delete the value
							xmlFree ((void*)defaultName);
						}
					}
					else
					{
						// Throw exception
						warning (true, "read", "XML In block (%s) line %d, no filename found for the .typ file.",
							child->name, (sint)child->line);
					}
				}
				else if (stricmp (typeName, "Dfn") == 0)
				{
					Entries[childNumber].TypeElement = UFormDfn::EntryDfn;
					dfn = true;

					// Load the filename
					if (!Entries[childNumber].Filename.empty ())
					{
						// Load the filename
						Entries[childNumber].Dfn = loader.loadFormDfn (Entries[childNumber].Filename.c_str (), forceLoad);
						if ((Entries[childNumber].Dfn == NULL) && !forceLoad)
						{
							// Throw exception
							warning (true, "read", "XML In block (%s) line %d, file not found %s.",
								child->name, (sint)child->line, Entries[childNumber].Filename.c_str ());
						}
					}
					else
					{
						// Throw exception
						warning (true, "read", "XML In block (%s) line %d, no filename found for the .typ file.",
							child->name, (sint)child->line);
					}
				}
				else if (stricmp (typeName, "DfnPointer") == 0)
				{
					Entries[childNumber].TypeElement = UFormDfn::EntryVirtualDfn;
				}
				else
				{
					// Throw exception
					warning (true, "read", "XML Syntax error in block (%s) line %d, element has not a valid type name attribut \"Type = %s\".",
						child->name, (sint)child->line, typeName);
				}

				// Delete the value
				xmlFree ((void*)typeName);
			}
			else
			{
				// Throw exception
				warning (true, "read", "XML Syntax error in block (%s) line %d, element has no type name attribut \"Type = [Type][Dfn][DfnPointer]\".",
					child->name, (sint)child->line);
			}

			// Get the array attrib
			Entries[childNumber].Array = false;
			const char* arrayFlag = (const char*)xmlGetProp (child, (xmlChar*)"Array");
			if (arrayFlag)
			{
				Entries[childNumber].Array =  (stricmp (arrayFlag, "true") == 0);

				// Delete the value
				xmlFree ((void*)arrayFlag);
			}
		}
		else
		{
			// Throw exception
			warning (true, "read", "XML Syntax error in block (%s) line %d, aguments Name not found.",
				root->name, (sint)root->line);
		}

		// Next child
		child = CIXml::getNextChildNode (child, "ELEMENT");
		childNumber++;
	}

	// Read the header
	Header.read (root);
}

// ***************************************************************************

uint CFormDfn::countParentDfn (uint32 round) const
{
	// Checkout recursive calls
	if (round > NLGEORGES_MAX_RECURSION)
	{
		// Turn around..
		warning (false, "countParentDfn", "Recursive call on the same DFN, look for loop inheritances.");
		return 0;
	}

	uint count = 0;
	uint i;
	for (i=0; i<Parents.size (); i++)
	{
		count += Parents[i].Parent->countParentDfn (round+1);
	}
	return count+1;
}

// ***************************************************************************

void CFormDfn::getParentDfn (std::vector<CFormDfn*> &array, uint32 round)
{
	// Checkout recursive calls
	if (round > NLGEORGES_MAX_RECURSION)
	{
		// Turn around..
		warning (false, "getParentDfn", "Recursive call on the same DFN, look for loop inheritances.");
		return;
	}

	//uint count = 0;
	uint i;
	for (i=0; i<Parents.size (); i++)
	{
		Parents[i].Parent->getParentDfn (array, round+1);
	}
	array.push_back (this);
}

// ***************************************************************************

void CFormDfn::getParentDfn (std::vector<const CFormDfn*> &array, uint32 round) const
{
	// Checkout recursive calls
	if (round > NLGEORGES_MAX_RECURSION)
	{
		// Turn around..
		warning (false, "getParentDfn", "Recursive call on the same DFN, look for loop inheritances.");
		return;
	}

	//uint count = 0;
	uint i;
	for (i=0; i<Parents.size (); i++)
	{
		Parents[i].Parent->getParentDfn (array, round+1);
	}
	array.push_back (this);
}

// ***************************************************************************

uint CFormDfn::getNumParent () const
{
	return (uint)Parents.size ();
}

// ***************************************************************************

CFormDfn *CFormDfn::getParent (uint parent) const
{
	return Parents[parent].Parent;
}

// ***************************************************************************

const string& CFormDfn::getParentFilename (uint parent) const
{
	return Parents[parent].ParentFilename;
}

// ***************************************************************************

uint CFormDfn::getNumEntry () const
{
	return (uint)Entries.size();
}

// ***************************************************************************

void CFormDfn::setNumEntry (uint size)
{
	Entries.resize (size);
}

// ***************************************************************************

const CFormDfn::CEntry &CFormDfn::getEntry (uint entry) const
{
	return Entries[entry];
}

// ***************************************************************************

CFormDfn::CEntry &CFormDfn::getEntry (uint entry)
{
	return Entries[entry];
}

// ***************************************************************************

void CFormDfn::setNumParent (uint size)
{
	Parents.resize (size);
}

// ***************************************************************************

void CFormDfn::setParent (uint parent, CFormLoader &loader, const std::string &filename)
{
	if (filename.empty())
		Parents[parent].Parent = NULL;
	else
		Parents[parent].Parent = loader.loadFormDfn (filename, false);
	Parents[parent].ParentFilename = filename;
}

// ***************************************************************************

void CFormDfn::CEntry::setType (CFormLoader &loader, const std::string &filename)
{
	TypeElement = EntryType;
	Dfn = NULL;
	Filename = filename;
	Type = loader.loadType (filename);
}

void CFormDfn::CEntry::setType( TEntryType type )
{
	TypeElement = type;
}

// ***************************************************************************

void CFormDfn::CEntry::setDfn (CFormLoader &loader, const std::string &filename)
{
	TypeElement = EntryDfn;
	Filename = filename;
	Type = NULL;
	Dfn = loader.loadFormDfn (filename, false);
}

// ***************************************************************************

void CFormDfn::CEntry::setDfnPointer ()
{
	TypeElement = EntryVirtualDfn;
	Filename.clear();
	Type = NULL;
	Dfn = NULL;
}

// ***************************************************************************

const std::string &CFormDfn::CEntry::getName () const
{
	return Name;
}

// ***************************************************************************

void CFormDfn::CEntry::setName (const std::string &name)
{
	Name = name;
}

// ***************************************************************************

const std::string &CFormDfn::CEntry::getDefault () const
{
	return Default;
}

// ***************************************************************************

void CFormDfn::CEntry::setDefault (const std::string &def)
{
	Default = def;
}

// ***************************************************************************

void CFormDfn::CEntry::setArrayFlag (bool flag)
{
	Array = flag;
}

// ***************************************************************************

bool CFormDfn::CEntry::getArrayFlag () const
{
	return Array;
}

// ***************************************************************************

UFormDfn::TEntryType CFormDfn::CEntry::getType () const
{
	return TypeElement;
}

// ***************************************************************************

const std::string &CFormDfn::CEntry::getFilename() const
{
	return Filename;
}

// ***************************************************************************

void CFormDfn::CEntry::setFilename (const std::string &def)
{
	Filename = def;
}

// ***************************************************************************

CType *CFormDfn::CEntry::getTypePtr ()
{
	return Type;
}

// ***************************************************************************

CFormDfn *CFormDfn::CEntry::getDfnPtr ()
{
	return Dfn;
}

// ***************************************************************************

const CType *CFormDfn::CEntry::getTypePtr () const
{
	return Type;
}

// ***************************************************************************

const CFormDfn *CFormDfn::CEntry::getDfnPtr () const
{
	return Dfn;
}

// ***************************************************************************

CFormDfn *CFormDfn::getSubDfn (uint index, uint &dfnIndex)
{
	// Get the sub DFN
	vector<CFormDfn*> parentDfn;
	parentDfn.reserve (countParentDfn ());
	getParentDfn (parentDfn);

	// For each parent
	uint dfn;
	dfnIndex = index;
	uint parentSize = (uint)parentDfn.size();
	for (dfn=0; dfn<parentSize; dfn++)
	{
		// Good element ?
		uint size = (uint)parentDfn[dfn]->Entries.size ();
		if (dfnIndex<size)
			return parentDfn[dfn];
		dfnIndex -= size;
	}

	// Should be found..
	nlstop;
	return NULL;
}

// ***************************************************************************

const CFormDfn *CFormDfn::getSubDfn (uint index, uint &dfnIndex) const
{
	// Get the sub DFN
	vector<const CFormDfn*> parentDfn;
	parentDfn.reserve (countParentDfn ());
	getParentDfn (parentDfn);

	// For each parent
	uint dfn;
	dfnIndex = index;
	uint parentSize = (uint)parentDfn.size();
	for (dfn=0; dfn<parentSize; dfn++)
	{
		// Good element ?
		uint size = (uint)parentDfn[dfn]->Entries.size ();
		if (dfnIndex<size)
			return parentDfn[dfn];
		dfnIndex -= size;
	}

	// Should be found..
	nlstop;
	return NULL;
}

// ***************************************************************************

bool CFormDfn::getEntryType (uint entry, TEntryType &type, bool &array) const
{
	if (entry < Entries.size ())
	{
		type = Entries[entry].TypeElement;
		array = Entries[entry].Array;
		return true;
	}
	warning (false, "getEntryType", "Wrong entry ID.");
	return false;
}

// ***************************************************************************

bool CFormDfn::getEntryFilename (uint entry, std::string& filename) const
{
	if (entry < Entries.size ())
	{
		if (Entries[entry].TypeElement != EntryVirtualDfn)
		{
			filename = Entries[entry].Filename;
			return true;
		}
		warning (false, "getEntryFilename", "The entry is a virtual DFN.");
		return false;
	}
	warning (false, "getEntryFilename", "Wrong entry ID.");
	return false;
}

// ***************************************************************************

bool CFormDfn::getEntryFilenameExt (uint entry, std::string& filename) const
{
	if (entry < Entries.size ())
	{
		filename = Entries[entry].FilenameExt;
		return true;
	}
	warning (false, "getEntryFilenameExt", "Wrong entry ID.");
	return false;
}

// ***************************************************************************

bool CFormDfn::getEntryIndexByName (uint &entry, const	std::string &name) const
{
	uint	entryIndex=0;
	while	(entryIndex<Entries.size ())
	{
		if (Entries[entryIndex].Name==name)
		{
			entry=entryIndex;
			return	true;
		}
		entryIndex++;
	}
	entry = std::numeric_limits<uint>::max();
	return	false;
}

// ***************************************************************************

bool CFormDfn::getEntryName (uint entry, std::string &name) const
{
	if (entry < Entries.size ())
	{
		name = Entries[entry].Name;
		return true;
	}
	warning (false, "getEntryName", "Wrong entry ID.");
	return false;
}

// ***************************************************************************

bool	CFormDfn::getEntryDfn (uint entry, UFormDfn **dfn)
{
	if (entry < Entries.size ())
	{
		if (Entries[entry].TypeElement == EntryDfn)
		{
			*dfn = Entries[entry].Dfn;
			return true;
		}
		else
			warning (false, "getEntryDfn", "This entry is not a DFN.");
	}
	warning (false, "getEntryDfn", "Wrong entry ID.");
	return false;
}

bool	CFormDfn::getEntryByName (const std::string &name, CFormDfn::CEntry **entry)
{
	int	entryIndex=(int)Entries.size ()-1;
	while (entryIndex>=0)
	{
		CEntry	*entryPtr=&Entries[entryIndex];
		if (entryPtr->getName()==name)
		{
			*entry=entryPtr;
			return	true;
		}
		entryIndex--;
	}
	*entry=NULL;
	return	false;
}

bool	CFormDfn::getEntryDfnByName (const std::string &name, UFormDfn **dfn)
{
	CFormDfn::CEntry	*entry;
	if	(getEntryByName (name, &entry))
	{
		*dfn=entry->getDfnPtr();
		return	true;
	}
	*dfn=NULL;
	return	false;
}

bool	CFormDfn::isAnArrayEntryByName	(const std::string &name)	const
{
	CFormDfn::CEntry	*entry;
	if	(const_cast<CFormDfn*>(this)->getEntryByName (name, &entry))
	{
		return	entry->getArrayFlag();
	}
	return	false;
}

// ***************************************************************************

bool CFormDfn::getEntryType (uint entry, UType **type)
{
	if (entry < Entries.size ())
	{
		if (Entries[entry].TypeElement == EntryType)
		{
			*type = Entries[entry].Type;
			return true;
		}
		else
			warning (false, "getEntryType", "This entry is not a type.");
	}
	warning (false, "getEntryType", "Wrong entry ID.");
	return false;
}

// ***************************************************************************

uint CFormDfn::getNumParents () const
{
	return (uint)Parents.size ();
}

// ***************************************************************************

bool CFormDfn::getParent (uint parent, UFormDfn **parentRet)
{
	if (parent < Parents.size ())
	{
		*parentRet = Parents[parent].Parent;
		return true;
	}
	warning (false, "getParent", "Wrong parent ID.");
	return false;
}


// ***************************************************************************

bool CFormDfn::getParentFilename (uint parent, std::string &filename) const
{
	if (parent < Parents.size ())
	{
		filename = Parents[parent].ParentFilename;
		return true;
	}
	warning (false, "getParentFilename", "Wrong parent ID.");
	return false;
}

// ***************************************************************************

const std::string& CFormDfn::getComment () const
{
	return Header.Comments;
}

// ***************************************************************************

const std::string &CFormDfn::CEntry::getFilenameExt() const
{
	return FilenameExt;
}

// ***************************************************************************

void CFormDfn::CEntry::setFilenameExt (const std::string &ext)
{
	FilenameExt = ext;
}

// ***************************************************************************

void CFormDfn::warning (bool exception, const std::string &function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	NLGEORGES::warning (exception, "(CFormDfn::%s) in form DFN (%s) : %s", function.c_str(), _Filename.c_str (), buffer);
}

// ***************************************************************************

void CFormDfn::getDependencies (std::set<std::string> &dependencies) const
{
	// Scan only if not already inserted
	if (dependencies.insert (toLowerAscii(CFile::getFilename (_Filename))).second)
	{
		// Add parents
		uint i;
		for (i=0; i<Parents.size (); i++)
		{
			Parents[i].Parent->getDependencies (dependencies);
		}

		// Add entries
		for (i=0; i<Entries.size (); i++)
		{
			if (Entries[i].getDfnPtr ())
				Entries[i].getDfnPtr ()->getDependencies (dependencies);
			if (Entries[i].getTypePtr ())
			{
				dependencies.insert (toLowerAscii(CFile::getFilename (Entries[i].getFilename())));
			}
		}
	}
}

// ***************************************************************************

} // NLGEORGES

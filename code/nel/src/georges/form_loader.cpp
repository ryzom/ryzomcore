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

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/i_xml.h"

#include "nel/georges/u_form.h"

#include "nel/georges/form_loader.h"
#include "nel/georges/type.h"
#include "nel/georges/form.h"
#include "nel/georges/form_dfn.h"

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
// UFormLoader
// ***************************************************************************

UFormLoader *UFormLoader::createLoader ()
{
	return new CFormLoader;
}

// ***************************************************************************

void UFormLoader::releaseLoader (UFormLoader *loader)
{
	delete ((CFormLoader*)loader);
}

// ***************************************************************************
// CFormLoader
// ***************************************************************************
CFormLoader::~CFormLoader()
{
}

CType *CFormLoader::loadType (const std::string &filename)
{
	// Lower string filename
	string lowerStr = toLower(filename);
	lowerStr = CFile::getFilename (lowerStr);

	// Already in the map ?
	TTypeMap::iterator ite = _MapType.find (lowerStr);
	if (ite != _MapType.end() && (ite->second != NULL) )
	{
		// Return the pointer
		return ite->second;
	}
	else
	{
		// Create the type
		CType *type = new CType;

		// Load the type
		try
		{
			// Open the file
			string name = CPath::lookup (filename, false, false);
			if (name.empty())
				name = filename;
			CIFile file;
			if (file.open (name))
			{
				// Init an xml stream
				CIXml read;
				read.init (file);

				// Read the type
				type->read (read.getRootNode ());
			}
			else
			{
				// Output error
				warning (false, "loadType", "Can't open the form file (%s).", filename.c_str());

				// Delete the type
				delete type;
				type = NULL;
			}
		}
		catch (const Exception &e)
		{
			// Output error
			warning (false, "loadType", "Error while loading the form (%s): %s", filename.c_str(), e.what());

			// Delete the type
			delete type;
			type = NULL;
		}

		// Loaded ?
		if (type)
		{
			// Insert a new entry
			_MapType[lowerStr]= type;
			ite = _MapType.find (lowerStr);
			//CType *typeType = ite->second;
			//			int toto = 0;
		}
		return type;
	}
}

// ***************************************************************************

CFormDfn *CFormLoader::loadFormDfn (const std::string &filename, bool forceLoad)
{
	// Lower string filename
	string lowerStr = toLower(filename);
	lowerStr = CFile::getFilename (lowerStr);

	// Already in the map ?
	TFormDfnMap::iterator ite = _MapFormDfn.find (lowerStr);
	if (ite != _MapFormDfn.end() && ite->second)
	{
		// Return the pointer
		return ite->second;
	}
	else
	{
		// Create the formDfn
		CFormDfn *formDfn = new CFormDfn;

		// Insert the form first
		_MapFormDfn[lowerStr] = formDfn;

		// Load the type
		try
		{
			// Open the file
			string name = CPath::lookup (filename, false, false);
			if (name.empty())
				name = filename;
			CIFile file;
			if (file.open (name))
			{
				// Init an xml stream
				CIXml read;
				read.init (file);

				// Read the type
				formDfn->read (read.getRootNode (), *this, forceLoad, filename);
			}
			else
			{
				// Output error
				warning (false, "loadFormDfn", "Can't open the form file (%s).", filename.c_str());

				// Delete the formDfn
				delete formDfn;
				formDfn = NULL;
				_MapFormDfn.erase (lowerStr);
			}
		}
		catch (const Exception &e)
		{
			// Output error
			warning (false, "loadFormDfn", "Error while loading the form (%s): %s", filename.c_str(), e.what());

			// Delete the formDfn
			delete formDfn;
			formDfn = NULL;
			_MapFormDfn.erase (lowerStr);
		}

		return formDfn;
	}
}

// ***************************************************************************

UForm *CFormLoader::loadForm (const std::string &filename)
{
	// Lower string filename
	string lowerStr = toLower((string)filename);
	lowerStr = CFile::getFilename (lowerStr);

	// Already in the map ?
	TFormMap::iterator ite = _MapForm.find (lowerStr);
	if (ite != _MapForm.end() && ite->second)
	{
		// Return the pointer
		return (CForm*)ite->second;
	}
	else
	{
		// Create the form
		CForm *form = new CForm;

		// Insert the form first
		_MapForm[lowerStr] = form;

		// Load the type
		try
		{
			// Get the form DFN filename
			string name = CFile::getFilename (filename);
			string::size_type index = name.rfind ('.');
			if (index == string::npos)
			{
				// Output error
				warning (false, "loadForm", "Form name is invalid (%s). It should have the extension of its DFN type.", name.c_str ());

				// Delete the form
				delete form;
				form = NULL;
				_MapForm.erase (lowerStr);
			}
			name = name.substr (index+1);
			name += ".dfn";

			// Load the dfn
			CFormDfn	*dfn = loadFormDfn (name, false);
			if (dfn)
			{
				// Open the file
				name = CPath::lookup (filename, false, false);
				if (name.empty())
					name = filename;
				CIFile file;
				if (file.open (name))
				{
					// Init an xml stream
					CIXml read;
					read.init (file);

					// Read the form
					form->read (read.getRootNode (), *this, dfn, filename);
				}
				else
				{
					// Output error
					warning (false, "loadForm", "Can't open the form file (%s).", filename.c_str());

					// Delete the form
					delete form;
					form = NULL;
					_MapForm.erase (lowerStr);
				}
			}
			else
			{
				// Output error
				warning (false, "loadForm", "Can't open the dfn file (%s).", name.c_str ());

				// Delete the form
				delete form;
				form = NULL;
				_MapForm.erase (lowerStr);
			}
		}
		catch (const Exception &e)
		{
			// Output error
			warning (false, "loadForm", "Error while loading the form (%s): %s", filename.c_str(), e.what());

			// Delete the form
			delete form;
			form = NULL;
			_MapForm.erase (lowerStr);
		}

		return form;
	}
}

// ***************************************************************************

UFormDfn *CFormLoader::loadFormDfn (const std::string &filename)
{
	return loadFormDfn (filename, false);
}

// ***************************************************************************

UType *CFormLoader::loadFormType (const std::string &filename)
{
	return loadType (filename);
}

// ***************************************************************************

void CFormLoader::warning (bool exception, const std::string &function, const char *format, ... ) const
{
	// Make a buffer string
	va_list args;
	va_start( args, format );
	char buffer[1024];
	vsnprintf( buffer, 1024, format, args );
	va_end( args );

	// Set the warning
	NLGEORGES::warning (exception, "(CFormLoader::%s) : %s", function.c_str(), buffer);
}

// ***************************************************************************

} // NLGEORGES

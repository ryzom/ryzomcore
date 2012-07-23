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

#ifndef NL_FORM_H
#define NL_FORM_H

#include "nel/misc/types_nl.h"
#include "nel/georges/u_form.h"
#include "form_elm.h"
#include "header.h"

extern bool convertFormFile (const char *oldFileName, const char *newFileName);

namespace NLGEORGES
{

class UFormElm;

/**
  * This class implement a georges form.
  */
class CForm : public UForm
{
	friend class CFormLoader;
	friend bool convertFormFile (const char *oldFileName, const char *newFileName);
public:

	enum { HeldElementCount = 4 };

	// From UForm
	UFormElm&		getRootNode ();
	const UFormElm& getRootNode () const;
	const std::string &getComment () const;
	void			write (class NLMISC::IStream &stream, bool georges4CVS);
	void			getDependencies (std::set<std::string> &dependencies) const;
	uint			getNumParent () const;
	UForm			*getParentForm (uint parent) const;

#ifdef NL_OS_WINDOWS
#  pragma warning (disable : 4355)
#endif
	CForm ();
	~CForm ();

	// Clean the form. Erase parents.
	void				clean ();

	// ** Types

	// ** Header
	CFileHeader			Header;

	// ** Body

	/// Vector of CFormElm*
	CFormElmStruct		Elements;

	/// Backup slots
	CFormElmStruct		*HeldElements[HeldElementCount];

	// ** IO functions
	// Set the filename before saving the form
	void				write (xmlDocPtr doc, const char *filename, bool georges4CVS);

	// ** Parent access

	// Insert parent before parent indexed "before".
	bool				insertParent (uint before, const char *filename, CForm *parent);

	// Remove a parent from parent list
	void				removeParent (uint parent);

	// Get a parent
	CForm *				getParent (uint parent) const;
	const std::string	&getParentFilename (uint parent) const;

	// Get parent count
	uint				getParentCount () const;

	// Clear parents
	void				clearParents ();

	// Get the form filename with extension
	const std::string	&getFilename () const;

	// Error handling
	void				warning (bool exception, const char *function, const char *format, ... ) const;

private:

	// A parent structure
	class CParent
	{
	public:
		std::string					ParentFilename;
		NLMISC::CSmartPtr<CForm>	Parent;
	};

	/// Pointer on the parent
	std::vector<CParent>			ParentList;

	// CFormLoader call it
	// Set the filename before reading the form
	void				read (xmlNodePtr node, CFormLoader &loader, CFormDfn *dfn, const char *filename);

	// Called by read
	void				readParent (const char *parent, CFormLoader &loader);

	// The form filename
	std::string			_Filename;
};

} // NLGEORGES

#endif // NL_FORM_H

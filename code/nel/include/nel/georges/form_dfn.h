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

#ifndef NL_FORM_DFN_H
#define NL_FORM_DFN_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/georges/u_form_dfn.h"
#include "nel/georges/u_form_elm.h"
#include "header.h"
#include "type.h"

bool convertDfnFile (const std::string &oldFileName, const std::string &newFileName);

namespace NLGEORGES
{

class CFormLoader;

/**
  * This class is the defnition for a family of forms.
  */
class CFormDfn : public UFormDfn
{
	friend class CForm;
	friend class CType;
	friend class CFormElm;
	friend class CFormLoader;
	friend class CFormElmStruct;
	friend bool convertDfnFile (const std::string &oldFileName, const std::string &newFileName);
public:

	// Default cstr
	CFormDfn ()
	{
		Round = 0xffffffff;
	}

	virtual ~CFormDfn () { }

	// A form definition entry
	class CEntry
	{
		friend class CType;
		friend class CForm;
		friend class CFormElm;
		friend class CFormDfn;
		friend class CFormElmStruct;
		friend bool convertDfnFile (const std::string &oldFileName, const std::string &newFileName);
	public:

		CEntry ()
		{
			TypeElement = EntryType;
			FilenameExt = "*.*";
		}

		// Get the type
		class CType					*getTypePtr ();

		// Get the type
		const CType					*getTypePtr () const;

		// Get the dfn
		CFormDfn					*getDfnPtr ();

		// Get the dfn
		const CFormDfn				*getDfnPtr () const;

		// Get type flag
		TEntryType					getType () const;

		// Set as a type
		void						setType (CFormLoader &loader, const std::string &filename);

		void						setType (TEntryType type);

		// Set as a dfn
		void						setDfn (CFormLoader &loader, const std::string &filename);

		// Set as a dfn pointer
		void						setDfnPointer ();

		// Get element Name
		const std::string			&getName () const;

		// Set element Name
		void						setName (const std::string &name);

		// Get the filename
		const std::string			&getFilename() const;

		// Get the filename extension
		const std::string			&getFilenameExt() const;

		// Set the filename
		void						setFilename (const std::string &def);

		// Set the filename extension
		void						setFilenameExt (const std::string &ext);

		// Get default value
		const std::string			&getDefault () const;

		// Set default value
		void						setDefault (const std::string &def);

		// Set array flag
		void						setArrayFlag (bool flag);

		// Get array flag
		bool						getArrayFlag () const;

	private:
		// Entry name
		std::string					Name;

		// What is the type of the element ?
		TEntryType					TypeElement;

		// Is an array of this type ?
		bool						Array;

		// The filename
		std::string					Filename;

		// The default value for atom
		std::string					Default;

		// The filename
		std::string					FilenameExt;

		// Smart ptr on the type or the dfn
		NLMISC::CSmartPtr<CType>	Type;

		// Smart ptr on the type or the dfn
		NLMISC::CSmartPtr<CFormDfn>	Dfn;
	};

	// Parent DFN
	class CParent
	{
	public:
		// The parent filename
		std::string						ParentFilename;

		// The parent smart
		NLMISC::CSmartPtr<CFormDfn>	Parent;
	};

	void							addEntry( const std::string &name );

	void							removeEntry( uint idx );

	// ** IO functions
	void							write (xmlDocPtr root, const std::string &filename);

	// Count parent DFN
	uint							countParentDfn (uint32 round=0) const;

	// Get parent DFN
	void							getParentDfn (std::vector<CFormDfn*> &array, uint32 round=0);

	// Get parent DFN
	void							getParentDfn (std::vector<const CFormDfn*> &array, uint32 round=0) const;

	// Get num parent
	uint							getNumParent () const;

	// Get parent count
	void							setNumParent (uint size);

	// Set a parent
	void							setParent (uint parent, CFormLoader &loader, const std::string &filename);

	// Get a parent
	CFormDfn						*getParent (uint parent) const;

	// Get a parent string
	const std::string				&getParentFilename (uint parent) const;

	// Get num entry
	uint							getNumEntry () const;

	// Set num entry
	void							setNumEntry (uint size);

	// Get an entry
	const CEntry					&getEntry (uint entry) const;

	// Get an entry
	CEntry							&getEntry (uint entry);

	// Form UFormDfn
	bool							getEntryType (uint entry, TEntryType &type, bool &array) const;
	bool							getEntryName (uint entry, std::string &name) const;
	bool							getEntryIndexByName (uint &entry, const	std::string &name) const;
	bool							getEntryDfn (uint entry, UFormDfn **dfn);

	bool							getEntryByName			(const std::string &name, CFormDfn::CEntry **entry);
	bool							getEntryDfnByName		(const std::string &name, UFormDfn **dfn);
	bool							isAnArrayEntryByName	(const std::string &name)	const;

	bool							getEntryType (uint entry, UType **type);
	uint							getNumParents () const;
	bool							getParent (uint parent, UFormDfn **parentRet);
	const std::string				&getComment () const;
	bool							getEntryFilename (uint entry, std::string& filename) const;
	bool							getEntryFilenameExt (uint entry, std::string& filename) const;
	bool							getParentFilename (uint parent, std::string &filename) const;
	void							getDependencies (std::set<std::string> &dependencies) const;

	// Get the sub dfn of a dfn
	CFormDfn						*getSubDfn (uint index, uint &dfnIndex);
	const CFormDfn					*getSubDfn (uint index, uint &dfnIndex) const;

	// Header
	CFileHeader						Header;

	// Error handling
	void							warning (bool exception, const std::string &function, const char *format, ... ) const;

private:
	// The parents array
	std::vector<CParent>			Parents;

	// A vector of entries
	std::vector<CEntry>				Entries;

	// Recursive call count
	mutable uint32					Round;

	// The form DFN filename
	std::string						_Filename;

private:
	// Read method called by the form loader
	void							read (xmlNodePtr doc, CFormLoader &loader, bool forceLoad, const std::string &filename);
};

} // NLGEORGES

#endif // NL_FORM_DFN_H

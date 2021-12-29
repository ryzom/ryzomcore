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

#ifndef NL_U_FORM_DFN_H
#define NL_U_FORM_DFN_H

#include "nel/misc/types_nl.h"
#include "u_form_elm.h"
#include "nel/misc/smart_ptr.h"

namespace NLGEORGES
{

class UType;

/**
  * Georges DFN ifle interface
  *
  * \author Cyril 'Hulud' Corvazier
  * \author Nevrax France
  * \date 2002
  */
class UFormDfn : public NLMISC::CRefCount // Deprecated , public UFormElm
{
public:

  virtual ~UFormDfn() { }


	// ** Common methods


	// Type of dfn entry
	enum TEntryType
	{
		EntryType,
		EntryDfn,
		EntryVirtualDfn,
	};

	/**
	  * Return the number of entry in this DFN
	  */
	virtual uint getNumEntry () const = 0;

	/**
	  * Return the entry type.
	  * Doesn't look in parents DFN.
	  *
	  * \param entry is the entry id to get the type.
	  * \param type will be filled with the entry type.
	  * \param array is true if the entry is an array, else false.
	  * \return true if successed, false if the entry doesn't exist.
	  */
	virtual bool getEntryType (uint entry, TEntryType &type, bool &array) const = 0;

	/**
	  * Return the entry name
	  * Doesn't look in parents DFN.
	  *
	  * \param entry is the entry id to get the dfn pointer.
	  * \param name will be filled with the entry name.
	  * \return true if successed, false if the entry doesn't exist.
	  */
	virtual bool	getEntryName (uint entry, std::string &name) const = 0;

	virtual	bool	getEntryIndexByName (uint &entry, const	std::string &name) const = 0;

	/**
	  * Return the filename of the type or the DFN.
	  * Doesn't look in parents DFN.
	  *
	  * \param entry is the entry id to get the dfn pointer.
	  * \param name will be filled with the entry filename.
	  * \return true if successed, false if the entry doesn't exist or is a virtual DFN.
	  */
	virtual bool getEntryFilename (uint entry, std::string &name) const = 0;

	/**
	  * Return the filename extension used by the DFN entry.
	  * Doesn't look in parents DFN.
	  *
	  * \param entry is the entry id to get the dfn pointer.
	  * \param name will be filled with the entry filename.
	  * \return true if successed, false if the entry doesn't exist or is a virtual DFN.
	  */
	virtual bool getEntryFilenameExt (uint entry, std::string &name) const = 0;

	/**
	  * Return the entry DFN pointer
	  * Doesn't look in parents DFN.
	  *
	  * \param entry is the entry id to get the dfn pointer.
	  * \param dfn will be filled with the DFN pointer.
	  * \return true if successed, false if the entry doesn't exist or is not a DFN.
	  */
	virtual bool getEntryDfn (uint entry, UFormDfn **dfn) = 0;

	/**
	  * Return the entry DFN pointer
	  * Doesn't look in parents DFN.
	  *
	  * \param name is the supposed name of the dfn.
	  * \param dfn will be filled with the DFN pointer.
	  * \return true if successed, false if the entry doesn't exist or is not a DFN.
	  */
	virtual bool	getEntryDfnByName		(const std::string &name, UFormDfn **dfn) = 0;
	virtual bool	isAnArrayEntryByName	(const std::string &name)	const = 0;

	/**
	  * Return the entry Type pointer
	  * Doesn't look in parents DFN.
	  *
	  * \param entry is the entry id to get the dfn pointer.
	  * \param type will be filled with the TYPE pointer.
	  * \return true if successed, false if the entry doesn't exist or is not a Type.
	  */
	virtual bool getEntryType (uint entry, UType **type) = 0;

	/**
	  * Get the number of parent DFN.
	  * Doesn't look in parents DFN for parents.
	  */
	virtual uint getNumParents () const = 0;

	/**
	  * Get a parent.
	  *
	  * \param entry is the entry id to get the dfn pointer.
	  * \param parent will be filled with the parent pointer.
	  * \return true if successed, false if the parent doesn't exist.
	  */
	virtual bool getParent (uint parent, UFormDfn **parentRet) = 0;

	/**
	  * Get a parent filename.
	  *
	  * \param entry is the entry id to get the dfn pointer.
	  * \param parent will be filled with the parent pointer.
	  * \return true if successed, false if the parent doesn't exist.
	  */
	virtual bool getParentFilename (uint parent, std::string &filename) const = 0;

	/**
	  * Get the comment
	  */
	virtual const std::string &getComment () const = 0;

	/**
	  * Get dependency files
	  */
	virtual void	getDependencies (std::set<std::string> &dependencies) const = 0;
};

} // NLGEORGES

#endif // NL_U_FORM_DFN_H

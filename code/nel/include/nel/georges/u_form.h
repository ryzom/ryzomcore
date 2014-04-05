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

#ifndef NL_U_FORM_H
#define NL_U_FORM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include <set>

namespace NLMISC
{
class IStream;
}

namespace NLGEORGES
{

class UFormElm;

/**
  * This class provide an interface to access Georges form
  */
class UForm : public NLMISC::CRefCount
{
public:

	virtual ~UForm ();

	/**
	  * Access form nodes
	  */

	/// Get a mutable pointer on the root element of the form. It is a struct node.
	virtual UFormElm&		getRootNode () = 0;

	/// Get a const pointer on the root element of the form. It is a struct node.
	virtual const UFormElm& getRootNode () const = 0;

	/** Write the form in a stream.
	  *
	  * \param stream is the stream used to write the form
	  */
	virtual void			write (NLMISC::IStream &stream) = 0;

	/**
	  * Access form parents
	  */

	/// Get a mutable pointer on the root element of the form. It is a struct node.
	virtual uint			getNumParent () const = 0;

	/// Get a mutable pointer on the root element of the form. It is a struct node.
	virtual UForm			*getParentForm (uint parent) const = 0;

	// Get the form filename with extension
	virtual const std::string &getFilename () const = 0;

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

#endif // NL_U_FORM_H

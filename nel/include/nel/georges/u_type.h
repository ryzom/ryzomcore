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

#ifndef NL_U_TYPE_H
#define NL_U_TYPE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

namespace NLGEORGES
{

/**
 * Georges type interface
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class UType : public NLMISC::CRefCount
{
public:

  virtual ~UType() { }

	// ** Type
	enum TType
	{
		UnsignedInt=0,
		SignedInt,
		Double,
		String,
		Color,
		TypeCount
	};

	/**
	  * Get the type of this type.
	  */
	virtual TType				getType () const = 0;

	/**
	  * Get the default value of this type.
	  */
	virtual const std::string	&getDefault () const = 0;

	/**
	  * Get the min value of this type.
	  */
	virtual const std::string	&getMin () const = 0;

	/**
	  * Get the max value of this type.
	  */
	virtual const std::string	&getMax () const = 0;

	/**
	  * Get the increment value of this type.
	  */
	virtual const std::string	&getIncrement () const = 0;

	/**
	  * Get the definition count for this type.
	  */
	virtual uint				getNumDefinition () const = 0;

	/**
	  * Get a definition for this type.
	  *
	  * index is the index of the defnition you want to get.
	  * If the method returns true, label will be filled with
	  * the definition label and value will be filled with the
	  * defnition value.
	  * The method returns false if the index is invalid. In this
	  * case, label and value are not changed.
	  */
	virtual bool				getDefinition (uint index, std::string &label, std::string &value) const = 0;

	/**
	  * Get the comments of type.
	  */
	virtual const std::string	&getComment () const = 0;

	/**
	  * Get dependency files
	  */
	virtual void	getDependencies (std::set<std::string> &dependencies) const = 0;
};


} // NLGEORGES


#endif // NL_U_TYPE_H

/* End of u_type.h */

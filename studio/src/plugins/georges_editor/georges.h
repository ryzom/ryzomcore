// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian JAEKEL <aj@elane2k.com>
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

#ifndef GEORGES_H
#define GEORGES_H

// Misc

// STL includes
#include <string>

// NeL includes

// Qt includes

// Project includes

namespace NLGEORGES
{
	class UType;
	class UForm;
	class UFormDfn;
	class UFormLoader;
}

using namespace NLGEORGES;

namespace GeorgesQt 
{

	/**
	@class CGeorges
	A CGeorges class loading and viewing sheets.
	*/
	class CGeorges
	{
	public:
		/// Default constructor.
		CGeorges();
		virtual ~CGeorges();

		// Load the given form root
		UForm* loadForm(std::string formName);
		// Load a dfn
		UFormDfn* loadFormDfn(std::string formName);
		// Load a type
		UType *loadFormType (std::string formName);

		// A form loader
		UFormLoader	*FormLoader;

	};/* class CGeorges */

} /* namespace GeorgesQt */

#endif // GEORGES_H

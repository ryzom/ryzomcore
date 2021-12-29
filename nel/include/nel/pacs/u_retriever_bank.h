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

#ifndef NL_U_RETRIEVER_BANK_H
#define NL_U_RETRIEVER_BANK_H

#include "nel/misc/types_nl.h"

namespace NLMISC
{
	class IProgressCallback;
}

namespace NLPACS {


/**
 * TODO Class description
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class URetrieverBank
{
public:

	/**
	  * Create a retriever bank.
	  *
	  * \param retrieverBank is the global retriever bank path file name. This method use CPath to find the retriever
	  * \return the retriever bank interface or NULL if the bank was not found.
	  */
	static URetrieverBank	*createRetrieverBank (const char* retrieverBank, bool loadAll = true);

	/**
	  * Delete a retriever bank.
	  */
	static void				deleteRetrieverBank (URetrieverBank *retrieverBank);
};


} // NLPACS


#endif // NL_U_RETRIEVER_BANK_H

/* End of u_retriever_bank.h */

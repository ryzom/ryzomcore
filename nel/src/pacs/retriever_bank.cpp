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

#include "stdpacs.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/progress_callback.h"

#include "nel/pacs/retriever_bank.h"

using namespace std;
using namespace NLMISC;

// CRetrieverBank methods implementation

NLPACS::URetrieverBank *NLPACS::URetrieverBank::createRetrieverBank (const char *retrieverBank, bool loadAll)
{

	CIFile	file;
	if (file.open( CPath::lookup(retrieverBank) ))
	{
		CRetrieverBank	*bank = new CRetrieverBank();

		bank->_AllLoaded = loadAll;
		bank->_NamePrefix = CFile::getFilenameWithoutExtension(retrieverBank);

		file.serial(*bank);

		return static_cast<URetrieverBank *>(bank);
	}
	else
		return NULL;
}


void	NLPACS::URetrieverBank::deleteRetrieverBank (NLPACS::URetrieverBank *retrieverBank)
{
	// Cast
//	nlassert (dynamic_cast<NLPACS::CRetrieverBank*>(retrieverBank));
	NLPACS::CRetrieverBank* r=static_cast<NLPACS::CRetrieverBank*>(retrieverBank);

	// Delete
	delete r;
}

void	NLPACS::CRetrieverBank::clean()
{
	uint	i;
	for (i=0; i<_Retrievers.size(); ++i)
	{
		_Retrievers[i].flushFullOrderedChains();
	}
}

// end of CRetrieverBank methods implementation

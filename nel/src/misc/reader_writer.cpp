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

#include "stdmisc.h"

#include "nel/misc/reader_writer.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

#ifndef NL_CPP17

CReaderWriter::CReaderWriter()
{
	_ReadersLevel = 0;
}

CReaderWriter::~CReaderWriter()
{
	// here some checks to avoid a reader/writer still working while we flush the mutexes...
}

#endif

void reader_writer_dummy_cpp__()
{
	CReaderWriter readerWriter;
	CRWSynchronized<int> num;
	{
		CRWSynchronized<int>::CReadAccessor access0(&num);
		CRWSynchronized<int>::CReadAccessor access1(&num);
	}
	{
		CRWSynchronized<int>::CWriteAccessor accessW(&num);
	}
}

} // NLMISC

/* end of file */

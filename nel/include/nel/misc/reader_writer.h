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

#ifndef NL_READER_WRITER_H
#define NL_READER_WRITER_H

#include "types_nl.h"
#include "mutex.h"


namespace NLMISC {

/**
 * This class allows a reader/writer ressource usage policy.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CReaderWriter
{
private:

	volatile CMutex	_Fairness;
	volatile CMutex	_ReadersMutex;
	volatile CMutex	_RWMutex;
	volatile sint	_ReadersLevel;

public:

	CReaderWriter();
	~CReaderWriter();

	void			enterReader()
	{
		const_cast<CMutex&>(_Fairness).enter();
		const_cast<CMutex&>(_ReadersMutex).enter();
		++_ReadersLevel;
		if (_ReadersLevel == 1)
			const_cast<CMutex&>(_RWMutex).enter();
		const_cast<CMutex&>(_ReadersMutex).leave();
		const_cast<CMutex&>(_Fairness).leave();
	}

	void			leaveReader()
	{
		const_cast<CMutex&>(_ReadersMutex).enter();
		--_ReadersLevel;
		if (_ReadersLevel == 0)
			const_cast<CMutex&>(_RWMutex).leave();
		const_cast<CMutex&>(_ReadersMutex).leave();
	}

	void			enterWriter()
	{
		const_cast<CMutex&>(_Fairness).enter();
		const_cast<CMutex&>(_RWMutex).enter();
		const_cast<CMutex&>(_Fairness).leave();
	}

	void			leaveWriter()
	{
		const_cast<CMutex&>(_RWMutex).leave();
	}
};

/**
 * This class uses a CReaderWriter object to implement a synchronized object (see mutex.h for standard CSynchronized.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
template <class T>
class CRWSynchronized
{
public:

	class CReadAccessor
	{
		CRWSynchronized<T>		*_RWSynchronized;

	public:

		CReadAccessor(CRWSynchronized<T> *cs)
		{
			_RWSynchronized = cs;
			const_cast<CReaderWriter&>(_RWSynchronized->_RWSync).enterReader();
		}

		~CReadAccessor()
		{
			const_cast<CReaderWriter&>(_RWSynchronized->_RWSync).leaveReader();
		}

		const T		&value()
		{
			return const_cast<const T&>(_RWSynchronized->_Value);
		}
	};

	class CWriteAccessor
	{
		CRWSynchronized<T>		*_RWSynchronized;

	public:

		CWriteAccessor(CRWSynchronized<T> *cs)
		{
			_RWSynchronized = cs;
			const_cast<CReaderWriter&>(_RWSynchronized->_RWSync).enterWriter();
		}

		~CWriteAccessor()
		{
			const_cast<CReaderWriter&>(_RWSynchronized->_RWSync).leaveWriter();
		}

		T		&value()
		{
			return const_cast<T&>(_RWSynchronized->_Value);
		}
	};

private:
	friend class CRWSynchronized::CReadAccessor;
	friend class CRWSynchronized::CWriteAccessor;

	volatile CReaderWriter		_RWSync;

	volatile T					_Value;

};

} // NLMISC


#endif // NL_READER_WRITER_H

/* End of reader_writer.h */

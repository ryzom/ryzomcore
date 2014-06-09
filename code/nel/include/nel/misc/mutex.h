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

#ifndef NL_MUTEX_H
#define NL_MUTEX_H

#include "types_nl.h"
#include "time_nl.h"
#include "common.h"
#include <map>

struct SDL_mutex;
typedef int SDL_SpinLock;

namespace NLMISC {

#define CFastMutex CAtomicLock
#define CFairMutex CMutex
#define CUnfairMutex CMutex

class CMutex
{
public:
	/// Constructor
	CMutex();
	CMutex(const std::string &name);

	/// Destructor
	~CMutex();

	/// Enter
	void enter();

	/// Leave
	void leave();

private:
	SDL_mutex *m_SDLMutex;

};

class CAtomicLock
{
public:
	/// Constructor
	CAtomicLock();

	/// Destructor
	~CAtomicLock();

	/// Same as constructor, useful for init in a shared memory block
	void init();

	/// Enter
	void enter();

	/// Leave
	void leave();

private:
	SDL_SpinLock m_SDLSpinLock;

};

/**
 * This class ensure that the Value is accessed by only one thread. First you have to create a CSynchronized class with your type.
 * Then, if a thread want to modify or do anything on it, you create a CAccessor in a \b sub \b scope. You can modify the value
 * of the CSynchronized using the value() function \b until the end of the scope. So you have to put the smaller scope as you can.
 *
 *\code
 // the value that you want to be thread safe.
 CSynchronized<int> val;
 { // create a new scope for the access
   // get an access to the value
   CSynchronized<int>::CAccessor acces(&val);
   // now, you have a thread safe access until the end of the scope, so you can do whatever you want. for example, change the value
   acces.value () = 10;
 } // end of the access
 *\endcode
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
template <class T, class TMutex = CMutex>
class CSynchronized
{
public:

	CSynchronized(const std::string &name) : m_Mutex(name) { }

	/**
	 * This class give you a thread safe access to the CSynchronized Value. Look at the example in the CSynchronized.
	 */
	class CAccessor
	{
		CSynchronized<T> *Synchronized;
	public:

		/// get the mutex or wait
		CAccessor(CSynchronized<T> *cs)
		{
			Synchronized = cs;
			const_cast<CMutex &>(Synchronized->m_Mutex).enter();
		}

		/// release the mutex
		~CAccessor()
		{
			const_cast<CMutex &>(Synchronized->m_Mutex).leave();
		}

		/// access to the Value
		T &value()
		{
			return const_cast<T &>(Synchronized->m_Value);
		}
	};

private:

	friend class CSynchronized::CAccessor;

	/// The mutex of the synchronized value.
	volatile TMutex m_Mutex;

	/// The synchronized value.
	volatile T m_Value;
};

template <class T>
class CUnfairSynchronized : public CSynchronized<T, CUnfairMutex> { };

template <class T>
class CFairSynchronized : public CSynchronized<T, CFairMutex> { };

template <class T>
class CFastSynchronized : public CSynchronized<T, CFastMutex> { };

/** Helper class that allow easy usage of mutex to protect
 *	a local block of code with an existing mutex.
 */
template <class TMutex = CMutex>
class CAutoMutex
{
	TMutex	&_Mutex;

	// forbeden copy or assignent
	CAutoMutex(const CAutoMutex &/* other */)
	{
	}

	CAutoMutex &operator = (const CAutoMutex &/* other */)
	{
		return *this;
	}

public:
	CAutoMutex(TMutex &mutex)
		:	_Mutex(mutex)
	{
		_Mutex.enter();
	}

	~CAutoMutex()
	{
		_Mutex.leave();
	}

};

} // NLMISC

#endif // NL_MUTEX_H

/* End of mutex.h */

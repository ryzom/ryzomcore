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

#ifndef NL_PS_ATTRIB_H
#define NL_PS_ATTRIB_H



#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/ps_allocator.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include "nel/misc/common.h"


namespace NL3D {

/** a container that is like a vector, but snapped to (1<<snapPower) byte memory pages
  */
template <class T, const uint snapPower = 5>
class CSnappedVector
{
public:
	typedef T *iterator;
	typedef const T *const_iterator;
	typedef T value_type;

	CSnappedVector() : _Size(0), _Capacity(0), _Start(NULL), _Tab(NULL) {}
	~CSnappedVector()
	{
		nlassert(_Size <= _Capacity);
		for (iterator it = _Tab, endIt = _Tab + _Size; it != endIt; ++it)
		{
			it->~T();
		}
		delete _Start;
	}
	iterator begin(void) { return _Tab; }
	const_iterator begin(void) const { return _Tab; }
	iterator end(void) { return _Tab + _Size; }
	const_iterator end(void) const { return _Tab + _Size; }

	T &operator[](uint index)
	{
		#ifdef NL_DEBUG
			nlassert(index < _Size && _Size);
		#endif
		return _Tab[index];
	}
	const T &operator[](uint index) const
	{
		#ifdef NL_DEBUG
			nlassert(index < _Size && _Size);
		#endif
		return _Tab[index];
	}

	T &back()
	{
		#ifdef NL_DEBUG
			nlassert(_Size > 0);
		#endif
		return _Tab[_Size - 1];
	}

	const T &back() const
	{
		#ifdef NL_DEBUG
			nlassert(_Size > 0);
		#endif
		return _Tab[_Size - 1];
	}

	bool empty() const { return _Size == 0; }

	/// set a new usable size
	void reserve(uint capacity)
	{
		if (capacity < _Capacity) return;
		uint8 *newStart = NULL;
		try
		{
			newStart = new uint8[sizeof(T) * capacity + (1 << snapPower)];
			T *newTab = (T *) ( (size_t) (newStart + (1 << snapPower))  & ~((1 << snapPower) - 1)); // snap to a page



			for (iterator src = _Tab, end = _Tab + (capacity < _Size ? capacity : _Size), dest = newTab;
			     src != end;
				 ++ src, ++dest)
			{
				new ((void *) dest) T(*src); // copy object
			}

			// swap datas
			std::swap(_Start, newStart);
			std::swap(_Tab, newTab);

			// destroy previous objects. We assume that we can't have exceptions raised from destructors
			for (iterator it = newTab /* old tab */, endIt = newTab + _Size; it != endIt; ++ it)
			{
				it->~T();
			}

			// set new size
			_Capacity = capacity;
			_Size    = capacity < _Size ? capacity : _Size;


			// delete old vect (that was swapped with the new one)
			delete [] newStart;
			nlassert(_Size <= _Capacity);
		}
		catch (...)
		{
			delete [] newStart;
			throw;
		}

	}
	void resize(uint size)
	{
		nlassert(size < (1 << 16));
		if (size < _Size)
		{
			for (iterator it = _Tab + size, endIt = _Tab + _Size; it != endIt; ++it)
			{
				it->~T();
			}
		}
		else
		{
			if (size > _Capacity)
			{
				reserve(size);
			}
			for (iterator it = _Tab + _Size, endIt = _Tab + size; it != endIt; ++it)
			{
				new ((void *) it) T();
			}
		}

		_Size = size;
		nlassert(_Size <= _Capacity);
	}

	void push_back(const T &t)
	{
		if (!_Size)
		{
			reserve(2);
			new ((void *) _Tab) T(t);
			_Size = 1;
		}
		else
		if (_Size < _Capacity)
		{
			new ((void *) (_Tab + _Size)) T(t);
			++_Size;
		}
		else
		if (_Size == _Capacity)
		{
			if (_Capacity == 1)
			{
				reserve(2);
			}
			else
			{
				reserve(_Capacity + (_Capacity>>1));
			}
			nlassert(_Size <= _Capacity);
			new ((void *) (_Tab + _Size)) T(t);
			++_Size;
		}
	}

	void pop_back()
	{
		nlassert(_Size);
		_Tab[_Size - 1].~T();
		--_Size;
	}


	uint capacity() const  { return _Capacity; }
	uint size() const { return _Size; }

	/// serialization
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		if (f.isReading())
		{
			clear();
			uint32 size, maxsize;
			f.serial(size, maxsize);
			reserve(maxsize);
			for (uint k = 0; k < size; ++k)
			{
				T tmp;
				f.serial(tmp);
				push_back(tmp);
			}
		}
		else
		{
			f.serial(_Size, _Capacity);
			for (uint k = 0; k < _Size; ++k)
			{
				f.serial(_Tab[k]);
			}
		}
	}

	// clear
	void clear() { resize(0); }

protected:

	uint8 *_Start;   // real allocation address
	T *_Tab;       // first element
	uint32 _Size;    // used elements
	uint32 _Capacity; // max size
};






/**
 * This class is intended to store an attribute list in a located or in a located bindable
 * such as speed, color and so on. It is important to remember that a located holds all instance of object of
 * one type (force, emitter, particles or both...).
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */

template <typename T> class CPSAttrib
{
public:

	/// \name Object
	//@{
			/// ctor
			CPSAttrib();

			/// Serialization method
			void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

			// swap with another vector
			void swap(CPSAttrib<T> &other);
	//@}

	/// \name Useful typedefs
	//@{
		/** Container used by this class to store its datas.
		  * The container type is likely to change depending on memory requirement.
		  */
		//typedef CSnappedVector<T> TContType;
		typedef typename CPSVector<T>::V TContType;

		/// The type used by the container. Its is the type used to instanciate this template.
		typedef T value_type;

		/// an iterator on the datas
		typedef typename TContType::iterator iterator;
		/// a const iterator on the datas
		typedef typename TContType::const_iterator const_iterator;
	//@}


	/// \name Size of the container
	//@{
		/** Resize the attributes tab. This tells what is the max number of element in this tab, but don't add elements.
		  * The behaviour is much like std::vector::reserve
		  */
		void					resize(uint32 nbInstances);

		/// return the number of instance in the container
		uint32 getSize(void) const { return (uint32)_Tab.size(); }

		/// return the max number of instance in the container
		uint32 getMaxSize(void) const { return _MaxSize; }

	//@}


	/// \name Element access.
	//@{
		/// get a const reference on an attribute instance
		const T &				operator[](uint32 index) const
		{
			#ifdef NL_DEBUG
				nlassert(index < _Tab.size());
			#endif
			return _Tab[index];
		}

		/// get a reference on an attribute instance
		T &						operator[](uint32 index)
		{
			#ifdef NL_DEBUG
				nlassert(index < _Tab.size());
			#endif
			return _Tab[index];
		}

		// get a const reference on the last element
		const T &back() const
		{
			return _Tab.back();
		}

		// get a reference on the last element
		T &back()
		{
			return _Tab.back();
		}


	//@}



	/// \name Iterator / enumeration
	//@{


		/// Get an iterator at the beginning of the container
		iterator				begin(void) { return _Tab.begin(); }

		/// Get an iterator at the end of the container
		iterator				end(void) { return _Tab.end(); }

		/// Get a  const_iterator at the beginning of the container
		const_iterator			begin(void) const { return _Tab.begin(); }

		/// Get a  const_iterator at the end of the container
		const_iterator			end(void) const { return _Tab.end(); }
	//@}

	/// \name Add / remove methods
	//@{
		/**
		 * create a new object in the tab. It is append at the end of it
		 * \return the index if there were enough room for it or -1 else
		 */
		sint32 insert(const T &t = T() );

		/// remove an object from the tab
		void remove(uint32 index);

		/// clear the container
		void clear(void)
		{
			_Tab.clear();
		}
	//@}

protected:
	TContType _Tab;
	uint32    _MaxSize; // the max number of elements that can be stored
};





/////////////////////////////////////////////////////////////////////////
//					IMPLEMENTATION									   //
/////////////////////////////////////////////////////////////////////////

template <typename T>
CPSAttrib<T>::CPSAttrib()
{
	_MaxSize = DefaultMaxLocatedInstance;
}


template <typename T>
void CPSAttrib<T>::resize(uint32 nbInstances)
{
	nlassert(nbInstances < (1 << 16));
	_Tab.reserve(nbInstances);
	_MaxSize = nbInstances;
}


template <typename T>
sint32 CPSAttrib<T>::insert(const T &t)
{
	if (_Tab.size() == _MaxSize && _Tab.size() > DefaultMaxLocatedInstance)
	{
		return -1;
	}
	_Tab.push_back(t);
	return (sint32)_Tab.size() - 1;
}


template <typename T>
void CPSAttrib<T>::remove(uint32 index)
{
	nlassert(index < _Tab.size());
	// we copy the last element in place of this one
	if (index != _Tab.size() - 1)
	{
		_Tab[index] = _Tab[_Tab.size() - 1];
	}
	_Tab.pop_back();

}

template <typename T>
void CPSAttrib<T>::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// version 4 to 5 => bug with size being > capacity
	sint ver = f.serialVersion(5);

	// in the first version, size was duplicated, we were using a std::vector ...
	if (ver == 1)
	{
		if(f.isReading())
		{
			uint32 size;
			f.serial(size);
			f.serial(_MaxSize);
			_Tab.reserve(_MaxSize);
			f.serial(size); // useless but, we were previously doing a serialCont... compatibility purpose only
			T tmp;
			// Read the vector
			for(uint i = 0; i < size; i++)
			{
				f.serial(tmp);
				_Tab.push_back(tmp);
			}
			nlassert(_Tab.size() == size);
		}
		else
		{
			uint32 size = (uint32)_Tab.size();
			f.serial(size);
			f.serial(_MaxSize);
			f.serial(size);
			// write the vector
			for(uint i = 0; i < size; i++)
			{
				f.serial(_Tab[i]);
			}
		}
	}

	if (ver == 2) // this version didn't work well, it relied on the capacity of the container to store the max number of instances
	{
		nlassert(0);
	/*	f.serial(_Tab);
		if (f.isReading())
		{
			_MaxSize = _Tab.capacity();
		}*/
	}

	if (ver >= 3)
	{
		f.serial(_MaxSize);
		_Tab.reserve(_MaxSize);
		//f.serial(_Tab);


		if (f.isReading())
		{
			_Tab.clear();
			uint32 size, maxsize;
			if (ver == 3)
			{
				f.serial(size, maxsize);
				//_Tab.reserve(maxsize);
			}
			else
			{
				f.serial(size);
				maxsize = _MaxSize;
			}
			if (ver > 4)
			{
				_Tab.resize(size);
				for (uint k = 0; k < size; ++k)
				{
					f.serial(_Tab[k]);
				}
			}
			else
			{
				// bug for version 4: size may be > maxsize
				if (size <= maxsize)
				{
					// ok, no bug
					_Tab.resize(size);
					for (uint k = 0; k < size; ++k)
					{
						f.serial(_Tab[k]);
					}
				}
				else
				{
					// size > maxsize, not good ..!
					_Tab.resize(maxsize);
					uint k;
					for (k = 0; k < maxsize; ++k)
					{
						f.serial(_Tab[k]);
					}
					T dummy;
					for (; k < size; ++k)
					{
						f.serial(dummy);
					}
				}
			}
		}
		else
		{
			uint32 size = (uint32)_Tab.size(), capacity = (uint32)_Tab.capacity();
			if (ver == 3)
			{
				f.serial(size, capacity);
			}
			else
			{
				f.serial(size);
			}
			for (uint k = 0; k < size; ++k)
			{
				f.serial(_Tab[k]);
			}
		}
	}
}

template <typename T>
void CPSAttrib<T>::swap(CPSAttrib<T> &other)
{
	std::swap(_MaxSize, other._MaxSize);
	_Tab.swap(other._Tab);
}

// here we give some definition for common types

typedef CPSAttrib<NLMISC::CVector> TPSAttribVector;
typedef CPSAttrib<NLMISC::CRGBA>   TPSAttribRGBA;
typedef CPSAttrib<float>		   TPSAttribFloat;
typedef CPSAttrib<uint32>		   TPSAttribUInt;
typedef CPSAttrib<uint8>		   TPSAttribUInt8;
typedef CPSAttrib<TAnimationTime>  TPSAttribTime;

} // NL3D

#endif // NL_PS_ATTRIB_H

/* End of ps_attrib.h */

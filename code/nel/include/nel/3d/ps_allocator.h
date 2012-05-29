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

#ifndef NL_PS_ALLOCATOR_H
#define NL_PS_ALLOCATOR_H

#include "nel/misc/time_nl.h"
#include "nel/misc/contiguous_block_allocator.h"
//
#include <map>

namespace NL3D
{

#ifdef NL_OS_WINDOWS
#	if (__SGI_STL_PORT > 0x449) && (__SGI_STL_PORT < 0x460) || !defined(NL_DONT_USE_EXTERNAL_CODE)

		// fast mem alloc of particle systems only on windows for now
		//
		// PS_FAST_ALLOC // for fast alloc
//#		define PS_FAST_ALLOC
#	endif
#endif // NL_OS_WINDOWS


#ifndef PS_FAST_ALLOC
	// the following macros, that redefines new & delete for classes of ps, is now empty
	// NB : When using NL_MEMORY, we solve the redefinition of new errors as follow :
	//
	// PS_FAST_OBJ_ALLOC
#	define PS_FAST_OBJ_ALLOC
	// Partial specialization tips for vectors. For non-windows version it just fallback on the default allocator
	// To define a vector of type T, one must do : CPSVector<T>::V MyVector;
	template <class T> struct CPSVector
	{
		typedef std::vector<T, std::allocator<T> > V;
	};
	// partial specialisation tips for multimap
	template <class T, class U, class Pr = std::less<T> > struct CPSMultiMap
	{
#ifdef NL_OS_MAC
	    typedef std::multimap<T, U, Pr > M;
#else
	    typedef std::multimap<T, U, Pr, std::allocator<T> > M;
#endif
	};

#else

	// current contiguous block allocator to be used by the particle system allocator. std::allocator is used if no such allocator is provided
	extern NLMISC::CContiguousBlockAllocator *PSBlockAllocator;

	// an stl allocator for objects used in particle systems (vectors ..)

// STLPort 4.5.x version of allocator : written by Nevrax
#if(__SGI_STL_PORT > 0x449) && (__SGI_STL_PORT < 0x460)
	template<class T> class CPSAllocator
	{
	public:
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T *pointer;
		typedef const T *const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		pointer address(reference x) const { return &x; }
		const_pointer address(const_reference x) const { return &x; }
		CPSAllocator() {}
		CPSAllocator<T>& operator=(const CPSAllocator<T> &other) { return *this; }
	private:
		typedef NLMISC::CContiguousBlockAllocator *TBlocAllocPtr;
	public:
		pointer allocate(size_type n, const void *hint)
		{
			++NumPSAlloc;
			TBlocAllocPtr *result;
			// prefix the block to say it comes from a contiguous block allocator or not
			if (PSBlockAllocator)
			{
				result = (TBlocAllocPtr *) PSBlockAllocator->alloc(n * sizeof(T) + sizeof(TBlocAllocPtr *));
				*result = PSBlockAllocator; // mark as a block from block allocator
			}
			else
			{
				 result = (TBlocAllocPtr *) _Alloc.allocate(n * sizeof(T) + sizeof(TBlocAllocPtr *));
				 *result = NULL; // mark as a stl block
			}
			return (pointer) (result + 1); // usable space starts after header
		}
		void deallocate(pointer p, size_type n)
		{
			++NumDealloc;
			--NumPSAlloc;
			if (!p) return;
			pointer realAddress = (pointer) ((uint8 *) p - sizeof(TBlocAllocPtr *));
			if (* (TBlocAllocPtr *) realAddress)
			{
				// block comes from a block allocator
				(*(TBlocAllocPtr *) realAddress)->free((void *) realAddress, n * sizeof(T) + sizeof(TBlocAllocPtr *));
			}
			else
			{
				// block comes from the stl allocator
				_Alloc.deallocate((uint8 *) realAddress, n * sizeof(T) + sizeof(TBlocAllocPtr *));
			}
		}
		void construct(pointer p, const T& val) { new (p) T(val); }
		void destroy(pointer p) { p->~T(); }
		// stl rebind
		template <class _Tp, class U>
			CPSAllocator<U>& __stl_alloc_rebind(CPSAllocator<_Tp>& __a, const U*)
		{
			return (CPSAllocator<U>&)(__a);
		}

		template <class _Tp, class U>
			CPSAllocator<U> __stl_alloc_create(const CPSAllocator<_Tp>&, const U*)
		{
			return CPSAllocator<U>();
		}

	private:
		std::allocator<uint8> _Alloc;
	};

// STLPort 4.6.x and 5.0.x version of allocator : written by Neverborn Entertainment
#elif(__SGI_STL_PORT > 0x459)

	template<class T> class CPSAllocator
	{
	public:

		typedef T value_type;
		typedef value_type *pointer;
		typedef const T *const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

#ifdef _STLP_MEMBER_TEMPLATE_CLASSES
		template <class U> struct rebind {
				typedef CPSAllocator<U> other;
		};
#endif

		CPSAllocator() _STLP_NOTHROW {}

#ifdef _STLP_MEMBER_TEMPLATES
		template <class U> CPSAllocator(const CPSAllocator<U>&) _STLP_NOTHROW {}
#endif

		CPSAllocator(const CPSAllocator<T>&) _STLP_NOTHROW {}

		CPSAllocator<T>& operator=(const CPSAllocator<T> &other) { return *this; }
		~CPSAllocator() _STLP_NOTHROW {}

		pointer address(reference x) const { return &x; }
		const_pointer address(const_reference x) const { return &x; }

	private:

		typedef NLMISC::CContiguousBlockAllocator *TBlocAllocPtr;

	public:

		pointer allocate(size_type n, const void* = 0)
		{
			++NumPSAlloc;
			TBlocAllocPtr *result;
			// prefix the block to say it comes from a contiguous block allocator or not
			if (PSBlockAllocator)
			{
			    result = (TBlocAllocPtr *) PSBlockAllocator->alloc(n * sizeof(T) + sizeof(TBlocAllocPtr *));
				*result = PSBlockAllocator; // mark as a block from block allocator
			}
			else
			{
				result = (TBlocAllocPtr *) _Alloc.allocate(n * sizeof(T) + sizeof(TBlocAllocPtr *));
				*result = NULL; // mark as a stl block
			}
			return (pointer) (result + 1); // usable space starts after header
		}

		void deallocate(pointer p, size_type n)
		{
			++NumDealloc;
			--NumPSAlloc;
			if (!p) return;
			pointer realAddress = (pointer) ((uint8 *) p - sizeof(TBlocAllocPtr *));
			if (* (TBlocAllocPtr *) realAddress)
			{
				// block comes from a block allocator
				(*(TBlocAllocPtr *) realAddress)->free((void *) realAddress, n * sizeof(T) + sizeof(TBlocAllocPtr *));
			}
			else
			{
				// block comes from the stl allocator
				_Alloc.deallocate((uint8 *) realAddress, n * sizeof(T) + sizeof(TBlocAllocPtr *));
			}
		}

		void construct(pointer p, const T& val) { new (p) T(val); }
		void destroy(pointer p) { p->~T(); }

		size_type max_size() const _STLP_NOTHROW  { return size_t(-1) / sizeof(value_type); }

		// stl rebind
		template <class _Tp, class U> CPSAllocator<U>& __stl_alloc_rebind(CPSAllocator<_Tp>& __a, const U*)
		{
			return (CPSAllocator<U>&)(__a);
		}

		// stl create
		template <class _Tp, class U> CPSAllocator<U> __stl_alloc_create(const CPSAllocator<_Tp>&, const U*)
		{
			return CPSAllocator<U>();
		}

	private:
		std::allocator<uint8> _Alloc;
	};

#endif

	// allocation of objects of ps (to be used inside base class declaration, replaces operator new & delete)
#	define PS_FAST_OBJ_ALLOC \
	void *operator new(size_t size) { return PSFastMemAlloc((uint) size); }\
	void operator delete(void *block) { PSFastMemFree(block); }


	// Partial specialization tips for vectors
	// To define a vector of type T, one must do : CPSVector<T>::V MyVector;
	template <class T> struct CPSVector
	{
		typedef std::vector<T, CPSAllocator<T> > V;
	};
	// partial specialisation tips for multimap
	template <class T, class U, class Pr = std::less<T> > struct CPSMultiMap
	{
		typedef std::multimap<T, U, Pr, CPSAllocator<T> > M;
	};

	extern uint NumPSAlloc;
	extern uint NumDealloc;

	// allocation of memory block for objects of a particle system
	void *PSFastMemAlloc(uint numBytes);
	void PSFastMemFree(void *block);

#endif // PS_FAST_ALLOC

} // NL3D

#endif // NL_PS_ALLOCATOR_H

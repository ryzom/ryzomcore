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

#ifndef NL_RADIX_SORT_H
#define NL_RADIX_SORT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"


namespace NL3D {


// ***************************************************************************
/**
 * A class which sort elements T with radix sort algorithm.
 *	T must follow the following interface:
 *		- uint32	getRadixKey() const;
 *		- have a correct operator=()
 *
 *	getRadixKey() return the unsigned key for this element.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
template<class T> class CRadixSort
{
public:

	/** Constructor
	 *	\param keyDepth default is 32 bits, but if as example you're sure that you
	 *		just use 14 bits, you can set keyDepth=14. Clamped to 1,32.
	 *	\param digitDepth default is 8 bits. The sort will do ceil(keyDepth/digitDepth) pass in Sort.
	 *		But be aware that a CRadixSort object has a size of (1<<digitDepth) * 8 bytes. (2K for digitDepth=8).
	 *		Be aware too that values>8 is not a really good idea because of cache, and random access...
	 *		And this REALLY impact (eg: from 1 to 100 if you choose 16 instead than 8...)
	 *		Clamped to 1,min(16, keyDepth).
	 */
	CRadixSort(uint keyDepth= 32, uint digitDepth= 8)
	{
		// setup
		_KeyDepth= keyDepth;
		NLMISC::clamp(_KeyDepth, 1U, 32U);
		_DigitDepth= digitDepth;
		NLMISC::clamp(_DigitDepth, 1U, 16U);
		_DigitDepth= std::min(_DigitDepth, _KeyDepth);

		// resize the array of digits.
		_DigitSize= 1<<_DigitDepth;
		_SortDigits.resize(_DigitSize);
	}

	/**	Sort an array of T, in (keyDepth/digitSize)*O(size). Elements are reordered in increasing order.
	 *	\param array0 ptr on elements to be sorted.
	 *	\param array1 an array of T which must be allocated with same size.
	 *	\param size the number of elements.
	 *	\return the array which have sorted elements. Actually return array0 if
	 *		ceil(keyDepth/digitDepth) is pair, else return array1.
	 *		The other array has elements in undefined order.
	 */
	T				*sort(T *array0, T *array1, uint size) {return doSort(array0, array1, size, true);}


	/**	same as sort(), but elements are reordered in decreasing order
	 *	\see sort()
	 */
	T				*reverse_sort(T *array0, T *array1, uint size) {return doSort(array0, array1, size, false);}


// ***********************
private:

	struct	CSortDigit
	{
		// For a digit, how many elements it has in the array.
		uint	Count;
		// current ptr where to write in destArray
		T		*Ptr;
	};

	// setup
	uint			_KeyDepth;
	uint			_DigitDepth;
	uint			_DigitSize;
	// We sort digits per digit (default is byte per byte)
	std::vector<CSortDigit>		_SortDigits;


	/// radix sort algorithm
	T				*doSort(T *arraySrc, T *arrayDst, uint size, bool increasingOrder)
	{
		if(size==0)
			return arraySrc;

		// for all digits.
		for(uint	digit=0; digit< _KeyDepth; digit+=_DigitDepth )
		{
			sint	i;
			// how many bits do we shift?
			uint	digitShift= digit;
			uint	digitMask= _DigitSize - 1;

			// Init digit count.
			for(i=0; i<(sint)_DigitSize; i++)
			{
				_SortDigits[i].Count= 0;
			}

			// for all elements in array, count the usage of current digit.
			T	*srcPtr= arraySrc;
			for(i=0; i<(sint)size; i++, srcPtr++)
			{
				// get the key for this element
				uint32	key= srcPtr->getRadixKey();
				// get the actual digit of interest
				key>>= digitShift;
				key&= digitMask;
				// increment the use of this digit.
				_SortDigits[key].Count++;
			}

			// for all digit, init start Ptr.
			T	*dstPtr= arrayDst;
			if(increasingOrder)
			{
				for(i=0; i<(sint)_DigitSize; i++)
				{
					// setup the dest ptr for this digit.
					_SortDigits[i].Ptr= dstPtr;
					// increment the ptr of digit usage
					dstPtr+= _SortDigits[i].Count;
				}
			}
			else
			{
				// reverse order of copy for digits, so the biggest one will
				// copy in the beginning of the array
				for(i=_DigitSize-1; i>=0; i--)
				{
					// setup the dest ptr for this digit.
					_SortDigits[i].Ptr= dstPtr;
					// increment the ptr of digit usage
					dstPtr+= _SortDigits[i].Count;
				}
			}

			// for all elements, sort for this digit, by copying from src to dest.
			srcPtr= arraySrc;
			for(i=0; i<(sint)size; i++, srcPtr++)
			{
				// get the key for this element
				uint32	key= srcPtr->getRadixKey();
				// get the actual digit of interest
				key>>= digitShift;
				key&= digitMask;
				// copy to good digit dst, and increment dest ptr.
				*(_SortDigits[key].Ptr++)= *srcPtr;
			}

			// arraDst has now values of arraySrc sorted for the current digit.
			// now, restart with next digit, so swap src / dst
			std::swap(arraySrc, arrayDst);
		}

		// return the array correctly sorted. because of last swap, this is arraySrc
		return arraySrc;
	}

};


} // NL3D


#endif // NL_RADIX_SORT_H

/* End of radix_sort.h */

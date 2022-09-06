// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef FIXED_SIZE_INT_VECTOR_H
#define FIXED_SIZE_INT_VECTOR_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"


/**
 * The low level data container used to store the data for a CDB instance.
 * This container is vector-like with the significant difference that the
 * size of the vector is assumed to be fixed.
 * A special custom allocator is used for managing data blocks allocated and freed here
 * NOTE: The custom allocator never returns memory to the system - it retains all RAM that it has ever allocated
 */
template <typename T>
class CFixedSizeIntVector
{
public:
	/// ctor
	CFixedSizeIntVector();

	/// dtor - calls release()
	~CFixedSizeIntVector();

	/// init
	/// This metod sets the size of the container, allocating memory either from the free data buffer pool or,
	/// if need be, from system memory
	/// Initialises all container contents to 0
	void init(uint32 size,const T& value);

	/// release
	/// free up the data buffer used by this container (return it to the free data buffer pool)
	void release();

	/// stl classic size() accessor
	uint size() const;

	/// classic operator[] - includes a bounds check even in release builds
	T& operator[](uint32 index);

	/// classic const operator[] - includes a bounds check even in release builds
	const T& operator[](uint32 index) const;

private:
	/// the size of the current data buffer
	uint32 _Size;

	/// the data buffer (a pointer to the data)
	typedef T* TDataBufferPtr;
	TDataBufferPtr _DataBuffer;

private:
	// a vector of data buffers (there is no size info here - they are assumed to be all of the same size)
	// this data type is used to contain the vector of free (reusable) buffers of a given size
	typedef std::vector<TDataBufferPtr> TDataBufferPtrVector;

	/// static method used to wrap a static map of size to TDataBufferPtrVector
	static TDataBufferPtrVector& getFreeDataBufferVector(uint32 size);

	/// static method used to wrap a static map of size to uint32 data buffer counter
	static uint32& getDataBufferCounter(uint32 size);
};

typedef CFixedSizeIntVector<sint64> CFixedSizeVectorSint64;
typedef CFixedSizeIntVector<uint64> CFixedSizeVectorUint64;
typedef CFixedSizeIntVector<sint32> CFixedSizeVectorSint32;
typedef CFixedSizeIntVector<uint32> CFixedSizeVectorUint32;
typedef CFixedSizeIntVector<sint16> CFixedSizeVectorSint16;
typedef CFixedSizeIntVector<uint16> CFixedSizeVectorUint16;

template <typename T>
CFixedSizeIntVector<T>::CFixedSizeIntVector(): _Size(0), _DataBuffer(NULL)
{
}

template <typename T>
CFixedSizeIntVector<T>::~CFixedSizeIntVector()
{
	// free up our data buffer
	release();
}

template <typename T>
void CFixedSizeIntVector<T>::init(uint32 size,const T& value)
{
	// make sure we are not already initialised
	nlassert(_Size==0);

	// setup the new size
	_Size=size;
	nlassert(_Size!=0);

	// if there's a spare data buffer of this size kicking about then use it
	TDataBufferPtrVector& theFreeDataBufferPtrVector= getFreeDataBufferVector(_Size);
	if (!theFreeDataBufferPtrVector.empty())
	{
		// there are spare data buffers of this size so pop the next one off the back of the buffer vector
		_DataBuffer= theFreeDataBufferPtrVector.back();
		theFreeDataBufferPtrVector.pop_back();
	}
	else
	{
		// no spare buffer of this size was available so allocate a new one
		uint32& theDataBufferCounter= getDataBufferCounter(_Size);
		++theDataBufferCounter;
		nldebug("CFixedSizeDataVector_resize allocating a new buffer of size %u x %u (%u buffers in all)",_Size,sizeof(T),theDataBufferCounter);
		_DataBuffer= new T[_Size];
		nlassert(_DataBuffer!=NULL);
	}

	// clear out the data in the buffer
	for (uint32 i=_Size;i--;)
	{
		_DataBuffer[i]= value;
	}
}

template <typename T>
void CFixedSizeIntVector<T>::release()
{
	// if we have a data buffer then add it to the relevant free data buffer vector in the free data buffer pool
	if (_DataBuffer!=NULL)
	{
		getFreeDataBufferVector(_Size).push_back(_DataBuffer);
	}
	_DataBuffer= NULL;
	_Size= 0;
}

template <typename T>
T& CFixedSizeIntVector<T>::operator[](uint32 index)
{
	// test for array overflows
	if (index>=_Size) nlerror("Array overflow in Client Property Database");
	return _DataBuffer[index];
}

template <typename T>
const T& CFixedSizeIntVector<T>::operator[](uint32 index) const
{
	// delegate to the non-const operator[]
	return const_cast<CFixedSizeIntVector*>(this)->operator[](index);
}

template <typename T>
uint CFixedSizeIntVector<T>::size() const
{
	return _Size;
}

template <typename T>
std::vector<T*>& CFixedSizeIntVector<T>::getFreeDataBufferVector(uint32 size)
{
	// a static map of buffer size to vector of free buffers of this size
	typedef std::map<uint32,TDataBufferPtrVector> TFreeDataBufferVectors;
	static TFreeDataBufferVectors freeDataBufferVectors;

	return freeDataBufferVectors[size];
}

template <typename T>
uint32& CFixedSizeIntVector<T>::getDataBufferCounter(uint32 size)
{
	// a static map of buffer size to counter of buffers of this size
	typedef std::map<uint32,uint32> TDataBufferCounters;
	static TDataBufferCounters dataBufferCounters;

	return dataBufferCounters[size];
}

#endif // FIXED_SIZE_INT_VECTOR_H

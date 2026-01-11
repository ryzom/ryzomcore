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

#include "nel/misc/bit_set.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace	NLMISC
{

// must be defined elsewhere
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif



// ***************************************************************************
CBitSet::CBitSet()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	NumBits= 0;
	MaskLast= 0;
}
CBitSet::CBitSet(uint numBits)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	NumBits= 0;
	MaskLast= 0;
	resize(numBits);
}
CBitSet::CBitSet(const CBitSet &bs)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
	NumBits= bs.NumBits;
	MaskLast= bs.MaskLast;
	Array= bs.Array;
}
CBitSet::~CBitSet()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/
}
CBitSet	&CBitSet::operator=(const CBitSet &bs)
{
	NumBits= bs.NumBits;
	MaskLast= bs.MaskLast;
	Array= bs.Array;

	return *this;
}


// ***************************************************************************
void	CBitSet::clear()
{
	Array.clear();
	NumBits= 0;
	MaskLast=0;
}
void	CBitSet::resize(uint numBits)
{
	if(numBits==0)
		clear();

	NumBits= numBits;
	Array.resize( (NumBits+NL_BITLEN-1) / NL_BITLEN );
	uint	nLastBits= NumBits & (NL_BITLEN-1) ;
	// Generate the mask for the last word.
	if(nLastBits==0)
		MaskLast= ~((uint)0);
	else
		MaskLast= (1<< nLastBits) -1;

	// reset to 0.
	clearAll();
}
void	CBitSet::resizeNoReset(uint numBits, bool value)
{
	if(numBits==0)
		clear();

	uint oldNum=NumBits;
	NumBits= numBits;
	Array.resize( (NumBits+NL_BITLEN-1) / NL_BITLEN );
	uint	nLastBits= NumBits & (NL_BITLEN-1) ;
	// Generate the mask for the last word.
	if(nLastBits==0)
		MaskLast= ~((uint)0);
	else
		MaskLast= (1<< nLastBits) -1;

	// Set new bit to value
	for (uint i=oldNum; i<(uint)NumBits; i++)
		set(i, value);
}
void	CBitSet::setAll()
{
	const vector<uint32>::size_type s = Array.size();
	fill_n(Array.begin(), s, ~((uint)0));

	if (s)
		Array[s-1]&= MaskLast;
}
void	CBitSet::clearAll()
{
	fill_n(Array.begin(), Array.size(), 0);
}


// ***************************************************************************
CBitSet	CBitSet::operator~() const
{
	CBitSet	ret;

	ret= *this;
	ret.flip();
	return ret;
}
CBitSet	CBitSet::operator&(const CBitSet &bs) const
{
	CBitSet	ret;

	ret= *this;
	ret&=bs;
	return ret;
}
CBitSet	CBitSet::operator|(const CBitSet &bs) const
{
	CBitSet	ret;

	ret= *this;
	ret|=bs;
	return ret;
}
CBitSet	CBitSet::operator^(const CBitSet &bs) const
{
	CBitSet	ret;

	ret= *this;
	ret^=bs;
	return ret;
}


// ***************************************************************************
void	CBitSet::flip()
{
	if(NumBits==0)
		return;

	for(sint i=0;i<(sint)Array.size();i++)
		Array[i]= ~Array[i];

	Array[Array.size()-1]&= MaskLast;
}
CBitSet	&CBitSet::operator&=(const CBitSet &bs)
{
	if(NumBits==0)
		return *this;

	vector<uint32>::size_type minSize = min(Array.size(), bs.Array.size());
	vector<uint32>::size_type i;
	for(i=0;i<minSize;i++)
		Array[i]= Array[i] & bs.Array[i];
	for(i=minSize;i<Array.size();i++)
		Array[i]=0;

	Array[Array.size()-1]&= MaskLast;

	return *this;
}
CBitSet	&CBitSet::operator|=(const CBitSet &bs)
{
	if(NumBits==0)
		return *this;

	vector<uint32>::size_type minSize = min(Array.size(), bs.Array.size());
	vector<uint32>::size_type i;
	for(i=0;i<minSize;i++)
		Array[i]= Array[i] | bs.Array[i];
	// Do nothing for bits word from minSize to Array.size().

	Array[Array.size()-1]&= MaskLast;

	return *this;
}
CBitSet	&CBitSet::operator^=(const CBitSet &bs)
{
	if(NumBits==0)
		return *this;

	vector<uint32>::size_type minSize= min(Array.size(), bs.Array.size());
	vector<uint32>::size_type i;
	for(i=0;i<minSize;i++)
		Array[i]= Array[i] ^ bs.Array[i];
	// Do nothing for bits word from minSize to Array.size().

	Array[Array.size()-1]&= MaskLast;

	return *this;
}


// ***************************************************************************
bool	CBitSet::operator==(const CBitSet &bs) const
{
	if(NumBits!=bs.NumBits)
		return false;

	for(sint i=0;i<(sint)Array.size();i++)
	{
		if(Array[i]!=bs.Array[i])
			return false;
	}
	return true;
}
bool	CBitSet::operator!=(const CBitSet &bs) const
{
	return (!operator==(bs));
}
bool	CBitSet::compareRestrict(const CBitSet &bs) const
{
	sint	n=min(NumBits, bs.NumBits);
	if(n==0) return true;

	sint	nA= (n+NL_BITLEN-1) / NL_BITLEN;
	uint	mask;

	uint	nLastBits= n & (NL_BITLEN-1) ;
	// Generate the mask for the last common word.
	if(nLastBits==0)
		mask= ~((uint)0);
	else
		mask= (1<< nLastBits) -1;


	for(sint i=0;i<nA-1;i++)
	{
		if(Array[i]!=bs.Array[i])
			return false;
	}
	if( (Array[nA-1]&mask) != (bs.Array[nA-1]&mask) )
		return false;


	return true;
}
bool	CBitSet::allSet()
{
	if(NumBits==0)
		return false;
	for(sint i=0;i<(sint)Array.size()-1;i++)
	{
		if( Array[i]!= (~((uint)0)) )
			return false;
	}
	if( Array[Array.size()-1]!= MaskLast )
		return false;
	return true;
}
bool	CBitSet::allCleared()
{
	if(NumBits==0)
		return false;
	for(sint i=0;i<(sint)Array.size();i++)
	{
		if( Array[i]!= 0 )
			return false;
	}
	return true;
}



void	CBitSet::serial(IStream &f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	(void)f.serialVersion(0);
	uint32	sz=0;
	vector<uint32>	array32;

	// Must support any size of uint.
	if(f.isReading())
	{
		f.serial(sz);
		resize(sz);

		f.serialCont(array32);
		for(sint i=0;i<(sint)sz;i++)
		{
			uint32	a=array32[i/32];
			a&= 1<<(i&31);
			set(i, a!=0);
		}
	}
	else
	{
		sz= size();
		f.serial(sz);

		array32.resize(sz/32);
		fill_n(array32.begin(), array32.size(), 0);
		for(sint i=0;i<(sint)sz;i++)
		{
			if(get(i))
				array32[i/32]|= 1<<(i&31);
		}
		f.serialCont(array32);
	}
}


/*
 * Return a string representing the bitfield with 1 and 0 (from left to right)
 */
std::string	CBitSet::toString() const
{
	string s;
	for ( sint i=0; i!=(sint)size(); ++i )
	{
		s += (get(i) ? '1' : '0');
	}
	return s;
}


}


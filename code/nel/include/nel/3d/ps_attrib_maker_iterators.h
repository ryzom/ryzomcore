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

#ifndef NL_PS_ATTRIB_MAKER_ITERATORS_H
#define NL_PS_ATTRIB_MAKER_ITERATORS_H

#include "nel/3d/ps_attrib.h"
#include "nel/3d/ps_iterator.h"



namespace NL3D
{

	/** We define a set of iterator object that can advance with a fixed point step in the source container
	  * We have 2 version for each iterator : iterators that advance with a step of 1, and iterators that advance
	  * with a fixed point (16 : 16).
	  */

	template <typename TBaseIter>
	struct CPSBaseIterator
	{
		TBaseIter 	Iter;
		/// TPSAttribFloat::const_iterator Iter;
		CPSBaseIterator() {}
		CPSBaseIterator(TBaseIter it) : Iter(it) {}
		GET_INLINE float get() const { return Iter.get(); }
		void  advance() { Iter.advance(); }
	};

	/** This special iterator on a vector attributes enables to convert the speed to its norm
	 *  It is for private use only, and it has not all the functionnalities of an iterator.
	 *  The src datas can't be modified as we return the norm, and not a reference on the value
	 */
	template <class TBaseIter>
	struct CVectNormIterator : CPSBaseIterator<TBaseIter>
	{
		GET_INLINE float get() const { return this->Iter.get().norm(); }
		CVectNormIterator(const TBaseIter &it) : CPSBaseIterator<TBaseIter>(it) {}
	};

	/** This special iterator return random values every time it is read
	 *  It is for private use only, and it has not all the functionnalities of an iterator.
	 */

	struct CRandomIterator
	{
		GET_INLINE float get() const { return float(rand() * (1 / double(RAND_MAX))); } // this may be optimized with a table...
		void  advance() {}
		void  advance(uint /* quantity */) {}
	};

	/// this iterator just return the same value
	struct CDecalIterator
	{
		float Value;
		GET_INLINE float get() const { return Value; }
		void  advance() {}
		void  advance(uint /* quantity */) {}
	};

	/// iterator that use dist to compute the value
	template <class TBaseIter>
	struct CDistIterator : CPSBaseIterator<TBaseIter>
	{
		NLMISC::CVector V;
		float Offset;
		CDistIterator(const TBaseIter &it) : CPSBaseIterator<TBaseIter>(it) {}
	};


	/// this iterator perform a dot prod with a vector, add an offset and take the fabs of the result
	template <class TBaseIter>
	struct CFDot3AddIterator : CDistIterator<TBaseIter>
	{
		GET_INLINE
		float get() const
		{
			const float r = fabsf(this->Iter.get() * this->V + this->Offset);
			return r > MaxInputValue ? MaxInputValue : r;
		}
		CFDot3AddIterator(const TBaseIter &it) : CDistIterator<TBaseIter>(it) {}
	};

	/// this iterator perform a dot prod with a vector, add an offset and take the square of the result
	template <class TBaseIter>
	struct CFSquareDot3AddIterator : CDistIterator<TBaseIter>
	{
		float get() const
		{
			float r = this->Iter.get() * this->V + this->Offset;
			r *= r;
			return r > MaxInputValue ? MaxInputValue : r;
		}
		CFSquareDot3AddIterator(const TBaseIter &it) : CDistIterator<TBaseIter>(it) {}
	};

	/// this iterator perform a dot prod with a vector, add an offset. If it is negatif it return MaxInputValue, and take the abs of the result
	template <class TBaseIter>
	struct CFClampDot3AddIterator : CDistIterator<TBaseIter>
	{
		GET_INLINE
		float get() const
		{
			const float r = this->Iter.get() * this->V + this->Offset;
			if (r < 0.f) return MaxInputValue;
			return r > MaxInputValue ? MaxInputValue : r;
		}
		CFClampDot3AddIterator(const TBaseIter &it) : CDistIterator<TBaseIter>(it) {}
	};


	/// this iterator perform a dot prod with a vector, add an offset. If it is negatif it return MaxInputValue, and take the square of the result
	template <class TBaseIter>
	struct CFClampSquareDot3AddIterator : CDistIterator<TBaseIter>
	{
		GET_INLINE
		float get() const
		{
			float r = this->Iter.get() * this->V + this->Offset;
			if (r < 0) return MaxInputValue;
			r *= r;
			return r > MaxInputValue ? MaxInputValue : r;
		}
		CFClampSquareDot3AddIterator(const TBaseIter &it) : CDistIterator<TBaseIter>(it) {}
	};


} /// NL3D


#endif

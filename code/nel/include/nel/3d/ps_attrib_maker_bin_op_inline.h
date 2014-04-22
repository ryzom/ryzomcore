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


#ifndef NL_PS_ATTRIB_MAKER_BIN_OP_H
	#error Do not include this file! include ps_attrib_maker_bin_op_inline instead!
#endif

#ifndef NL_PS_ATTRIB_MAKER_BIN_OP_INLINE_H
#define NL_PS_ATTRIB_MAKER_BIN_OP_INLINE_H



namespace NL3D {


/** Some template functions and some specialization for binary operations
  * We don't override the usual operators, because we may want behaviour such as saturation, and this may be misleading
  * with usual operators
  */

template <class T>
inline T PSBinOpModulate(T arg1, T arg2) { return arg1 * arg2; }
template <class T>
inline T PSBinOpAdd(T arg1, T arg2) { return arg1 + arg2; }
template <class T>
inline T PSBinOpSubtract(T arg1, T arg2) { return arg1 - arg2; }

template <>
inline CPlaneBasis PSBinOpModulate(CPlaneBasis p1, CPlaneBasis p2)
{
	// we compute p1 * p2
	NLMISC::CVector z = p1.X ^ p1.Y;
	CPlaneBasis r;
	r.X.x = p2.X.x * p1.X.x + p2.X.y * p1.Y.x + p2.X.z * z.x;
	r.X.y = p2.X.x * p1.X.y + p2.X.y * p1.Y.y + p2.X.z * z.y;
	r.X.z = p2.X.x * p1.X.z + p2.X.y * p1.Y.z + p2.X.z * z.z;

	r.Y.x = p2.Y.x * p1.X.x + p2.Y.y * p1.Y.x + p2.Y.z * z.x;
	r.Y.y = p2.Y.x * p1.X.y + p2.Y.y * p1.Y.y + p2.Y.z * z.y;
	r.Y.z = p2.Y.x * p1.X.z + p2.Y.y * p1.Y.z + p2.Y.z * z.z;

	return r;

}
template <>
inline CPlaneBasis PSBinOpAdd(CPlaneBasis /* p1 */, CPlaneBasis /* p2 */)
{
	nlassert(0); // not allowed for now
	return CPlaneBasis(NLMISC::CVector::Null);
}
template <>
inline CPlaneBasis PSBinOpSubtract(CPlaneBasis /* p1 */, CPlaneBasis /* p2 */)
{
	nlassert(0); // not allowed for now
	return CPlaneBasis(NLMISC::CVector::Null);
}


template <>
inline uint32 PSBinOpSubtract(uint32 lhs, uint32 rhs)
{
	return rhs > lhs ? 0 : lhs - rhs; // avoid overflow
}


template <>
inline NLMISC::CRGBA PSBinOpModulate(NLMISC::CRGBA t1, NLMISC::CRGBA t2)
{
	NLMISC::CRGBA result;
	result.modulateFromColor(t1, t2);
	return result;
}
template <>
inline NLMISC::CRGBA PSBinOpAdd(NLMISC::CRGBA t1, NLMISC::CRGBA t2)
{
	NLMISC::CRGBA r;
	uint S = t1.R + t2.R; if (S > 255) S = 255; r.R = (uint8) S;
	S = t1.G + t2.G; if (S > 255) S = 255; r.G = (uint8) S;
	S = t1.B + t2.B; if (S > 255) S = 255; r.B = (uint8) S;
	return r;
}
template <>
inline NLMISC::CRGBA PSBinOpSubtract(NLMISC::CRGBA t1, NLMISC::CRGBA t2)
{
	NLMISC::CRGBA r;
	sint S = t1.R - t2.R; if (S < 0) S = 0; r.R = (uint8) S;
	S = t1.G - t2.G; if (S < 0) S = 0; r.G = (uint8) S;
	S = t1.B - t2.B; if (S < 0) S = 0; r.B = (uint8) S;
	return r;
}


/////////////////////////////////////////////////////////////////////////////
// CPSAttribMakerBinOp specializations to return the correct min/max value //
/////////////////////////////////////////////////////////////////////////////
// *************************************************************************************************************
/** template specialization implementations
  * They're useful to get the correct min / max values depending on the type
  */


// ***********************************************************************
template <>
inline uint32 CPSAttribMakerBinOp<uint32>::getMinValue(void) const
{
	nlassert(_Arg[0] && _Arg[1]);
	switch(_Op)
	{
		case 	CPSBinOp::selectArg1: return _Arg[0]->getMinValue();
		case	CPSBinOp::selectArg2: return _Arg[1]->getMinValue();
		case	CPSBinOp::modulate:   return _Arg[0]->getMinValue() * _Arg[1]->getMinValue();
		case	CPSBinOp::add:		  return _Arg[0]->getMinValue() + _Arg[1]->getMinValue();
		case	CPSBinOp::subtract:
		{
			uint32 lhs = _Arg[0]->getMinValue();
			uint32 rhs = _Arg[1]->getMaxValue();
			return rhs > lhs ? 0 : lhs - rhs;
		}
		break;
		default:
			nlassert(0);
		break;
	};
	return 0;
}

// ***********************************************************************
template <>
inline uint32 CPSAttribMakerBinOp<uint32>::getMaxValue(void) const
{
	nlassert(_Arg[0] && _Arg[1]);
	switch(_Op)
	{
		case 	CPSBinOp::selectArg1: return _Arg[0]->getMaxValue();
		case	CPSBinOp::selectArg2: return _Arg[1]->getMaxValue();
		case	CPSBinOp::modulate:   return _Arg[0]->getMaxValue() * _Arg[1]->getMaxValue();
		case	CPSBinOp::add:		  return _Arg[0]->getMaxValue() + _Arg[1]->getMaxValue();
		case	CPSBinOp::subtract:
		{
			uint32 lhs = _Arg[0]->getMaxValue();
			uint32 rhs = _Arg[1]->getMinValue();
			return rhs > lhs ? 0 : lhs - rhs;
		}
		break;
		default:
			nlassert(0);
		break;
	};
	return 0;
}

// ***********************************************************************
template <>
inline sint32 CPSAttribMakerBinOp<sint32>::getMinValue(void) const
{
	nlassert(_Arg[0] && _Arg[1]);
	switch(_Op)
	{
		case 	CPSBinOp::selectArg1: return _Arg[0]->getMinValue();
		case	CPSBinOp::selectArg2: return _Arg[1]->getMinValue();
		case	CPSBinOp::modulate:
		{
			// we're dealing with signed values
			sint32 min0 = _Arg[0]->getMinValue();
			sint32 min1 = _Arg[1]->getMinValue();
			sint32 max0 = _Arg[0]->getMaxValue();
			sint32 max1 = _Arg[1]->getMaxValue();
			return NLMISC::minof(min0 * min1, min0 * max1, max0 * min1, max0 * max1);
		}
		case	CPSBinOp::add:		  return _Arg[0]->getMinValue() + _Arg[1]->getMinValue();
		case	CPSBinOp::subtract:   return _Arg[0]->getMinValue() - _Arg[1]->getMaxValue();
		default:
			nlassert(0);
		break;
	};
	return 0;
}

// ***********************************************************************
template <>
inline sint32 CPSAttribMakerBinOp<sint32>::getMaxValue(void) const
{
	nlassert(_Arg[0] && _Arg[1]);
	switch(_Op)
	{
		case 	CPSBinOp::selectArg1: return _Arg[0]->getMaxValue();
		case	CPSBinOp::selectArg2: return _Arg[1]->getMaxValue();
		case	CPSBinOp::modulate:
		{
			// we're dealing with signed values
			sint32 min0 = _Arg[0]->getMinValue();
			sint32 min1 = _Arg[1]->getMinValue();
			sint32 max0 = _Arg[0]->getMaxValue();
			sint32 max1 = _Arg[1]->getMaxValue();
			return NLMISC::maxof(min0 * min1, min0 * max1, max0 * min1, max0 * max1);
		}
		case	CPSBinOp::add:		  return _Arg[0]->getMaxValue() + _Arg[1]->getMaxValue();
		case	CPSBinOp::subtract:   return _Arg[0]->getMaxValue() - _Arg[1]->getMinValue();
		default:
			nlassert(0);
		break;
	};
	return 0;
}

// ***********************************************************************
template <>
inline float CPSAttribMakerBinOp<float>::getMinValue(void) const
{
	nlassert(_Arg[0] && _Arg[1]);
	switch(_Op)
	{
		case 	CPSBinOp::selectArg1: return _Arg[0]->getMinValue();
		case	CPSBinOp::selectArg2: return _Arg[1]->getMinValue();
		case	CPSBinOp::modulate:
		{
			// we're dealing with signed values
			float min0 = _Arg[0]->getMinValue();
			float min1 = _Arg[1]->getMinValue();
			float max0 = _Arg[0]->getMaxValue();
			float max1 = _Arg[1]->getMaxValue();
			return NLMISC::minof(min0 * min1, min0 * max1, max0 * min1, max0 * max1);
		}
		case	CPSBinOp::add:		  return _Arg[0]->getMinValue() + _Arg[1]->getMinValue();
		case	CPSBinOp::subtract:   return _Arg[0]->getMinValue() - _Arg[1]->getMaxValue();
		default:
			nlassert(0);
		break;
	};
	return 0;
}

// ***********************************************************************
template <>
inline float CPSAttribMakerBinOp<float>::getMaxValue(void) const
{
	nlassert(_Arg[0] && _Arg[1]);
	switch(_Op)
	{
		case 	CPSBinOp::selectArg1: return _Arg[0]->getMaxValue();
		case	CPSBinOp::selectArg2: return _Arg[1]->getMaxValue();
		case	CPSBinOp::modulate:
		{
			// we're dealing with signed values
			float min0 = _Arg[0]->getMinValue();
			float min1 = _Arg[1]->getMinValue();
			float max0 = _Arg[0]->getMaxValue();
			float max1 = _Arg[1]->getMaxValue();
			return NLMISC::maxof(min0 * min1, min0 * max1, max0 * min1, max0 * max1);
		}
		case	CPSBinOp::add:		  return _Arg[0]->getMaxValue() + _Arg[1]->getMaxValue();
		case	CPSBinOp::subtract:   return _Arg[0]->getMaxValue() - _Arg[1]->getMinValue();
		default:
			nlassert(0);
		break;
	};
	return 0;
}

////////////////////////////////////////
// CPSAttribMakerBinOp implementation //
////////////////////////////////////////



//=================================================================================================================
/// copy ctor
template <class T>
inline CPSAttribMakerBinOp<T>::CPSAttribMakerBinOp(const CPSAttribMakerBinOp &other) : CPSAttribMaker<T>(other) // parent copy ctor
{
	std::auto_ptr<CPSAttribMaker<T> > a0(NLMISC::safe_cast<CPSAttribMaker<T> *>(other._Arg[0]->clone()))
									, a1(NLMISC::safe_cast<CPSAttribMaker<T> *>(other._Arg[1]->clone()));
	this->_Op =		other._Op;
	this->_Size =   other._Size;
	this->_MaxSize =  other._MaxSize;
	this->_Arg[0] = a0.release();
	this->_Arg[1] = a1.release();
}

//=================================================================================================================
template <class T>
inline CPSAttribMakerBinOp<T>::CPSAttribMakerBinOp() : _Op(CPSBinOp::selectArg1), _Size(0), _MaxSize(0)
{
	_Arg[0] = _Arg[1] = NULL;
	this->_HasMemory  = true;
}

//=================================================================================================================
template <class T>
inline void CPSAttribMakerBinOp<T>::clean(void)
{
	delete _Arg[0];
	delete _Arg[1];
}

//=================================================================================================================
template <class T>
inline CPSAttribMakerBinOp<T>::~CPSAttribMakerBinOp()
{
	clean();
}

//=================================================================================================================
/// cplane basis template specialization for supportOp
template <>
inline bool CPSAttribMakerBinOp<CPlaneBasis>::supportOp(CPSBinOp::BinOp op)
{
	return  (op == CPSBinOp::selectArg1 || op == CPSBinOp::selectArg2 || op == CPSBinOp::modulate);
}


//=================================================================================================================
template <class T>
inline T CPSAttribMakerBinOp<T>::get (CPSLocated *loc, uint32 index)
{
	switch (_Op)
	{
		case CPSBinOp::selectArg1:
			return _Arg[0]->get(loc, index);
		break;
		case CPSBinOp::selectArg2:
			return _Arg[1]->get(loc, index);
		break;
		case CPSBinOp::modulate:
			return PSBinOpModulate(_Arg[0]->get(loc, index), _Arg[1]->get(loc, index));
		break;
		case CPSBinOp::add:
			return PSBinOpAdd(_Arg[0]->get(loc, index), _Arg[1]->get(loc, index));
		break;
		case CPSBinOp::subtract:
			return PSBinOpSubtract(_Arg[0]->get(loc, index), _Arg[1]->get(loc, index));
		break;
		default: break;
	}

	nlstop;
	return T();
}

//=================================================================================================================
template <class T>
inline T CPSAttribMakerBinOp<T>::get(const CPSEmitterInfo &info)
{
	switch (_Op)
	{
	case CPSBinOp::selectArg1:
		return _Arg[0]->get(info);
		break;
	case CPSBinOp::selectArg2:
		return _Arg[1]->get(info);
		break;
	case CPSBinOp::modulate:
		return PSBinOpModulate(_Arg[0]->get(info), _Arg[1]->get(info));
		break;
	case CPSBinOp::add:
		return PSBinOpAdd(_Arg[0]->get(info), _Arg[1]->get(info));
		break;
	case CPSBinOp::subtract:
		return PSBinOpSubtract(_Arg[0]->get(info), _Arg[1]->get(info));
		break;
	default: break;
	}

	nlstop;
	return T();
}


// for private use
void MakePrivate(uint8 * dest, const NLMISC::CRGBA *src1, const NLMISC::CRGBA *src2, uint32 stride, uint32 numAttrib, CPSBinOp::BinOp op);
// for private use
template <class T>
inline void MakePrivate(uint8 * dest, const T *src1, const T *src2, uint32 stride, uint32 numAttrib, CPSBinOp::BinOp op)
{
	uint8 *destEnd = dest + (stride * numAttrib);

	switch (op)
	{
		case CPSBinOp::modulate:
		{
			while (dest != destEnd)
			{
				* (T *) dest = PSBinOpModulate(*src1 ++, *src2 ++);
				dest += stride;
			}
		}
		break;
		case CPSBinOp::add:
		{
			while (dest != destEnd)
			{
				* (T *) dest = PSBinOpAdd(*src1 ++, *src2 ++);
				dest += stride;
			}
		}
		break;
		case CPSBinOp::subtract:
		while (dest != destEnd)
		{
			* (T *) dest = PSBinOpSubtract(*src1 ++, *src2 ++);
			dest += stride;
		}
		break;
		default: break;
	}
}


//=================================================================================================================
template <class T>
inline void   *CPSAttribMakerBinOp<T>::makePrivate(T *buf1,
												   T *buf2,
												   CPSLocated *loc,
												   uint32 startIndex,
												   void *tab,
												   uint32 stride,
												   uint32 numAttrib,
												   bool allowNoCopy /* = false */,
												   uint32 srcStep /* = (1 << 16)*/,
												   bool	forceClampEntry /*= false*/
												  ) const
{
	uint8 *dest = (uint8 *) tab;
	uint leftToDo = numAttrib, toProcess;
	nlassert(_Arg[0] && _Arg[1]);
	switch (_Op)
	{
		case CPSBinOp::selectArg1:
			return _Arg[0]->make(loc, startIndex, tab, stride, numAttrib, allowNoCopy, srcStep, forceClampEntry);
			break;
		case CPSBinOp::selectArg2:
			return _Arg[1]->make(loc, startIndex, tab, stride, numAttrib, allowNoCopy, srcStep, forceClampEntry);
			break;
		default: break;
	}



	while (leftToDo)
	{
		toProcess = leftToDo > PSBinOpBufSize ? PSBinOpBufSize : leftToDo;
		T *src1 = (T *) _Arg[0]->make(loc, startIndex + (numAttrib - leftToDo), &buf1[0], sizeof(T), toProcess, true, srcStep, forceClampEntry);
		T *src2 = (T *) _Arg[1]->make(loc, startIndex + (numAttrib - leftToDo), &buf2[0], sizeof(T), toProcess, true, srcStep, forceClampEntry);
		MakePrivate(dest, src1, src2, stride, toProcess, _Op);
		leftToDo -= toProcess;
	}

	return tab;
}

//=================================================================================================================
template <class T>
inline void   *CPSAttribMakerBinOp<T>::make	  (CPSLocated *loc,
										   uint32 startIndex,
										   void *tab,
										   uint32 stride,
										   uint32 numAttrib,
										   bool allowNoCopy /* = false */,
										   uint32 srcStep /* = (1 << 16)*/,
										   bool forceClampEntry /* = false */
										  ) const
{
	/** init the tab used for computations. we use a trick to avoid ctor calls,
	  * but they may be used for some types in the future , so a specilization
	  * of this method could be added in these case.
	  */
	uint8 tab1[PSBinOpBufSize * sizeof(T)];
	uint8 tab2[PSBinOpBufSize * sizeof(T)];
	return makePrivate((T *) &tab1[0], (T *) &tab2[0], loc, startIndex, tab, stride, numAttrib, allowNoCopy, srcStep, forceClampEntry);
}


// for private use
void Make4Private(uint8 * dest, const NLMISC::CRGBA *src1, const NLMISC::CRGBA *src2, uint32 stride, uint32 numAttrib, CPSBinOp::BinOp op);

// for private use
template <class T>
inline void Make4Private(uint8 * dest, const T *src1, const T *src2, uint32 stride, uint32 numAttrib, CPSBinOp::BinOp op)
{
	const uint stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride2 << 1;
	uint8 *destEnd = dest + ((stride<<2) * numAttrib);
	switch (op)
	{
		case CPSBinOp::modulate:
		{
			while (dest != destEnd)
			{
				// compute one value, and duplicate if 4 times
				* (T *) dest = PSBinOpModulate(*src1 ++, *src2 ++);
				* (T *) (dest + stride) = * (T *) dest;
				* (T *) (dest + stride2) = * (T *) dest;
				* (T *) (dest + stride3) = * (T *) dest;
				dest += stride4;
			}
		}
		break;
		case CPSBinOp::add:
		{
			while (dest != destEnd)
			{
				// compute one value, and duplicate if 4 times
				* (T *) dest = PSBinOpAdd(*src1 ++, *src2 ++);
				* (T *) (dest + stride) = * (T *) dest;
				* (T *) (dest + stride2) = * (T *) dest;
				* (T *) (dest + stride3) = * (T *) dest;
				dest += stride4;
			}
		}
		break;
		case CPSBinOp::subtract:
			while (dest != destEnd)
			{
				// compute one value, and duplicate if 4 times
				* (T *) dest = PSBinOpSubtract(*src1 ++, *src2 ++);
				* (T *) (dest + stride) = * (T *) dest;
				* (T *) (dest + stride2) = * (T *) dest;
				* (T *) (dest + stride3) = * (T *) dest;
				dest += stride4;
			}
		break;
		default: break;
	}
}

//=================================================================================================================
template <class T>
inline void    CPSAttribMakerBinOp<T>::make4Private(T *buf1,
												    T *buf2,
													CPSLocated *loc,
													uint32 startIndex,
													void *tab,
													uint32 stride,
													uint32 numAttrib,
													uint32 srcStep /* = (1 << 16)*/
												   ) const
{
	uint8 *dest = (uint8 *) tab;
	uint leftToDo = numAttrib, toProcess;
	nlassert(_Arg[0] && _Arg[1]);
	switch (_Op)
	{
		case CPSBinOp::selectArg1:
			_Arg[0]->make4(loc, startIndex, tab, stride, numAttrib, srcStep);
			return;
			break;
		case CPSBinOp::selectArg2:
			_Arg[1]->make4(loc, startIndex, tab, stride, numAttrib, srcStep);
			return;
			break;
		default: break;
	}

	while (leftToDo)
	{
		toProcess = leftToDo > PSBinOpBufSize ? PSBinOpBufSize : leftToDo;
		T *src1 = (T *) _Arg[0]->make(loc, startIndex + (numAttrib - leftToDo), &buf1[0], sizeof(T), toProcess, true, srcStep);
		T *src2 = (T *) _Arg[1]->make(loc, startIndex + (numAttrib - leftToDo), &buf2[0], sizeof(T), toProcess, true, srcStep);

		Make4Private(dest, src1, src2, stride, toProcess, _Op);
		leftToDo -= toProcess;
	}
}


//=================================================================================================================
template <class T>
inline void    CPSAttribMakerBinOp<T>::make4	  (CPSLocated *loc,
										   uint32 startIndex,
										   void *tab,
										   uint32 stride,
										   uint32 numAttrib,
										   uint32  srcStep /*= (1 << 16) */
										  ) const

{
	/** init the tab used for computations. we use a trick to avoid ctor calls,
	  * but they may be used for some types in the future , so a specilization
	  * of this method could be added in these case.
	  */
	uint8 tab1[PSBinOpBufSize * sizeof(T)];
	uint8 tab2[PSBinOpBufSize * sizeof(T)];
	make4Private((T *) &tab1[0], (T *) &tab2[0], loc, startIndex, tab, stride, numAttrib, srcStep);
}


// for private use
void MakeNPrivate(uint8 * dest, const NLMISC::CRGBA *src1, const NLMISC::CRGBA *src2, uint32 stride, uint32 numAttrib, CPSBinOp::BinOp op, uint nbReplicate);

// for private use
template <class T>
inline void MakeNPrivate(uint8 * dest,
				  const T *src1,
				  const T *src2,
				  uint32 stride,
				  uint32 numAttrib,
				  CPSBinOp::BinOp op,
				  uint nbReplicate,
				  uint32 /* srcStep */ = (1 << 16)
				 )
{
	uint k;
	uint8 *destEnd = dest + ((stride * nbReplicate) * numAttrib);
	switch (op)
	{
		case CPSBinOp::modulate:
		{
			while (dest != destEnd)
			{
				* (T *) dest = PSBinOpModulate(*src1 ++, *src2 ++);
				k = (nbReplicate - 1);
				do
				{
					* (T *) (dest + stride) = *(T *) dest;
					dest += stride;
				}
				while (--k);
				dest += stride;
			}
		}
		break;
		case CPSBinOp::add:
		{
			while (dest != destEnd)
			{
				* (T *) dest = PSBinOpAdd(*src1 ++, *src2 ++);
				k = (nbReplicate - 1);
				do
				{
					* (T *) (dest + stride) = *(T *) dest;
					dest += stride;
				}
				while (--k);
				dest += stride;
			}
		}
		break;
		case CPSBinOp::subtract:
			while (dest != destEnd)
			{
				* (T *) dest = PSBinOpSubtract(*src1 ++, *src2 ++);
				k = (nbReplicate - 1);
				do
				{
					* (T *) (dest + stride) = *(T *) dest;
					dest += stride;
				}
				while (--k);
				dest += stride;
			}
		break;
		default: break;
	}
}

//=================================================================================================================
template <class T>
inline void	CPSAttribMakerBinOp<T>::makeNPrivate(T *buf1,
												 T *buf2,
												 CPSLocated *loc,
												 uint32 startIndex,
												 void *tab,
												 uint32 stride,
												 uint32 numAttrib,
												 uint32 nbReplicate,
												 uint32 srcStep /*= (1 << 16)*/
												) const
{
	uint8 *dest = (uint8 *) tab;
	uint leftToDo = numAttrib, toProcess;
	nlassert(_Arg[0] && _Arg[1]);
	switch (_Op)
	{
		case CPSBinOp::selectArg1:
			_Arg[0]->makeN(loc, startIndex, tab, stride, numAttrib, nbReplicate, srcStep);
			return;
			break;
		case CPSBinOp::selectArg2:
			_Arg[1]->makeN(loc, startIndex, tab, stride, numAttrib, nbReplicate, srcStep);
			return;
			break;
		default: break;
	}

	while (leftToDo)
	{
		toProcess = leftToDo > PSBinOpBufSize ? PSBinOpBufSize : leftToDo;
		T *src1 = (T *) _Arg[0]->make(loc, startIndex + (numAttrib - leftToDo), &buf1[0], sizeof(T), toProcess, true, srcStep);
		T *src2 = (T *) _Arg[1]->make(loc, startIndex + (numAttrib - leftToDo), &buf2[0], sizeof(T), toProcess, true, srcStep);

		MakeNPrivate(dest, src1, src2, stride, toProcess, _Op, nbReplicate);
		leftToDo -= toProcess;
	}
}

//=================================================================================================================
template <class T>
inline void CPSAttribMakerBinOp<T>::makeN(CPSLocated *loc,
									  uint32 startIndex,
									  void *tab,
									  uint32 stride,
									  uint32 numAttrib,
									  uint32 nbReplicate,
									  uint32 /* srcStep = (1 << 16)*/
									 ) const

{
	/** init the tab used for computations. we use a trick to avoid ctor calls,
	  * but they may be used for some types in the future , so a specilization
	  * of this method could be added in these case.
	  */
	uint8 tab1[PSBinOpBufSize * sizeof(T)];
	uint8 tab2[PSBinOpBufSize * sizeof(T)];
	makeNPrivate((T *) &tab1[0], (T *) &tab2[0], loc, startIndex, tab, stride, numAttrib, nbReplicate);
}

//=================================================================================================================
template <class T>
inline void    CPSAttribMakerBinOp<T>::serial		  (NLMISC::IStream &f) throw(NLMISC::EStream)
{
	if (f.isReading())
	{
		clean();
	}
	f.serialVersion(1);
	f.serialEnum(_Op);
	f.serialPolyPtr(_Arg[0]);
	f.serialPolyPtr(_Arg[1]);
	f.serial(_Size, _MaxSize);

}

//=================================================================================================================
template <class T>
inline void    CPSAttribMakerBinOp<T>::deleteElement (uint32 index)
{
	if (_Arg[0]->hasMemory())	_Arg[0]->deleteElement(index);
	if (_Arg[1]->hasMemory())	_Arg[1]->deleteElement(index);
	nlassert(_Size != 0);
	--_Size;
}

//=================================================================================================================
template <class T>
inline void    CPSAttribMakerBinOp<T>::newElement	  (const CPSEmitterInfo &info)
{
	if (_Arg[0]->hasMemory())	_Arg[0]->newElement(info);
	if (_Arg[1]->hasMemory())	_Arg[1]->newElement(info);
	if (_Size != _MaxSize)
	{
		++_Size;
	}
}

//=================================================================================================================
template <class T>
inline void	CPSAttribMakerBinOp<T>::resize		  (uint32 capacity, uint32 nbPresentElements)
{
	nlassert(capacity < (1 << 16));
	_MaxSize = capacity;
	_Size = nbPresentElements;
	if (_Arg[0]->hasMemory())	_Arg[0]->resize(capacity, nbPresentElements);
	if (_Arg[1]->hasMemory())	_Arg[1]->resize(capacity, nbPresentElements);
}





} // NL3D


#endif // NL_PS_ATTRIB_MAKER_BIN_OP_INLINE_H

/* End of ps_attrib_maker_bin_op.h */

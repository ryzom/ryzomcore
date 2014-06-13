/*

Copyright (c) 2010-2014, Jan BOON
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef NLMISC_VECTORF_OPERATORS_H
#define NLMISC_VECTORF_OPERATORS_H
#include "types_nl.h"

// System includes
#ifdef NL_HAS_SSE2
#	include <xmmintrin.h>
#	include <emmintrin.h>
#endif

// STL includes

// NLMISC includes

// Project includes

namespace NLMISC {


NL_MATH_OPERATORS_USING_FUNCTIONS_ADD_SUB_MUL_DIV(CVector1F, CVector1F &, const CVector1F &, CVector1F &)
NL_MATH_OPERATORS_USING_FUNCTIONS_ADD_SUB(CVector2F, CVector2F &, const CVector2F &, CVector2F &)
NL_MATH_OPERATORS_USING_FUNCTIONS_ADD_SUB(CVector3F, CVector3F &, const CVector3F &, CVector3F &)
NL_MATH_OPERATORS_USING_FUNCTIONS_ADD_SUB(CVector4F, CVector4F &, const CVector4F &, CVector4F &)



NL_FORCE_INLINE bool operator==(const CVector3F &l, const CVector3F &r)
{
#ifdef NL_HAS_SSE2
	return (_mm_movemask_ps(_mm_cmpeq_ps(l.mm, r.mm)) & 0x07) == 0x07;
#else
	return l.x == r.x && l.y == r.y && l.z == r.z;
#endif
}

NL_FORCE_INLINE bool operator!=(const CVector3F &l, const CVector3F &r)
{
#ifdef NL_HAS_SSE2
	return (_mm_movemask_ps(_mm_cmpneq_ps(l.mm, r.mm)) & 0x07) != 0;
#else
	return l.x != r.x || l.y != r.y || l.z != r.z;
#endif
}



NL_FORCE_INLINE bool operator==(const CVector4F &l, const CVector4F &r)
{
#ifdef NL_HAS_SSE2
	return (_mm_movemask_ps(_mm_cmpeq_ps(l.mm, r.mm)) & 0x0F) == 0x0F;
#else
	return l.x == r.x && l.y == r.y && l.z == r.z && l.w == r.w;
#endif
}

NL_FORCE_INLINE bool operator!=(const CVector4F &l, const CVector4F &r)
{
#ifdef NL_HAS_SSE2
	return (_mm_movemask_ps(_mm_cmpneq_ps(l.mm, r.mm)) & 0x0F) != 0;
#else
	return l.x != r.x || l.y != r.y || l.z != r.z || l.w != r.w;
#endif
}



} /* namespace NLMISC */

#endif /* #ifndef NLMISC_VECTORF_OPERATORS_H */

/* end of file */

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

#ifndef NLMISC_MATRIXF_MATH_H
#define NLMISC_MATRIXF_MATH_H
#include "types_nl.h"

// System includes
#ifdef NL_HAS_SSE2
#	include <xmmintrin.h>
#	include <emmintrin.h>
#endif

// STL includes

// Project includes
#include "vectorf.h"
#ifndef NL_HAS_SSE2
#	include "matrix.h"
#endif

namespace NLMISC {


NL_FORCE_INLINE CMatrix44F identity44F()
{
#ifdef NL_HAS_SSE2
	CMatrix44F res;
	res.a = set4F(1.0f, 0.0f, 0.0f, 0.0f);
	res.b = set4F(0.0f, 1.0f, 0.0f, 0.0f);
	res.c = set4F(0.0f, 0.0f, 1.0f, 0.0f);
	res.d = set4F(0.0f, 0.0f, 0.0f, 1.0f);
	return res;
#else
	CMatrix44F res;
	res.m.identity();
	return res;
#endif
}


NL_FORCE_INLINE CMatrix44F mul(const CMatrix44F &left, const CMatrix44F &right)
{
#ifdef NL_HAS_SSE2
	// Hopefully the compiler inlines it similar to this
	
	// CMatrix44F right; // store
	CMatrix44F res; // store
	
	// CMatrix44F left; // register
	CVector4F rowright; // register
	// CVector4F multiplied; // register
	// CVector4F splatted; // register
	CVector4F resultrow; // register
	
	rowright = right.a;
	resultrow = mul(left.a, xxxx(rowright));
	resultrow = add(resultrow, mul(left.b, yyyy(rowright)));
	resultrow = add(resultrow, mul(left.c, zzzz(rowright)));
	resultrow = add(resultrow, mul(left.d, wwww(rowright)));
	res.a = resultrow;
	
	rowright = right.b;
	resultrow = mul(left.a, xxxx(rowright));
	resultrow = add(resultrow, mul(left.b, yyyy(rowright)));
	resultrow = add(resultrow, mul(left.c, zzzz(rowright)));
	resultrow = add(resultrow, mul(left.d, wwww(rowright)));
	res.b = resultrow;
	
	rowright = right.c;
	resultrow = mul(left.a, xxxx(rowright));
	resultrow = add(resultrow, mul(left.b, yyyy(rowright)));
	resultrow = add(resultrow, mul(left.c, zzzz(rowright)));
	resultrow = add(resultrow, mul(left.d, wwww(rowright)));
	res.c = resultrow;
	
	rowright = right.d;
	resultrow = mul(left.a, xxxx(rowright));
	resultrow = add(resultrow, mul(left.b, yyyy(rowright)));
	resultrow = add(resultrow, mul(left.c, zzzz(rowright)));
	resultrow = add(resultrow, mul(left.d, wwww(rowright)));
	res.d = resultrow;
	
	return res;
#else
	CMatrix44F res;
	res.m = left.m * right.m;
	return res;
#endif
}


} /* namespace NLMISC */

#endif /* #ifndef NLMISC_MATRIXF_MATH_H */

/* end of file */

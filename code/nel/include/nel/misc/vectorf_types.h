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

#ifndef NLMISC_VECTORF_TYPES_H
#define NLMISC_VECTORF_TYPES_H
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

#ifdef NL_HAS_SSE2
typedef NL_ALIGN_SSE2 struct { __m128 mm; } CVector1F; // 12 of 16 bytes wasted
#else
typedef struct { float x; } CVector1F;
#endif

#ifdef NL_HAS_SSE2
typedef NL_ALIGN_SSE2 struct { __m128 mm; } CVector2F; // 8 of 16 bytes wasted
#else
typedef struct { float x, y; } CVector2F;
#endif

#ifdef NL_HAS_SSE2
typedef NL_ALIGN_SSE2 struct { __m128 mm; } CVector3F; // 4 of 16 bytes wasted
#else
typedef struct { float x, y, z; } CVector3F;
#endif

#ifdef NL_HAS_SSE2
typedef NL_ALIGN_SSE2 struct { __m128 mm; } CVector4F;
#else
typedef struct { float x, y, z, w; } CVector4F;
#endif

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_VECTORF_TYPES_H */

/* end of file */

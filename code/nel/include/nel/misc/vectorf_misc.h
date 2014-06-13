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

#ifndef NLMISC_VECTORF_MISC_H
#define NLMISC_VECTORF_MISC_H
#include "types_nl.h"

// STL includes

// NLMISC includes

// Project includes



#define NL_ASSIMILATE_BI_FUNCTION(__return_type, __function_name, __parameter_type) \
NL_FORCE_INLINE __return_type __function_name(__parameter_type v0, __parameter_type v1, __parameter_type v2) \
{ \
	return __function_name(__function_name(v0, v1), v2); \
} \
 \
NL_FORCE_INLINE __return_type __function_name(__parameter_type v0, __parameter_type v1, __parameter_type v2, __parameter_type v3) \
{ \
	return __function_name(__function_name(__function_name(v0, v1), v2), v3); \
} \
 \
NL_FORCE_INLINE __return_type __function_name(__parameter_type v0, __parameter_type v1, __parameter_type v2, __parameter_type v3, __parameter_type v4) \
{ \
	return __function_name(__function_name(__function_name(__function_name(v0, v1), v2), v3), v4); \
} \
 \
NL_FORCE_INLINE __return_type __function_name(__parameter_type v0, __parameter_type v1, __parameter_type v2, __parameter_type v3, __parameter_type v4, __parameter_type v5) \
{ \
	return __function_name(__function_name(__function_name(__function_name(__function_name(v0, v1), v2), v3), v4), v5); \
} \
 \
NL_FORCE_INLINE __return_type __function_name(__parameter_type v0, __parameter_type v1, __parameter_type v2, __parameter_type v3, __parameter_type v4, __parameter_type v5, __parameter_type v6) \
{ \
	return __function_name(__function_name(__function_name(__function_name(__function_name(__function_name(v0, v1), v2), v3), v4), v5), v6); \
} \
 \
NL_FORCE_INLINE __return_type __function_name(__parameter_type v0, __parameter_type v1, __parameter_type v2, __parameter_type v3, __parameter_type v4, __parameter_type v5, __parameter_type v6, __parameter_type v7) \
{ \
	return __function_name(__function_name(__function_name(__function_name(__function_name(__function_name(__function_name(v0, v1), v2), v3), v4), v5), v6), v7); \
} \



#define NL_CLAMP_USING_MIN_MAX(__return_type, __parameter_type) \
NL_FORCE_INLINE __return_type clamp(__parameter_type value, __parameter_type minValue, __parameter_type maxValue) \
{ \
	return minF(maxF(minValue, value), maxValue); \
} \



#define NL_MATH_OPERATORS_USING_FUNCTIONS_ADD_SUB(__return_type, __ref_return_type, __in_parameter_type, __out_parameter_type) \
\
NL_FORCE_INLINE __return_type operator+(__in_parameter_type l, __in_parameter_type r) \
{ \
	return add(l, r); \
} \
\
NL_FORCE_INLINE __ref_return_type operator+=(__out_parameter_type l, __in_parameter_type r) \
{ \
	l = add(l, r); \
	return l; \
} \
\
NL_FORCE_INLINE __return_type operator-(__in_parameter_type l, __in_parameter_type r) \
{ \
	return sub(l, r); \
} \
\
NL_FORCE_INLINE __ref_return_type operator-=(__out_parameter_type l, __in_parameter_type r) \
{ \
	l = sub(l, r); \
	return l; \
} \



#define NL_MATH_OPERATORS_USING_FUNCTIONS_MUL_DIV(__return_type, __ref_return_type, __in_parameter_type, __out_parameter_type) \
\
NL_FORCE_INLINE __return_type operator*(__in_parameter_type l, __in_parameter_type r) \
{ \
	return mul(l, r); \
} \
\
NL_FORCE_INLINE __ref_return_type operator*=(__out_parameter_type l, __in_parameter_type r) \
{ \
	l = mul(l, r); \
	return l; \
} \
\
NL_FORCE_INLINE __return_type operator/(__in_parameter_type l, __in_parameter_type r) \
{ \
	return div(l, r); \
} \
\
NL_FORCE_INLINE __ref_return_type operator/=(__out_parameter_type l, __in_parameter_type r) \
{ \
	l = div(l, r); \
	return l; \
} \



#define NL_MATH_OPERATORS_USING_FUNCTIONS_ADD_SUB_MUL_DIV(__return_type, __ref_return_type, __in_parameter_type, __out_parameter_type) \
\
NL_MATH_OPERATORS_USING_FUNCTIONS_ADD_SUB(__return_type, __ref_return_type, __in_parameter_type, __out_parameter_type) \
NL_MATH_OPERATORS_USING_FUNCTIONS_MUL_DIV(__return_type, __ref_return_type, __in_parameter_type, __out_parameter_type) \



namespace NLMISC {

} /* namespace NLMISC */

#endif /* #ifndef NLMISC_VECTORF_MISC_H */

/* end of file */

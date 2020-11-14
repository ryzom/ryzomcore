/**
 * \file containers.h
 * \brief CContainers
 * \date 2012-04-10 13:57GMT
 * \author Unknown (Unknown)
 * CContainers
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE.
 * RYZOM CORE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * RYZOM CORE is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef NLSOUND_CONTAINERS_H
#define NLSOUND_CONTAINERS_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

namespace NLSOUND {
	class CSourceCommon;

/// Hasher functor for hashed container with pointer key.
template <class Pointer>
struct THashPtr : public std::unary_function<const Pointer &, size_t>
{
	enum { bucket_size = 4, min_buckets = 8, };
	size_t operator () (const Pointer &ptr) const
	{
		//CHashSet<uint>::hasher	h;
		// transtype the pointer into int then hash it
		//return h.operator()(uint(uintptr_t(ptr)));
		return (size_t)(uintptr_t)ptr;
	}
	inline bool operator() (const Pointer &ptr1, const Pointer &ptr2) const
	{
		// delegate the work to someone else as well?
		return (uintptr_t)ptr1 < (uintptr_t)ptr2;
	}
};

typedef CHashSet<CSourceCommon*, THashPtr<CSourceCommon*> > TSourceContainer;

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_CONTAINERS_H */

/* end of file */

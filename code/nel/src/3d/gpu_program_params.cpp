/**
 * \file gpu_program_params.cpp
 * \brief CGPUProgramParams
 * \date 2013-09-07 22:17GMT
 * \author Jan Boon (Kaetemi)
 * CGPUProgramParams
 */

/* 
 * Copyright (C) 2013  by authors
 * 
 * This file is part of NL3D.
 * NL3D is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * NL3D is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with NL3D.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include <nel/3d/gpu_program_params.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

// Project includes
#include <nel/3d/driver.h>

using namespace std;
// using namespace NLMISC;

namespace NL3D {

CGPUProgramParams::CGPUProgramParams() : m_First(s_End), m_Last(s_End)
{
	
}

CGPUProgramParams::~CGPUProgramParams()
{
	
}

void CGPUProgramParams::set1f(uint index, float f0)
{
	float *f = getPtrFByOffset(allocOffset(index, 1, 1, Float));
	f[0] = f0;
}

void CGPUProgramParams::set2f(uint index, float f0, float f1)
{
	float *f = getPtrFByOffset(allocOffset(index, 2, 1, Float));
	f[0] = f0;
	f[1] = f1;
}

void CGPUProgramParams::set3f(uint index, float f0, float f1, float f2)
{
	float *f = getPtrFByOffset(allocOffset(index, 3, 1, Float));
	f[0] = f0;
	f[1] = f1;
	f[2] = f2;
}

void CGPUProgramParams::set4f(uint index, float f0, float f1, float f2, float f3)
{
	float *f = getPtrFByOffset(allocOffset(index, 4, 1, Float));
	f[0] = f0;
	f[1] = f1;
	f[2] = f2;
	f[3] = f3;
}

void CGPUProgramParams::set1i(uint index, int i0)
{
	int *i = getPtrIByOffset(allocOffset(index, 1, 1, Int));
	i[0] = i0;
}

void CGPUProgramParams::set2i(uint index, int i0, int i1)
{
	int *i = getPtrIByOffset(allocOffset(index, 2, 1, Int));
	i[0] = i0;
	i[1] = i1;
}

void CGPUProgramParams::set3i(uint index, int i0, int i1, int i2)
{
	int *i = getPtrIByOffset(allocOffset(index, 3, 1, Int));
	i[0] = i0;
	i[1] = i1;
	i[2] = i2;
}

void CGPUProgramParams::set4i(uint index, int i0, int i1, int i2, int i3)
{
	int *i = getPtrIByOffset(allocOffset(index, 4, 1, Int));
	i[0] = i0;
	i[1] = i1;
	i[2] = i2;
	i[3] = i3;
}

void CGPUProgramParams::set3f(uint index, const NLMISC::CVector& v)
{
	float *f = getPtrFByOffset(allocOffset(index, 3, 1, Float));
	f[0] = v.x;
	f[1] = v.y;
	f[2] = v.z;
}

void CGPUProgramParams::set4x4f(uint index, const NLMISC::CMatrix& m)
{
	// TODO: Verify this!
	float *f = getPtrFByOffset(allocOffset(index, 4, 4, Float));
	NLMISC::CMatrix mt = m;
	mt.transpose();
	mt.get(f);
}

void CGPUProgramParams::set4fv(uint index, size_t num, const float *src)
{
	float *f = getPtrFByOffset(allocOffset(index, 4, num, Float));
	size_t nb = 4 * num;
	for (uint c = 0; c < nb; ++c)
		f[c] = src[c];
}

/// Allocate specified number of components if necessary
size_t CGPUProgramParams::allocOffset(uint index, uint size, uint count, TType type)
{
	nlassert(count > 0); // this code will not properly handle 0
	nlassert(size > 0); // this code will not properly handle 0
	nlassert(index < 0xFFFF); // sanity check

	uint nbComponents = size * count;
	size_t offset = getOffset(index);
	if (offset != s_End)
	{
		if (getCountByOffset(offset) >= nbComponents)
		{
			m_Meta[offset].Type = type;
			m_Meta[offset].Size = size;
			m_Meta[offset].Count = count;
			return offset;
		}
		if (getCountByOffset(offset) < nbComponents)
		{
			freeOffset(offset);
		}
	}

	// Allocate space
	offset = m_Meta.size();
	uint blocks = getNbRegistersByComponents(nbComponents); // per 4 components
	m_Meta.resize(offset + blocks);
	m_Vec.resize(offset + blocks);

	// Store offset in map
	if (index >= m_Map.size())
		m_Map.resize(index + 1, s_End);
	m_Map[index] = offset;

	// Fill
	m_Meta[offset].Index = index;
	m_Meta[offset].Size = size;
	m_Meta[offset].Count = count;
	m_Meta[offset].Type = type;
	m_Meta[offset].Prev = m_Last;
	m_Meta[offset].Next = s_End;

	// Link
	if (m_Last == s_End)
	{
		m_First = m_Last = offset;
	}
	else
	{
		nlassert(m_Meta[m_Last].Next == s_End); // code error otherwise
		m_Meta[m_Last].Next = offset;
		m_Last = offset;
	}

	return offset;
}

/// Return offset for specified index
size_t CGPUProgramParams::getOffset(uint index) const
{
	if (index >= m_Map.size())
		return s_End;
	return m_Map[index];
}

/// Remove by offset
void CGPUProgramParams::freeOffset(size_t offset)
{
	if (offset == m_Last)
	{
		nlassert(m_Meta[offset].Next == s_End);
		m_Last = m_Meta[offset].Prev;
	}
	else
	{
		nlassert(m_Meta[offset].Next != s_End);		
		m_Meta[m_Meta[offset].Next].Prev = m_Meta[offset].Prev;
	}
	if (offset == m_First)
	{
		nlassert(m_Meta[offset].Prev == s_End);
		m_First = m_Meta[offset].Next;
	}
	else
	{
		nlassert(m_Meta[offset].Prev != s_End);
		m_Meta[m_Meta[offset].Prev].Next = m_Meta[offset].Next;
	}
}

} /* namespace NL3D */

/* end of file */

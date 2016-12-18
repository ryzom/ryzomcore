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

#include "std3d.h"
#include "nel/3d/gpu_program_params.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include "nel/misc/vector.h"
#include "nel/misc/matrix.h"

// Project includes
#include "nel/3d/driver.h"

using namespace std;
// using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

const size_t CGPUProgramParams::s_End = -1;

CGPUProgramParams::CGPUProgramParams() : m_First(s_End), m_Last(s_End)
{
}

CGPUProgramParams::~CGPUProgramParams()
{
}

void CGPUProgramParams::copy(CGPUProgramParams *params)
{
	size_t offset = params->getBegin();
	while (offset != params->getEnd())
	{
		uint index = params->getIndexByOffset(offset);
		const std::string &name = params->getNameByOffset(offset);
		size_t local;
		uint size = params->getSizeByOffset(offset);
		uint count = params->getCountByOffset(offset);
		uint nbComponents = size * count;
		if (index)
		{
			local = allocOffset(index, size, count, params->getTypeByOffset(offset));
			if (!name.empty())
			{
				map(index, name);
			}
		}
		else
		{
			nlassert(!name.empty());
			local = allocOffset(name, size, count, params->getTypeByOffset(offset));
		}
		
		uint32 *src = params->getPtrUIByOffset(offset);
		uint32 *dst = getPtrUIByOffset(local);

		for (uint c = 0; c < nbComponents; ++c)
		{
			dst[c] = src[c];
		}

		offset = params->getNext(offset);
	}
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

void CGPUProgramParams::set1i(uint index, sint32 i0)
{
	sint32 *i = getPtrIByOffset(allocOffset(index, 1, 1, Int));
	i[0] = i0;
}

void CGPUProgramParams::set2i(uint index, sint32 i0, sint32 i1)
{
	sint32 *i = getPtrIByOffset(allocOffset(index, 2, 1, Int));
	i[0] = i0;
	i[1] = i1;
}

void CGPUProgramParams::set3i(uint index, sint32 i0, sint32 i1, sint32 i2)
{
	sint32 *i = getPtrIByOffset(allocOffset(index, 3, 1, Int));
	i[0] = i0;
	i[1] = i1;
	i[2] = i2;
}

void CGPUProgramParams::set4i(uint index, sint32 i0, sint32 i1, sint32 i2, sint32 i3)
{
	sint32 *i = getPtrIByOffset(allocOffset(index, 4, 1, Int));
	i[0] = i0;
	i[1] = i1;
	i[2] = i2;
	i[3] = i3;
}

void CGPUProgramParams::set1ui(uint index, uint32 ui0)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(index, 1, 1, UInt));
	ui[0] = ui0;
}

void CGPUProgramParams::set2ui(uint index, uint32 ui0, uint32 ui1)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(index, 2, 1, UInt));
	ui[0] = ui0;
	ui[1] = ui1;
}

void CGPUProgramParams::set3ui(uint index, uint32 ui0, uint32 ui1, uint32 ui2)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(index, 3, 1, UInt));
	ui[0] = ui0;
	ui[1] = ui1;
	ui[2] = ui2;
}

void CGPUProgramParams::set4ui(uint index, uint32 ui0, uint32 ui1, uint32 ui2, uint32 ui3)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(index, 4, 1, UInt));
	ui[0] = ui0;
	ui[1] = ui1;
	ui[2] = ui2;
	ui[3] = ui3;
}

void CGPUProgramParams::set3f(uint index, const NLMISC::CVector& v)
{
	float *f = getPtrFByOffset(allocOffset(index, 3, 1, Float));
	f[0] = v.x;
	f[1] = v.y;
	f[2] = v.z;
}

void CGPUProgramParams::set4f(uint index, const NLMISC::CVector& v, float f3)
{
	float *f = getPtrFByOffset(allocOffset(index, 4, 1, Float));
	f[0] = v.x;
	f[1] = v.y;
	f[2] = v.z;
	f[3] = f3;
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

void CGPUProgramParams::set4iv(uint index, size_t num, const sint32 *src)
{
	sint32 *i = getPtrIByOffset(allocOffset(index, 4, num, Int));
	size_t nb = 4 * num;
	for (uint c = 0; c < nb; ++c)
		i[c] = src[c];
}

void CGPUProgramParams::set4uiv(uint index, size_t num, const uint32 *src)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(index, 4, num, UInt));
	size_t nb = 4 * num;
	for (uint c = 0; c < nb; ++c)
		ui[c] = src[c];
}

void CGPUProgramParams::unset(uint index)
{
	size_t offset = getOffset(index);
	if (offset != getEnd())
	{
		freeOffset(offset);
	}
}

void CGPUProgramParams::set1f(const std::string &name, float f0)
{
	float *f = getPtrFByOffset(allocOffset(name, 1, 1, Float));
	f[0] = f0;
}

void CGPUProgramParams::set2f(const std::string &name, float f0, float f1)
{
	float *f = getPtrFByOffset(allocOffset(name, 2, 1, Float));
	f[0] = f0;
	f[1] = f1;
}

void CGPUProgramParams::set3f(const std::string &name, float f0, float f1, float f2)
{
	float *f = getPtrFByOffset(allocOffset(name, 3, 1, Float));
	f[0] = f0;
	f[1] = f1;
	f[2] = f2;
}

void CGPUProgramParams::set4f(const std::string &name, float f0, float f1, float f2, float f3)
{
	float *f = getPtrFByOffset(allocOffset(name, 4, 1, Float));
	f[0] = f0;
	f[1] = f1;
	f[2] = f2;
	f[3] = f3;
}

void CGPUProgramParams::set1i(const std::string &name, sint32 i0)
{
	sint32 *i = getPtrIByOffset(allocOffset(name, 1, 1, Int));
	i[0] = i0;
}

void CGPUProgramParams::set2i(const std::string &name, sint32 i0, sint32 i1)
{
	sint32 *i = getPtrIByOffset(allocOffset(name, 2, 1, Int));
	i[0] = i0;
	i[1] = i1;
}

void CGPUProgramParams::set3i(const std::string &name, sint32 i0, sint32 i1, sint32 i2)
{
	sint32 *i = getPtrIByOffset(allocOffset(name, 3, 1, Int));
	i[0] = i0;
	i[1] = i1;
	i[2] = i2;
}

void CGPUProgramParams::set4i(const std::string &name, sint32 i0, sint32 i1, sint32 i2, sint32 i3)
{
	sint32 *i = getPtrIByOffset(allocOffset(name, 4, 1, Int));
	i[0] = i0;
	i[1] = i1;
	i[2] = i2;
	i[3] = i3;
}

void CGPUProgramParams::set1ui(const std::string &name, uint32 ui0)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(name, 1, 1, UInt));
	ui[0] = ui0;
}

void CGPUProgramParams::set2ui(const std::string &name, uint32 ui0, uint32 ui1)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(name, 2, 1, UInt));
	ui[0] = ui0;
	ui[1] = ui1;
}

void CGPUProgramParams::set3ui(const std::string &name, uint32 ui0, uint32 ui1, uint32 ui2)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(name, 3, 1, UInt));
	ui[0] = ui0;
	ui[1] = ui1;
	ui[2] = ui2;
}

void CGPUProgramParams::set4ui(const std::string &name, uint32 ui0, uint32 ui1, uint32 ui2, uint32 ui3)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(name, 4, 1, UInt));
	ui[0] = ui0;
	ui[1] = ui1;
	ui[2] = ui2;
	ui[3] = ui3;
}

void CGPUProgramParams::set3f(const std::string &name, const NLMISC::CVector& v)
{
	float *f = getPtrFByOffset(allocOffset(name, 3, 1, Float));
	f[0] = v.x;
	f[1] = v.y;
	f[2] = v.z;
}

void CGPUProgramParams::set4f(const std::string &name, const NLMISC::CVector& v, float f3)
{
	float *f = getPtrFByOffset(allocOffset(name, 4, 1, Float));
	f[0] = v.x;
	f[1] = v.y;
	f[2] = v.z;
	f[3] = f3;
}

void CGPUProgramParams::set4x4f(const std::string &name, const NLMISC::CMatrix& m)
{
	// TODO: Verify this!
	float *f = getPtrFByOffset(allocOffset(name, 4, 4, Float));
	NLMISC::CMatrix mt = m;
	mt.transpose();
	mt.get(f);
}

void CGPUProgramParams::set4fv(const std::string &name, size_t num, const float *src)
{
	float *f = getPtrFByOffset(allocOffset(name, 4, num, Float));
	size_t nb = 4 * num;
	for (uint c = 0; c < nb; ++c)
		f[c] = src[c];
}

void CGPUProgramParams::set4iv(const std::string &name, size_t num, const sint32 *src)
{
	sint32 *i = getPtrIByOffset(allocOffset(name, 4, num, Int));
	size_t nb = 4 * num;
	for (uint c = 0; c < nb; ++c)
		i[c] = src[c];
}

void CGPUProgramParams::set4uiv(const std::string &name, size_t num, const uint32 *src)
{
	uint32 *ui = getPtrUIByOffset(allocOffset(name, 4, num, UInt));
	size_t nb = 4 * num;
	for (uint c = 0; c < nb; ++c)
		ui[c] = src[c];
}

void CGPUProgramParams::unset(const std::string &name)
{
	size_t offset = getOffset(name);
	if (offset != getEnd())
	{
		freeOffset(offset);
	}
}

void CGPUProgramParams::map(uint index, const std::string &name)
{
	size_t offsetIndex = getOffset(index);
	size_t offsetName = getOffset(name);
	if (offsetName != getEnd())
	{
		// Remove possible duplicate
		if (offsetIndex != getEnd())
		{
			freeOffset(offsetIndex);
		}

		// Set index
		m_Meta[offsetName].Index = index;

		// Map index to name
		if (index >= m_Map.size())
			m_Map.resize(index + 1, s_End);
		m_Map[index] = offsetName;
	}
	else if (offsetIndex != getEnd())
	{
		// Set name
		m_Meta[offsetIndex].Name = name;

		// Map name to index
		m_MapName[name] = offsetIndex;
	}
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
	offset = allocOffset(size, count, type);

	// Fill
	m_Meta[offset].Index = index;

	// Store offset in map
	if (index >= m_Map.size())
		m_Map.resize(index + 1, s_End);
	m_Map[index] = offset;

	return offset;
}

/// Allocate specified number of components if necessary
size_t CGPUProgramParams::allocOffset(const std::string &name, uint size, uint count, TType type)
{
	nlassert(count > 0); // this code will not properly handle 0
	nlassert(size > 0); // this code will not properly handle 0
	nlassert(!name.empty()); // sanity check

	uint nbComponents = size * count;
	size_t offset = getOffset(name);
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
	offset = allocOffset(size, count, type);

	// Fill
	m_Meta[offset].Name = name;

	// Store offset in map
	m_MapName[name] = offset;

	return offset;
}

/// Allocate specified number of components if necessary
size_t CGPUProgramParams::allocOffset(uint size, uint count, TType type)
{
	uint nbComponents = size * count;

	// Allocate space
	size_t offset = m_Meta.size();
	uint blocks = getNbRegistersByComponents(nbComponents); // per 4 components
	m_Meta.resize(offset + blocks);
	m_Vec.resize(offset + blocks);

	// Fill
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

size_t CGPUProgramParams::getOffset(const std::string &name) const
{
	std::map<std::string, size_t>::const_iterator it = m_MapName.find(name);
	if (it == m_MapName.end())
		return s_End;
	return it->second;
}

/// Remove by offset
void CGPUProgramParams::freeOffset(size_t offset)
{
	uint index = getIndexByOffset(offset);
	if (index != std::numeric_limits<uint>::max())
	{
		if (m_Map.size() > index)
		{
			m_Map[index] = getEnd();
		}
	}
	const std::string &name = getNameByOffset(offset);
	if (!name.empty())
	{
		if (m_MapName.find(name) != m_MapName.end())
		{
			m_MapName.erase(name);
		}
	}
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

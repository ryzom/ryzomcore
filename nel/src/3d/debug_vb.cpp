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

#include "std3d.h"
#include "nel/3d/debug_vb.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

static void checkRange(const uint8 *min, const uint8 *max, const uint8 *start, uint length, const std::string &msg)
{
	if (start + length < min || start >= max)
	{
		nlwarning("Buffer start = %p, end = %p, accessed range = (%p, %p), %s", min, max, start, start + length, msg.c_str());
		nlassert(0);
	}
}

// ************************************************************************************
void nlCheckVertexBuffer(const CVertexBuffer &vb, const uint8 *ptr)
{
	CVertexBufferRead vba;
	vb.lock (vba);
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vb.getNumVertices() * vb.getVertexSize(), ptr, 0, vb.getName());
}

// ************************************************************************************
void nlCheckVBA(CVertexBufferRead &vba, const uint8 *ptr)
{
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vba.getParent()->getNumVertices() * vba.getParent()->getVertexSize(), ptr, 0, vba.getParent()->getName());
}

// ************************************************************************************
void nlCheckVBA(CVertexBufferReadWrite &vba, const uint8 *ptr)
{
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vba.getParent()->getNumVertices() * vba.getParent()->getVertexSize(), ptr, 0, vba.getParent()->getName());
}

// ************************************************************************************
void nlCheckVBARange(CVertexBufferRead &vba, const uint8 *ptStart, uint length)
{
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vba.getParent()->getNumVertices() * vba.getParent()->getVertexSize(), ptStart, length, vba.getParent()->getName());
}

// ************************************************************************************
void nlCheckVBARange(CVertexBufferReadWrite &vba, const uint8 *ptStart, uint length)
{
	checkRange((uint8 *) vba.getVertexCoordPointer(), (uint8 *) vba.getVertexCoordPointer() + vba.getParent()->getNumVertices() * vba.getParent()->getVertexSize(), ptStart, length, vba.getParent()->getName());
}

// ************************************************************************************
void nlCheckIBARange(CIndexBufferReadWrite &iba, const uint8 *ptStart, uint length)
{
	checkRange((uint8 *) iba.getPtr(), (uint8 *) iba.getPtr() + iba.getParent()->getNumIndexes() * iba.getIndexNumBytes(), ptStart, length, iba.getParent()->getName());
}

// ************************************************************************************
void nlCheckIBARange(CIndexBufferRead &iba, const uint8 *ptStart, uint length)
{
	checkRange((uint8 *) iba.getPtr(), (uint8 *) iba.getPtr() + iba.getParent()->getNumIndexes() * iba.getIndexNumBytes(), ptStart, length, iba.getParent()->getName());
}

// ************************************************************************************
void nlCheckIBA(CIndexBufferReadWrite &iba, const uint8 *ptStart)
{
	checkRange((uint8 *) iba.getPtr(), (uint8 *) iba.getPtr() + iba.getParent()->getNumIndexes() * iba.getIndexNumBytes(), ptStart, 0, iba.getParent()->getName());
}

// ************************************************************************************
void nlCheckIBA(CIndexBufferRead &iba, const uint8 *ptStart)
{
	checkRange((uint8 *) iba.getPtr(), (uint8 *) iba.getPtr() + iba.getParent()->getNumIndexes() * iba.getIndexNumBytes(), ptStart, 0, iba.getParent()->getName());
}

} // NL3D

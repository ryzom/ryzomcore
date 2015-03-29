/*

Copyright (C) 2015  Jan Boon <jan.boon@kaetemi.be>
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

#include "std3d.h"
#include <nel/3d/uniform_buffer_format.h>

#include <nel/misc/debug.h>
#include <nel/misc/wang_hash.h>

namespace NL3D {

const sint CUniformBufferFormat::s_TypeAlignment[] = {
	4, // Float
	8,
	16,
	16,
	4, // SInt
	8,
	16,
	16,
	4, // UInt
	8,
	16,
	16,
	4, // Bool
	8,
	16,
	16,
	16, // FloatMat2
	16,
	16,
	16, // FloatMat2x3
	16,
	16, // FloatMat3x2
	16,
	16, // FloatMat4x2
	16,
};

const sint CUniformBufferFormat::s_TypeSize[] = {
	4, // Float
	8,
	12,
	16,
	4, // SInt
	8,
	12,
	16,
	4, // UInt
	8,
	12,
	16,
	4, // Bool
	8,
	12,
	16,
	16 + 16, // FloatMat2
	16 + 16 + 16, // FloatMat3
	16 + 16 + 16 + 16, // FloatMat4
	16 + 16, // FloatMat2x3
	16 + 16, // FloatMat2x4
	16 + 16 + 16, // FloatMat3x2
	16 + 16 + 16, // FloatMat3x4
	16 + 16 + 16 + 16, // FloatMat4x2
	16 + 16 + 16 + 16, // FloatMat4x3
};

sint CUniformBufferFormat::push(const std::string &name, TType type, sint count)
{
	nlassert(count > 0);
	sint baseAlign = count == 1
		? s_TypeAlignment[type]
		: ((s_TypeAlignment[type] + 15) & ~0xF);
	sint baseOffset = m_Entries.size()
		? m_Entries.back().Offset + m_Entries.back().size()
		: 0;
	sint alignOffset = baseOffset;
	alignOffset += (baseAlign - 1);
	alignOffset &= ~(baseAlign - 1); // Note: alignment MUST BE power of 2 for this to work
	m_Entries.resize(m_Entries.size() + 1);
	CEntry &entry = m_Entries.back();
	entry.Name = NLMISC::CStringMapper::map(name);
	entry.Type = type;
	entry.Offset = alignOffset;
	entry.Count = count;
#if (HAVE_X86_64)
	m_Hash = NLMISC::wangHash64(m_Hash ^ ((uint64)type | ((uint64)count << 32)));
#else
	m_Hash = NLMISC::wangHash(m_Hash ^ (uint32)type);
	m_Hash = NLMISC::wangHash(m_Hash ^ (uint32)count);
#endif
	return alignOffset;
}

void testUniformBufferFormat(CUniformBufferFormat &ubf)
{
	sint offset;
	offset = ubf.push("a", CUniformBufferFormat::Float);
	nlassert(offset == 0);
	offset = ubf.push("b", CUniformBufferFormat::FloatVec2);
	nlassert(offset == 8);
	offset = ubf.push("c", CUniformBufferFormat::FloatVec3);
	nlassert(offset == 16);
	offset = ubf.push("d", CUniformBufferFormat::FloatVec4);
	nlassert(offset == 32);
	offset = ubf.push("e", CUniformBufferFormat::FloatVec2);
	nlassert(offset == 48);
	offset = ubf.push("g", CUniformBufferFormat::Float);
	nlassert(offset == 56);
	offset = ubf.push("h", CUniformBufferFormat::Float, 2);
	nlassert(offset == 64);
	offset = ubf.push("i", CUniformBufferFormat::FloatMat2x3);
	nlassert(offset == 96);
	offset = ubf.push("j", CUniformBufferFormat::FloatVec3);
	nlassert(offset == 128);
	offset = ubf.push("k", CUniformBufferFormat::FloatVec2);
	nlassert(offset == 144);
	offset = ubf.push("l", CUniformBufferFormat::Float, 2);
	nlassert(offset == 160);
	offset = ubf.push("m", CUniformBufferFormat::FloatVec2);
	nlassert(offset == 192);
	offset = ubf.push("n", CUniformBufferFormat::FloatMat3, 2);
	nlassert(offset == 208);
	offset = ubf.push("o", CUniformBufferFormat::FloatVec3);
	nlassert(offset == 304);
}

} /* namespace NL3D */

/* end of file */

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

#ifndef NL_UNIFORM_BUFFER_FORMAT_H
#define NL_UNIFORM_BUFFER_FORMAT_H

#include <nel/misc/types_nl.h>
#include <nel/misc/string_mapper.h>

namespace NL3D {

// Uniform buffer format generation following glsl std140 rules
class CUniformBufferFormat
{
public:
	// When changing, update
	//     - s_TypeAlignment
	//     - s_TypeSize
	//     - NL3D::NLDRIVERGL3::s_TypeKeyword
	enum TType
	{
		Float, // float
		FloatVec2, // CVector2D
		FloatVec3,
		FloatVec4, // CVector
		SInt, // sint32
		SIntVec2,
		SIntVec3,
		SIntVec4,
		UInt, // uint32
		UIntVec2,
		UIntVec3,
		UIntVec4,
		Bool,
		BoolVec2,
		BoolVec3,
		BoolVec4,
		FloatMat2,
		FloatMat3,
		FloatMat4, // CMatrix
		FloatMat2x3,
		FloatMat2x4,
		FloatMat3x2,
		FloatMat3x4,
		FloatMat4x2,
		FloatMat4x3,
	};

	struct CEntry
	{
		NLMISC::TStringId Name;
		TType Type;
		sint Offset;
		sint Count;

		inline sint stride() const
		{
			return Count == 1
				? s_TypeSize[Type]
				: ((s_TypeSize[Type] + 15) & ~0xF);
		}
		inline sint size() const
		{
			return stride() * Count;
		}
	};

	// Push a variable. Returns the byte offset in uniform buffer
	// Note: Does not check for duplicate names. However, names must be unique
	sint push(const std::string &name, TType type, sint count = 1);

	inline const CEntry &get(sint i) const { return m_Entries[i]; }
	inline size_t size() const { return m_Entries.size(); }
	inline void clear() { m_Entries.clear(); }

private:
	static const sint s_TypeAlignment[];
	static const sint s_TypeSize[];

	typedef std::vector<CEntry> TEntries;
	TEntries m_Entries;

};

void testUniformBufferFormat(CUniformBufferFormat &ubf);

} /* namespace NL3D */

#endif /* #ifndef NL_UNIFORM_BUFFER_FORMAT_H */

/* end of file */

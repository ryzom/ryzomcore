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

/* 
**** IMPORTANT ********************
**** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
***********************************
*/

// Uniform buffer format generation following glsl std140 rules
class CUniformBufferFormat
{
public:
	CUniformBufferFormat() : m_Hash(0) { }

	// When changing, update
	//     - s_TypeAlignment
	//     - s_TypeSize
	//     - NL3D::NLDRIVERGL3::s_TypeKeyword
	enum TType
	{
		Float, // float
		FloatVec2, // CVector2f
		FloatVec3, // CVector
		FloatVec4, // CVectorH
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
		inline sint offset(int i) const
		{
			return Offset + (stride() * i);
		}
	};

	// Push a variable. Returns the byte offset in uniform buffer
	// Note: Does not check for duplicate names. However, names must be unique
	sint push(const std::string &name, TType type, sint count = 1);

	inline const CEntry &get(sint i) const { return m_Entries[i]; }
	inline size_t count() const { return m_Entries.size(); } // Return number of entries
	inline void clear() { m_Entries.clear(); m_Hash = 0; }

	inline sint size() const { return m_Entries.size() ? (m_Entries.back().Offset + m_Entries.back().size()) : 0; } // Return size of format in bytes
	inline size_t hash() const { return m_Hash; }

	// Get the offset by entry id (counted from 0 in the order of addition to the format) and index of array
	inline sint offset(sint entry, sint index = 0) const { m_Entries[entry].offset(index); }

private:
	static const sint s_TypeAlignment[];
	static const sint s_TypeSize[];

	typedef std::vector<CEntry> TEntries;
	TEntries m_Entries;
	size_t m_Hash;

};

void testUniformBufferFormat(CUniformBufferFormat &ubf);

} /* namespace NL3D */

namespace std {
	
template <>
struct hash<NL3D::CUniformBufferFormat>
{
	size_t operator()(const NL3D::CUniformBufferFormat & v) const { return v.hash(); }
};

} /* namespace std */

#endif /* #ifndef NL_UNIFORM_BUFFER_FORMAT_H */

/* end of file */

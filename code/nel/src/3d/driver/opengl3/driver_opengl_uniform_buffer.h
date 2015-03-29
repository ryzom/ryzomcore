// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  by authors
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

#ifndef NL_DRIVER_OPENGL_UNIFORM_BUFFER_H
#define NL_DRIVER_OPENGL_UNIFORM_BUFFER_H

#include "nel/misc/types_nl.h"

namespace NL3D {
namespace NLDRIVERGL3 {

// NOTE: It is completely safe to reorder these indices.
// Always use the defines.
#define NL_BUILTIN_CAMERA_BIND 0 // Builtin uniform buffer bound by driver, set by camera transformation
#define NL_BUILTIN_MODEL_BIND 1 // Builtin uniform buffer bound by driver, set by model transformation
#define NL_BUILTIN_MATERIAL_BIND 2 // Builtin uniform buffer bound by material
#define NL_USER_ENV_BIND 3 // User-specified uniform buffer bound by user
#define NL_USER_VERTEX_PROGRAM_BIND 4 // User-specified uniform buffer bound by vertex program
#define NL_USER_GEOMETRY_PROGRAM_BIND 5 // User-specified uniform buffer bound by geometry program
#define NL_USER_PIXEL_PROGRAM_BIND 6 // User-specified uniform buffer bound by pixel program
#define NL_USER_MATERIAL_BIND 7 // User-specified uniform buffer bound by material

extern const char *GLSLHeaderUniformBuffer;

} // NLDRIVERGL3
} // NL3D

#include "nel/misc/string_mapper.h"

namespace NL3D {

// Uniform buffer format generation following glsl std140 rules
class CUniformBufferFormat
{
public:
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

		inline sint stride()
		{
			return Count == 1
				? s_TypeSize[Type]
				: ((s_TypeSize[Type] + 15) & ~0xF);
		}
		inline sint size()
		{
			return stride() * Count;
		}
	};

	// Push a variable. Returns the byte offset in uniform buffer
	sint push(const std::string &name, TType type, sint count = 1)
	{
		nlassert(count > 0);
		sint baseAlign = count == 1 
			? s_TypeAlignment[type] 
			: ((s_TypeAlignment[type] + 15) & ~0xF);
		sint baseOffset = m_Entries.size()
			? m_Entries.back().Offset + m_Entries.back().stride()
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
		return baseOffset;
	}

	inline const CEntry *get() { return &m_Entries[0]; }
	inline size_t size() { return m_Entries.size(); }
	
private:
	static const sint s_TypeAlignment[];
	static const sint s_TypeSize[];

	typedef std::vector<CEntry> TEntries;
	TEntries m_Entries;

};

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
	16 + 8, // FloatMat2
	16 + 16 + 12, // FloatMat3
	16 + 16 + 16 + 16, // FloatMat4
	16 + 12, // FloatMat2x3
	16 + 16, // FloatMat2x4
	16 + 16 + 8, // FloatMat3x2
	16 + 16 + 16, // FloatMat3x4
	16 + 16 + 16 + 8, // FloatMat4x2
	16 + 16 + 16 + 12, // FloatMat4x3
};

}

#endif // NL_DRIVER_OPENGL_UNIFORM_BUFFER_H

/* end of file */

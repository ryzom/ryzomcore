/**
 * \file gpu_program_params.h
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

#ifndef NL3D_GPU_PROGRAM_PARAMS_H
#define NL3D_GPU_PROGRAM_PARAMS_H
#include <nel/misc/types_nl.h>

// STL includes
#include <map>
#include <vector>

// NeL includes

// Project includes

namespace NLMISC {
	class CVector;
	class CMatrix;
}

namespace NL3D {

/**
 * \brief CGPUProgramParams
 * \date 2013-09-07 22:17GMT
 * \author Jan Boon (Kaetemi)
 * A storage for USERCODE-PROVIDED parameters for GPU programs.
 * Allows for fast updating and iteration of parameters.
 * NOTE TO DRIVER IMPLEMENTORS: DO NOT USE FOR STORING COPIES 
 * OF HARDCODED DRIVER MATERIAL PARAMETERS OR DRIVER PARAMETERS!!!
 * The 4-component alignment that is done in this storage
 * class is necessary to simplify support for register-based
 * assembly shaders, which require setting per 4 components.
 */
class CGPUProgramParams
{
public:
	enum TType { Float, Int, UInt };
	struct CMeta { uint Index, Size, Count; TType Type; size_t Next, Prev; }; // size is element size, count is nb of elements

private:
	union CVec { float F[4]; sint32 I[4]; uint32 UI[4]; };

public:
	CGPUProgramParams();
	virtual ~CGPUProgramParams();

	// Copy from another params storage
	void copy(CGPUProgramParams *params);

	// Set by index, available only when the associated program has been compiled
	void set1f(uint index, float f0);
	void set2f(uint index, float f0, float f1);
	void set3f(uint index, float f0, float f1, float f2);
	void set4f(uint index, float f0, float f1, float f2, float f3);
	void set1i(uint index, sint32 i0);
	void set2i(uint index, sint32 i0, sint32 i1);
	void set3i(uint index, sint32 i0, sint32 i1, sint32 i2);
	void set4i(uint index, sint32 i0, sint32 i1, sint32 i2, sint32 i3);
	void set3f(uint index, const NLMISC::CVector& v);
	void set4f(uint index, const NLMISC::CVector& v, float f3);
	void set4x4f(uint index, const NLMISC::CMatrix& m);
	void set4fv(uint index, size_t num, const float *src);
	void unset(uint index);

	// Set by name, it is recommended to use index when repeatedly setting an element
	void set1f(const std::string &name, float f0);
	void set2f(const std::string &name, float f0, float f1);
	void set3f(const std::string &name, float f0, float f1, float f2);
	void set4f(const std::string &name, float f0, float f1, float f2, float f3);
	void set1i(const std::string &name, sint32 i0);
	void set2i(const std::string &name, sint32 i0, sint32 i1);
	void set3i(const std::string &name, sint32 i0, sint32 i1, sint32 i2);
	void set4i(const std::string &name, sint32 i0, sint32 i1, sint32 i2, sint32 i3);
	void set3f(const std::string &name, const NLMISC::CVector& v);
	void set4f(const std::string &name, const NLMISC::CVector& v, float f3);
	void set4x4f(const std::string &name, const NLMISC::CMatrix& m);
	void set4fv(const std::string &name, size_t num, const float *src);
	void unset(const std::string &name);

	/// Maps the given name to the given index, on duplicate entry the data set by name will be prefered as it can be assumed to have been set after the data set by index
	void map(uint index, const std::string &name);

	// Internal
	/// Allocate specified number of components if necessary
	size_t allocOffset(uint index, uint size, uint count, TType type);
	size_t allocOffset(const std::string &name, uint size, uint count, TType type);
	/// Return offset for specified index
	size_t getOffset(uint index) const;
	size_t getOffset(const std::string &name) const;
	/// Remove by offset
	void freeOffset(size_t offset);

	// Iteration (returns the offsets for access using getFooByOffset)
	inline size_t getBegin() const { return m_Meta.size() ? m_First : s_End; }
	inline size_t getNext(size_t offset) const { return m_Meta[offset].Next; }
	inline size_t getEnd() const { return s_End; }

	// Data access
	inline uint getSizeByOffset(size_t offset) const { return m_Meta[offset].Size; } // size of element (4 for float4)
	inline uint getCountByOffset(size_t offset) const { return m_Meta[offset].Count; } // number of elements (usually 1)
	inline uint getNbComponentsByOffset(size_t offset) const { return m_Meta[offset].Size * m_Meta[offset].Count; } // nb of components (size * count)
	inline float *getPtrFByOffset(size_t offset) { return m_Vec[offset].F; }
	inline sint32 *getPtrIByOffset(size_t offset) { return m_Vec[offset].I; }
	inline uint32 *getPtrUIByOffset(size_t offset) { return m_Vec[offset].UI; }
	inline TType getTypeByOffset(size_t offset) const { return m_Meta[offset].Type; }
	inline uint getIndexByOffset(size_t offset) const { return m_Meta[offset].Index; }
	const std::string *getNameByOffset(size_t offset) const; // non-optimized for dev tools only, may return NULL if name unknown

	// Utility
	static inline uint getNbRegistersByComponents(uint nbComponents) { return (nbComponents + 3) >> 2; } // vector register per 4 components

private:
	std::vector<CVec> m_Vec;
	std::vector<CMeta> m_Meta;
	std::vector<size_t> m_Map; // map from index to offset
	std::map<std::string, size_t> m_MapName; // map from name to offset
	size_t m_First;
	size_t m_Last;
	static const size_t s_End = -1;

}; /* class CGPUProgramParams */

} /* namespace NL3D */

#endif /* #ifndef NL3D_GPU_PROGRAM_PARAMS_H */

/* end of file */

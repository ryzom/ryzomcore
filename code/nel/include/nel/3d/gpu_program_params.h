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

namespace NL3D {

/**
 * \brief CGPUProgramParams
 * \date 2013-09-07 22:17GMT
 * \author Jan Boon (Kaetemi)
 * A storage for user-provided parameters for GPU programs.
 * Allows for fast updating and iteration of parameters.
 * NOTE TO DRIVER IMPLEMENTORS: DO NOT USE FOR STORING COPIES 
 * OF HARDCODED MATERIAL PARAMETERS OR DRIVER PARAMETERS!!!
 */
class CGPUProgramParams
{
public:
	enum TType { Float, Int };
	struct CMeta { uint Index, Count; TType Type; size_t Next, Prev; };

private:
	union CVec { float F[4]; sint32 I[4]; };

public:
	CGPUProgramParams();
	virtual ~CGPUProgramParams();

	void set(uint index, float f0, float f1, float f2, float f3);
	void set(uint index, int i0, int i1, int i2, int i3);

	// Internal
	/// Allocate specified number of components if necessary
	size_t allocOffset(uint index, uint count, TType type);
	/// Return offset for specified index
	size_t getOffset(uint index) const;
	/// Remove by offset
	void freeOffset(size_t offset);

	// Iteration
	size_t getBegin() const { return m_Meta.size() ? m_First : s_End; }
	size_t getNext(size_t offset) const { return m_Meta[offset].Next; }
	size_t getEnd() const { return s_End; }

	// Data access
	uint getCountByOffset(size_t offset) { return m_Meta[offset].Count; }
	float *getPtrFByOffset(size_t offset) { return m_Vec[offset].F; }
	int *getPtrIByOffset(size_t offset) { return m_Vec[offset].I; }
	TType getTypeByOffset(size_t offset) { return m_Meta[offset].Type; }

private:
	std::vector<CVec> m_Vec;
	std::vector<CMeta> m_Meta;
	std::vector<size_t> m_Map; // map from index to buffer index
	size_t m_First;
	size_t m_Last;
	static const size_t s_End = -1;

}; /* class CGPUProgramParams */

} /* namespace NL3D */

#endif /* #ifndef NL3D_GPU_PROGRAM_PARAMS_H */

/* end of file */

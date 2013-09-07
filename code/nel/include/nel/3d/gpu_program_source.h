/**
 * \file gpu_program_source.h
 * \brief CGPUProgramSource
 * \date 2013-09-07 14:54GMT
 * \author Jan Boon (Kaetemi)
 * CGPUProgramSource
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

#ifndef NL3D_GPU_PROGRAM_SOURCE_H
#define NL3D_GPU_PROGRAM_SOURCE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/string_mapper.h>

// Project includes
#include <nel/3d/gpu_program.h>

namespace NL3D {

/**
 * \brief CGPUProgramSource
 * \date 2013-09-07 14:54GMT
 * \author Jan Boon (Kaetemi)
 * A single GPU program with a specific profile.
 */
struct CGPUProgramSource : public NLMISC::CRefCount
{
public:
	std::string DisplayName;

	/// Minimal required profile for this GPU program
	IGPUProgram::TProfile Profile;
	
	const char *CodePtr;
	/// Copy the source code string
	inline void setCode(const char *source) { CodeCopy = source; CodePtr = &source[0]; }
	/// Set pointer to source code string without copying the string
	inline void setCodePtr(const char *sourcePtr) { CodeCopy.clear(); CodePtr = sourcePtr; }
	
	/// CVertexProgramInfo/CPixelProgramInfo/... NeL features
	uint Features;

	/// Map with known parameter indices, used for assembly programs
	std::map<std::string, uint> ParamIndices;
	
private:
	std::string CodeCopy;
	
}; /* class CGPUProgramSource */

/**
 * \brief CGPUProgramSourceCont
 * \date 2013-09-07 14:54GMT
 * \author Jan Boon (Kaetemi)
 * Container for the source code of a single GPU program, allowing
 * variations in different language profiles.
 */
struct CGPUProgramSourceCont : public NLMISC::CRefCount
{
public:
	std::vector<NLMISC::CSmartPtr<CGPUProgramSource> > Sources;
	
}; /* class CGPUProgramSourceCont */

} /* namespace NL3D */

#endif /* #ifndef NL3D_GPU_PROGRAM_SOURCE_H */

/* end of file */

/**
 * \file gpu_program.h
 * \brief IGPUProgram
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * IGPUProgram
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

#ifndef NL3D_GPU_PROGRAM_H
#define NL3D_GPU_PROGRAM_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <nel/misc/smart_ptr.h>

// Project includes

namespace NL3D {

// List typedef.
class	IDriver;
class	IGPUProgramDrvInfos;
typedef	std::list<IGPUProgramDrvInfos*>	TGPUPrgDrvInfoPtrList;
typedef	TGPUPrgDrvInfoPtrList::iterator		ItGPUPrgDrvInfoPtrList;

// Class for interaction of vertex program with Driver.
// IGPUProgramDrvInfos represent the real data of the GPU program, stored into the driver (eg: just a GLint for opengl).
class IGPUProgramDrvInfos : public NLMISC::CRefCount
{
private:
	IDriver					*_Driver;
	ItGPUPrgDrvInfoPtrList	_DriverIterator;

public:
	IGPUProgramDrvInfos (IDriver *drv, ItGPUPrgDrvInfoPtrList it);
	// The virtual dtor is important.
	virtual ~IGPUProgramDrvInfos(void);

	virtual uint getUniformIndex(const char *name) const = 0;
};

// Features exposed by a program. Used to set builtin parameters on user provided shaders.
// This is only used for user provided shaders, not for builtin shaders, 
// as it is a slow method which has to go through all of the options every time.
// Builtin shaders should set all flags to 0.
struct CGPUProgramFeatures
{
	CGPUProgramFeatures() : DriverFlags(0), MaterialFlags(0) { }

	// Driver builtin parameters
	enum TDriverFlags
	{
		// Matrices
		ModelView								= 0x00000001, 
		ModelViewInverse						= 0x00000002, 
		ModelViewTranspose						= 0x00000004, 
		ModelViewInverseTranspose				= 0x00000008, 

		Projection								= 0x00000010, 
		ProjectionInverse						= 0x00000020, 
		ProjectionTranspose						= 0x00000040, 
		ProjectionInverseTranspose				= 0x00000080, 

		ModelViewProjection						= 0x00000100, 
		ModelViewProjectionInverse				= 0x00000200, 
		ModelViewProjectionTranspose			= 0x00000400, 
		ModelViewProjectionInverseTranspose		= 0x00000800, 

		// Fog
		Fog										= 0x00001000, 
	};
	uint32 DriverFlags;
	// uint NumLights;

	enum TMaterialFlags
	{
		/// Use the CMaterial texture stages as the textures for a Pixel Program
		TextureStages							= 0x00000001, 
		TextureMatrices							= 0x00000002, 
	};
	// Material builtin parameters
	uint32 MaterialFlags;
};

// Stucture used to cache the indices of builtin parameters which are used by the drivers
// Not used for parameters of specific nl3d programs
struct CGPUProgramIndex
{
	enum TName
	{
		ModelView, 
		ModelViewInverse, 
		ModelViewTranspose, 
		ModelViewInverseTranspose, 

		Projection, 
		ProjectionInverse, 
		ProjectionTranspose, 
		ProjectionInverseTranspose, 

		ModelViewProjection, 
		ModelViewProjectionInverse, 
		ModelViewProjectionTranspose, 
		ModelViewProjectionInverseTranspose, 

		Fog, 

		NUM_UNIFORMS
	};
	static const char *Names[NUM_UNIFORMS];
	uint Indices[NUM_UNIFORMS];
};

/**
 * \brief IGPUProgram
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * A generic GPU program
 */
class IGPUProgram : public NLMISC::CRefCount
{
public:
	enum TProfile
	{
		none = 0,

		// types
		// Vertex Shader = 0x01
		// Pixel Shader = 0x02
		// Geometry Shader = 0x03

		// nel - 0x31,type,bitfield
		nelvp = 0x31010001, // VP supported by CVertexProgramParser, similar to arbvp1, can be translated to vs_1_1

		// direct3d - 0xD9,type,major,minor
		// vertex programs
		vs_1_1 = 0xD9010101,
		vs_2_0 = 0xD9010200,
		// vs_2_sw = 0xD9010201, // not sure...
		// vs_2_x = 0xD9010202, // not sure...
		// vs_3_0 = 0xD9010300, // not supported
		// pixel programs
		ps_1_1 = 0xD9020101, 
		ps_1_2 = 0xD9020102, 
		ps_1_3 = 0xD9020103, 
		ps_1_4 = 0xD9020104, 
		ps_2_0 = 0xD9020200, 
		// ps_2_x = 0xD9020201, // not sure...
		// ps_3_0 = 0xD9020300, // not supported

		// opengl - 0x61,type,bitfield
		// vertex programs
		// vp20 = 0x61010001, // NV_vertex_program1_1, outdated
		arbvp1 = 0x61010002, // ARB_vertex_program
		vp30 = 0x61010004, // NV_vertex_program2
		vp40 = 0x61010008, // NV_vertex_program3 + NV_fragment_program3
		gp4vp = 0x61010010, // NV_gpu_program4
		gp5vp = 0x61010020, // NV_gpu_program5
		// pixel programs
		// fp20 = 0x61020001, // very limited and outdated, unnecessary
		// fp30 = 0x61020002, // NV_fragment_program, now arbfp1, redundant
		arbfp1 = 0x61020004, // ARB_fragment_program
		fp40 = 0x61020008, // NV_fragment_program2, arbfp1 with "OPTION NV_fragment_program2;\n"
		gp4fp = 0x61020010, // NV_gpu_program4
		gp5fp = 0x61020020, // NV_gpu_program5
		// geometry programs
		gp4gp = 0x61030001, // NV_gpu_program4
		gp5gp = 0x61030001, // NV_gpu_program5

		// glsl - 0x65,type,version
		glsl330v = 0x65010330, // GLSL vertex program version 330
		glsl330f = 0x65020330, // GLSL fragment program version 330
		glsl330g = 0x65030330, // GLSL geometry program version 330
	};

	struct CSource : public NLMISC::CRefCount
	{
	public:
		std::string DisplayName;

		/// Minimal required profile for this GPU program
		IGPUProgram::TProfile Profile;

		const char *SourcePtr;
		size_t SourceLen;
		/// Copy the source code string
		inline void setSource(const std::string &source) { SourceCopy = source; SourcePtr = &SourceCopy[0]; SourceLen = SourceCopy.size(); }
		inline void setSource(const char *source) { SourceCopy = source; SourcePtr = &SourceCopy[0]; SourceLen = SourceCopy.size(); }
		/// Set pointer to source code string without copying the string
		inline void setSourcePtr(const char *sourcePtr, size_t sourceLen) { SourceCopy.clear(); SourcePtr = sourcePtr; SourceLen = sourceLen; }
		inline void setSourcePtr(const char *sourcePtr) { SourceCopy.clear(); SourcePtr = sourcePtr; SourceLen = strlen(sourcePtr); }

		/// CVertexProgramInfo/CPixelProgramInfo/... NeL features
		CGPUProgramFeatures Features;

		/// Map with known parameter indices, used for assembly programs
		std::map<std::string, uint> ParamIndices;
		
	private:
		std::string SourceCopy;
	};

public:
	IGPUProgram();
	virtual ~IGPUProgram();

	// Manage the sources, not allowed after compilation.
	// Add multiple sources using different profiles, the driver will use the first one it supports.
	inline size_t getSourceNb() const { return m_Sources.size(); };
	inline CSource *getSource(size_t i) const { return m_Sources[i]; };
	inline size_t addSource(CSource *source) { nlassert(!m_Source); m_Sources.push_back(source); return (m_Sources.size() - 1); }
	inline void removeSource(size_t i) { nlassert(!m_Source); m_Sources.erase(m_Sources.begin() + i); }

	// Get the idx of a parameter (ogl: uniform, d3d: constant, etcetera) by name. Invalid name returns ~0
	inline uint getUniformIndex(const char *name) const { return m_DrvInfo->getUniformIndex(name); };
	inline uint getUniformIndex(CGPUProgramIndex::TName name) const { return m_Index.Indices[name]; }

	// Get feature information of the current program
	inline CSource *source() const { return m_Source; };
	inline const CGPUProgramFeatures &features() const { return m_Source->Features; };
	inline TProfile profile() const { return m_Source->Profile; }

	// Build feature info, called automatically by the driver after compile succeeds
	void buildInfo(CSource *source);

	// Override this to build additional info in a subclass
	virtual void buildInfo();

protected:
	/// The progam source
	std::vector<NLMISC::CSmartPtr<CSource> >				m_Sources;

	/// The source used for compilation
	NLMISC::CSmartPtr<CSource>								m_Source;
	CGPUProgramIndex										m_Index;

public:
	/// The driver information. For the driver implementation only.
	NLMISC::CRefPtr<IGPUProgramDrvInfos>					m_DrvInfo;

}; /* class IGPUProgram */

} /* namespace NL3D */

#endif /* #ifndef NL3D_GPU_PROGRAM_H */

/* end of file */

/**
 * \file gpu_program.h
 * \brief CGPUProgram
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

/**
 * \brief CVertexProgramInfo
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * Read-only information structure.
 */
struct CVertexProgramInfo
{
public:
	std::string DisplayName;

	enum TFeatures
	{
		// World
		// transform

		// Lights
		Ambient = 0x0001, 
		Sun = 0x0002, 
		PointLight0 = 0x0004, 
		PointLight1 = 0x0008, 
		PointLight2 = 0x0010, 

		// Lights, additional parameters for user shaders
		/// Adds an enabled/disabled parameter to all of the lights
		DynamicLights = 0x0020, 
	};

	/// Bitfield containing features used by this vertex program.
	uint Features;

	/// Indices of parameters used by features.

	/// Lights, NeL supports:
	/// - Ambient light
	uint AmbientIdx; // (Ambient)
	/// - One directional light
	uint SunDirectionIdx; // (Sun)
	uint SunDiffuseIdx; // (Sun)
	/// - Zero to three point lights
	uint PointLight0PositionIdx; // (PointLight0)
	uint PointLight0DiffuseIdx; // (PointLight0)
	uint PointLight1PositionIdx; // (PointLight1)
	uint PointLight1DiffuseIdx; // (PointLight1)
	uint PointLight2PositionIdx; // (PointLight2)
	uint PointLight2DiffuseIdx; // (PointLight2)

	/// DynamicLights
	uint SunEnabledIdx; // (DynamicLights && Sun)
	uint PointLight0EnabledIdx; // (DynamicLights && PointLight0)
	uint PointLight1EnabledIdx; // (DynamicLights && PointLight1)
	uint PointLight2EnabledIdx; // (DynamicLights && PointLight2)
};

/**
 * \brief CPixelProgramInfo
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * Read-only information structure.
 */
struct CPixelProgramInfo
{
public:
	std::string DisplayName;
	
	enum TFeatures
	{
		/// Use texture stages from CMaterial as texture parameters
		MaterialTextures = 0x0001, 
		/// Set driver fog parameters on this program
		Fog = 0x0002, 
		/// Adds an enabled/disabled parameter to the fog, for user shaders
		DynamicFog = 0x0004, 
	};

	// Bitfield containing features used by this pixel program
	uint Features;

	// Indices of parameters used by features
	uint FogEnabledIdx; // (Fog && DynamicFog) nlFogEnabled, fog enabled
	uint FogStartEndIdx; // (Fog) nlFogStartEnd, start and end of fog
	uint FogColorIdx; // (Fog) nlFogColor, fog color
};

/**
 * \brief CGeometryProgramInfo
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * Read-only information structure.
 */
struct CGeometryProgramInfo
{
public:
	/*enum TFeatures
	{
		
	};*/
	/// Bitfield containing features used by this pixel program.
	// uint Features;
	/// Indices of parameters used by features.
};

/**
 * \brief CGPUProgram
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * A compiled GPU program
 */
class IGPUProgram : public NLMISC::CRefCount
{
public:
	enum TProfile
	{
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

public:
	IGPUProgram();
	virtual ~IGPUProgram();

	/// Get the idx of a parameter (ogl: uniform, d3d: constant, etcetera) by name. Invalid name returns ~0
	virtual uint getParamIdx(char *name) const = 0;

	void buildVPInfo(const char *displayName, uint features);
	void buildPPInfo(const char *displayName, uint features);
	void buildGPInfo(const char *displayName, uint features);

	inline const CVertexProgramInfo *getVPInfo() const { return Info.VertexProgram; }
	inline const CPixelProgramInfo *getPPInfo() const { return Info.PixelProgram; }
	inline const CGeometryProgramInfo *getGPInfo() const { return Info.GeometryProgram; }

private:
	union
	{
		CVertexProgramInfo *VertexProgram;
		CPixelProgramInfo *PixelProgram;
		CGeometryProgramInfo *GeometryProgram;
		void *Ptr;
	} Info;
	
}; /* class IGPUProgram */

} /* namespace NL3D */

#endif /* #ifndef NL3D_GPU_PROGRAM_H */

/* end of file */

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

#ifndef NL_VERTEX_PROGRAM_H
#define NL_VERTEX_PROGRAM_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/gpu_program.h"
#include "nel/3d/gpu_program_source.h"

#include <list>

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

class CVertexProgram : public IGPUProgram
{
public:
	/// Constructor
	CVertexProgram(CGPUProgramSourceCont *programSource);
	CVertexProgram(const char *nelvp);
	/// Destructor
	virtual ~CVertexProgram ();

	/// Build feature information
	void buildInfo(const char *displayName, uint features);
	/// Get feature information
	inline const CVertexProgramInfo *getInfo() const { return _Info; }

private:

	/// Feature information
	CVertexProgramInfo							*_Info;
};

} // NL3D


#endif // NL_VERTEX_PROGRAM_H

/* End of vertex_program.h */

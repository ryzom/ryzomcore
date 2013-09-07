/**
 * \file gpu_program.cpp
 * \brief CGPUProgram
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * CGPUProgram
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

#include <nel/misc/types_nl.h>
#include <nel/3d/gpu_program.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/string_mapper.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace NL3D {

IGPUProgram::IGPUProgram()
{
	Info.Ptr = NULL;
}

IGPUProgram::~IGPUProgram()
{
	delete Info.Ptr;
	Info.Ptr = NULL;
}

void IGPUProgram::buildVPInfo(const char *displayName, uint features)
{
	nlassert(Info.VertexProgram == NULL);
	Info.VertexProgram = new CVertexProgramInfo();
	CVertexProgramInfo *info = Info.VertexProgram;
	info->DisplayName = displayName;
	info->Features = features;
	if (features & CVertexProgramInfo::Ambient)
	{
		info->AmbientIdx = getParamIdx("nlAmbient");
		if (info->AmbientIdx == ~0)
		{
			nlwarning("Missing 'nlAmbient' in gpu program '%s', Ambient disabled", displayName);
			info->Features &= ~CVertexProgramInfo::Ambient;
		}
	}
	if (features & CVertexProgramInfo::Sun)
	{
		if (features & CVertexProgramInfo::DynamicLights)
		{
			info->SunEnabledIdx = getParamIdx("nlSunEnabled");
			if (info->SunEnabledIdx == ~0)
			{
				nlwarning("Missing 'nlSunEnabled' in gpu program '%s', DynamicLights disabled", displayName);
				info->Features &= ~CVertexProgramInfo::DynamicLights;
			}
		}
		info->SunDirectionIdx = getParamIdx("nlSunDirection");
		info->SunDiffuseIdx = getParamIdx("nlSunDiffuse");
		if (info->SunDirectionIdx == ~0
			|| info->SunDiffuseIdx == ~0)
		{
			nlwarning("Missing 'nlSunDirection/nlSunDiffuse' in gpu program '%s', Sun disabled", displayName);
			info->Features &= ~CVertexProgramInfo::Sun;
		}
	}
	if (features & CVertexProgramInfo::PointLight0)
	{
		if (features & CVertexProgramInfo::DynamicLights)
		{
			info->PointLight0EnabledIdx = getParamIdx("nlPointLight0Enabled");
			if (info->PointLight0EnabledIdx == ~0)
			{
				nlwarning("Missing 'nlPointLight0Enabled' in gpu program '%s', DynamicLights disabled", displayName);
				info->Features &= ~CVertexProgramInfo::DynamicLights;
			}
		}
		info->PointLight0PositionIdx = getParamIdx("nlPointLight0Position");
		info->PointLight0DiffuseIdx = getParamIdx("nlPointLight0Diffuse");
		if (info->PointLight0PositionIdx == ~0
			|| info->PointLight0DiffuseIdx == ~0)
		{
			nlwarning("Missing 'nlPointLight0Position/nlPointLight0Diffuse' in gpu program '%s', PointLight0 disabled", displayName);
			info->Features &= ~CVertexProgramInfo::PointLight0;
		}
	}
	if (features & CVertexProgramInfo::PointLight1)
	{
		if (features & CVertexProgramInfo::DynamicLights)
		{
			info->PointLight1EnabledIdx = getParamIdx("nlPointLight1Enabled");
			if (info->PointLight1EnabledIdx == ~0)
			{
				nlwarning("Missing 'nlPointLight1Enabled' in gpu program '%s', DynamicLights disabled", displayName);
				info->Features &= ~CVertexProgramInfo::DynamicLights;
			}
		}
		info->PointLight1PositionIdx = getParamIdx("nlPointLight1Position");
		info->PointLight1DiffuseIdx = getParamIdx("nlPointLight1Diffuse");
		if (info->PointLight1PositionIdx == ~0
			|| info->PointLight1DiffuseIdx == ~0)
		{
			nlwarning("Missing 'nlPointLight1Position/nlPointLight1Diffuse' in gpu program '%s', PointLight1 disabled", displayName);
			info->Features &= ~CVertexProgramInfo::PointLight1;
		}
	}
	if (features & CVertexProgramInfo::PointLight2)
	{
		if (features & CVertexProgramInfo::DynamicLights)
		{
			info->PointLight2EnabledIdx = getParamIdx("nlPointLight2Enabled");
			if (info->PointLight2EnabledIdx == ~0)
			{
				nlwarning("Missing 'nlPointLight2Enabled' in gpu program '%s', DynamicLights disabled", displayName);
				info->Features &= ~CVertexProgramInfo::DynamicLights;
			}
		}
		info->PointLight2PositionIdx = getParamIdx("nlPointLight2Position");
		info->PointLight2DiffuseIdx = getParamIdx("nlPointLight2Diffuse");
		if (info->PointLight2PositionIdx == ~0
			|| info->PointLight2DiffuseIdx == ~0)
		{
			nlwarning("Missing 'nlPointLight2Position/nlPointLight2Diffuse' in gpu program '%s', PointLight2 disabled", displayName);
			info->Features &= ~CVertexProgramInfo::PointLight2;
		}
	}
}

void IGPUProgram::buildPPInfo(const char *displayName, uint features)
{
	nlassert(Info.PixelProgram == NULL);
	Info.PixelProgram = new CPixelProgramInfo();
	CPixelProgramInfo *info = Info.PixelProgram;
	info->DisplayName = displayName;
	info->Features = features;
	if (features & CPixelProgramInfo::Fog)
	{
		if (features & CPixelProgramInfo::DynamicFog)
		{
			info->FogEnabledIdx = getParamIdx("nlFogEnabled");
			if (info->FogEnabledIdx == ~0)
			{
				nlwarning("Missing 'nlFogEnabled' in gpu program '%s', DynamicFog disabled", displayName);
				info->Features &= ~CPixelProgramInfo::DynamicFog;
			}
		}
		info->FogStartEndIdx = getParamIdx("nlFogStartEnd");
		info->FogColorIdx = getParamIdx("nlFogColor");
		if (info->FogStartEndIdx == ~0
			|| info->FogStartEndIdx == ~0)
		{
			nlwarning("Missing 'nlFogStartEnd/nlFogColor' in gpu program '%s', Fog disabled", displayName);
			info->Features &= ~CPixelProgramInfo::Fog;
		}
	}
}

void IGPUProgram::buildGPInfo(const char *displayName, uint features)
{
	nlassert(Info.GeometryProgram == NULL);
	Info.GeometryProgram = new CGeometryProgramInfo();
}


} /* namespace NL3D */

/* end of file */

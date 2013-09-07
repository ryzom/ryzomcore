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

#include "std3d.h"

#include "nel/3d/vertex_program.h"

#include "nel/3d/driver.h"


namespace NL3D
{

// ***************************************************************************

CVertexProgram::CVertexProgram(CGPUProgramSourceCont *programSource) : _Info(NULL), IGPUProgram(programSource)
{
	
}

// ***************************************************************************

CVertexProgram::CVertexProgram(const char *nelvp) : _Info(NULL)
{
	CGPUProgramSource *source = new CGPUProgramSource();
	_ProgramSource = new CGPUProgramSourceCont();
	_ProgramSource->Sources.push_back(source);
	source->Profile = IGPUProgram::nelvp;
	source->setSource(nelvp);
	source->Features = 0;
}

// ***************************************************************************

CVertexProgram::~CVertexProgram ()
{
	delete _Info;
	_Info = NULL;
}

// ***************************************************************************

void CVertexProgram::buildInfo(const char *displayName, uint features)
{
	nlassert(_Info == NULL);
	_Info = new CVertexProgramInfo();
	CVertexProgramInfo *info = _Info;
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

} // NL3D

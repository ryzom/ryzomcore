/** \file driver_direct3d_pixel_program.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_pixel_program.cpp,v 1.1.2.4 2007/07/09 15:26:35 legallo Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2000-2007  Nevrax Ltd.
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stddirect3d.h"

#include "driver_direct3d.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

CPixelProgramDrvInfosD3D::CPixelProgramDrvInfosD3D(IDriver *drv, ItGPUPrgDrvInfoPtrList it) : IProgramDrvInfos (drv, it)
{
	H_AUTO_D3D(CPixelProgramDrvInfosD3D_CPixelProgamDrvInfosD3D)
	Shader = NULL;
}

// ***************************************************************************

CPixelProgramDrvInfosD3D::~CPixelProgramDrvInfosD3D()
{
	H_AUTO_D3D(CPixelProgramDrvInfosD3D_CPixelProgramDrvInfosD3DDtor)
	if (Shader)
		Shader->Release();
}

// ***************************************************************************

bool CDriverD3D::supportPixelProgram (CPixelProgram::TProfile profile) const
{
	H_AUTO_D3D(CDriverD3D_supportPixelProgram_profile)
	return ((profile & 0xFFFF0000) == 0xD9020000)
		&& (_PixelProgramVersion >= (uint16)(profile & 0x0000FFFF));
}

// ***************************************************************************

bool CDriverD3D::compilePixelProgram(CPixelProgram *program)
{
	// Program setuped ?
	if (program->m_DrvInfo==NULL)
	{
		// Find a supported pixel program profile
		IProgram::CSource *source = NULL;
		for (uint i = 0; i < program->getSourceNb(); ++i)
		{
			if (supportPixelProgram(program->getSource(i)->Profile))
			{
				source = program->getSource(i);
			}
		}
		if (!source)
		{
			nlwarning("No supported source profile for pixel program");
			return false;
		}

		_GPUPrgDrvInfos.push_front (NULL);
		ItGPUPrgDrvInfoPtrList itPix = _GPUPrgDrvInfos.begin();
		CPixelProgramDrvInfosD3D *drvInfo;
		*itPix = drvInfo = new CPixelProgramDrvInfosD3D(this, itPix);

		// Create a driver info structure
		program->m_DrvInfo = *itPix;

		LPD3DXBUFFER pShader;
		LPD3DXBUFFER pErrorMsgs;
		if (D3DXAssembleShader(source->SourcePtr, source->SourceLen, NULL, NULL, 0, &pShader, &pErrorMsgs) == D3D_OK)
		{
			if (_DeviceInterface->CreatePixelShader((DWORD*)pShader->GetBufferPointer(), &(getPixelProgramD3D(*program)->Shader)) != D3D_OK)
				return false;
		}
		else
		{
			nlwarning ("Can't assemble pixel program:");
			nlwarning ((const char*)pErrorMsgs->GetBufferPointer());
			return false;
		}

		// Set parameters for assembly programs
		drvInfo->ParamIndices = source->ParamIndices;

		// Build the feature info
		program->buildInfo(source);
	}

	return true;
}

// ***************************************************************************

bool CDriverD3D::activePixelProgram(CPixelProgram *program)
{
	H_AUTO_D3D(CDriverD3D_activePixelProgram )
	if (_DisableHardwarePixelProgram)
		return false;

	// Set the pixel program
	if (program)
	{
		if (!CDriverD3D::compilePixelProgram(program)) return false;

		CPixelProgramDrvInfosD3D *info = static_cast<CPixelProgramDrvInfosD3D *>((IProgramDrvInfos*)program->m_DrvInfo);
		_PixelProgramUser = program;
		setPixelShader(info->Shader);
	}
	else
	{
		setPixelShader(NULL);
		_PixelProgramUser = NULL;
	}

	return true;
}

// ***************************************************************************

void CDriverD3D::disableHardwarePixelProgram()
{
	H_AUTO_D3D(CDriverD3D_disableHardwarePixelProgram)
	_DisableHardwarePixelProgram = true;
	_PixelProgram = false;
}

} // NL3D

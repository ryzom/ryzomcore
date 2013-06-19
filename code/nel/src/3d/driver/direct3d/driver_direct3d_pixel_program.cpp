/** \file driver_direct3d_pixel_program.cpp
 * Direct 3d driver implementation
 *
 * $Id: driver_direct3d_pixel_program.cpp,v 1.1.2.4 2007/07/09 15:26:35 legallo Exp $
 *
 * \todo manage better the init/release system (if a throw occurs in the init, we must release correctly the driver)
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stddirect3d.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D 
{

// ***************************************************************************

CPixelProgramDrvInfosD3D::CPixelProgramDrvInfosD3D(IDriver *drv, ItPixelPrgDrvInfoPtrList it) : IPixelProgramDrvInfos (drv, it)
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

bool CDriverD3D::isPixelProgramSupported () const
{
	H_AUTO_D3D(CDriverD3D_isPixelProgramSupported)
	return _PixelProgram;
}

bool CDriverD3D::isPixelProgramSupported (TPixelProgramProfile profile) const
{
	H_AUTO_D3D(CDriverD3D_isPixelProgramSupported_profile)
	return ((profile & 0xFFFF0000) == 0xD3D00000)
		&& (_PixelProgramVersion >= (uint16)(profile & 0x0000FFFF));
}

// ***************************************************************************

bool CDriverD3D::activePixelProgram(CPixelProgram *program)
{
	H_AUTO_D3D(CDriverD3D_activePixelProgram )
	if (_DisableHardwarePixelProgram)
		return false;

	// Setup or unsetup ?
	if (program)
	{
		// Program setuped ?
		if (program->_DrvInfo==NULL)
		{
			_PixelPrgDrvInfos.push_front (NULL);
			ItPixelPrgDrvInfoPtrList itPix = _PixelPrgDrvInfos.begin();
			*itPix = new CPixelProgramDrvInfosD3D(this, itPix);

			// Create a driver info structure
			program->_DrvInfo = *itPix;

			const std::string &dest = program->getProgram();

			LPD3DXBUFFER pShader;
			LPD3DXBUFFER pErrorMsgs;
			if (D3DXAssembleShader (dest.c_str(), dest.size(), NULL, NULL, 0, &pShader, &pErrorMsgs) == D3D_OK)
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
		}
	}

	// Set the pixel program
	if (program)
	{
		CPixelProgramDrvInfosD3D *info = static_cast<CPixelProgramDrvInfosD3D *>((IPixelProgramDrvInfos*)program->_DrvInfo);
		setPixelShader (info->Shader);

		float z = 0;
		float o = 1;
		setRenderState (D3DRS_FOGSTART, *((DWORD*) (&o)));
		setRenderState (D3DRS_FOGEND, *((DWORD*) (&z)));
	}
	else
	{
		setPixelShader (NULL);

		// Set the old fog range
		setRenderState (D3DRS_FOGSTART, *((DWORD*) (&_FogStart)));
		setRenderState (D3DRS_FOGEND, *((DWORD*) (&_FogEnd)));
	}

	return true;
}

// ***************************************************************************

void CDriverD3D::setPixelProgramConstant (uint index, float f0, float f1, float f2, float f3)
{
	H_AUTO_D3D(CDriverD3D_setPixelProgramConstant)
	if (!_PixelProgram)
	{
		#ifdef NL_DEBUG
			nlwarning("No pixel programs available!!");
		#endif
		return;
	}
	const float tabl[4] = {f0, f1, f2, f3};
	setPixelShaderConstant (index, tabl);
}

// ***************************************************************************

void CDriverD3D::setPixelProgramConstant (uint index, double d0, double d1, double d2, double d3)
{
	H_AUTO_D3D(CDriverD3D_setPixelProgramConstant )
	if (!_PixelProgram)
	{
		#ifdef NL_DEBUG
			nlwarning("No pixel programs available!!");
		#endif
		return;
	}
	const float tabl[4] = {(float)d0, (float)d1, (float)d2, (float)d3};
	setPixelShaderConstant (index, tabl);
}

// ***************************************************************************

void CDriverD3D::setPixelProgramConstant (uint index, const NLMISC::CVector& value)
{
	H_AUTO_D3D(CDriverD3D_setPixelProgramConstant )
	if (!_PixelProgram)
	{
		#ifdef NL_DEBUG
			nlwarning("No pixel programs available!!");
		#endif
		return;
	}
	const float tabl[4] = {value.x, value.y, value.z, 0};
	setPixelShaderConstant (index, tabl);
}

// ***************************************************************************

void CDriverD3D::setPixelProgramConstant (uint index, const NLMISC::CVectorD& value)
{
	H_AUTO_D3D(CDriverD3D_setPixelProgramConstant )
	if (!_PixelProgram)
	{
		#ifdef NL_DEBUG
			nlwarning("No pixel programs available!!");
		#endif
		return;
	}
	const float tabl[4] = {(float)value.x, (float)value.y, (float)value.z, 0};
	setPixelShaderConstant (index, tabl);
}

// ***************************************************************************

void CDriverD3D::setPixelProgramConstant (uint index, uint num, const float *src)
{
	H_AUTO_D3D(CDriverD3D_setPixelProgramConstant )
	if (!_PixelProgram)
	{
		#ifdef NL_DEBUG
			nlwarning("No pixel programs available!!");
		#endif
		return;
	}
	uint i;
	for (i=0; i<num; i++)
		setPixelShaderConstant (index+i, src+i*4);
}

// ***************************************************************************

void CDriverD3D::setPixelProgramConstant (uint index, uint num, const double *src)
{
	H_AUTO_D3D(CDriverD3D_setPixelProgramConstant )
	if (!_PixelProgram)
	{
		#ifdef NL_DEBUG
			nlwarning("No pixel programs available!!");
		#endif
		return;
	}
	uint i;
	for (i=0; i<num; i++)
	{
		const float tabl[4] = {(float)src[0], (float)src[1], (float)src[2], (float)src[3]};
		setPixelShaderConstant (index+i, tabl);
		src += 4;
	}
}

// ***************************************************************************

void CDriverD3D::setPixelProgramConstantMatrix (uint index, IDriver::TMatrix matrix, IDriver::TTransform transform)
{
	H_AUTO_D3D(CDriverD3D_setPixelProgramConstantMatrix)
	if (!_PixelProgram)
	{
		#ifdef NL_DEBUG
			nlwarning("No pixel programs available!!");
		#endif
		return;
	}
	D3DXMATRIX mat;
	D3DXMATRIX *matPtr;
	switch (matrix)
	{
		case IDriver::ModelView:
			matPtr = &_D3DModelView;
		break;
		case IDriver::Projection:
			matPtr = &(_MatrixCache[remapMatrixIndex (D3DTS_PROJECTION)].Matrix);
		break;
		case IDriver::ModelViewProjection:
			matPtr = &_D3DModelViewProjection;
		break;
	}

	if (transform != IDriver::Identity)
	{
		mat = *matPtr;
		matPtr = &mat;
		switch(transform)
		{
			case IDriver::Inverse:
				D3DXMatrixInverse (&mat, NULL, &mat);
			break;		
			case IDriver::Transpose:
				D3DXMatrixTranspose (&mat, &mat);
			break;
			case IDriver::InverseTranspose:
				D3DXMatrixInverse (&mat, NULL, &mat);
				D3DXMatrixTranspose (&mat, &mat);
			break;
		}
	}

	setPixelProgramConstant (index, matPtr->_11, matPtr->_21, matPtr->_31, matPtr->_41);
	setPixelProgramConstant (index+1, matPtr->_12, matPtr->_22, matPtr->_32, matPtr->_42);
	setPixelProgramConstant (index+2, matPtr->_13, matPtr->_23, matPtr->_33, matPtr->_43);
	setPixelProgramConstant (index+3, matPtr->_14, matPtr->_24, matPtr->_34, matPtr->_44);
}

// ***************************************************************************

void CDriverD3D::disableHardwarePixelProgram()
{
	H_AUTO_D3D(CDriverD3D_disableHardwarePixelProgram)
	_DisableHardwarePixelProgram = true;
	_PixelProgram = false;
}

} // NL3D

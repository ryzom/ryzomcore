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

#include "stddirect3d.h"

#include "driver_direct3d.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

void CDriverD3D::setUniform4f(TProgram program, uint index, float f0, float f1, float f2, float f3)
{
	H_AUTO_D3D(CDriverD3D_setUniform4f);

	const float tabl[4] = { f0, f1, f2, f3 };
	switch (program)
	{
	case VertexProgram:
		if (_VertexProgram)
		{
			setVertexProgramConstant(index, tabl);
		}
		break;
	case PixelProgram:
		if (_PixelProgram)
		{
			setPixelShaderConstant(index, tabl);
		}
		break;
	}
}

void CDriverD3D::setUniform4fv(TProgram program, uint index, size_t num, const float *src)
{
	H_AUTO_D3D(CDriverD3D_setUniform4fv);

	switch (program)
	{
	case VertexProgram:
		if (_VertexProgram)
		{
			for (uint i = 0; i < num; ++i)
			{
				setVertexProgramConstant(index + i, src + (i * 4));
			}
		}
		break;
	case PixelProgram:
		if (_PixelProgram)
		{
			for (uint i = 0; i < num; ++i)
			{
				setPixelShaderConstant(index + i, src + (i * 4));
			}
		}
		break;
	}
}

void CDriverD3D::setUniform1f(TProgram program, uint index, float f0)
{
	CDriverD3D::setUniform4f(program, index, f0, 0.f, 0.f, 0.f);
}

void CDriverD3D::setUniform2f(TProgram program, uint index, float f0, float f1)
{
	CDriverD3D::setUniform4f(program, index, f0, f1, 0.f, 0.f);
}

void CDriverD3D::setUniform3f(TProgram program, uint index, float f0, float f1, float f2)
{
	CDriverD3D::setUniform4f(program, index, f0, f1, f2, 0.0f);
}

void CDriverD3D::setUniform1i(TProgram program, uint index, sint32 i0)
{

}

void CDriverD3D::setUniform2i(TProgram program, uint index, sint32 i0, sint32 i1)
{

}

void CDriverD3D::setUniform3i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2)
{

}

void CDriverD3D::setUniform4i(TProgram program, uint index, sint32 i0, sint32 i1, sint32 i2, sint32 i3)
{

}

void CDriverD3D::setUniform1ui(TProgram program, uint index, uint32 ui0)
{

}

void CDriverD3D::setUniform2ui(TProgram program, uint index, uint32 ui0, uint32 ui1)
{

}

void CDriverD3D::setUniform3ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2)
{

}

void CDriverD3D::setUniform4ui(TProgram program, uint index, uint32 ui0, uint32 ui1, uint32 ui2, uint32 ui3)
{

}

void CDriverD3D::setUniform3f(TProgram program, uint index, const NLMISC::CVector& v)
{
	CDriverD3D::setUniform4f(program, index, v.x, v.y, v.z, 0.f);
}

void CDriverD3D::setUniform4f(TProgram program, uint index, const NLMISC::CVector& v, float f3)
{
	CDriverD3D::setUniform4f(program, index, v.x, v.y, v.z, f3);
}

void CDriverD3D::setUniform4x4f(TProgram program, uint index, const NLMISC::CMatrix& m)
{
	H_AUTO_D3D(CDriverD3D_setUniform4x4f);

	// TODO: Verify this!
	NLMISC::CMatrix mat = m;
	mat.transpose();
	const float *md = mat.get();

	CDriverD3D::setUniform4fv(program, index, 4, md);
}

void CDriverD3D::setUniform4iv(TProgram program, uint index, size_t num, const sint32 *src)
{
	
}

void CDriverD3D::setUniform4uiv(TProgram program, uint index, size_t num, const uint32 *src)
{
	
}

void CDriverD3D::setUniformMatrix(NL3D::IDriver::TProgram program, uint index, NL3D::IDriver::TMatrix matrix, NL3D::IDriver::TTransform transform)
{
	H_AUTO_D3D(CDriverD3D_setUniformMatrix);
	
	D3DXMATRIX mat;
	D3DXMATRIX *matPtr = NULL;
	switch (matrix)
	{
		case IDriver::ModelView:
			matPtr = &_D3DModelView;
		break;
		case IDriver::Projection:
			matPtr = &(_MatrixCache[remapMatrixIndex(D3DTS_PROJECTION)].Matrix);
		break;
		case IDriver::ModelViewProjection:
			matPtr = &_D3DModelViewProjection;
		break;
	}
	if (transform != IDriver::Identity)
	{
		switch (transform)
		{
			case IDriver::Inverse:
				D3DXMatrixInverse(&mat, NULL, matPtr);
			break;
			case IDriver::Transpose:
				D3DXMatrixTranspose(&mat, matPtr);
			break;
			case IDriver::InverseTranspose:
				D3DXMatrixInverse(&mat, NULL, matPtr);
				D3DXMatrixTranspose(&mat, &mat);
			break;
		}
		matPtr = &mat;
	}

	D3DXMatrixTranspose(&mat, matPtr);
	
	CDriverD3D::setUniform4fv(program, index, 4, &mat.m[0][0]);
}

void CDriverD3D::setUniformFog(NL3D::IDriver::TProgram program, uint index)
{
	H_AUTO_D3D(CDriverD3D_setUniformFog)

	/* "oFog" must always be between [1, 0] what ever you set in D3DRS_FOGSTART and D3DRS_FOGEND (1 for no fog, 0 for full fog).
	The Geforce4 TI 4200 (drivers 53.03 and 45.23) doesn't accept other values for "oFog". */
	const float delta = _FogEnd - _FogStart;
	CDriverD3D::setUniform4f(program, index, 
		-_D3DModelView._13 / delta, 
		-_D3DModelView._23 / delta, 
		-_D3DModelView._33 / delta, 
		1 - (_D3DModelView._43 - _FogStart) / delta);
}

} // NL3D

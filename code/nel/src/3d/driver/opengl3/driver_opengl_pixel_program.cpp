// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  by authors
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

#include "stdopengl.h"

#include <sstream>

#include "nel/3d/vertex_program.h"
#include "nel/3d/light.h"

#include "driver_opengl.h"
#include "driver_opengl_program.h"
#include "driver_opengl_vertex_buffer_hard.h"

namespace NL3D {

#ifdef NL_STATIC
namespace NLDRIVERGL3 {
#endif

bool operator<(const CPPBuiltin &left, const CPPBuiltin &right)
{	
	// Material state

	// Driver state
	if (left.VertexFormat != right.VertexFormat)
		return left.VertexFormat < right.VertexFormat;
	if (left.Fog != right.Fog)
		return right.Fog;

	return false;
}

namespace /* anonymous */ {
	
void ppGenerate(std::string &result, CPPBuiltin &desc)
{

}

} /* anonymous namespace */

void CDriverGL3::generateBuiltinPixelProgram(CMaterial &mat)
{
	CMaterialDrvInfosGL3 *matDrv = static_cast<CMaterialDrvInfosGL3 *>((IMaterialDrvInfos *)(mat._MatDrvInfo));
	nlassert(matDrv);

	std::set<CPPBuiltin>::iterator it = m_PPBuiltinCache.find(matDrv->PPBuiltin);
	if (it != m_PPBuiltinCache.end())
	{
		matDrv->PPBuiltin.PixelProgram = it->PixelProgram;
		return;
	}

	std::string result;
	ppGenerate(result, matDrv->PPBuiltin);

	CPixelProgram *program = new CPixelProgram();
	IProgram::CSource *src = new IProgram::CSource();
	src->Profile = IProgram::glsl330f;
	src->DisplayName = "Builtin Pixel Program (" + NLMISC::toString(m_PPBuiltinCache.size()) + ")";
	src->setSource(result);
	program->addSource(src);

	nldebug("GL3: Generate '%s'", src->DisplayName.c_str());

	if (!compilePixelProgram(program))
	{
		delete program;
		program = NULL;
	}

	matDrv->PPBuiltin.PixelProgram = program;
	m_PPBuiltinCache.insert(matDrv->PPBuiltin);
}

void CPPBuiltin::checkDriverStateTouched(CDriverGL3 *driver)
{
	if (VertexFormat != driver->m_VPBuiltinCurrent.VertexFormat)
	{
		VertexFormat = driver->m_VPBuiltinCurrent.VertexFormat;
		Touched = true;
	}
	if (Fog != driver->m_VPBuiltinCurrent.Fog)
	{
		Fog = driver->m_VPBuiltinCurrent.Fog;
		Touched = true;
	}
}

void CPPBuiltin::checkMaterialStateTouched(CMaterial &mat)
{
	
}

#ifdef NL_STATIC
} // NLDRIVERGL3
#endif

} // NL3D


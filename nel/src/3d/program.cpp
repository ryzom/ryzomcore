/**
 * \file program.cpp
 * \brief IProgram
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * IProgram
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2013-2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/types_nl.h"
#include "nel/3d/program.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include "nel/misc/string_mapper.h"

// Project includes
#include "nel/3d/driver.h"

using namespace std;
// using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D {

// ***************************************************************************

IProgramDrvInfos::IProgramDrvInfos(IDriver *drv, ItGPUPrgDrvInfoPtrList it)
{
	_Driver = drv;
	_DriverIterator = it;
}

// ***************************************************************************

IProgramDrvInfos::~IProgramDrvInfos ()
{
	_Driver->removeGPUPrgDrvInfoPtr(_DriverIterator);
}

// ***************************************************************************

IProgram::IProgram()
	: m_CompileFailed(false)
{

}

// ***************************************************************************

IProgram::~IProgram()
{
	// Must kill the drv mirror of this program.
	m_DrvInfo.kill();
}

const char *CProgramIndex::Names[NUM_UNIFORMS] = 
{
	"modelView", 
	"modelViewInverse", 
	"modelViewTranspose", 
	"modelViewInverseTranspose",

	"projection", 
	"projectionInverse", 
	"projectionTranspose", 
	"projectionInverseTranspose", 

	"modelViewProjection", 
	"modelViewProjectionInverse", 
	"modelViewProjectionTranspose", 
	"modelViewProjectionInverseTranspose",

	"normalMatrix",
	"viewMatrix",

	"fog",
	"fogParams",
	"fogColor",
	"fogDensity",

	"materialColor",
	//"diffuseColor",

	"alphaRef",

	"constant0",
	"constant1",
	"constant2",
	"constant3",
	"constant4",
	"constant5",
	"constant6",
	"constant7",
	"constant8",
	"constant9",
	"constant10",
	"constant11",
	"constant12",
	"constant13",
	"constant14",
	"constant15",
	"constant16",
	"constant17",
	"constant18",
	"constant19",
	"constant20",
	"constant21",
	"constant22",
	"constant23",
	"constant24",
	"constant25",
	"constant26",
	"constant27",
	"constant28",
	"constant29",
	"constant30",
	"constant31",

	"sampler0",
	"sampler1",
	"sampler2",
	"sampler3",
	"sampler4",
	"sampler5",
	"sampler6",
	"sampler7",
	"sampler8",
	"sampler9",
	"sampler10",
	"sampler11",
	"sampler12",
	"sampler13",
	"sampler14",
	"sampler15",
	"sampler16",
	"sampler17",
	"sampler18",
	"sampler19",
	"sampler20",
	"sampler21",
	"sampler22",
	"sampler23",
	"sampler24",
	"sampler25",
	"sampler26",
	"sampler27",
	"sampler28",
	"sampler29",
	"sampler30",
	"sampler31",

	"texMatrix0",
	"texMatrix1",
	"texMatrix2",
	"texMatrix3",

	"texGen0ObjectPlaneS",
	"texGen0ObjectPlaneT",
	"texGen0ObjectPlaneP",
	"texGen0ObjectPlaneQ",

	"texGen1ObjectPlaneS",
	"texGen1ObjectPlaneT",
	"texGen1ObjectPlaneP",
	"texGen1ObjectPlaneQ",

	"texGen2ObjectPlaneS",
	"texGen2ObjectPlaneT",
	"texGen2ObjectPlaneP",
	"texGen2ObjectPlaneQ",

	"texGen3ObjectPlaneS",
	"texGen3ObjectPlaneT",
	"texGen3ObjectPlaneP",
	"texGen3ObjectPlaneQ",

	"texGen0EyePlaneS",
	"texGen0EyePlaneT",
	"texGen0EyePlaneP",
	"texGen0EyePlaneQ",

	"texGen1EyePlaneS",
	"texGen1EyePlaneT",
	"texGen1EyePlaneP",
	"texGen1EyePlaneQ",

	"texGen2EyePlaneS",
	"texGen2EyePlaneT",
	"texGen2EyePlaneP",
	"texGen2EyePlaneQ",

	"texGen3EyePlaneS",
	"texGen3EyePlaneT",
	"texGen3EyePlaneP",
	"texGen3EyePlaneQ",
	
	"selfIllumination",

	"light0DirOrPos",
	"light1DirOrPos",
	"light2DirOrPos",
	"light3DirOrPos",
	"light4DirOrPos",
	"light5DirOrPos",
	"light6DirOrPos",
	"light7DirOrPos",

	"light0ColAmb",
	"light1ColAmb",
	"light2ColAmb",
	"light3ColAmb",
	"light4ColAmb",
	"light5ColAmb",
	"light6ColAmb",
	"light7ColAmb",

	"light0ColDiff",
	"light1ColDiff",
	"light2ColDiff",
	"light3ColDiff",
	"light4ColDiff",
	"light5ColDiff",
	"light6ColDiff",
	"light7ColDiff",

	"light0ColSpec",
	"light1ColSpec",
	"light2ColSpec",
	"light3ColSpec",
	"light4ColSpec",
	"light5ColSpec",
	"light6ColSpec",
	"light7ColSpec",

	"light0Shininess",
	"light1Shininess",
	"light2Shininess",
	"light3Shininess",
	"light4Shininess",
	"light5Shininess",
	"light6Shininess",
	"light7Shininess",

	"light0ConstAttn",
	"light1ConstAttn",
	"light2ConstAttn",
	"light3ConstAttn",
	"light4ConstAttn",
	"light5ConstAttn",
	"light6ConstAttn",
	"light7ConstAttn",

	"light0LinAttn",
	"light1LinAttn",
	"light2LinAttn",
	"light3LinAttn",
	"light4LinAttn",
	"light5LinAttn",
	"light6LinAttn",
	"light7LinAttn",

	"light0QuadAttn",
	"light1QuadAttn",
	"light2QuadAttn",
	"light3QuadAttn",
	"light4QuadAttn",
	"light5QuadAttn",
	"light6QuadAttn",
	"light7QuadAttn",

	"light0SpotDir",
	"light1SpotDir",
	"light2SpotDir",
	"light3SpotDir",
	"light4SpotDir",
	"light5SpotDir",
	"light6SpotDir",
	"light7SpotDir",

	"light0SpotCutoff",
	"light1SpotCutoff",
	"light2SpotCutoff",
	"light3SpotCutoff",
	"light4SpotCutoff",
	"light5SpotCutoff",
	"light6SpotCutoff",
	"light7SpotCutoff",

	"light0SpotExp",
	"light1SpotExp",
	"light2SpotExp",
	"light3SpotExp",
	"light4SpotExp",
	"light5SpotExp",
	"light6SpotExp",
	"light7SpotExp",

	"clipPlane0",
	"clipPlane1",
	"clipPlane2",
	"clipPlane3",
	"clipPlane4",
	"clipPlane5",

	"embmMatrix0",
	"embmMatrix1",
	"embmMatrix2",
	"embmMatrix3",

	"bump0ScaleBias",
	"bump1ScaleBias",

	// Megashader control uniforms
	"nlLighting",
	"nlLightMode0",
	"nlLightMode1",
	"nlLightMode2",
	"nlLightMode3",
	"nlLightMode4",
	"nlLightMode5",
	"nlLightMode6",
	"nlLightMode7",
	"nlTexGenMode0",
	"nlTexGenMode1",
	"nlTexGenMode2",
	"nlTexGenMode3",
	"nlVertexColorLighted",
	"nlVertexFormat",
	"nlClipPlaneMask",
	"nlShader",
	"nlTextureActive",
	"nlTexEnvMode0",
	"nlTexEnvMode1",
	"nlTexEnvMode2",
	"nlTexEnvMode3",
	"nlAlphaTest",
	"nlFogMode",
	"nlWorldSpaceNormal",
	"nlWorldSpacePosition",
	"nlNumPerPixelLights",
	"nlFogEnabled",
	"nlUVRouting",
	"cameraForward",
	"samplerCube0",
	"samplerCube1",
	"samplerCube2",
	"samplerCube3",

	// Light table per-object uniforms
	"nlLightIndex0",
	"nlLightIndex1",
	"nlLightIndex2",
	"nlLightIndex3",
	"nlLightIndex4",
	"nlLightIndex5",
	"nlLightIndex6",
	"nlLightIndex7",
	"nlLightFactor0",
	"nlLightFactor1",
	"nlLightFactor2",
	"nlLightFactor3",
	"nlLightFactor4",
	"nlLightFactor5",
	"nlLightFactor6",
	"nlLightFactor7",
	"nlMaterialDiffuse",
	"nlMaterialSpecular",
	"nlMaterialShininess",
	"pzbCameraPos",
	"cameraWorldPos",
	"nlLightMapScale",
	"specularTexMtx",

	// Per-pixel lighting uniforms for pixel programs (raw values, not pre-multiplied)
	"nlPpLightMode0", "nlPpLightMode1", "nlPpLightMode2", "nlPpLightMode3",
	"nlPpLightMode4", "nlPpLightMode5", "nlPpLightMode6", "nlPpLightMode7",
	"ppLight0DirOrPos", "ppLight1DirOrPos", "ppLight2DirOrPos", "ppLight3DirOrPos",
	"ppLight4DirOrPos", "ppLight5DirOrPos", "ppLight6DirOrPos", "ppLight7DirOrPos",
	"ppLight0ColDiff", "ppLight1ColDiff", "ppLight2ColDiff", "ppLight3ColDiff",
	"ppLight4ColDiff", "ppLight5ColDiff", "ppLight6ColDiff", "ppLight7ColDiff",
	"ppLight0ColSpec", "ppLight1ColSpec", "ppLight2ColSpec", "ppLight3ColSpec",
	"ppLight4ColSpec", "ppLight5ColSpec", "ppLight6ColSpec", "ppLight7ColSpec",
	"ppLight0ConstAttn", "ppLight1ConstAttn", "ppLight2ConstAttn", "ppLight3ConstAttn",
	"ppLight4ConstAttn", "ppLight5ConstAttn", "ppLight6ConstAttn", "ppLight7ConstAttn",
	"ppLight0LinAttn", "ppLight1LinAttn", "ppLight2LinAttn", "ppLight3LinAttn",
	"ppLight4LinAttn", "ppLight5LinAttn", "ppLight6LinAttn", "ppLight7LinAttn",
	"ppLight0QuadAttn", "ppLight1QuadAttn", "ppLight2QuadAttn", "ppLight3QuadAttn",
	"ppLight4QuadAttn", "ppLight5QuadAttn", "ppLight6QuadAttn", "ppLight7QuadAttn",
	"ppLight0SpotDir", "ppLight1SpotDir", "ppLight2SpotDir", "ppLight3SpotDir",
	"ppLight4SpotDir", "ppLight5SpotDir", "ppLight6SpotDir", "ppLight7SpotDir",
	"ppLight0SpotCutoff", "ppLight1SpotCutoff", "ppLight2SpotCutoff", "ppLight3SpotCutoff",
	"ppLight4SpotCutoff", "ppLight5SpotCutoff", "ppLight6SpotCutoff", "ppLight7SpotCutoff",
	"ppLight0SpotExp", "ppLight1SpotExp", "ppLight2SpotExp", "ppLight3SpotExp",
	"ppLight4SpotExp", "ppLight5SpotExp", "ppLight6SpotExp", "ppLight7SpotExp"
};

void IProgram::buildInfo(CSource *source)
{
	// nlassert(!m_Source); // VALID: When deleting driver and creating new one.

	m_Source = source;

	// Fill index cache
	for (int i = 0; i < CProgramIndex::NUM_UNIFORMS; ++i)
	{
		m_Index.Indices[i] = getUniformIndex(m_Index.Names[i]);
	}

	buildInfo();
}

void IProgram::buildInfo()
{

}

// ***************************************************************************

CShaderProgram::CShaderProgram()
{
}

CShaderProgram::~CShaderProgram()
{
}

} /* namespace NL3D */

/* end of file */

/**
 * \file program.cpp
 * \brief IProgram
 * \date 2013-09-07 15:00GMT
 * \author Jan Boon (Kaetemi)
 * IProgram
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
#include <nel/3d/program.h>

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/string_mapper.h>

// Project includes
#include <nel/3d/driver.h>

using namespace std;
// using namespace NLMISC;

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
	"fogStart",
	"fogEnd",
	"fogColor",

	"materialColor",
	"diffuseColor",

	"constant0",
	"constant1",
	"constant2",
	"constant3",

	"sampler0",
	"sampler1",
	"sampler2",
	"sampler3",

	"texMatrix0",
	"texMatrix1",
	"texMatrix2",
	"texMatrix3",
	
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
	"light7QuadAttn"
};

void IProgram::buildInfo(CSource *source)
{
	nlassert(!m_Source);

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

} /* namespace NL3D */

/* end of file */

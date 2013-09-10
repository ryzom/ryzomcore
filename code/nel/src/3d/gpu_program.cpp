/**
 * \file gpu_program.cpp
 * \brief IGPUProgram
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

#include <nel/misc/types_nl.h>
#include <nel/3d/gpu_program.h>

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

IGPUProgramDrvInfos::IGPUProgramDrvInfos(IDriver *drv, ItGPUPrgDrvInfoPtrList it)
{
	_Driver = drv;
	_DriverIterator = it;
}

// ***************************************************************************

IGPUProgramDrvInfos::~IGPUProgramDrvInfos ()
{
	_Driver->removeGPUPrgDrvInfoPtr(_DriverIterator);
}

// ***************************************************************************

IGPUProgram::IGPUProgram()
{
	
}

// ***************************************************************************

IGPUProgram::~IGPUProgram()
{
	// Must kill the drv mirror of this program.
	m_DrvInfo.kill();
}

const char *CGPUProgramIndex::Names[NUM_UNIFORMS] = 
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

	"fog", 
};

void IGPUProgram::buildInfo(CSource *source)
{
	nlassert(!m_Source);

	m_Source = source;

	// Fill index cache
	for (int i = 0; i < CGPUProgramIndex::NUM_UNIFORMS; ++i)
	{
		m_Index.Indices[i] = getUniformIndex(m_Index.Names[i]);
	}

	buildInfo();
}

void IGPUProgram::buildInfo()
{
	
}

} /* namespace NL3D */

/* end of file */

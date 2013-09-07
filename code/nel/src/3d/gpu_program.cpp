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

IGPUProgram::IGPUProgram(CGPUProgramSourceCont *programSource) : _ProgramSource(programSource)
{
	
}

// ***************************************************************************

IGPUProgram::~IGPUProgram()
{
	// Must kill the drv mirror of this program.
	_DrvInfo.kill();
}

} /* namespace NL3D */

/* end of file */

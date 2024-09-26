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

	"fog", 
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

} /* namespace NL3D */

/* end of file */

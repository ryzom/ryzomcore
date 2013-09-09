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

void IGPUProgram::buildInfo(CSource *source)
{
	nlassert(!m_Source);

	m_Source = source;

	// Fill index cache
	CGPUProgramFeatures &features = m_Source->Features;
	TProfile profile = m_Source->Profile; // for special cases

	if (features.DriverFlags & CGPUProgramFeatures::ModelView)
	{
		m_Indices.ModelView = getUniformIndex("nlModelView");
		if (m_Indices.ModelView == ~0)
		{
			nlwarning("Missing 'nlModelView' in gpu program '%s', ModelView disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelView;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewInverse)
	{
		m_Indices.ModelViewInverse = getUniformIndex("nlModelViewInverse");
		if (m_Indices.ModelViewInverse == ~0)
		{
			nlwarning("Missing 'nlModelViewInverse' in gpu program '%s', ModelViewInverse disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewInverse;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewTranspose)
	{
		m_Indices.ModelViewTranspose = getUniformIndex("nlModelViewTranspose");
		if (m_Indices.ModelViewTranspose == ~0)
		{
			nlwarning("Missing 'nlModelViewTranspose' in gpu program '%s', ModelViewTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewInverseTranspose)
	{
		m_Indices.ModelViewInverseTranspose = getUniformIndex("nlModelViewInverseTranspose");
		if (m_Indices.ModelViewInverseTranspose == ~0)
		{
			nlwarning("Missing 'nlModelViewInverseTranspose' in gpu program '%s', ModelViewInverseTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewInverseTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::Projection)
	{
		m_Indices.Projection = getUniformIndex("nlProjection");
		if (m_Indices.Projection == ~0)
		{
			nlwarning("Missing 'nlProjection' in gpu program '%s', Projection disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::Projection;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ProjectionInverse)
	{
		m_Indices.ProjectionInverse = getUniformIndex("nlProjectionInverse");
		if (m_Indices.ProjectionInverse == ~0)
		{
			nlwarning("Missing 'nlProjectionInverse' in gpu program '%s', ProjectionInverse disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ProjectionInverse;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ProjectionTranspose)
	{
		m_Indices.ProjectionTranspose = getUniformIndex("nlProjectionTranspose");
		if (m_Indices.ProjectionTranspose == ~0)
		{
			nlwarning("Missing 'nlProjectionTranspose' in gpu program '%s', ProjectionTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ProjectionTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ProjectionInverseTranspose)
	{
		m_Indices.ProjectionInverseTranspose = getUniformIndex("nlProjectionInverseTranspose");
		if (m_Indices.ProjectionInverseTranspose == ~0)
		{
			nlwarning("Missing 'nlProjectionInverseTranspose' in gpu program '%s', ProjectionInverseTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ProjectionInverseTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewProjection)
	{
		m_Indices.ModelViewProjection = getUniformIndex("nlModelViewProjection");
		if (m_Indices.ModelViewProjection == ~0)
		{
			nlwarning("Missing 'nlModelViewProjection' in gpu program '%s', ModelViewProjection disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewProjection;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewProjectionInverse)
	{
		m_Indices.ModelViewProjectionInverse = getUniformIndex("nlModelViewProjectionInverse");
		if (m_Indices.ModelViewProjectionInverse == ~0)
		{
			nlwarning("Missing 'nlModelViewProjectionInverse' in gpu program '%s', ModelViewProjectionInverse disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewProjectionInverse;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewProjectionTranspose)
	{
		m_Indices.ModelViewProjectionTranspose = getUniformIndex("nlModelViewProjectionTranspose");
		if (m_Indices.ModelViewProjectionTranspose == ~0)
		{
			nlwarning("Missing 'nlModelViewProjectionTranspose' in gpu program '%s', ModelViewProjectionTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewProjectionTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewProjectionInverseTranspose)
	{
		m_Indices.ModelViewProjectionInverseTranspose = getUniformIndex("nlModelViewProjectionInverseTranspose");
		if (m_Indices.ModelViewProjectionInverseTranspose == ~0)
		{
			nlwarning("Missing 'nlModelViewProjectionInverseTranspose' in gpu program '%s', ModelViewProjectionInverseTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewProjectionInverseTranspose;
		}
	}
	
	//
	// Rough example, modify as necessary.
	//
	/*if (features.DriverFlags & CGPUProgramFeatures::DriverAmbient || features.MaterialFlags & CGPUProgramFeatures::MaterialAmbient)
	{
		m_Indices.Ambient = getUniformIndex("nlAmbient");
		if (m_Indices.Ambient == ~0)
		{
			nlwarning("Missing 'nlAmbient' in gpu program '%s', Ambient disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::DriverAmbient;
			features.MaterialFlags &= ~CGPUProgramFeatures::MaterialAmbient;
		}
	}*/

	buildInfo();
}

void IGPUProgram::buildInfo()
{
	
}

} /* namespace NL3D */

/* end of file */

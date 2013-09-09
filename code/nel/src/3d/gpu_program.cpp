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
		m_Indices.ModelView = getUniformIndex("modelView");
		if (m_Indices.ModelView == ~0)
		{
			nlwarning("Missing 'modelView' in gpu program '%s', ModelView disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelView;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewInverse)
	{
		m_Indices.ModelViewInverse = getUniformIndex("modelViewInverse");
		if (m_Indices.ModelViewInverse == ~0)
		{
			nlwarning("Missing 'modelViewInverse' in gpu program '%s', ModelViewInverse disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewInverse;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewTranspose)
	{
		m_Indices.ModelViewTranspose = getUniformIndex("modelViewTranspose");
		if (m_Indices.ModelViewTranspose == ~0)
		{
			nlwarning("Missing 'modelViewTranspose' in gpu program '%s', ModelViewTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewInverseTranspose)
	{
		m_Indices.ModelViewInverseTranspose = getUniformIndex("modelViewInverseTranspose");
		if (m_Indices.ModelViewInverseTranspose == ~0)
		{
			nlwarning("Missing 'modelViewInverseTranspose' in gpu program '%s', ModelViewInverseTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewInverseTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::Projection)
	{
		m_Indices.Projection = getUniformIndex("projection");
		if (m_Indices.Projection == ~0)
		{
			nlwarning("Missing 'projection' in gpu program '%s', Projection disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::Projection;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ProjectionInverse)
	{
		m_Indices.ProjectionInverse = getUniformIndex("projectionInverse");
		if (m_Indices.ProjectionInverse == ~0)
		{
			nlwarning("Missing 'projectionInverse' in gpu program '%s', ProjectionInverse disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ProjectionInverse;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ProjectionTranspose)
	{
		m_Indices.ProjectionTranspose = getUniformIndex("projectionTranspose");
		if (m_Indices.ProjectionTranspose == ~0)
		{
			nlwarning("Missing 'projectionTranspose' in gpu program '%s', ProjectionTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ProjectionTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ProjectionInverseTranspose)
	{
		m_Indices.ProjectionInverseTranspose = getUniformIndex("projectionInverseTranspose");
		if (m_Indices.ProjectionInverseTranspose == ~0)
		{
			nlwarning("Missing 'projectionInverseTranspose' in gpu program '%s', ProjectionInverseTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ProjectionInverseTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewProjection)
	{
		m_Indices.ModelViewProjection = getUniformIndex("modelViewProjection");
		if (m_Indices.ModelViewProjection == ~0)
		{
			nlwarning("Missing 'modelViewProjection' in gpu program '%s', ModelViewProjection disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewProjection;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewProjectionInverse)
	{
		m_Indices.ModelViewProjectionInverse = getUniformIndex("modelViewProjectionInverse");
		if (m_Indices.ModelViewProjectionInverse == ~0)
		{
			nlwarning("Missing 'modelViewProjectionInverse' in gpu program '%s', ModelViewProjectionInverse disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewProjectionInverse;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewProjectionTranspose)
	{
		m_Indices.ModelViewProjectionTranspose = getUniformIndex("modelViewProjectionTranspose");
		if (m_Indices.ModelViewProjectionTranspose == ~0)
		{
			nlwarning("Missing 'modelViewProjectionTranspose' in gpu program '%s', ModelViewProjectionTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewProjectionTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::ModelViewProjectionInverseTranspose)
	{
		m_Indices.ModelViewProjectionInverseTranspose = getUniformIndex("modelViewProjectionInverseTranspose");
		if (m_Indices.ModelViewProjectionInverseTranspose == ~0)
		{
			nlwarning("Missing 'modelViewProjectionInverseTranspose' in gpu program '%s', ModelViewProjectionInverseTranspose disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::ModelViewProjectionInverseTranspose;
		}
	}
	if (features.DriverFlags & CGPUProgramFeatures::Fog)
	{
		m_Indices.Fog = getUniformIndex("fog");
		if (m_Indices.Fog == ~0)
		{
			nlwarning("Missing 'fog' in gpu program '%s', Fog disabled", source->DisplayName.c_str());
			features.DriverFlags &= ~CGPUProgramFeatures::Fog;
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

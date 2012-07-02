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

#include "std3d.h"


#include "nel/3d/ps_util.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_particle2.h"
#include "nel/3d/ps_mesh.h"


namespace NL3D
{
	void CPSUtil::registerParticles()
	{
		NL_PS_FUNC(	CPSUtil_registerParticles)
		NLMISC_REGISTER_CLASS(CPSFanLight);
		NLMISC_REGISTER_CLASS(CPSTailDot);
		NLMISC_REGISTER_CLASS(CPSRibbon);
		NLMISC_REGISTER_CLASS(CPSRibbonLookAt);
		NLMISC_REGISTER_CLASS(CPSShockWave);
		NLMISC_REGISTER_CLASS(CPSFace);
		NLMISC_REGISTER_CLASS(CPSMesh);
		NLMISC_REGISTER_CLASS(CPSConstraintMesh);
		NLMISC_REGISTER_CLASS(CPSDot);
		NLMISC_REGISTER_CLASS(CPSFaceLookAt);

		CPSRotated2DParticle::initRotTable(); // init the precalc rot table for face lookat
		CPSFanLight::initFanLightPrecalc();
		CPSDot::initVertexBuffers();
		CPSQuad::initVertexBuffers();
		CPSConstraintMesh::initPrerotVB();
	}
} // NL3D

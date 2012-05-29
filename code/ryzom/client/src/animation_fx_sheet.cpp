// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#include "stdpch.h"
#include "animation_fx_sheet.h"
#include "nel/georges/u_form_elm.h"
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_animation.h"
#include "nel/3d/u_track.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_scene.h"


extern NL3D::UScene *Scene;

//*******************************************************************************
// CAnimationFX
//*******************************************************************************

//-----------------------------------------------
// CAnimationFX
//-----------------------------------------------
CAnimationFX::CAnimationFX(const std::string &psName /* = "" */, const float *userParams /*=NULL*/)
{
	PosTrack = NULL;
	Sheet = NULL;
	if ((!psName.empty()) || (userParams != NULL))
	{
		Sheet = new CAnimationFXSheet(psName, userParams);
	}
}// CAnimationFX //

//-----------------------------------------------
// init
//-----------------------------------------------
void CAnimationFX::init(CAnimationFXSheet *sheet, NL3D::UAnimationSet *as)
{
	Sheet = sheet;
	// Dont need to call the build track
}// init //


//-----------------------------------------------
// buildTrack
//-----------------------------------------------
void CAnimationFX::buildTrack(NL3D::UAnimationSet *as)
{
	nlassert(Sheet != NULL);
	if (!as) return;
	if (Sheet->TrajectoryAnim.empty()) return;
	std::string animName = Sheet->TrajectoryAnim;
	NLMISC::strlwr(animName);
	uint id = as->getAnimationIdByName(animName);
	NL3D::UAnimation *anim = NULL;
	if (id != NL3D::UAnimationSet::NotFound)
	{
		anim = as->getAnimation(id);
	}
	else
	{
		// try to load anim
		uint id = as->addAnimation(animName.c_str(), animName.c_str());
		if (id != NL3D::UAnimationSet::NotFound)
		{
			anim = as->getAnimation(id);
		}
	}
	if (anim)
	{
		PosTrack = anim->getTrackByName("pos");
	}
}// buildTrack //

//-----------------------------------------------
//createMatchingInstance
//-----------------------------------------------
NL3D::UParticleSystemInstance CAnimationFX::createMatchingInstance() const
{
	nlassert(Sheet != NULL);
	if (Sheet->PSName.empty()) return NULL;
	if (nlstricmp(Sheet->PSName, "none") == 0) return NULL;
	NL3D::UInstance inst = Scene->createInstance(Sheet->PSName);
	if (!inst.empty())
	{
		NL3D::UParticleSystemInstance fx;
		fx.cast(inst);
		if (fx.empty())
		{
			nlwarning("Bad shape type for a fxsheet shape : must be a particle system");
			Scene->deleteInstance(inst);
			return NULL;
		}
		else
		{
			for(uint l = 0; l < 4; ++l)
			{
				fx->setUserParam(l, Sheet->UserParam[l]);
			}
			fx->setUserColor(Sheet->Color);
			return fx;
		}
	}
	return NULL;
}// createMatchingInstance //

//*******************************************************************************
// CAnimationFXSet
//*******************************************************************************

//-----------------------------------------------
// buildTrack
//-----------------------------------------------
void CAnimationFXSet::buildTrack(NL3D::UAnimationSet *as)
{
	if (!as) return;
	for(uint k = 0; k < FX.size(); ++k)
	{
		FX[k].buildTrack(as);
	}
}// buildTrack //

// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "animation_fx.h"
#include "nel/georges/u_form_elm.h"
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_animation.h"
#include "nel/3d/u_track.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_particle_system_instance.h"
#include "nel/3d/u_scene.h"
#include "client_sheets/animation_fx_set_sheet.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

extern NL3D::UScene *Scene;


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
void CAnimationFX::init(const CAnimationFXSheet *sheet, NL3D::UAnimationSet *as)
{
	Sheet = sheet;
	buildTrack(as);
}// init //


//-----------------------------------------------
// buildTrack
//-----------------------------------------------
void CAnimationFX::buildTrack(NL3D::UAnimationSet *as)
{
	nlassert(Sheet != NULL);
	if (!as) return;
	if (Sheet->TrajectoryAnim.empty()) return;
	std::string animName = NLMISC::toLowerAscii(Sheet->TrajectoryAnim);
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
NL3D::UParticleSystemInstance CAnimationFX::createMatchingInstance(const float *customUserParams /* = NULL */) const
{
	NL3D::UParticleSystemInstance fx;
	nlassert(Sheet != NULL);
	if (Sheet->PSName.empty()) return NULL;
	NL3D::UInstance inst = Scene->createInstance(Sheet->PSName);
	if (!inst.empty())
	{
		fx.cast (inst);
		if (fx.empty())
		{
			nlwarning("Bad shape type for a fxsheet shape : must be a particle system");
			Scene->deleteInstance(inst);
			return NULL;
		}
		else
		{
			if (customUserParams != NULL)
			{
				for(uint l = 0; l < 4; ++l)
				{
					fx.setUserParam(l, customUserParams[l]);
				}
			}
			else
			{
				for(uint l = 0; l < 4; ++l)
				{
					fx.setUserParam(l, Sheet->UserParam[l]);
				}
			}
			fx.setUserColor(Sheet->Color);
		}
	}
	return fx;
}// createMatchingInstance //

// *******************************************************************************
// CAnimationFXSet
// *******************************************************************************

//-----------------------------------------------
// init
//-----------------------------------------------
void CAnimationFXSet::init(const CAnimationFXSetSheet *sheet, NL3D::UAnimationSet *as)
{
	if (!sheet) return;
	Sheet = sheet;
	FX.resize(sheet->FX.size());
	for(uint k = 0; k < sheet->FX.size(); ++k)
	{
		FX[k].init(&sheet->FX[k], as);
	}
}

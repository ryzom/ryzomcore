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

#include "gabarit.h"
#include "nel/3d/u_skeleton.h"
#include "nel/3d/u_bone.h"
#include "nel/3d/u_driver.h"
#include "nel/misc/algo.h"
#include "nel/misc/progress_callback.h"

#include "game_share/gender.h"

#include <algorithm>

using namespace NLMISC;
using namespace NL3D;
using namespace std;
//using CSkeletonGabarit::EBoneCategory;


H_AUTO_DECL(RZ_CSkeletonGabarit)

extern UDriver  *Driver;

CGabaritSet GabaritSet;

static const struct
{
	const char						*Name;
	CSkeletonGabarit::EBoneCategory Category;
} boneInfos[] =
{
	/*
	{ "Bip01 Head_Second",		CSkeletonGabarit::Other },
	{ "Bip01 Ponytail1_Second", CSkeletonGabarit::Other },
	{ "Bip01 L Finger0_Second", CSkeletonGabarit::Arm },
	{ "Bip01 L Finger1_Second", CSkeletonGabarit::Arm },
	{ "Bip01 L Finger2_Second", CSkeletonGabarit::Arm },
	{ "Bip01 L Finger3_Second", CSkeletonGabarit::Arm },
	{ "Bip01 L Finger4_Second", CSkeletonGabarit::Arm },
	{ "Bip01 L Toe0_Second",	CSkeletonGabarit::Legs },
	{ "Bip01 R Toe0_Second",	CSkeletonGabarit::Legs }
	*/
	{ "Bip01 Pelvis",			CSkeletonGabarit::Torso },
	{ "Bip01 Spine",			CSkeletonGabarit::Torso },
	{ "Bip01 Spine1",			CSkeletonGabarit::Torso },
	{ "Bip01 Spine2",			CSkeletonGabarit::Torso },
	{ "Bip01 Neck",				CSkeletonGabarit::Other },
	{ "Bip01 Head",				CSkeletonGabarit::Other },
	{ "Bip01 Ponytail1",		CSkeletonGabarit::Other },
	{ "Bip01 Ponytail11",		CSkeletonGabarit::Other },
	{ "Bip01 L Clavicle",		CSkeletonGabarit::Torso },
	{ "Bip01 L UpperArm",		CSkeletonGabarit::Arm },
	{ "Bip01 L Forearm",		CSkeletonGabarit::Arm },
	{ "Bip01 L Hand",			CSkeletonGabarit::Arm },
	{ "Bip01 L Finger0",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger01",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger02",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger1",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger11",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger12",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger2",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger21",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger22",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger3",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger31",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger32",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger4",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger41",		CSkeletonGabarit::Arm },
	{ "Bip01 L Finger42",		CSkeletonGabarit::Arm },
	{ "Bip01 R Clavicle",		CSkeletonGabarit::Torso },
	{ "Bip01 R UpperArm",		CSkeletonGabarit::Arm },
	{ "Bip01 R Forearm",		CSkeletonGabarit::Arm },
	{ "Bip01 R Hand",			CSkeletonGabarit::Arm },
	{ "Bip01 R Finger0",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger01",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger02",		CSkeletonGabarit::Arm },
// TRAP : not present in the skeleton : no more used ?	{ "Bip01 R Finger0R",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger1",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger11",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger12",		CSkeletonGabarit::Arm },
// TRAP : not present in the skeleton : no more used ?	{ "Bip01 R Finger1R",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger2",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger21",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger22",		CSkeletonGabarit::Arm },
// TRAP : not present in the skeleton : no more used ?	{ "Bip01 R Finger2R",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger3",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger31",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger32",		CSkeletonGabarit::Arm },
// TRAP : not present in the skeleton : no more used ?	{ "Bip01 R Finger3R",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger4",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger41",		CSkeletonGabarit::Arm },
	{ "Bip01 R Finger42",		CSkeletonGabarit::Arm },
// TRAP : not present in the skeleton : no more used ?	{ "Bip01 R Finger4R",		CSkeletonGabarit::Arm },
	{ "Bip01 L Thigh",			CSkeletonGabarit::Legs },
	{ "Bip01 L Calf",			CSkeletonGabarit::Legs },
	{ "Bip01 L Foot",			CSkeletonGabarit::Legs },
	{ "Bip01 L Toe0",			CSkeletonGabarit::Legs },
	{ "Bip01 R Thigh",			CSkeletonGabarit::Legs },
	{ "Bip01 R Calf",			CSkeletonGabarit::Legs },
	{ "Bip01 R Foot",			CSkeletonGabarit::Legs },
	{ "Bip01 R Toe0",			CSkeletonGabarit::Legs },
	{ "sein_droit",				CSkeletonGabarit::Breast },
	{ "sein_gauche",			CSkeletonGabarit::Breast }
};

//===================================================================================
CSkeletonGabarit::CSkeletonGabarit()
{
	H_AUTO_USE(RZ_CSkeletonGabarit)
	nlctassert(sizeof(boneInfos) / sizeof(boneInfos[0]) == NumBones);
	std::fill(BoneScale, BoneScale + NumBones, CVector(1, 1, 1));
	std::fill(BoneSkinScale, BoneSkinScale + NumBones, CVector(1, 1, 1));
}


//===================================================================================
void CSkeletonGabarit::buildFromSkeleton(NL3D::USkeleton src, const std::string &skelName, bool bMale)
{
	H_AUTO_USE(RZ_CSkeletonGabarit)
	nlassert(!src.empty());
	// get the bones we need
	for(uint k = 0; k < NumBones; ++k)
	{
		sint boneID = src.getBoneIdByName(boneIndexToName(k));
		if (boneID == -1)
		{
			// If the skeleton is male and we try to get breast its normal that we dont found it
			if ( ! ((boneIndexToCategory(k) == CSkeletonGabarit::Breast) && (bMale)) )
				nlwarning("CSkeletonGabarit : can't get bone %s for skeleton %s", boneIndexToName(k), skelName.c_str());
			BoneScale[k].set(1, 1, 1);
			BoneSkinScale[k].set(1, 1, 1);
		}
		else
		{
			UBone bone = src.getBone((uint) boneID);
			bone.setTransformMode(UTransformable::RotQuat);
			BoneScale[k] = bone.getScale();
			BoneSkinScale[k] = bone.getSkinScale();
		}
	}
	// get size
	/*NLMISC::CAABBox bbox;
	src.computeCurrentBBox(bbox, NULL, 0.f, true);
	HeightScale = 2.f * bbox.getHalfSize().z;
	*/

	// yoyo patch
	{
		HeightScale= 1.f;
		sint boneId = src.getBoneIdByName("Bip01 R Thigh");
		if (boneId != -1)
		{
			UBone bone = src.getBone(boneId);
			HeightScale = bone.getScale().x;
		}
	}
}

//===================================================================================
const char *CSkeletonGabarit::boneIndexToName(uint id)
{
	H_AUTO_USE(RZ_CSkeletonGabarit)
	nlassert(id < NumBones);
	return boneInfos[id].Name;
}

//===================================================================================
CSkeletonGabarit::EBoneCategory CSkeletonGabarit::boneIndexToCategory(uint id)
{
	H_AUTO_USE(RZ_CSkeletonGabarit)
	nlassert(id < NumBones);
	return boneInfos[id].Category;
}

//===================================================================================
//===================================================================================

void CGabaritSet::loadGabarits (NLMISC::IProgressCallback &progress)
{
	H_AUTO_USE(RZ_CSkeletonGabarit)
	static const char *genderPrefix[] = { "HOM_skel", "HOF_skel" };
	static const char *racePrefix[]   = { "TR_", "FY_", "MA_", "ZO_" };
	static const char *heightPrefix[] = { "_Small", "_Mid", "_Big" };
	static const char *widthPrefix[]  = { "_Slim", "", "_Fat" };

	// create a dummy scene to load the skeletons
	UScene *scene = Driver->createScene(false);
	if (!scene)
	{
		nlwarning("CGabaritSet::loadGabarits : can't create scene to load skeletons");
	}

	for(uint g = 0; g < NumGender; ++g)
	{
		// Progress bar
		progress.progress ((float)g/(float)NumGender);
		progress.pushCropedValues ((float)g/(float)NumGender, (float)(g+1)/(float)NumGender);

		for(uint r = 0; r < NumRace; ++r)
		{
			// Progress bar
			progress.progress ((float)r/(float)NumRace);
			progress.pushCropedValues ((float)r/(float)NumRace, (float)(r+1)/(float)NumRace);

			for(uint h = 0; h < NumHeights; ++h)
			{
				// Progress bar
				progress.progress ((float)h/(float)NumHeights);
				progress.pushCropedValues ((float)h/(float)NumHeights, (float)(h+1)/(float)NumHeights);

				for(uint w = 0; w < NumWidths; ++w)
				{
					// Progress bar
					progress.progress ((float)w/(float)NumWidths);

					string skelName = string(racePrefix[r]) + genderPrefix[g];
					if (h != 1 || w != 1)
					{
						skelName += string(heightPrefix[h]) + widthPrefix[w];
					}
					skelName += ".skel";
					USkeleton skel = scene->createSkeleton(skelName);
					if (skel.empty())
					{
						nlwarning("CGabaritSet::loadGabarits : can't load skeleton %s", skelName.c_str());
					}
					else
					{
						_Gabarit[g][r][h][w].buildFromSkeleton(skel, skelName, g == GSGENDER::male);
						scene->deleteSkeleton(skel);
					}
				}

				// Progress bar
				progress.popCropedValues ();
			}

			// Progress bar
			progress.popCropedValues ();
		}

		// Progress bar
		progress.popCropedValues ();
	}
	Driver->deleteScene(scene);
}

//===================================================================================
sint CGabaritSet::peopleToIndex(EGSPD::CPeople::TPeople people)
{
	H_AUTO_USE(RZ_CSkeletonGabarit)
	sint race = -1;
	switch (people)
	{
		case EGSPD::CPeople::Fyros: race = 1; break;
		case EGSPD::CPeople::Tryker: race = 0; break;
		case EGSPD::CPeople::Matis: race = 2; break;
		case EGSPD::CPeople::Zorai: race = 3; break;
		default:
			nlwarning("CGabaritSet::peopleToIndex : not a supported race");
		break;
	}
	return race;
}

//===================================================================================
void CGabaritSet::applyGabarit(NL3D::USkeleton dest, uint gender, EGSPD::CPeople::TPeople people, float height, float torsoWidth, float armsWidth, float legsWidth, float breastSize, float *finalHeightScale)
{
	H_AUTO_USE(RZ_CSkeletonGabarit)
	if (dest.empty())
	{
		nlwarning("<CGabaritSet::applyGabarit> Skeleton NULL (gender=%d People=%d)",gender,people);
		return;
	}
	if (gender >= NumGender) return;

	sint race = peopleToIndex(people);
	if (race == -1) return;

	clamp(height, -1, 1);
	clamp(torsoWidth, -1, 1);
	clamp(armsWidth, -1, 1);
	clamp(legsWidth, -1, 1);
	clamp(breastSize, -1, 1);


	// we blend between the 4 nearest gabarits
	//   Slim -1  Normal 0  Fat 1
	//     +--------+--------+     Big 1
	//     |        |        |
	//     |        |        |
	//     |        |        |
	//     |        |        |
	//     +--------+--------+     Mid 0
	//     |        |        |
	//     |        |        |
	//     |        |        |
	//     |        |        |
	//     +--------+--------+     Small - 1


	// a set of 4 gabarit between which to blend
	//  TL     TR
	//  +------+
	//  |      |
	//  |      |
	//  |      |
	//  +------+
	//  BL     BR
	struct CGabaritBlend
	{
		CSkeletonGabarit *TL;
		CSkeletonGabarit *TR;
		CSkeletonGabarit *BL;
		CSkeletonGabarit *BR;
	};

	// Choose the right gabarits
	float          *widthsTab[CSkeletonGabarit::NumBoneCategory] = { &armsWidth, &torsoWidth, &legsWidth, &breastSize };
	CGabaritBlend  gbTab[CSkeletonGabarit::NumBoneCategory];
	float          blendTab[CSkeletonGabarit::NumBoneCategory];

	// build each set of 4 gabarits
	uint k;
	for(k = 0; k < CSkeletonGabarit::NumBoneCategory; ++k)
	{
		float width = *widthsTab[k];
		gbTab[k].BL = &_Gabarit[gender][race][height >= 0 ? 1 : 0][width >= 0 ? 1 : 0];
		gbTab[k].BR = &_Gabarit[gender][race][height >= 0 ? 1 : 0][width >= 0 ? 2 : 1];
		gbTab[k].TL = &_Gabarit[gender][race][height >= 0 ? 2 : 1][width >= 0 ? 1 : 0];
		gbTab[k].TR = &_Gabarit[gender][race][height >= 0 ? 2 : 1][width >= 0 ? 2 : 1];
		//
		blendTab[k] = width >= 0 ? width : 1.f + width;
	}

	float heightBlend = height >= 0 ? height : 1.f + height;


	// blend each bone
	for(k = 0; k < CSkeletonGabarit::NumBones; ++k)
	{
		// get the bone in the dest skeleton
		sint boneID = dest.getBoneIdByName(CSkeletonGabarit::boneIndexToName(k));
		if (boneID != -1)
		{
			UBone bone = dest.getBone(boneID);
			CSkeletonGabarit::EBoneCategory boneCategory = CSkeletonGabarit::boneIndexToCategory(k);
			if (boneCategory != CSkeletonGabarit::Other)
			{
				const CGabaritBlend &gb = gbTab[boneCategory]; // takes the gabarit blend matching the bone category
				bone.setTransformMode(UTransform::RotQuat);

				// standard scale
				CVector scale = NLMISC::computeBilinear(gb.BL->BoneScale[k],
														gb.BR->BoneScale[k],
														gb.TR->BoneScale[k],
														gb.TL->BoneScale[k],
														blendTab[boneCategory],
														heightBlend);
				bone.setScale(scale);

				// skin scale
				CVector skinScale= NLMISC::computeBilinear( gb.BL->BoneSkinScale[k],
															gb.BR->BoneSkinScale[k],
															gb.TR->BoneSkinScale[k],
															gb.TL->BoneSkinScale[k],
															blendTab[boneCategory],
															heightBlend);
				bone.setSkinScale(skinScale);
			}
		}
	}

	if (finalHeightScale)
	{
		*finalHeightScale = NLMISC::computeBilinear(gbTab[0].BL->HeightScale,
											   gbTab[0].BR->HeightScale,
											   gbTab[0].TR->HeightScale,
											   gbTab[0].TL->HeightScale,
											   0.f,
											   heightBlend);
	}


}

//===================================================================================
float CGabaritSet::getRefHeightScale(uint gender, EGSPD::CPeople::TPeople people)
{
	H_AUTO_USE(RZ_CSkeletonGabarit)
	nlassert(gender < 2);
	sint peopleIndex = peopleToIndex(people);
	if (peopleIndex == -1) return 0.f;
	return _Gabarit[gender][peopleIndex][1][1].HeightScale; // return mean size
}

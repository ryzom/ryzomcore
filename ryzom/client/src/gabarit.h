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




#ifndef CL_GABARIT_H
#define CL_GABARIT_H


#include "game_share/people.h"


namespace NL3D
{
	class USkeleton;
}

namespace NLMISC
{
	class IProgressCallback;
}

/** This is a description of a skeleton gabarit
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
struct CSkeletonGabarit
{
	enum { NumBones	= 56 }; // 61
	enum EBoneCategory { Arm = 0, Torso, Legs, Breast, Other, NumBoneCategory = Other};
	// the scale for each bone
	NLMISC::CVector BoneScale[NumBones];
	// the Skin Scale for each bone
	NLMISC::CVector BoneSkinScale[NumBones];
	// height
	float   HeightScale;
	// ctor
	CSkeletonGabarit();
	// build values from a skeleton
	void buildFromSkeleton(NL3D::USkeleton src, const std::string &skelName, bool bMale);
	// convert a bone ID to its name
	static const char *boneIndexToName(uint id);
	// convert a bone ID to its category
	static EBoneCategory boneIndexToCategory(uint id);
};


/** This can load gabarit and blend them on a skeleton
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CGabaritSet
{
public:
	enum { NumGender = 2, NumRace = 4, NumHeights = 3, NumWidths = 3 };
public:
	// load all ryzom gabarits
	void loadGabarits (NLMISC::IProgressCallback &progress);
	/** Blend gabarits, and apply the result to the given skeleton
	  * \param height from -1 (smallest) to 1 (tallest)
	  * \param torsoWidth from -1  to 1
	  * \param legsWidth from -1 to 1
	  * \param finalHeightScale if not NULL, will be filled with height scale of the skeleton (factor)
	  */
	void applyGabarit(NL3D::USkeleton dest, uint gender, EGSPD::CPeople::TPeople race,
		              float height,
					  float torsoWidth,
					  float armsWidth,
					  float legsWidth,
					  float breastSize,
					  float *finalHeightScale
					 );
	// get the reference (mean) size for a race
	float getRefHeightScale(uint gender, EGSPD::CPeople::TPeople race);
private:
	CSkeletonGabarit _Gabarit[NumGender][NumRace][NumHeights][NumWidths];
private:
	sint peopleToIndex(EGSPD::CPeople::TPeople people);
};


// The gabarit set in ryzom
extern CGabaritSet GabaritSet;








#endif

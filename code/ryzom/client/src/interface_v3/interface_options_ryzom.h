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


#ifndef IF_OPTIONS_RZ
#define IF_OPTIONS_RZ

#include "nel/gui/interface_options.h"

using namespace NLGUI;

// ***************************************************************************
class CMissionIconList : public CInterfaceOptions
{
public:
	CMissionIconList( const TCtorParam &param ) : CInterfaceOptions( param ){}
	~CMissionIconList(){}
	virtual bool parse (xmlNodePtr cur);
	sint32 getBackTexID(uint index) const { return index >= IconBackTexID.size() ? -1 : IconBackTexID[index]; }
	sint32 getTexID(uint index) const { return index >= IconTexID.size() ? -1 : IconTexID[index]; }
private:
	std::vector<sint32> IconBackTexID;
	std::vector<sint32> IconTexID;
};

// ***************************************************************************
/** Describe an animation Set container, used for multiple CCharacter3d for instance
 */
class COptionsAnimationSet : public CInterfaceOptions
{
public:
	COptionsAnimationSet( const TCtorParam &/* param */ );
	// see code for important release note
	virtual ~COptionsAnimationSet();
	virtual bool parse (xmlNodePtr cur);

	// tool fct to get the face anim name from a name (append "_face" before .anim)
	static std::string	getFaceAnimName(const std::string &animName);

public:
	NL3D::UAnimationSet		*AnimationSet;

	struct	CAnim
	{
		// Indexes in this animation set
		uint		AnimId;
		// true if must apply the race/gender scale to the position (not in rare case)
		bool		ApplyRaceScalePos;
		CAnim()
		{
			AnimId= -1;
			ApplyRaceScalePos= true;
		}
	};

	// Male and female Animation
	std::vector<CAnim>		AnimMale;
	std::vector<CAnim>		AnimFemale;

};

#endif

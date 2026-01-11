// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_MUSIC_SOURCE_H
#define NL_MUSIC_SOURCE_H

#include "nel/misc/types_nl.h"
#include "nel/sound/source_common.h"


namespace NLSOUND {


// ***************************************************************************
/**
 * A source that play music
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2004
 */
class CMusicSource : public CSourceCommon
{
public:
	/// Constructor
	CMusicSource	(class CMusicSound *sound=NULL, bool spawn=false, TSpawnEndCallback cb=0, void *cbUserParam = 0, NL3D::CCluster *cluster = 0, CGroupController *groupController = NULL);
	/// Destructor
	~CMusicSource	();

	/// Return the sound binded to the source (or NULL if there is no sound)
	virtual TSoundId				getSound();

	virtual void					play();
	/// Stop playing
	virtual void					stop();

	TSOURCE_TYPE					getType() const								{return SOURCE_MUSIC;}

private:

	class CMusicSound				*_MusicSound;
};


} // NLSOUND


#endif // NL_MUSIC_SOURCE_H

/* End of music_source.h */

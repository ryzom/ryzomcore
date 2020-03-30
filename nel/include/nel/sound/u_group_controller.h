/**
 * \file u_group_controller.h
 * \brief UGroupController
 * \date 2012-04-10 12:49GMT
 * \author Jan Boon (Kaetemi)
 * UGroupController
 */

// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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

#ifndef NLSOUND_U_GROUP_CONTROLLER_H
#define NLSOUND_U_GROUP_CONTROLLER_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes

// Project includes

#define NLSOUND_SHEET_V1_DEFAULT_SOUND_GROUP_CONTROLLER "sound:effects:game"
#define NLSOUND_SHEET_V1_DEFAULT_SOUND_MUSIC_GROUP_CONTROLLER "sound:music:game"
#define NLSOUND_SHEET_V1_DEFAULT_SOUND_STREAM_GROUP_CONTROLLER "sound:dialog:game"

namespace NLSOUND {

/**
 * \brief UGroupController
 * \date 2012-04-10 12:49GMT
 * \author Jan Boon (Kaetemi)
 * UGroupController
 */
class UGroupController
{
public:
	virtual void setGain(float gain) = 0;
	virtual float getGain() = 0;

protected:
	virtual ~UGroupController() { }

}; /* class UGroupController */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_U_GROUP_CONTROLLER_H */

/* end of file */

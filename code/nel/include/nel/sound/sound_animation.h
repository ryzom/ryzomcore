// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#ifndef NL_SOUND_ANIM_TRACK_H
#define NL_SOUND_ANIM_TRACK_H

#include "nel/sound/sound_anim_manager.h"

namespace NLMISC
{
	class IStream;
}

namespace NLSOUND {

class CSoundAnimMarker;
class UAudioMixer;

typedef std::vector<CSoundAnimMarker*> TMarkerVector;



class CSoundAnimation
{
public:

	enum
	{
		NoId = -1
	};

	CSoundAnimation(std::string& name, TSoundAnimId id) : _Id(id), _Name(name), _Dirty(false) {}

	virtual ~CSoundAnimation() {}

	/** Add a new marker */
	virtual void				addMarker(CSoundAnimMarker* marker);

	/** Remove a marker */
	virtual void				removeMarker(CSoundAnimMarker* marker);

	/** Return the number of markers in this track
	 */
	virtual uint32				countMarkers() const					{ return (uint32)_Markers.size(); }

	/** Return a marker of this track given its index */
	virtual CSoundAnimMarker*	getMarker(uint32 i)						{ return _Markers[i]; }

	/** Return the name of the animation  */
	virtual std::string&		getName()								{ return _Name; }


	/** Load from an xml file */
	virtual void				load();

	/** Save to an xml document */
	virtual void				save();

	/** Return the filename of the animation */
	virtual std::string&		getFilename()							{ return _Filename; }

	/** Set the filename of the animation
	 */
	virtual void				setFilename(std::string& name)			{ _Filename = name; }

	/** Returns whether the sound animation changed since the last save. (Editing support) */
	virtual bool				isDirty()								{ return _Dirty; }

	/** Set the dirty flag (Editing support) */
	virtual void				setDirty(bool b)						{ _Dirty = b; }

	/** Play the sounds of the markers that fall within the specified time interval. */
	virtual void				play(UAudioMixer* mixer, float lastTime, float curTime, NL3D::CCluster *cluster, CSoundContext &context);

protected:

	/// Sort all the markers according to their time
	virtual void				sort();

	/** The unique ID of the animation */
	TSoundAnimId				_Id;

	/** The name of the animation */
	std::string					_Name;

	/** The set of markers */
	TMarkerVector				_Markers;

	/** The filename */
	std::string					_Filename;

	/** Has the sound animation changed since the last save? (Editing support) */
	bool						_Dirty;

};

} // namespace NLSOUND

#endif // NL_SOUND_ANIM_TRACK_H

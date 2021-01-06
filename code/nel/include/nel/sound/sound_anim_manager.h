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

#ifndef NL_SOUND_ANIM_MANAGER_H
#define NL_SOUND_ANIM_MANAGER_H


#include "nel/misc/vector.h"
#include "nel/sound/u_source.h"

namespace NL3D
{
	class CCluster;
}

namespace NLSOUND {


//class CSoundAnimPlayer;
class CSoundAnimation;
class UAudioMixer;

/*// Comparison for const char*
struct eqsndanim
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};
*/

///
typedef sint32 TSoundAnimId;

const TSoundAnimId CSoundAnimationNoId = -1;

///
typedef sint32 TSoundAnimPlayId;

/// Animation name-to-id hash map
typedef CHashMap<std::string, TSoundAnimId> TSoundAnimMap;

/// Animation vector
typedef std::vector<CSoundAnimation*> TSoundAnimVector;

/// The set of players
//typedef std::set<CSoundAnimPlayer*> TPlayerSet;



class CSoundAnimManager
{
public:

	static CSoundAnimManager* instance() { return _Instance; }
	static void release() { delete _Instance; }

	CSoundAnimManager(UAudioMixer* mixer);
	virtual ~CSoundAnimManager();

	/** Load the sound animation with the specified name.
	 *  Returns the id of the new animation, or CSoundAnimationNoId if the
	 *  file could not be found.
	 *  \param name The name of the animation to load.
	 */
	virtual TSoundAnimId			loadAnimation(std::string& name);

	/** Create a new sound animation with the specified name.
	 *  Returns CSoundAnimation::NoId if the creation fails (duplicate name).
	 *  \param name The name of the animation to load.
	 */
	virtual TSoundAnimId			createAnimation(std::string& name);

	/** Save the sound animation in the specified file.
	 *  \param filename The name of the file to save the animation in.
	 */
	virtual void					saveAnimation(CSoundAnimation* anim, std::string& filname);

	/** Start playing a sound animation. Returns true if the animation was
	 *  found and is playing.
 	 *  \param name The name of the animation to load.
	 */
	virtual TSoundAnimId			getAnimationFromName(std::string& name);

	/** Returns the animation corresponding to a name.
	 *  \param name The name of the animation to load.
	 */
	virtual CSoundAnimation*		findAnimation(std::string& name);

	/** Start playing a sound animation. Returns an id number of this playback instance
	 *  or -1 if the animation was not found.
 	 *  \param name The id of the animation to play.
	 */
	virtual TSoundAnimPlayId		playAnimation(TSoundAnimId id, float time, NL3D::CCluster *cluster, CSoundContext &context);

	/** Start playing a sound animation. Returns an id number of this playback instance
	 *  or -1 if the animation was not found.
 	 *  \param name The name of the animation to play.
	 */
	virtual TSoundAnimPlayId		playAnimation(std::string& name, float time, NL3D::CCluster *cluster, CSoundContext &context);

	/** Stop the playing of a sound animation.
 	 *  \param name The playback id that was returned by playAnimation.
	 */
	virtual void					stopAnimation(TSoundAnimPlayId playbackId);

	/** Returns true if the animation with the specified playback ID is playing
 	 *  \param name The playback id that was returned by playAnimation.
	 */
	virtual bool					isPlaying(TSoundAnimPlayId playbackId);

	/** Update all the sound animations during playback.
	 */
	virtual void					update(float lastTime, float curTime);

	/** Play all the events of an animation in the interval between lastTime and curTime.
	 *  Both lastTime and curTime are measured relatively from the beginning of the
	 *  animation.
	 */
	virtual void					playAnimation(TSoundAnimId id, float lastTime, float curTime, NL3D::CCluster *cluster, CSoundContext &context);

	/** Convert back from an anim ID to the anim name.
	 */
	virtual std::string				idToName(TSoundAnimId id);

protected:

	/** The one and only singleton instance */
	static CSoundAnimManager*		_Instance;

	/** The mixer */
	UAudioMixer						*_Mixer;

	/** The conversion table from animation name to id */
	TSoundAnimMap					_IdMap;

	/** The vector of all defined animations */
	TSoundAnimVector				_Animations;

	/** The set of all active players */
	//TPlayerSet						_Players;

	/** The id of the next player */
	//TSoundAnimPlayId				_PlayerId;

	/** An auxilary vector to help remove players from the active set */
	//std::vector<CSoundAnimPlayer*>	_Garbage;

};

/**
 * ESoundFileNotFound
 */
/*
class ESoundAnimationNotFound : public NLMISC::Exception
{
public:
	ESoundAnimationNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sound animation not found: ")+filename).c_str() ) {}
};
*/

} // namespace NLSOUND

#endif // NL_SOUND_ANIM_MANAGER_H

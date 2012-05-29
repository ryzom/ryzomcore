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




#ifndef CL_ENTITY_ANIMATION_MANAGER_H
#define CL_ENTITY_ANIMATION_MANAGER_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
// 3d
#include "nel/3d/animation_time.h"
#include "nel/3d/u_animation.h"
#include "nel/3d/u_track.h"
// Client Sheets
#include "client_sheets/automaton_list_sheet.h"
#include "client_sheets/animation_set_list_sheet.h"
#include "client_sheets/emot_list_sheet.h"
// Client
#include "animation_set.h"
#include "animation_misc.h"
// STL
#include <string>


///////////
// USING //
///////////
using NL3D::CAnimationTime;


///////////
// CLASS //
///////////
namespace NL3D
{
	class UAnimationSet;
	class UPlayListManager;
	class UPlayList;
}

namespace NLMISC
{
	class IProgressCallback;
}

/**
 * Class with infos for each face emotions.
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2001
 */
class CFaceEmotion
{
public:
	std::map<std::string, float> _Anims;

	void addAnim(const std::string &filename, float percentage)
	{
		_Anims.insert(std::make_pair(filename, percentage));
	}
};


/**
 * Class to manage animation of entities displayed by the client
 * \author Guillaume PUZIN (GUIGUI)
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2001
 */
class CEntityAnimationManager
{
public:
	// Data tyype for Emotions for the face.
	typedef std::map<std::string, CFaceEmotion>			TFaceEmotions;

	typedef std::map<std::string, CAnimationSet>		TAnimSet;

private:
	/// the only one instance of the class
	static CEntityAnimationManager					*_Instance;

	/// playlist manager
	static NL3D::UPlayListManager					*_PlayListManager;

	/// The Animation Set with all character animations.
	NL3D::UAnimationSet								*_AnimationSet;

	/// list of animation set.
	TAnimSet										_AnimSet;

	/// Automaton used for animations
	CAutomatonListSheet								*_AutomatonList;

	/// Emots List
	CEmotListSheet									*_EmotList;
	//std::vector<TAnimStateId>						_Emots;

	/// Animation set cache
	std::vector<NL3D::UTrack*>						_AnimationSetPosCache;
	std::vector<NL3D::UTrack*>						_AnimationSetRotCache;



	/*
	/// Animation Sets for the face
	std::map<CTypeEntity, NL3D::UAnimationSet *>	_FaceAnimationSets;
	/// automaton for moving animations
	std::map<CTypeEntity, TFaceEmotions>			_FaceEmotionsPerType;
*/


	/**
	 *	Load table containing infos about face animations.
	 * \param type : table is different according to the type.
	 * \param fileName : the name of the file containing the animation infos.
	 */
//	void loadFaceAnimations(const CTypeEntity& type, const char * fileName);

protected :
	/// Constructor
	CEntityAnimationManager();

	/// Create an animation set for the Face.
//	void createFaceAnimationSet(const CTypeEntity& type);


public :
	/**
	 * Instanciate CEntityAnimationManager. There can be only one instance (singleton)
	 * \return CEntityAnimationManager * : Pointer on CEntityAnimationManager.
	 */
	static CEntityAnimationManager * getInstance();
	/// \warning If you the kept the pointer given by getInstance, it will be invalid.
	static void delInstance();

public :
	/// Destructor.
	~CEntityAnimationManager();

	/// Release
	void release();
	/// (Re-)Initialize

	void load(NLMISC::IProgressCallback &progress, bool forceRepack = false);

	/**
	 * Animate all the playlists.
	 * \param double time : play time.
	 */
	void animate(double time);
	void setup(double time);

	/**
	 * Return a ref on a state of the moving automaton defined by its key.
	 * \param string automatonName : the automaton's name.
	 * \param TAnimStateKey key : the key of the state.
	 * \return CMovingAutomatonState * : pointer on the moving automaton state or 0.
	 */
	const CAutomatonStateSheet *mState(const std::string &automaton, TAnimStateKey key);


	/**
	 * Delete a play list.
	 * \param UPlayList * pl : pointer on the play list to delete.
	 */
	void deletePlayList(NL3D::UPlayList * pl);

	/**
	 * Process all logical tracks(sound, etc..) which come along with the animation.
	 * \param string animListName : the animation list in witch to search.
	 * \param TAnimStateKey animationId : the id of the animation.
	 * \param CAnimationTime startTimeOffset :the start time in the animation play.
	 * \param CAnimationTime endTimeOffset : the end time in the animation play.
	 */
	void processLogic(const std::string &animListName, TAnimStateKey animationId, CAnimationTime startTimeOffset, CAnimationTime endTimeOffset, NL3D::UTrack *&soundTrack, std::vector<NL3D::CAnimationTime>& result);


	/** \name INFORMATION ABOUT ANIMATIONS
	 * Methods to get information about an animation.
	 */
	//@{
	/**
	 * Function to get the position in animation at timeOffset.
	 * \param idAnim : id of the animation.
	 * \param timeOffset : time for the interpolation.
	 * \param result : is the reference on the value to get the result (position).
	 * \return bool : true if the parameter result is valid.
	 */
	bool interpolate(uint idAnim, double timeOffset, NLMISC::CVector &result) const
	{
		H_AUTO ( RZ_Client_Entity_Anim_Mngr_Interpolate )

		// Check Params
		if(idAnim >= _AnimationSetPosCache.size())
		{
			//nlwarning("EAM:interpolate(vect): idAnim(%d) invalid.", idAnim);
			return false;
		}

		NL3D::UTrack *track = _AnimationSetPosCache[idAnim];
		if (track)
			return track->interpolate((CAnimationTime)timeOffset, result);
		return false;
	}
	/**
	 * Function to get the rotation in animation at timeOffset.
	 * \param idAnim : id of the animation.
	 * \param timeOffset : time for the interpolation.
	 * \param result : is the reference on the value to get the result (rotation).
	 * \return bool : true if the parameter result is valid.
	 */
	bool interpolate(uint idAnim, double timeOffset, NLMISC::CQuat &result) const
	{
		H_AUTO ( RZ_Client_Entity_Anim_Mngr_Interpolate )

		// Check Params
		if(idAnim >= _AnimationSetRotCache.size())
		{
			//nlwarning("EAM:interpolate(quat): idAnim(%d) invalid.", idAnim);
			return false;
		}

		NL3D::UTrack *track = _AnimationSetRotCache[idAnim];
		if (track)
			return track->interpolate((CAnimationTime)timeOffset, result);
		return false;
	}

	/**
	 * Return an animation length (in sec).
	 * \param string animName : Animation Name.
	 * \return double : the length of the animation.
	 * \warning This method never return a value <= 0.0 and so will return 1.0 instead.
	 * \warning This Method is slower than the one with the animation Id instead of the animation Name.
	 */
	double getAnimationLength(const std::string &animName) const;
	/**
	 * Return an animation length (in sec).
	 * \param idAnim : id of the animation.
	 * \return double : the length of the animation.
	 * \warning This method never return a value <= 0.0 and so will return 1.0 instead.
	 */
	double getAnimationLength(uint idAnim)const;

	/**
 	 * Get the average speed of an animation (in meters/sec).
	 * \param string animName : Animation Name.
	 * \return double : the average speed (in m/s).
	 */
	double getAnimationAverageSpeed(const std::string &animName) const;
	/**
 	 * Get the average speed of an animation (in meters/sec).
	 * \param idAnim : id of the animation.
	 * \return double : the average speed (in m/s).
	 */
	double getAnimationAverageSpeed(uint idAnim) const;
	//@}


	/**
	 * Get the minimum speed factor that can be used to play the animation.
	 * \param animSet : Set of animations used.
	 * \param animStateId : the animation State.
	 * \param idAnim : the id of the animation.
	 * \return double : the minimum speed factor or -1 if any pb.
	 */
	double getAnimMinSpeedFactor(const std::string &animSet, const TAnimStateId &animStateId, const CAnimation::TAnimId &idAnim) const;

	/**
	 * Get the maximum speed factor that can be used to play the animation
	 * \param animSet : Set of animations used.
	 * \param animStateId : the animation State.
	 * \param idAnim : the id of the animation.
	 * \return double : the maximum speed factor or -1 if any pb.
	 */
	double getAnimMaxSpeedFactor(const std::string &animSet, const TAnimStateId &animStateId, const CAnimation::TAnimId &idAnim) const;


	/**
	 * Return a pointer on the set according to the set name.
	 * \param animSet : name of the set.
	 * \return CAnimationSet * : pointer of the right Set or 0.
	 */
	const CAnimationSet *getAnimSet(const std::string &animSet) const;


	/**
	 * Get the animation set.
	 * \return UAnimationSet * : a pointer on the animation set or 0 if there is not animation set.
	 */
	NL3D::UAnimationSet * getAnimationSet() {return _AnimationSet;}

	/**
	 * Create a playlist.
	 * \return UPlayList * : a pointer on a play list or 0 if any pb.
	 */
	NL3D::UPlayList *createPlayList() const;


	/// Serialize a CEntityAnimationManager.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/** Count the number of emot
	 * \return uint : the number of emot already known.
	 */
	uint getNbEmots() {return (uint)_EmotList->Emots.size();}
	/** Method to get the emot associated to an index.
	 * \param index : number of the emot asked.
	 * \param result : will be filled with the name of the emot associated.
	 * \return bool : true if the result has been filled, false if the index is invalid.
	 */
	bool getEmot(uint index, TAnimStateId &result)
	{
		if(index < _EmotList->Emots.size())
		{
			result = _EmotList->Emots[index];
			return true;
		}
		else
			return false;
	}

	/// For Reload Sound feature
	void	resetAllSoundAnimId();
	void	reloadAllSoundAnim();

	/**
	 * Create an uninitialized playlist for the face animations.
	 * \param type : type of the play list to create.
	 * \return UPlayList * : a pointer on an initialized play list.
	 */
//	NL3D::UPlayList *CEntityAnimationManager::createFacePlayList(const CTypeEntity& type);

	/**
	 * Get an animation set for face.
	 * \param type : type of the animation set you want.
	 * \return UAnimationSet * : a pointer on the animation set or 0 if there is not animation set for this type.
	 */
//	NL3D::UAnimationSet * getFaceAnimationSet(const CTypeEntity& type);

	/**
	 *	Choose an animation for the face according to the type and the emotion.
	 * \param type : type of the face.
	 * \param emotion : emotion to play.
	 * \return uint : the index of the animation.
	 */
//	uint chooseFaceAnim(const CTypeEntity& type, const std::string &emotion);
};



#endif // CL_ENTITY_ANIMATION_MANAGER_H

/* End of entity_animation_manager.h */


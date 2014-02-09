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


/////////////
// INCLUDE //
/////////////
// client
#include "entity_animation_manager.h"
#include "animation_misc.h"
#include "animation_set.h"
#include "debug_client.h"
#include "sheet_manager.h"
#include "time_client.h"
#include "global.h"
// misc
#include "nel/misc/path.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/file.h"
#include "nel/misc/progress_callback.h"
// 3d
#include "nel/3d/u_skeleton.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_track.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_play_list_manager.h"
#include "nel/3d/u_animation_set.h"

///////////
// USING //
///////////
using namespace NLMISC;
using namespace std;
using namespace NLGEORGES;


////////////
// EXTERN //
////////////
extern NL3D::UScene * Scene;

////////////
// GLOBAL //
////////////

// Hierarchical timer
H_AUTO_DECL ( RZ_Client_Entity_Animation_Mngr )

////////////////////
// STATIC MEMBERS //
////////////////////
CEntityAnimationManager	*CEntityAnimationManager::_Instance			= 0;
NL3D::UPlayListManager	*CEntityAnimationManager::_PlayListManager	= 0;


////////////////////
// STATIC METHODS //
////////////////////
//---------------------------------------------------
// getInstance :
// Instanciate CEntityAnimationManager. There can be only one instance (singleton)
// \return CEntityAnimationManager * : Pointer on CEntityAnimationManager.
//---------------------------------------------------
CEntityAnimationManager * CEntityAnimationManager::getInstance()
{
	if(_Instance == 0)
	{
		_Instance = new CEntityAnimationManager();
		if(Scene)
			_PlayListManager = Scene->createPlayListManager();
		else
			nlwarning("CEntityAnimationManager::getInstance : Scene is not allocated.");
	}
	return _Instance;
}// instance //

//-----------------------------------------------
// delInstance :
// Release the current instance
// \warning If you the kept the pointer given by getInstance, it will be invalid.
//-----------------------------------------------
void CEntityAnimationManager::delInstance()
{
	// Release the singleton
	if(_Instance)
	{
		delete _Instance;
		_Instance = 0;
	}
}// delInstance //


/////////////
// METHODS //
/////////////
//---------------------------------------------------
// CEntityAnimationManager :
// Constructor.
//---------------------------------------------------
CEntityAnimationManager::CEntityAnimationManager()
{
	_AnimationSet = NULL;
	_AutomatonList = NULL;
	_EmotList = NULL;

}// CEntityAnimationManager //

//---------------------------------------------------
// ~CEntityAnimationManager :
// Destructor.
//---------------------------------------------------
CEntityAnimationManager::~CEntityAnimationManager()
{
	// Delete the playlist manager
	if(_PlayListManager)
	{
//		if(Scene)
//			Scene->deletePlayListManager(_PlayListManager);
		_PlayListManager = 0;
	}

	// Release all things initialized by the load method.
	release();
}// ~CEntityAnimationManager //

//-----------------------------------------------
// release
// Release all things initialized by the load method.
//-----------------------------------------------
void CEntityAnimationManager::release()
{
	// Animation set cache
	_AnimationSetPosCache.clear();
	_AnimationSetRotCache.clear();
	// Release animsets.
	_AnimSet.clear();
	// Release Automatons and Release the emots list
	// Nothing to release here : all is in the sheet manager so kept during all the time the game is running
	// Release the AnimationSet.
	if(_AnimationSet)
	{
		Driver->deleteAnimationSet(_AnimationSet);
		_AnimationSet = 0;
	}

	// owned by CSheetManager
	_AutomatonList = NULL;

}// release //

//---------------------------------------------------
// load :
//---------------------------------------------------
void CEntityAnimationManager::load(NLMISC::IProgressCallback &/* progress */, bool /* forceRepack */ /* = false */)
{
	// Whole initInGame profile
	NLMISC::TTime initStart;
	initStart = ryzomGetLocalTime ();

	// Before loading, release the old stuff.
	release();

	// Log in the animation file.
	//setDebugOutput(getLogDirectory() + "animation.dbg");
	setDebugOutput("");	// no log

	// Create the animation set for all entities.
	_AnimationSet = Driver->createAnimationSet();
	if(_AnimationSet == 0)
		pushDebugStr("_AnimationSet is Null !.");

	// Loading Emots
	pushInfoStr("COMPUTING EMOTS ...");
	// Initialize the emot list
	_EmotList = dynamic_cast<CEmotListSheet*>(SheetMngr.get(CSheetId("list.emot")));
	nlassert(_EmotList != NULL);

	pushInfoStr("COMPUTING AUTOMATON ...");
	// Initialize the automaton.
	_AutomatonList = dynamic_cast<CAutomatonListSheet*>(SheetMngr.get(CSheetId("automaton.automaton_list")));
	nlassert(_AutomatonList != NULL);

	pushInfoStr("COMPUTING ANIMATION SET ...");
	// Initialize the list of animsets
	CAnimationSetListSheet *pASLS =  dynamic_cast<CAnimationSetListSheet*>(SheetMngr.get(CSheetId("entities.animset_list")));
	nlassert(pASLS != NULL);
	for (uint32 i = 0; i < pASLS->AnimSetList.size(); ++i)
	{
		CAnimationSet as;
		string sTmp = strlwr(pASLS->AnimSetList[i].Name);
		sTmp = sTmp.substr(0,sTmp.rfind('.'));
		pair<map<string,CAnimationSet>::iterator, bool> it;
		it = _AnimSet.insert(pair<string,CAnimationSet>(sTmp,as));
		it.first->second.init (&pASLS->AnimSetList[i], _AnimationSet);
	}

	// Build 3d Animation Set (called after all animations added in the animation set).
	_AnimationSetPosCache.resize (0, NULL);
	_AnimationSetRotCache.resize (0, NULL);
	if(_AnimationSet)
	{
		_AnimationSet->build();

		// Build animation set cache
		uint i;
		const uint count = _AnimationSet->getNumAnimation ();
		_AnimationSetPosCache.resize (count, NULL);
		_AnimationSetRotCache.resize (count, NULL);
		for (i=0; i<count; i++)
		{
			// Get the animation
			NL3D::UAnimation *anim = _AnimationSet->getAnimation (i);
			if (anim)
			{
				// Get the tracks
				_AnimationSetPosCache[i] = anim->getTrackByName("pos");
				_AnimationSetRotCache[i] = anim->getTrackByName("rotquat");
			}
		}
	}

	// Flush debug information
	flushDebugStack("-- ANIMATIONS OTHER --");

	// Stop to Log in the animation file.
	setDebugOutput("");

	// Whole load profile
	nlinfo ("%d seconds for EAM->load()", (uint32)(ryzomGetLocalTime ()-initStart)/1000);

/*	// Memory debug
	uint32 nTotalAnimSet = _AnimSet.size();
	TAnimSet::iterator it = _AnimSet.begin();
	uint32 nTotalAnimState = 0;
	uint32 nTotalAnim = 0;
	uint32 nTotalAnimFx = 0;
	bool bOnce = false;
	while (it != _AnimSet.end())
	{
		CAnimationSet &rAS = it->second;
		uint32 nAS = rAS.getNumAnimationState();
		nTotalAnimState += nAS;
		for (uint i = 0; i < nAS; ++i)
		{
			CAnimationState *pAnimState = rAS.getAnimationStateByIndex(i);
			uint32 nAnim = pAnimState->getNumAnimation();
			nTotalAnim += nAnim;
			for (uint j = 0; j < nAnim; ++j)
			{
				CAnimation *pAnim = pAnimState->getAnimationByIndex(j);
				nTotalAnimFx += pAnim->getFXSet().FX.size();
				if (!bOnce)
				{
					bOnce = true;
					nlinfo ("%d %d %d", sizeof(rAS), sizeof(*pAnimState), sizeof(*pAnim));
				}
			}
		}

		it++;
	}

	nlinfo ("%d %d %d", nTotalAnimSet, nTotalAnimState, nTotalAnim, nTotalAnimFx);
*/
}// load //


//---------------------------------------------------
// animate :
// Animate all the playlists.
// \param double time : play time.
//---------------------------------------------------
void CEntityAnimationManager::animate(double time)
{
	H_AUTO_USE ( RZ_Client_Entity_Animation_Mngr )

	_PlayListManager->animate(time);
}// animate //


//---------------------------------------------------
// Setup :
// Setup all the playlists.
// \param double time : play time.
//---------------------------------------------------
void CEntityAnimationManager::setup(double time)
{
	H_AUTO_USE ( RZ_Client_Entity_Animation_Mngr )

	if (_PlayListManager) _PlayListManager->setup(time);
}// setup //


//---------------------------------------------------
// mState :
// Return a ref on a state of the moving automaton defined by its key.
// \param string automatonName : the automaton's name.
// \param TAnimStateKey key : the key of the state.
// \return CMovingAutomatonState * : pointer .
//---------------------------------------------------
const CAutomatonStateSheet *CEntityAnimationManager::mState(const string &automaton, TAnimStateKey key)
{
	// Search for the automaton
	std::map<std::string, CAutomatonSheet *>::iterator it = _AutomatonList->Automatons.find(automaton);
	if(it != _AutomatonList->Automatons.end())
	{
		// Get the state.
		const CAutomatonStateSheet *statePtr = ((*it).second)->state(key);
		if(statePtr == 0)
		{
			nlwarning("CEntityAnimationManager::mState: the state '%s' does not exist for the automaton '%s'", CAnimationState::getAnimationStateName(key).c_str(), automaton.c_str());
			return 0;
		}
		else
			return statePtr;
	}
	// Automaton not found
	else
		return 0;
}// mState //

//---------------------------------------------------
// deletePlayList:
// Delete a play list.
// \param UPlayList * pl : pointer on the play list to delete.
//---------------------------------------------------
void CEntityAnimationManager::deletePlayList(NL3D::UPlayList * pl)
{
	// Check if the playlist is not Null.
	if(pl)
	{
		_PlayListManager->deletePlayList(pl);
		pl = 0;
	}
}// deletePlayList //

/*
//---------------------------------------------------
// processLogic :
//
//---------------------------------------------------
void CEntityAnimationManager::processLogic(const std::string &animListName, TAnimStateKey animationId, CAnimationTime startTimeOffset, CAnimationTime endTimeOffset, UTrack *&soundTrack, vector<CAnimationTime>& result)
{
	result.clear();

	// Get the animation set according to the animation list name
	UAnimationSet *animSet = getAnimationSet(animListName);
	if(animSet)
	{
		// Get the animation Id.
		uint idAnim = animSet->getAnimationIdByName(animationId);
		if(idAnim != UAnimationSet::NotFound)
		{
			// Get the animation pointer and get the length.
			UAnimation *anim = animSet->getAnimation(idAnim);
			if(anim)
			{
				char *soundTrackName = "NoteTrack";
				soundTrack = anim->getTrackByName( soundTrackName );
				if( soundTrack != NULL )
				{
					UTrackKeyframer * soundTrackKF = dynamic_cast<UTrackKeyframer *>(soundTrack);
					if( soundTrackKF == NULL )
					{
						nlerror("The track %s is not a key framer track",soundTrackName);
					}

					soundTrackKF->getKeysInRange(startTimeOffset, endTimeOffset, result);
				}
			}
		}
	}
}// processLogic //
*/






//---------------------------------------------------
// getAnimationLength :
// Return an animation length (in sec).
// \param string animName : Animation Name.
// \return double : the length of the animation.
// \warning This method never return a value <= 0.0 and so will return 1.0 instead.
// \warning This Method is slower than the one with the animation Id instead of the animation Name.
//---------------------------------------------------
double CEntityAnimationManager::getAnimationLength(const std::string &animName) const
{
	double length = CAnimationMisc::getAnimationLength(_AnimationSet, animName);
	if(length <= 0.0)
		return 1.0;

	return length;
}// getAnimationLength //

//---------------------------------------------------
// getAnimationLength :
// Return an animation length (in sec).
// \param idAnim : id of the animation.
// \return double : the length of the animation.
// \warning This method never return a value <= 0.0 and so will return 1.0 instead.
// \warning This Method is slower than the one with the animation Id instead of the animation Name.
//---------------------------------------------------
double CEntityAnimationManager::getAnimationLength(uint idAnim) const
{
	if (idAnim == NL3D::UAnimationSet::NotFound) return 0.f;
	double length = CAnimationMisc::getAnimationLength(_AnimationSet, idAnim);
	if(length <= 0.0)
		return 1.0;

	return length;
}// getAnimationLength //


//---------------------------------------------------
// getAnimationAverageSpeed :
// Get the average speed of an animation (in meters/sec).
// \param string animName : Animation Name.
// \return double : the average speed (in m/s).
//---------------------------------------------------
double CEntityAnimationManager::getAnimationAverageSpeed(const std::string &animName) const
{
	return CAnimationMisc::getAnimationAverageSpeed(_AnimationSet, animName);
}// getAnimationAverageSpeed //

//---------------------------------------------------
// getAnimationAverageSpeed :
// Get the average speed of an animation (in meters/sec).
// \param string animName : Animation Name.
// \return double : the average speed (in m/s).
//---------------------------------------------------
double CEntityAnimationManager::getAnimationAverageSpeed(uint idAnim) const
{
	return CAnimationMisc::getAnimationAverageSpeed(_AnimationSet, idAnim);
}// getAnimationAverageSpeed //


//---------------------------------------------------
// getAnimSet :
// Return a pointer on the set according to the set name.
// \param animSet : name of the set.
// \return CAnimationSet * : pointer of the right Set or 0.
//---------------------------------------------------
const CAnimationSet *CEntityAnimationManager::getAnimSet(const std::string &animSet) const
{
	// Search the right animation set.
	TAnimSet::const_iterator itAnimSet = _AnimSet.find(animSet);
	if(itAnimSet == _AnimSet.end())
		// Set not found.
		return 0;

	// Return the pointer
	return &((*itAnimSet).second);
}// getAnimSet //


//---------------------------------------------------
// createPlayList :
// Create a playlist.
// \return UPlayList * : a pointer on a play list or 0 if any pb.
//---------------------------------------------------
NL3D::UPlayList *CEntityAnimationManager::createPlayList() const
{
	if (_PlayListManager == 0)
		return 0;

	return _PlayListManager->createPlayList(_AnimationSet);
}// createPlayList //



//-----------------------------------------------
// serial :
// Serialize a CEntityAnimationManager.
//-----------------------------------------------
void CEntityAnimationManager::serial(class NLMISC::IStream &/* f */) throw(NLMISC::EStream)
{
}// serial //


// ***************************************************************************
void	CEntityAnimationManager::resetAllSoundAnimId()
{
	TAnimSet::iterator	it(_AnimSet.begin()), last(_AnimSet.end());
	for(;it!=last;it++)
	{
		CAnimationSet	&animSet= it->second;
		uint	numAnimState= animSet.getNumAnimationState();
		for(uint i=0;i<numAnimState;i++)
		{
			CAnimationState	*state= animSet.getAnimationStateByIndex(i);
			if(state)
			{
				uint	numAnim= state->getNumAnimation();
				for(uint j=0;j<numAnim;j++)
				{
					CAnimation *anim= state->getAnimationByIndex(j);
					if(anim)
						anim->resetSoundAnim();
				}
			}
		}
	}
}

// ***************************************************************************
void	CEntityAnimationManager::reloadAllSoundAnim()
{
	TAnimSet::iterator	it(_AnimSet.begin()), last(_AnimSet.end());
	for(;it!=last;it++)
	{
		CAnimationSet	&animSet= it->second;
		uint	numAnimState= animSet.getNumAnimationState();
		for(uint i=0;i<numAnimState;i++)
		{
			CAnimationState	*state= animSet.getAnimationStateByIndex(i);
			if(state)
			{
				uint	numAnim= state->getNumAnimation();
				for(uint j=0;j<numAnim;j++)
				{
					CAnimation *anim= state->getAnimationByIndex(j);
					if(anim)
						anim->reloadSoundAnim();
				}
			}
		}
	}
}



// \todo GUIGUI : A REFAIRE.
/*
//---------------------------------------------------
// createFacePlayList :
// Create an uninitialized playlist for the face animations.
// \param type : type of the play list to create.
// \return UPlayList * : a pointer on an initialized play list.
//---------------------------------------------------
UPlayList * CEntityAnimationManager::createFacePlayList(const CTypeEntity& type)
{
	// get the animation set
	UAnimationSet	*animationSet	= getFaceAnimationSet(type);
	if(animationSet)
		return _PlayListManager->createPlayList(animationSet);
	else
		return 0;
}// createFacePlayList //

//---------------------------------------------------
// createFaceAnimationSet :
// Create an animation set for the Face.
//---------------------------------------------------
void CEntityAnimationManager::createFaceAnimationSet(const CTypeEntity& type)
{
	UAnimationSet * faceAnimationSet = Driver->createAnimationSet();
	if(faceAnimationSet == 0)
		nlwarning("CEntityAnimationManager::createFaceAnimationSet : cannot create an animation set for the Face.");
	// Insert the new animation set in the map
	else
		_FaceAnimationSets.insert(make_pair(type, faceAnimationSet));
}// createFaceAnimationSet //


//---------------------------------------------------
// getFaceAnimationSet :
// Get an animation set for face.
// \param type : type of the animation set you want.
// \return UAnimationSet * : a pointer on the animation set or 0 if there is not animation set for this type.
//---------------------------------------------------
UAnimationSet * CEntityAnimationManager::getFaceAnimationSet(const CTypeEntity& type)
{
	map<CTypeEntity, UAnimationSet *>::iterator itAnimSet = _FaceAnimationSets.find(type);
	if(itAnimSet == _FaceAnimationSets.end())
		return 0;
	else
		return (*itAnimSet).second;
}// getFaceAnimationSet //

//---------------------------------------------------
// chooseFaceAnim :
// Choose an animation for the face according to the type and the emotion.
// \param type : type of the face.
// \param emotion : emotion to play.
// \return uint : the index of the animation.
// \todo GUIGUI : make a real choice according to the percentage.
//---------------------------------------------------
uint CEntityAnimationManager::chooseFaceAnim(const CTypeEntity& type, const string &emotion)
{
	std::map<CTypeEntity, TFaceEmotions>::iterator itFEPT = _FaceEmotionsPerType.find(type);
	if(itFEPT == _FaceEmotionsPerType.end())
	{
		nlwarning("CEntityAnimationManager::chooseFaceAnim : Nothing associated to the type.");
		return UAnimationSet::NotFound;
	}

	TFaceEmotions &faceEmotions = (*itFEPT).second;
	TFaceEmotions::iterator itFE = faceEmotions.find(emotion);
	if(itFE == faceEmotions.end())
	{
		nlwarning("CEntityAnimationManager::chooseFaceAnim : Face Emotion '%s' does not exist for this type.", emotion.c_str());
		return UAnimationSet::NotFound;
	}

	CFaceEmotion &faceEmotion = (*itFE).second;
	if(faceEmotion._Anims.empty())
	{
		nlwarning("No animation in the Emotion '%s'.", emotion.c_str());
		return UAnimationSet::NotFound;
	}

	string anim = (*faceEmotion._Anims.begin()).first;
	UAnimationSet *animSet = getFaceAnimationSet(type);
	if(animSet)
		return animSet->getAnimationIdByName(anim);
	else
		return UAnimationSet::NotFound;
}// chooseFaceAnim //

//-----------------------------------------------
// loadFaceAnimations :
// Load table containing infos about face animations.
// \param type : table is different according to the type.
// \param fileName : the name of the file containing the animation infos.
// \todo GUIGUI : Check that we can add the same animation more than once with no problem or do i have to check if the animation is already in.
//-----------------------------------------------
void CEntityAnimationManager::loadFaceAnimations(const CTypeEntity& type, const char * fileName)
{
	// Get the animation set.
	UAnimationSet * animationSet = getFaceAnimationSet(type);
	if(!animationSet)
	{
		nlwarning("CEntityAnimationManager::loadFaceAnimations : animationSet is Null for the type -> Face Animations not loaded.");
		return;
	}

	// Create Face Emotions struct.
	TFaceEmotions faceEmotions;

	// CST loader
	CSTLoader cstl;

	// Build file format
	map<string, CSTLoader::TDataType> fileFormat;
	fileFormat.insert(make_pair(string("name"),			CSTLoader::STRING));
	fileFormat.insert(make_pair(string("filename"),		CSTLoader::STRING));
	fileFormat.insert(make_pair(string("percentage"),	CSTLoader::FLOAT));

	// Init loader
	cstl.init(fileName, fileFormat);
	// Read the file line per line.
	while(cstl.readLine())
	{
		// Get the name of the emotion.
		string	emotionName;
		string	animFilename;
		float	percentage;
		cstl.getStringValue	("name",		emotionName);
		cstl.getStringValue	("filename",	animFilename);
		cstl.getValue		("percentage",	percentage);

		// Try to add the anim in the animation set.
		if(animationSet->addAnimation((animFilename + ".anim").c_str(), animFilename.c_str()) == UAnimationSet::NotFound)
			nlwarning("CEntityAnimationManager::loadFaceAnimations : anim '%s' is not found.", (animFilename + ".anim").c_str());
		// The animation is right added in the set -> add anim in the right face emotion.
		else
		{
			// Attempt on find this name in the map.
			map<string, CFaceEmotion>::iterator itFaceEmotion = faceEmotions.find(emotionName);
			// This emotion already exist -> add new animation to the emotion.
			if(itFaceEmotion != faceEmotions.end())
			{
				CFaceEmotion &faceEmotion = (*itFaceEmotion).second;
				faceEmotion.addAnim(animFilename, percentage);
			}
			else
			{
				CFaceEmotion faceEmotion;
				faceEmotion.addAnim(animFilename, percentage);
				faceEmotions.insert(make_pair(emotionName, faceEmotion));
			}
		}
	}

	// Close file
	cstl.close();

	// Add Emotions for the type.
	_FaceEmotionsPerType.insert(make_pair(type, faceEmotions));
}// loadFaceAnimations //
*/











































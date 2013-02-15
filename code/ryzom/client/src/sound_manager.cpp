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


//////////////
// INCLUDES //
//////////////
// client
#include "sound_manager.h"
#include "pacs_client.h"
#include "light_cycle_manager.h"
#include "weather_manager_client.h"
#include "user_entity.h"
#include "entities.h"
#include "time_client.h"
#include "weather.h"
#include "global.h"
// game share
#include "game_share/cst_loader.h"

// If you compile using the nel distrib and you don't have the source, you must undef this symbol
#if !FINAL_VERSION
# define DEBUG_SOUND_IN_GAME
#endif

#if defined(DEBUG_SOUND_IN_GAME)
//sound
# include "nel/sound/clustered_sound.h"
# include "nel/sound/audio_mixer_user.h"
// 3d
# include "nel/3d/cluster.h"
# include "nel/3d/portal.h"
# include "nel/3d/dru.h"
# include "nel/3d/material.h"
# include "nel/3d/driver.h"
# include "nel/3d/driver_user.h"
# include "view.h"
#endif

extern CLightCycleManager	LightCycleManager;
extern NL3D::UDriver		*Driver;
extern NL3D::UCamera		MainCam;
extern NLLIGO::CLigoConfig	LigoConfig;

//////////////
// USING    //
//////////////
using namespace NLMISC;
using namespace NLPACS;
using namespace NLSOUND;
using namespace std;
using namespace NL3D;


extern NL3D::UScene		*Scene;

// The interpolation distance for the portals : 1 m
const float PORTAL_INTERPOLATE = 1.0f;

// Filter mapping
enum TFilterMapping
{
	DAY = 0,
	NIGHT = 1,
	MORNING = 2,
	DUSK = 3,
	SPRING = 4,
	SUMMER = 5,
	AUTUMN = 6,
	WINTER = 7,
	MIST = 8,
	FAIR1 = 9,
	FAIR2 = 10,
	FAIR3 = 11,
	CLOUDS = 12,
	RAIN1 = 13,
	RAIN2 = 14,
	SNOW1 = 15,
	THUNDER1 = 16,
	THUNDERSEVE1 = 17,
	THUNDERSAND1 = 18,
	THUNDERSAND2 = 19,
	THUNDERSAND3 = 20,
	BAD = 21,
	GOOD = 22,
	QUICK_MUTE = 23
};

//-----------------------------------------------
// constructor
//-----------------------------------------------
CSoundManager::CSoundManager(IProgressCallback * /* progressCallBack */)
:	_AudioMixer(NULL), 
	_GroupControllerEffects(NULL),
	_GroupControllerEffectsGame(NULL),  
	_EnvSoundRoot(NULL), 
	_Sources(NULL), 
	_UserEntitySoundLevel(1.0f)
{
	_EnableBackgroundMusicAtTime= 0;
	_GameMusicVolume= 1.f;
	_UserMusicVolume= 1.f;
	_FadeGameMusicVolume= 1.f;
	_FadeSFXVolume= 1.f;
	_UseGameMusicVolume= true;
	_EventMusicEnabled= true;
	_EventMusicLoop= false;
	_FadeGameMusicVolumeDueToEvent= 1.f;
//	init(progressCallBack);
} // CSoundManager




//-----------------------------------------------
// Destructor
//-----------------------------------------------
CSoundManager::~CSoundManager()
{
	_Sources.clear();

	_AttachedSources.clear(); // attached sources already deleted (they are also in the _Sources map)

	// detach the sound from the particule system
	NL3D::UParticleSystemSound::setPSSound(NULL);

	_GroupControllerEffects = NULL;
	_GroupControllerEffectsGame = NULL;

	// free the audio mixer (and delete all sources)
	delete _AudioMixer;
	_AudioMixer = NULL;

	// release sound anim properly
	releaseSoundAnim();

} // ~CSoundManager //


// ***************************************************************************
void	CSoundManager::releaseSoundAnim()
{
	// Parse all Entities to reset their current sound id
	EntitiesMngr.resetAllSoundAnimId();

	// Parse all animation to reset their Id
	if(EAM)
		EAM->resetAllSoundAnimId();

	// Delete the sound anim Singleton
	CSoundAnimManager::release();
}



// ***************************************************************************
void CSoundManager::drawSounds(float camHeight)
{
#if defined(DEBUG_SOUND_IN_GAME)
	// prepare the cameras for cluster display
	// draw the cluster bounding box on screen

	CMatrix	newCam;

	// get the user pos
	CVector pos(View.viewPos());
	pos += CVector(0,0,camHeight);

	// get the view vector ?
	CVector lookAt(View.view());
	lookAt.z = 0;
	lookAt.normalize();


	if (lookAt.y <0)
		newCam.rotateZ(float(Pi+/*-Pi/2+*/asin(lookAt.x)));
	else
		newCam.rotateZ(float(Pi+/*-Pi/2+*/Pi-asin(lookAt.x)));

	newCam.rotateX(float(-Pi/2));


	newCam.setPos(pos);


	Driver->setMatrixMode3D(MainCam);
	Driver->setViewMatrix(newCam.inverted());
	Driver->clearZBuffer();

	CMatrix model;
	model.identity();
	model.translate(-pos);
//	model.rotateX(mapRX);
//	model.rotateY(mapRY);
	model.translate(pos);


	Driver->setModelMatrix(model);

//	UAudioMixer *amu = UAudioMixer::instance();

	IDriver *idriver = static_cast<CDriverUser*>(Driver)->getDriver();
	CClusteredSound *cs = static_cast<CAudioMixerUser*>(_AudioMixer)->getClusteredSound();
	// Draw the audible clusters
	{
		CMaterial	mat;
		CMaterial	mat2;

		CClusteredSound::TClusterStatusMap::const_iterator first(cs->getAudibleClusters().begin()), last(cs->getAudibleClusters().end());
		for (; first != last; ++first)
		{
			NL3D::CCluster *cluster = first->first;

			const CAABBox &box = cluster->getBBox();
			CVector center(box.getCenter());
			CVector size(box.getHalfSize());

			CVector a(center+size), b(center-size);
			CVector s[8];
			s[0].set(a.x, a.y, a.z);
			s[1].set(a.x, b.y, a.z);
			s[2].set(b.x, b.y, a.z);
			s[3].set(b.x, a.y, a.z);
			s[4].set(a.x, a.y, b.z);
			s[5].set(a.x, b.y, b.z);
			s[6].set(b.x, b.y, b.z);
			s[7].set(b.x, a.y, b.z);

			CLine lines[12] =
			{
				CLine(s[0], s[1]),
				CLine(s[1], s[2]),
				CLine(s[2], s[3]),
				CLine(s[3], s[0]),
				CLine(s[4], s[5]),
				CLine(s[5], s[6]),
				CLine(s[6], s[7]),
				CLine(s[7], s[4]),
				CLine(s[0], s[4]),
				CLine(s[1], s[5]),
				CLine(s[2], s[6]),
				CLine(s[3], s[7]),
			};

			mat.setColor(CRGBA(255,0,0,255));
			mat.setZFunc(CMaterial::less);
			NL3D::CDRU::drawLinesUnlit(lines, 12, mat, *idriver);
			mat.setColor(CRGBA(80,0,0,255));
			mat.setZFunc(CMaterial::greater);
			NL3D::CDRU::drawLinesUnlit(lines, 12, mat, *idriver);


			mat2.setColor(CRGBA(0,0,255,50));
			mat2.setZFunc(CMaterial:: always);
			mat2.setSrcBlend(CMaterial::srcalpha);
			mat2.setDstBlend(CMaterial::one);
			mat2.setBlend(true);
			mat.setColor(CRGBA(0,0,255,255));
			mat.setZFunc(CMaterial::always);

			// draw the portals
			for (uint k = 0; k<cluster->getNbPortals(); ++k)
			{
				CPortal *portal = cluster->getPortal(k);
				std::vector<NLMISC::CVector> poly;
				portal->getPoly(poly);

				if (poly.size() > 2)
				{
					for (uint i=1; i<poly.size()-1; ++i)
					{
						CTriangleUV	tri;
						tri.V0 = poly[0];
						tri.V1 = poly[i];
						tri.V2 = poly[i+1];
						NL3D::CDRU::drawTrianglesUnlit(&tri, 1, mat2, *idriver);
						tri.V0 = poly[0];
						tri.V1 = poly[i+1];
						tri.V2 = poly[i];
						NL3D::CDRU::drawTrianglesUnlit(&tri, 1, mat2, *idriver);
					}
				}

				{
					for (uint i=0; i<poly.size(); ++i)
					{
						CLine line(poly[i], poly[(i+1)%poly.size()]);

						NL3D::CDRU::drawLinesUnlit(&line, 1, mat, *idriver);
					}
				}
			}
		}
	} // draw audible clusters

	CMaterial mat;
	mat.setColor(CRGBA(255,255,255,255));

/*	float d = float(Pi/10);
	for (float j=0; j<float(Pi*2); j+=d)
		NL3D::CDRU::drawLine(pos+CVector(LISTEN_DISTANCE*float(sin(j)), LISTEN_DISTANCE*float(cos(j)), 0), pos+CVector(LISTEN_DISTANCE*float(sin(j+d)), LISTEN_DISTANCE*float(cos(j+d)),0),  CRGBA(255,255,255,255), *idriver);

	for (j=0; j<float(Pi*2); j+=d)
		NL3D::CDRU::drawLine(pos+CVector(PORTAL_INTERPOLATE*float(sin(j)), PORTAL_INTERPOLATE*float(cos(j)), 0), pos+CVector(PORTAL_INTERPOLATE*float(sin(j+d)), PORTAL_INTERPOLATE*float(cos(j+d)),0),  CRGBA(255,255,255,255), *idriver);
*/
	mat.setColor(CRGBA(0,255,0,255));
	mat.setZFunc(CMaterial::always);
/*	for (uint k=0; k<debugLines.size(); ++k)
	{
//				CDRU::drawLinesUnlit(debugLines, mat, *idriver);
		NL3D::CDRU::drawLine(debugLines[k].V0, debugLines[k].V1, CRGBA(0,255,0,255), *idriver);
	}
*/	// Draw listener pos
	mat.setColor(CRGBA(0,255,0,255));
	{
		NL3D::CDRU::drawLine(pos+CVector(2,2,0), pos+CVector(-2,-2,0), CRGBA(255,255,0,255), *idriver);
		NL3D::CDRU::drawLine(pos+CVector(-2,2,0), pos+CVector(2,-2,0), CRGBA(255,255,0,255), *idriver);

//				CVector front = Camera->getMatrix().getI();
//		NL3D::CDRU::drawLine(pos, pos+front*4, CRGBA(255,255,0,255), *idriver);
	}
	// draw the virtual sound sound
	{
		CClusteredSound::TClusterStatusMap::const_iterator first(cs->getAudibleClusters().begin()), last(cs->getAudibleClusters().end());
		for (; first != last; ++first )
		{
			const CClusteredSound::CClusterSoundStatus &css = first->second;
			if (css.Direction != CVector::Null)
			{
				CVector dest = pos+css.Direction*css.Dist;

				NL3D::CDRU::drawLine(pos, dest, CRGBA(0,255,255,255), *idriver);
				NL3D::CDRU::drawLine(dest+CVector(0.5f,0.5f,0), dest+CVector(-0.5f,-0.5f,0), CRGBA(0, 255,255,255), *idriver);
				NL3D::CDRU::drawLine(dest+CVector(-0.5f,0.5f,0), dest+CVector(0.5f,-0.5f,0), CRGBA(0, 255,255,255), *idriver);
			}
		}
	}
	// draw the audio path
	{
		idriver->setupMaterial(mat);
		const std::vector<std::pair<NLMISC::CVector, NLMISC::CVector> > &lines = cs->getAudioPath();
		std::vector<std::pair<NLMISC::CVector, NLMISC::CVector> >::const_iterator first(lines.begin()), last(lines.end());
		for (; first != last; ++first)
		{
			NL3D::CDRU::drawLine(first->first, first->second, CRGBA(0,255,0,255), *idriver);
		}
	}
	// draw the sound source position
	{
		std::vector<std::pair<bool, CVector> > soundPos;
		_AudioMixer->getPlayingSoundsPos(true, soundPos);

		std::vector<std::pair<bool, CVector> >::iterator first(soundPos.begin()), last(soundPos.end());
		for (; first != last; ++first)
		{
			NL3D::CDRU::drawLine(first->second + CVector(0.5f,0.5f,0), first->second + CVector(-0.5f,-0.5f,0), CRGBA(255,0,255,255), *idriver);
			NL3D::CDRU::drawLine(first->second + CVector(0.5f,-0.5f,0), first->second + CVector(-0.5f,0.5f,0), CRGBA(255,0,255,255), *idriver);
		}
	}

#endif // DEBUG_SOUND_IN_GAME
}



UAudioMixer *CSoundManager::getMixer()
{
	return _AudioMixer;
}


void CSoundManager::switchSoundState ()
{
	_PlaySound = !_PlaySound;
	_AudioMixer->enable(_PlaySound);
}

void CSoundManager::loadContinent(const string &name, const NLMISC::CVector& pos)
{
	if (!_AudioMixer)
		return;

	// load already stop all sound and unload them before loading new one
	_AudioMixer->loadBackgroundSound (name, LigoConfig);
	_AudioMixer->setListenerPos(pos);
}


void CSoundManager::reset ()
{
	if (!_AudioMixer)
		return;

	NL3D::UParticleSystemSound::setPSSound(NULL);

	_GroupControllerEffects = NULL;
	_GroupControllerEffectsGame = NULL;

	delete _AudioMixer;
	_AudioMixer = NULL;

	// release sound anim properly
	releaseSoundAnim();

	init();

//	_AudioMixer->reset ();
}


//---------------------------------------------------
// init :
// Initialize the audio mixer, load the sound banks
//---------------------------------------------------
void CSoundManager::init(IProgressCallback *progressCallBack)
{
	_EnvSoundRoot = NULL;
	_PlaySound = true;

	_UserEntitySoundLevel = ClientCfg.UserEntitySoundLevel;

	const std::string &packedSheetPath = ClientCfg.SoundPackedSheetPath;
	const std::string &samplePath = ClientCfg.SampleBankDir;
//	try
//	{
		// reset particle sound
		NL3D::UParticleSystemSound::setPSSound(NULL);
		/*
		 * Create the audio mixer object and init it.
		 * If the sound driver cannot be loaded, an exception is thrown.
		 */
		_AudioMixer = UAudioMixer::createAudioMixer();

		if (!_AudioMixer)
			return;

		// Set the path to the sample directory
		_AudioMixer->setSamplePath(samplePath);
		// Set the path to the packed sheets

		if (ClientCfg.UpdatePackedSheet)
			_AudioMixer->setPackedSheetOption(packedSheetPath, true);
		else
			_AudioMixer->setPackedSheetOption("", false);

		UAudioMixer::TDriver	driverType= UAudioMixer::DriverAuto;
		if(ClientCfg.DriverSound==CClientConfig::SoundDrvFMod)
			driverType= UAudioMixer::DriverFMod;
		else if(ClientCfg.DriverSound==CClientConfig::SoundDrvOpenAL)
			driverType= UAudioMixer::DriverOpenAl;
		else if(ClientCfg.DriverSound==CClientConfig::SoundDrvDirectSound)
			driverType= UAudioMixer::DriverDSound;
		else if(ClientCfg.DriverSound==CClientConfig::SoundDrvXAudio2)
			driverType= UAudioMixer::DriverXAudio2;
		_AudioMixer->init(ClientCfg.MaxTrack, ClientCfg.UseEax, ClientCfg.UseADPCM, progressCallBack, false, driverType, ClientCfg.SoundForceSoftwareBuffer);
/*		int nbVoice = _AudioMixer->getPolyphony();
		_AudioMixer->setPriorityReserve(HighPri, max(1, nbVoice /2));
		nbVoice -= nbVoice /2;
		_AudioMixer->setPriorityReserve(MidPri, max(1, nbVoice /2));
		nbVoice -= nbVoice /2;
		_AudioMixer->setPriorityReserve(LowPri, max(1, nbVoice /2));
*/
		// when they are only 1 track available, enter in limiting mode.
		_AudioMixer->setLowWaterMark(1);

		/*
		 * Create the CSoundAnimManager. The CSoundAnimManager is a singleton.
		 * Access the singleton with CSoundAnimManager::instance().
		 */
		new CSoundAnimManager(_AudioMixer);

		// get the controller group for effects
		_GroupControllerEffects = _AudioMixer->getGroupController("sound:effects");
		_GroupControllerEffectsGame = _AudioMixer->getGroupController("sound:effects:game");

		// restore the volume
		SoundMngr->setSFXVolume(ClientCfg.SoundSFXVolume);
		SoundMngr->setGameMusicVolume(ClientCfg.SoundGameMusicVolume);

		// Init clustered sound system
		_AudioMixer->initClusteredSound(Scene, 0.01f, 100.0f, PORTAL_INTERPOLATE);

		// Init the background flags;
		for (uint i=0; i<NLSOUND::UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
			_BackgroundFlags.Flags[i] = false;
		_AudioMixer->setBackgroundFlags(_BackgroundFlags);

		/*
		 * Initialize listener's position
		 */
		CVector initpos ( 0.0f, 0.0f, 0.0f );
		//_AudioMixer->getListener()->setPos( initpos );
		_AudioMixer->setListenerPos(initpos);

		/*
		 * Init the particule sound system
		 */
		// attach the sound from the particule system
		NL3D::UParticleSystemSound::setPSSound(_AudioMixer);

		//loadPositionedSounds();

		// reload Sound Animation (if EAM already init)
		if(EAM)
			EAM->reloadAllSoundAnim();

		// Special option for DEV (easier to test when disabled)
		_AudioMixer->enableBackgroundMusicTimeConstraint(ClientCfg.EnableBackgroundMusicTimeConstraint);

/*	}
	catch(const Exception &e)
	{
		nlwarning( "Error: %s", e.what() );
	}
*/
} // init //

//---------------------------------------------------
// getSoundName:
//---------------------------------------------------
/*bool CSoundManager::getSoundName( TSound sound, std::string &name, const CVector& pos ) const
{
	/// \todo Malkav: take ground type into account, and all the different sounds
	switch (sound)
	{
		case WALK:
		{
			//name = "walk";
			UGlobalPosition globalPos = GR->retrievePosition( pos );
			uint32 material = GR->getMaterial( globalPos );

		}
		break;

		case RUN:
			name = "run";
			break;

		case USER_WALK:
			name = "user_walk";
			break;

		case USER_RUN:
			name = "user_run";
			break;

		default:
			return false;
	}

	return true;

} // getSoundName //
*/



//---------------------------------------------------
// addSource :
// add a new source to the world, attached to the specified entity
// return 0 if creation failed, sound id if creation was successful
//---------------------------------------------------
/*uint32 CSoundManager::addSource( TSound sound, const NLMISC::CVector &position, bool play, bool loop, const CEntityId &parentId )
{
	static std::string name;
	static TSource source;

	uint32	retValue = 0;

	if ( ! getSoundName ( sound, name, position ) )
		return 0;

	USource *pSource = _AudioMixer->createSource( name );

	if ( pSource  != NULL )
	{
		pSource ->setPos( position );

		pSource ->setLooping( loop );

		if (play)
			pSource ->play();

		//create the TSource object
		source.sound = sound;
		source.pSource = pSource;

		// attach the source to the entity, if specified
		if (parentId != CEntityId::Unknown)
			_AttachedSources.insert( TMultiMapEntityToSource::value_type( parentId, source ) );

		// set source id
		retValue = _NextId;
		_Sources.insert( TMapIdToSource::value_type( _NextId, source ) );
		++_NextId;
	}
	else
	{
		nlwarning( "Sound '%s' not found", name );
	}

	return retValue;
} // addSource //
*/

//-----------------------------------------------
// addSource :
// add a new source to the world, attached to the specified entity
// return 0 if creation failed, sound id if creation was successful
//-----------------------------------------------
CSoundManager::TSourceId CSoundManager::addSource( const NLMISC::CSheetId &soundName, const NLMISC::CVector &position, bool play, bool loop,  const CEntityId &id)
{
	uint32	retValue = 0;

	// Create a source
	USource *pSource = _AudioMixer->createSource( soundName );

	// If the source is valid.
	if(pSource == 0)
	{
		nlwarning("Sound '%s' not found !", /*CStringMapper::unmap(soundName).c_str()*/soundName.toString().c_str());
		return retValue;
	}

	// Load the properties for this sound.
// REMOVED because the pitch and the gain are now in the .nss (don't need .sdf anymore)
//	loadProperties(soundName, pSource);

	// Set the source position.
	pSource->setPos( position );

	// Is the source looping ?
	pSource->setLooping( loop );

	// Play the sound immedialty if asked.
	if(play)
	{
		pSource->play();
	}

	TSourceId sourceId = _Sources.insert(pSource);

	// attach the source to the entity, if specified
	if (id != CEntityId::Unknown )
	{
		_AttachedSources.insert( TMultiMapEntityToSource::value_type( id, sourceId ) );
	}

	// return the id of the source
	return sourceId;

} // addSource //


//-----------------------------------------------
// spawn a new source to the world
// return false if creation failed, true if creation was successful
//-----------------------------------------------
bool CSoundManager::spawnSource(const NLMISC::CSheetId &soundName, CSoundContext &context)
{
	if (!_PlaySound) return false;

	// Create a source
	// TODO : find the correct cluster
	USource *pSource = _AudioMixer->createSource( soundName, true, NULL, NULL, NULL, &context);

	// If the source is valid.
	if(pSource == 0)
	{
		nlwarning("Sound '%s' not found !", soundName.toString().c_str());
		return false;
	}

	// Set the source position.
	pSource->setPos (context.Position);
	pSource->play();

////nlinfo ("%.3f spawn source %p", (float)ryzomGetLocalTime ()/1000.0f, pSource);

	return true;

} // addSource //


//-----------------------------------------------
// spawn a new source to the world
// return false if creation failed, true if creation was successful
//-----------------------------------------------
bool CSoundManager::spawnSource(const NLMISC::CSheetId &soundName, const NLMISC::CVector &position)
{
	if (!_PlaySound) return false;

	// Create a source
	USource *pSource = _AudioMixer->createSource( soundName, true);

	// If the source is valid.
	if(pSource == 0)
	{
		nlwarning("Sound '%s' not found !", /*CStringMapper::unmap(soundName).c_str ()*/soundName.toString().c_str());
		return false;
	}

	// Set the source position.
	pSource->setPos (position);
	pSource->play();

////nlinfo ("%.3f spawn source %p", (float)ryzomGetLocalTime ()/1000.0f, pSource);

	return true;

} // addSource //



//---------------------------------------------------
// removeSource:
// remove a source
//---------------------------------------------------
void CSoundManager::removeSource(CSoundManager::TSourceId sourceId)
{
nldebug("remove the source : %d", sourceId);

/// \todo Malkav : optimize speed
nldebug("nb sources = %d", _Sources.size() );
	USource *pSource = _Sources.get(sourceId);
	if (pSource)
	{
		TMultiMapEntityToSource::iterator it = _AttachedSources.begin();//, itOld;

		for ( ; it != _AttachedSources.end() ; ++it)
		{
			if ( (*it).second == sourceId )
			{
				(*it).second = 0;
//				itOld = it;
//				++it;

				_AttachedSources.erase( it/*Old*/ );

				break;

//				if (it == _AttachedSources.end() )
//					break;
			}
		}

		// delete the source
		delete pSource;
		// i think there was something going on here
		_Sources.erase(sourceId);
	}
} // removeSource //



//---------------------------------------------------
// getSounds :
//
//---------------------------------------------------
/*bool CSoundManager::getSounds( uint32 material, TMoveType moveType, bool soft, std::vector<string>& sounds )
{
	return _StepSounds.getSounds( material, moveType, soft, sounds );

} // getSounds //*/



//---------------------------------------------------
// updateEntityPos :
// update the pos of all the sounds attached to that entity
//---------------------------------------------------
void CSoundManager::updateEntityPos( const CEntityId &id, const NLMISC::CVector &pos)
{
	TMultiMapEntityToSource::iterator it;
	const std::pair<TMultiMapEntityToSource::iterator, TMultiMapEntityToSource::iterator> range = _AttachedSources.equal_range( id );

	for ( it = range.first; it != range.second ; ++it)
	{
		_Sources.get((*it).second)->setPos( pos );
	}
} // updateEntityPos //


//---------------------------------------------------
// updateEntityVelocity :
// update the velocity of all the sounds attached to that entity
//---------------------------------------------------
void CSoundManager::updateEntityVelocity( const CEntityId &id, const NLMISC::CVector &velocity)
{
	TMultiMapEntityToSource::iterator it;
	const std::pair<TMultiMapEntityToSource::iterator, TMultiMapEntityToSource::iterator> range = _AttachedSources.equal_range( id );

	for ( it = range.first; it != range.second ; ++it)
	{
		_Sources.get((*it).second)->setVelocity( velocity );
	}

} // updateEntityVelocity //


//---------------------------------------------------
// updateEntityDirection :
// update the direction of all the sounds attached to that entity
//---------------------------------------------------
void CSoundManager::updateEntityDirection( const CEntityId &id, const NLMISC::CVector &dir )
{
	TMultiMapEntityToSource::iterator it;
	const std::pair<TMultiMapEntityToSource::iterator, TMultiMapEntityToSource::iterator> range = _AttachedSources.equal_range( id );

	for ( it = range.first; it != range.second ; ++it)
	{
		_Sources.get((*it).second)->setDirection( dir );
	}
} // updateEntityOrientation //


//---------------------------------------------------
// removeEntity :
// remove an entity from the view : delete all the sources attached to that entity
//---------------------------------------------------
void CSoundManager::removeEntity( const CEntityId &id)
{
	/// \todo Malkav : optimize speed

	TMultiMapEntityToSource::iterator it;
	const std::pair<TMultiMapEntityToSource::iterator, TMultiMapEntityToSource::iterator> range = _AttachedSources.equal_range( id );
	
	for ( it = range.first; it != range.second ; ++it)
	{
		TSourceId sourceId = (*it).second;
		if (sourceId)
		{
			USource *pSource = _Sources.get(sourceId);
			delete pSource;
			_Sources.erase(sourceId);
		}
	}

	_AttachedSources.erase( range.first, range.second );
} // removeEntity //




//---------------------------------------------------
// setSoundPosition :
//---------------------------------------------------
void CSoundManager::setSoundPosition(TSourceId sourceId, const NLMISC::CVector &position)
{
	if (!_PlaySound) return;

	USource *pSource = _Sources.get(sourceId);
	if (pSource) pSource->setPos(position);
} // setSoundPosition //


//---------------------------------------------------
// loopSound :
//---------------------------------------------------
void CSoundManager::loopSound(TSourceId sourceId, bool loop)
{
	if (!_PlaySound) return;

	USource *pSource = _Sources.get(sourceId);
	if (pSource) pSource->setLooping(loop);
} // loopSound //


//---------------------------------------------------
// playSound :
// start or stop playing sound
//---------------------------------------------------
void CSoundManager::playSound(TSourceId sourceId, bool play)
{
	if (!_PlaySound) return;

	USource *pSource = _Sources.get(sourceId);
	if (pSource)
	{
		if (play)
			pSource->play();
		else
			pSource->stop();
	}
} // loopSound //



//---------------------------------------------------
// isPlaying :
// return true if the source is playing
//---------------------------------------------------
bool CSoundManager::isPlaying(TSourceId sourceId)
{
	USource *pSource = _Sources.get(sourceId);
	if (pSource) return pSource->isPlaying();
	return false;
} // isPlaying //



//---------------------------------------------------
// setSoundForSource :
// set the sound associated to the specified source
//---------------------------------------------------
/*
bool CSoundManager::setSoundForSource( uint32 sourceId, TSound sound, const CVector& pos )
{
	// find the sound name
	static std::string soundName;
	soundName.clear();
	if ( ! getSoundName( sound , soundName, pos) )
		return false;

	TMapIdToSource::iterator it = _Sources.find( sourceId );
	if (it != _Sources.end() )
	{
		nlassert( (*it).second.pSource );

		// get the sound
		TSoundId soundId = _AudioMixer->getSoundId( soundName.c_str() );

		if (soundId != NULL)
		{
			(*it).second.pSource->setSound( soundId );
			(*it).second.sound = sound;
			nlinfo("sound has been set to %d",sound);
			return true;
		}
	}
	return false;
} // setSoundForSource //
*/



//---------------------------------------------------
// getSoundFromSource :
// get the sound type currently associated to a source
//---------------------------------------------------
/*CSoundManager::TSound	CSoundManager::getSoundFromSource( uint32 sourceId ) const
{
	TMapIdToSource::const_iterator it = _Sources.find( sourceId );
	if (it != _Sources.end() )
	{
		nlassert( (*it).second.pSource );

		return ( (*it).second.sound );
	}

	return NONE;
} // getSoundFromSource //
*/


//---------------------------------------------------
// setSourceGain :
// set the gain of the specified source
//---------------------------------------------------
void CSoundManager::setSourceGain(TSourceId sourceId, float gain)
{
	USource *pSource = _Sources.get(sourceId);
	if (pSource) pSource->setGain( gain );
} // setSourceGain //


//---------------------------------------------------
// getSourceGain :
// get the gain of the specified source (-1 if source not found)
//---------------------------------------------------
float CSoundManager::getSourceGain(TSourceId sourceId)
{
	USource *pSource = _Sources.get(sourceId);
	if (pSource) return pSource->getGain();
	return -1;
} // getSourceGain //


//---------------------------------------------------
// setSourcePitch :
// set the Pitch of the specified source
//---------------------------------------------------
void CSoundManager::setSourcePitch(TSourceId sourceId, float Pitch)
{
	USource *pSource = _Sources.get(sourceId);
	if (pSource) pSource->setPitch(Pitch);
} // setSourcePitch //


//---------------------------------------------------
// getSourcePitch :
// get the Pitch of the specified source (-1 if source not found)
//---------------------------------------------------
float CSoundManager::getSourcePitch(TSourceId sourceId)
{
	USource *pSource = _Sources.get(sourceId);
	if (pSource) return pSource->getPitch();
	return -1;
} // getSourcePitch //



//---------------------------------------------------
// loadProperties :
// Load the properties for this sound and apply them.
//---------------------------------------------------
void CSoundManager::loadProperties(const string &soundName, USource *source)
{
	// If the source does not exist -> return nothing to do.
	if(!source)
		return;

	// Search for the file.
	string filePath = CPath::lookup(soundName+".sdf");
	ifstream file(filePath.c_str(), ios::in);

	// Try to open the file.
	if(file.is_open())
	{
		char tmpBuff[260];
		char delimiterBox[] = "\t ";
		// While the end of the file is not reached.
		while(!file.eof())
		{
			// Get a line (teh line should not be more than _MAX_LINE_SIZE).
			file.getline(tmpBuff, 260);
			char *token = strtok(tmpBuff, delimiterBox);
			while(token != NULL)
			{
				// Get the pitch.
				if(strcmp(token, "Pitch:") == 0)
				{
					token = strtok(NULL, delimiterBox);
					if(token)
					{
						float pitch;
						fromString(token, pitch);
						nlinfo("Pitch: %f", pitch);
						source->setPitch(pitch);
					}
				}
				// Get the Gain.
				else if(strcmp(token, "Gain:") == 0)
				{
					token = strtok(NULL, delimiterBox);
					if(token)
					{
						float gain;
						fromString(token, gain);
						nlinfo("Gain: %f", gain);
						source->setGain(gain);
					}
				}

				// Next property.
				token = strtok(NULL, delimiterBox);
			}
		}
		// Close the file.
		file.close();
	}
}// loadProperties //




//---------------------------------------------------
// loadPositionedSounds :
//
//---------------------------------------------------
/*void CSoundManager::loadPositionedSounds()
{
	nlstopex(("class CStepSounds is not used anymore"));
	// cst loader
	CSTLoader cstl;

	// build file format
	map<string,CSTLoader::TDataType> fileFormat;
	fileFormat.insert( make_pair( string( "sound name" ) , CSTLoader::STRING ));
	fileFormat.insert( make_pair( string( "x" ) , CSTLoader::FLOAT ));
	fileFormat.insert( make_pair( string( "y" ) , CSTLoader::FLOAT ));
	fileFormat.insert( make_pair( string( "z" ) , CSTLoader::FLOAT ));
	fileFormat.insert( make_pair( string( "loop" ) , CSTLoader::BOOL ));


	string str = CPath::lookup("positioned_sounds.txt", false);
	if(str.empty())
	{
		nlwarning ("cant load positioned_sounds");
		return;
	}

	// init loader
	cstl.init( str, fileFormat );

	// read the file
	while( cstl.readLine() )
	{
		string soundName;
		cstl.getStringValue( "sound name", soundName );

		CVector sourcePos;
		cstl.getValue( "x", sourcePos.x );
		cstl.getValue( "y", sourcePos.y );
		cstl.getValue( "z", sourcePos.z );

		bool loop;
		cstl.getBoolValue( "loop", loop );

		uint32 sourceId = addSource( soundName, sourcePos, false , loop );

		if(sourceId != 0)
		{
			_PositionedSounds.push_back( sourceId );
		}
		else
		{
			nlwarning ("positioned sound: can't find sound: '%s'", soundName.c_str());
		}
	}

	cstl.close();

} // chooseSoundOnMaterial //
*/


//---------------------------------------------------
// playPositionedSounds :
//
//---------------------------------------------------
void CSoundManager::playPositionedSounds( const CVector& /* pos */ )
{
	if (!_PlaySound) return;

	list<uint32>::iterator itPSnd;
	for( itPSnd = _PositionedSounds.begin(); itPSnd != _PositionedSounds.end(); ++itPSnd )
	{
		USource *pSource = _Sources.get(*itPSnd);
		if (!pSource)
		{
			nlwarning("<CSoundManager::playPositionedSounds>  :  The source %d is unknown",*itPSnd);
		}
		else 
		{
			/*
			if( (pos - (*itSrc).second.getPos()).norm() < ...)
			{
				if( (*itSrc).second.pSource->isPlaying() == false )
				{
					(*itSrc).second.pSource->play();
				}
			}
			*/
			if (!pSource->isPlaying())
			{
				pSource->play();
			}
		}
	}

} // playPositionedSounds //


void CSoundManager::selectEnv( const std::string &/* tag */)
{
	// TODO : boris : temporarily removed.
/*	if (_EnvSoundRoot==NULL)
		return;

	_EnvSoundRoot->selectEnv (tag.c_str(), true);
*/
}

void CSoundManager::setFilterState(uint filterId, bool state)
{
	nlassert(filterId < NLSOUND::UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS);
	_BackgroundFlags.Flags[filterId] = state;

	_AudioMixer->setBackgroundFlags(_BackgroundFlags);
}



// ***************************************************************************
void CSoundManager::update ()
{
	// Filters : day = 0, night = 1, morning = 2, Dusk = 3
	// update the filter state for day/night management.
	switch(LightCycleManager.getState())
	{
	case CLightCycleManager::Day:
		_BackgroundFlags.Flags[DAY] = true;
		_BackgroundFlags.Flags[NIGHT] = false;
		_BackgroundFlags.Flags[MORNING] = false;
		_BackgroundFlags.Flags[DUSK] = false;
		break;
	case CLightCycleManager::Night:
		_BackgroundFlags.Flags[DAY] = false;
		_BackgroundFlags.Flags[NIGHT] = true;
		_BackgroundFlags.Flags[MORNING] = false;
		_BackgroundFlags.Flags[DUSK] = false;
		break;
	case CLightCycleManager::DayToNight:
		_BackgroundFlags.Flags[DAY] = true;
		_BackgroundFlags.Flags[NIGHT] = false;
		_BackgroundFlags.Flags[MORNING] = true;
		_BackgroundFlags.Flags[DUSK] = false;
		break;
	case CLightCycleManager::NightToDay:
		_BackgroundFlags.Flags[DAY] = false;
		_BackgroundFlags.Flags[NIGHT] = false;
		_BackgroundFlags.Flags[MORNING] = false;
		_BackgroundFlags.Flags[DUSK] = true;
		break;
	case CLightCycleManager::StateUnknown:
		//nlwarning("Unknown light cycle state reached.");
		break;
	}

	// update the filter for season.
	EGSPD::CSeason::TSeason season = CurrSeason;
	switch(season)
	{
	case EGSPD::CSeason::Spring:
		_BackgroundFlags.Flags[SPRING] = true;
		_BackgroundFlags.Flags[SUMMER] = false;
		_BackgroundFlags.Flags[AUTUMN] = false;
		_BackgroundFlags.Flags[WINTER] = false;
		break;
	case EGSPD::CSeason::Summer:
		_BackgroundFlags.Flags[SPRING] = false;
		_BackgroundFlags.Flags[SUMMER] = true;
		_BackgroundFlags.Flags[AUTUMN] = false;
		_BackgroundFlags.Flags[WINTER] = false;
		break;
	case EGSPD::CSeason::Autumn:
		_BackgroundFlags.Flags[SPRING] = false;
		_BackgroundFlags.Flags[SUMMER] = false;
		_BackgroundFlags.Flags[AUTUMN] = true;
		_BackgroundFlags.Flags[WINTER] = false;
		break;
	case EGSPD::CSeason::Winter:
		_BackgroundFlags.Flags[SPRING] = false;
		_BackgroundFlags.Flags[SUMMER] = false;
		_BackgroundFlags.Flags[AUTUMN] = false;
		_BackgroundFlags.Flags[WINTER] = true;
		break;
	default:
		//nlwarning("Updating unknown season.");
		break;
	}
	// TODO : update the filter state for weather effet
	CWeatherState weatherState = WeatherManager.getCurrWeatherState();



	enum TWeatherSoundEnv
	{
		WSEMist = 0, // not used
		WSEFair1,
		WSEFair2,
		WSEFair3,
		WSEClouds,
		WSERain1,
		WSERain2,
		WSESnow1,
		WSEThunder1,
		WSEThunderSeve,
		WSEThunderSand1, // not used
		WSEThunderSand2, // not used
		WSEThunderSand3, // not used
		WSECount
	};

	// For each weather setup, gives the matching weather sound env to used, when there are precipitation or not
	class CWeatherSetupSoundEnv
	{
	public:
		NLMISC::TStringId		SetupName; // setup nem as found is the .weather_setup sheets
		TWeatherSoundEnv		Sound;
		CWeatherSetupSoundEnv(const char *setupName, TWeatherSoundEnv sound)
		{
			SetupName = NLMISC::CStringMapper::map(setupName);
			Sound = sound;
		}
	};

	// association between weather setup names & sound env
	static const CWeatherSetupSoundEnv weatherSoundLUT[] =
	{
		CWeatherSetupSoundEnv("fair1", WSEFair1),
		CWeatherSetupSoundEnv("fair2", WSEFair2),
		CWeatherSetupSoundEnv("fair3", WSEFair3),
		CWeatherSetupSoundEnv("wind1", WSEClouds),
		CWeatherSetupSoundEnv("humidity1", WSERain1),
		CWeatherSetupSoundEnv("humidity2", WSERain2),
		CWeatherSetupSoundEnv("clouds1", WSERain1),
		CWeatherSetupSoundEnv("clouds2", WSERain2),
		CWeatherSetupSoundEnv("thunderseve", WSEThunderSeve),
		CWeatherSetupSoundEnv("storm", WSEThunder1),
		CWeatherSetupSoundEnv("snow", WSESnow1),
	};

	const CWeatherSetupSoundEnv *weatherSetupSoundEnv = NULL;
	for(uint k = 0; k < sizeof(weatherSoundLUT) / sizeof(weatherSoundLUT[0]); ++k)
	{
		if (weatherSoundLUT[k].SetupName == weatherState.BestSetupName)
		{
			weatherSetupSoundEnv = &weatherSoundLUT[k];
			break;
		}
	}

	// disable all weather setup sound flags except the current one
	for(uint k = 0; k < WSECount; ++k)
	{
		_BackgroundFlags.Flags[MIST+k] = false;
	}
	if (weatherSetupSoundEnv)
	{
		_BackgroundFlags.Flags[MIST + weatherSetupSoundEnv->Sound]  = true;
	}
	else
	{
		static bool bDisplayOnce = false;
		if (!bDisplayOnce)
		{
			nlinfo("The current weather setup '%s' is unknown !", CStringMapper::unmap(weatherState.BestSetupName).c_str());
			bDisplayOnce = true;
		}
	}

	// update the background flags in the mixer.
	_AudioMixer->setBackgroundFlags(_BackgroundFlags);

	// update background music enabling
	if(_EnableBackgroundMusicAtTime && CTime::getLocalTime()>_EnableBackgroundMusicAtTime)
	{
		setGameMusicMode(true, true);
	}

	// Special option for DEV (easier to test when disabled)
	_AudioMixer->enableBackgroundMusicTimeConstraint(ClientCfg.EnableBackgroundMusicTimeConstraint);

	// update game music volume because of event played or not
	updateEventAndGameMusicVolume();

	// update audio mixer
	_AudioMixer->update ();
}

// ***************************************************************************
void CSoundManager::updateAudioMixerOnly()
{
	if(!_AudioMixer)
		return;
	_AudioMixer->update ();
}

// ***************************************************************************
void CSoundManager::playBackgroundSound()
{
	_AudioMixer->playBackgroundSound();
}

// ***************************************************************************
void CSoundManager::stopBackgroundSound()
{
	_AudioMixer->stopBackgroundSound();
}

// ***************************************************************************
void		CSoundManager::playMusic(const string &fileName, uint xFadeTime, bool async, bool loop, bool forceGameMusicVolume)
{
	if(fileName.empty())
	{
		stopMusic(xFadeTime);
	}
	else
	{
		if(_AudioMixer)
		{
			// disable before the background music (before is important else may have bug)
			setGameMusicMode(false, forceGameMusicVolume);
			// play
			_AudioMixer->playMusic(fileName, xFadeTime, async, loop);
		}
	}
}

// ***************************************************************************
void		CSoundManager::stopMusic(uint xFadeTime)
{
	if(_AudioMixer)
	{
		// stop the music
		_AudioMixer->stopMusic(xFadeTime);
		// and reenable the background music system. after a delay if xFadeTime
		if(xFadeTime>0)
		{
			// enable background music later
			_EnableBackgroundMusicAtTime= CTime::getLocalTime()+xFadeTime;
		}
		else
		{
			// else enable now
			setGameMusicMode(true, true);
		}
	}
}

// ***************************************************************************
void		CSoundManager::playEventMusic(const string &fileName, uint xFadeTime, bool loop)
{
	// if event music not enabled (mp3 player), abort
	if(!_EventMusicEnabled)
		return;

	if(fileName.empty())
	{
		stopEventMusic("", xFadeTime);
	}
	else
	{
		if(_AudioMixer)
		{
			// if the music is not already currently playing (with same loop state)
			if(_EventMusicPlayed.empty() || nlstricmp(fileName, _EventMusicPlayed)!=0 || loop!=_EventMusicLoop)
			{
				// play
				_AudioMixer->playEventMusic(fileName, xFadeTime, true, loop);
				_EventMusicPlayed= fileName;
				_EventMusicLoop= loop;
			}
		}
	}
}

// ***************************************************************************
void		CSoundManager::stopEventMusic(const string &fileName, uint xFadeTime)
{
	if(_AudioMixer)
	{
		// if the event music is the one currently played, or if empty fileName
		if(fileName.empty() || nlstricmp(_EventMusicPlayed,fileName)==0)
		{
			// stop the music
			_AudioMixer->stopEventMusic(xFadeTime);
			// music no more played
			_EventMusicPlayed.clear();
		}
	}
}

// ***************************************************************************
void		CSoundManager::pauseMusic()
{
	if(_AudioMixer)
		_AudioMixer->pauseMusic();
}

// ***************************************************************************
void		CSoundManager::resumeMusic()
{
	if(_AudioMixer)
		_AudioMixer->resumeMusic();
}

// ***************************************************************************
bool		CSoundManager::isMusicEnded()
{
	if(_AudioMixer)
		return _AudioMixer->isMusicEnded();
	return false;
}

// ***************************************************************************
void		CSoundManager::setGameMusicVolume(float val)
{
	clamp(val,0.f,1.f);
	_GameMusicVolume= val;
	updateVolume();
}

// ***************************************************************************
void		CSoundManager::setUserMusicVolume(float val)
{
	clamp(val,0.f,1.f);
	_UserMusicVolume= val;
	updateVolume();
}

// ***************************************************************************
void		CSoundManager::setSFXVolume(float val)
{
	clamp(val,0.f,1.f);
	_SFXVolume= val;
	updateVolume();
}

// ***************************************************************************
void		CSoundManager::setGameMusicMode(bool enableBackground, bool useGameMusicVolume)
{
	// update background music state
	_EnableBackgroundMusicAtTime= 0;
	_AudioMixer->enableBackgroundMusic(enableBackground);
	_UseGameMusicVolume= useGameMusicVolume;
	// event music enabled if the background music is also enabled
	_EventMusicEnabled= enableBackground;
	// stop any event music if disabled
	if(!_EventMusicEnabled)
		stopEventMusic("", 500);
	// update music volume state
	updateVolume();
}

// ***************************************************************************
void		CSoundManager::updateVolume()
{
	if(_AudioMixer)
	{
		// update music volume
		if(_UseGameMusicVolume)
			// Game music (background music system)
			_AudioMixer->setMusicVolume(_GameMusicVolume*_FadeGameMusicVolume*_FadeGameMusicVolumeDueToEvent);
		else
			// User music volume (mp3 player) is not affected by fadein/fadeout
			_AudioMixer->setMusicVolume(_UserMusicVolume);

		// The event music volume get the game music volume, but is not faded (want music during teleport)
		_AudioMixer->setEventMusicVolume(_GameMusicVolume);

		// update sfx volume
		_GroupControllerEffects->setGain(_SFXVolume);
		_GroupControllerEffectsGame->setGain(_FadeSFXVolume);
	}
}

// ***************************************************************************
void		CSoundManager::fadeInGameSound(sint32 timeFadeMs)
{
	// if already at max, noop
	if(_FadeGameMusicVolume==1.f && _FadeSFXVolume==1.f)
		return;

	fadeGameSound(true, timeFadeMs);
}

// ***************************************************************************
void		CSoundManager::fadeOutGameSound(sint32 timeFadeMs)
{
	// if already at 0, noop
	if(_FadeGameMusicVolume==0.f && _FadeSFXVolume==0.f)
		return;

	fadeGameSound(false, timeFadeMs);
}

// ***************************************************************************
void		CSoundManager::fadeGameSound(bool fadeIn, sint32 timeFadeMs)
{
	// fade in
	clamp(_FadeGameMusicVolume, 0.f, 1.f);
	clamp(_FadeSFXVolume, 0.f, 1.f);
	TTime	t0= CTime::getLocalTime();
	float	dVolumeOverDt= 1.f / (timeFadeMs*0.001f);
	if(!fadeIn)
		dVolumeOverDt*= -1;
	while(fadeIn?(_FadeGameMusicVolume<1.f || _FadeSFXVolume<1.f):(_FadeGameMusicVolume>0.f || _FadeSFXVolume>0.f))
	{
		// update volume vars
		TTime	t1= CTime::getLocalTime();
		float	dt= (t1-t0)*0.001f;
		t0= t1;
		_FadeGameMusicVolume+= dVolumeOverDt*dt;
		_FadeSFXVolume+= dVolumeOverDt*dt;
		clamp(_FadeGameMusicVolume, 0.f, 1.f);
		clamp(_FadeSFXVolume, 0.f, 1.f);
		// update volume and sound
		updateVolume();
		update();
	}
}

// ***************************************************************************
void		CSoundManager::setupFadeSound(float sfxFade, float musicFade)
{
	clamp(sfxFade, 0.f, 1.f);
	clamp(musicFade, 0.f, 1.f);
	_FadeGameMusicVolume= musicFade;
	_FadeSFXVolume= sfxFade;
	updateVolume();
}

// ***************************************************************************
void		CSoundManager::updateEventAndGameMusicVolume()
{
	// update event music played state if not looping
	if(!_EventMusicPlayed.empty() && !_EventMusicLoop)
	{
		if(!_AudioMixer)
			_EventMusicPlayed.clear();
		else
		{
			if(_AudioMixer->isEventMusicEnded())
				_EventMusicPlayed.clear();
		}
	}

	// Fade
	const	sint32 timeFadeMs= 500;
	static TTime	t0= CTime::getLocalTime();
	TTime	t1= CTime::getLocalTime();
	float	dVolume= (t1-t0) / (float)timeFadeMs;
	t0= t1;

	// if event music is played, music lower the music
	if(!_EventMusicPlayed.empty())
		dVolume*= -1;
	float	f= _FadeGameMusicVolumeDueToEvent + dVolume;
	clamp(f, 0.f, 1.f);

	// if faded, update the volume
	if(f!=_FadeGameMusicVolumeDueToEvent)
	{
		_FadeGameMusicVolumeDueToEvent= f;
		updateVolume();
	}
}


//---------------------------------------------------
// CStepSounds :
//
//---------------------------------------------------
/*CStepSounds::CStepSounds()
{
	nlstopex(("class CStepSounds is not used anymore"));
	//init( CPath::lookup("materials_for_step_sounds.txt", false).c_str() );

} // CStepSounds //
*/

//---------------------------------------------------
// init :
//
//---------------------------------------------------
/*void CStepSounds::init( const char * fileName )
{
	nlstopex(("class CStepSounds is not used anymore"));
	if(fileName==NULL || strlen(fileName)==0 || fileName[0]=='\0')
	{
		nlwarning ("can't load step sound because no material step file");
		return;
	}

	// get all the materials used for step sounds
	list<uint32> materials;
	try
	{
		CConfigFile cf;
		cf.load( fileName );

		CConfigFile::CVar &cvMaterials = cf.getVar("Materials");
		for(sint i = 0; i < cvMaterials.size(); i++)
		{
			materials.push_back( cvMaterials.asInt(i) );
		}
	}
	catch (const EConfigFile &e)
	{
		nlerror("Problem in the file %s : %s", fileName,e.what ());
	}

	// load all sounds for each of these material
	list<uint32>::const_iterator itMaterials;
	for( itMaterials = materials.begin(); itMaterials != materials.end(); ++itMaterials )
	{
		CMaterialStepSounds materialStepSounds;
		materialStepSounds.load( CPath::lookup(string("sndmat_") + toString(*itMaterials) + string(".txt")) );
		_StepSoundsByMaterial.insert( make_pair(*itMaterials,materialStepSounds) );
	}

} // init //*/



//---------------------------------------------------
// getSounds :
//
//---------------------------------------------------
/*bool CStepSounds::getSounds( uint32 material, TMoveType moveType, bool soft, std::vector<string>& sounds )
{
	nlstopex(("class CStepSounds is not used anymore"));
	map<uint32,CMaterialStepSounds>::iterator itMatSounds = _StepSoundsByMaterial.find( material );
	if( itMatSounds == _StepSoundsByMaterial.end() )
	{
		return false;
	}
	else
	{
		return (*itMatSounds).second.getSounds( moveType, soft, sounds );
	}

} // getSounds //*/


//---------------------------------------------------
// readVar :
//
//---------------------------------------------------
/*void CMaterialStepSounds::readVar( CConfigFile& cf, TMoveType moveType, bool soft, char * varName )
{
	try
	{
		vector<string> sounds;
		CConfigFile::CVar &cvSounds = cf.getVar( varName );
		for(sint i = 0; i < cvSounds.size(); i++)
		{
			sounds.push_back( cvSounds.asString(i) );
		}
		_Sounds.insert( make_pair( make_pair(moveType,soft), sounds) );
	}
	catch (const EConfigFile &e)
	{
		nlwarning("Problem in the sounds by material config file : %s", e.what ());
	}

} // readVar //
*/


//---------------------------------------------------
// load :
//
//---------------------------------------------------
/*void CMaterialStepSounds::load( string fileName )
{
	CConfigFile cf;
	cf.load(CPath::lookup(fileName.c_str()));

	// walk soft sounds
	readVar(cf,WALK,true,"walk_sounds_soft");
	// walk hard sounds
	readVar(cf,WALK,false,"walk_sounds_hard");
	// run soft sounds
	readVar(cf,RUN,true,"run_sounds_soft");
	// run hard sounds
	readVar(cf,RUN,false,"run_sounds_hard");


	// user walk soft sounds
	readVar(cf,USER_WALK,true,"user_walk_sounds_soft");
	// user walk hard sounds
	readVar(cf,USER_WALK,false,"user_walk_sounds_hard");
	// user run soft sounds
	readVar(cf,USER_RUN,true,"user_run_sounds_soft");
	// user run hard sounds
	readVar(cf,USER_RUN,false,"user_run_sounds_hard");

} // load //
*/

/*//---------------------------------------------------
// getSounds :
//
//---------------------------------------------------
bool CMaterialStepSounds::getSounds( TMoveType moveType, bool soft, vector<string>& sounds )
{
	map<pair<TMoveType,bool>,vector<string> >::iterator itSounds = _Sounds.find( make_pair(moveType,soft) );
	if( itSounds == _Sounds.end() )
	{
		return false;
	}

	uint i;
	uint sz = (*itSounds).second.size();
	sounds.clear();
	sounds.resize( sz );
	for( i=0; i < sz; i++ )
	{
		sounds[i] = (*itSounds).second[i];
	}

	return true;

} // getSounds //*/

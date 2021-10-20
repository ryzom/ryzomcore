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




#ifndef CL_SCENE_PARSER_H
#define CL_SCENE_PARSER_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
// 3D Interface.
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_play_list_manager.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_instance_group.h"
// Std.
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <map>
#include <set>


///////////
// USING //
///////////
using NLMISC::CVector;
using NLMISC::CQuat;
using NL3D::UPlayListManager;
using NL3D::UAnimationSet;
using NL3D::UAnimation;
using NL3D::UPlayList;
using NL3D::UInstanceGroup;
/*using std::string;
using std::ifstream;
using std::list;
using std::vector;
using std::map;
using std::set;
using std::pair;*/


///////////
// CLASS //
///////////
/**
 * Class to parse a scene.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CSceneParser
{
private:
	static const uint MAX_LINE_SIZE;
	static const char *delimiter;

	/// Parse the file.
	void parse(ifstream &file);

	/// Skip empty lines and comment lines.
	void getNextValidLine(ifstream &file, char *buff);

	/// Day/Night.
	void parseTime();
	/// Parse the list of IG to load for the scene.
	void parseIG(ifstream &file, char *buff);
	/// Get the name of the IG to initialize entities.
	void parseIGInit();

	/// Parse Particle.
	void parseParticle(ifstream &file, char *buff);
	/// Parse Particle's Id.
	void parseParticleId();
	/// Parse Particle's IG.
	void parseParticleIG();
	/// Parse Particle's Cluster
	void parseParticleCluster();
	/// Parse Particle's Actor.
	void parseParticleActor();
	/// Parse Particle's anims.
	void parseParticleAnims(ifstream &file, char *buff);

	/// Parse an actor.
	void parseActor(ifstream &file, char *buff);
	/// Parse the current actor Id.
	void parseId();
	/// Parse the current actor Name.
	void parseName();
	/// Is the Actor a flyer.
	void parseFly();
	/// Parse Actor's Actor.
	void parseActorActor();
	/// Parse the current actor Skeleton Name.
	void parseSkel();
	/// Parse meshes used for the current actor.
	void parseMeshes(ifstream &file, char *buff);
	/// Parse anims used for the current actor.
	void parseAnims(ifstream &file, char *buff);

	/// Parse all anims used for the camera in all sequence.
	void parseCamAnims(ifstream &file, char *buff);

	/// Parse a sequence.
	void parseSequence(ifstream &file, char *buff);
	/// Parse the current sequence Id.
	void parseSeqId();
	/// Parse particle in the sequence.
	void parseSeqParticle(ifstream &file, char *buff);
	/// Parse the current particle's Id in the current sequence.
	void parseSeqParticleId();
	/// Parse anims used for the current particle in the current sequence.
	void parseSeqParticleAnims(ifstream &file, char *buff);
	/// Parse an actor in a sequence.
	void parseSeqActor(ifstream &file, char *buff);
	/// Parse the current actor's Id in the current sequence.
	void parseSeqActorId();
	/// Parse anims used for the current actor in the current sequence.
	void parseSeqActorAnims(ifstream &file, char *buff);
	/// Parse camera in the sequence.
	void parseSeqCam(ifstream &file, char *buff);

	/// Initialize Actors Position and rotation .
	void initActors();

	/// Reset the scene.
	void reset();
	/// Reset all particles.
	void resetParticles();
	/// Reset all actors.
	void resetActors();

	/// Initialize the Camera and reset it the second time.
	void applyCamera();
	/// Initialize IG for the scenery (IG present from start to end like trees or torch.
	void applyIG();
	/// Initialize the scene with the parameters loaded from the script for particles.
	void applyParticles();
	/// Initialize actors.
	void applyActors();

protected:
	bool	_Apply;
	sint	_Line;
	bool	_Day;
	string	_IG_Init;
	/// List of IG to load for the scene.
	list<string>			_IG;
	double		_TimeStart;
	double	_FrameRate;

	/// Class to stock parameters for a particle (system).
	class CParticle
	{
	public:
		sint				Id;
		string				IG;
		string				Cluster;
		sint				Actor;
		list<string>		Anims;

		UInstanceGroup		*IGPtr;
		vector<CVector>		IGPos;

		map<string, uint>	AnimToId;
		UAnimationSet		*AnimationSet;
		UPlayList			*PlayList;

		CParticle()
		{
			AnimationSet = 0;
			PlayList = 0;
		}

		void reset()
		{
			Id = -1;
			Actor = -1;
			Anims.clear();
			AnimationSet = 0;
			PlayList = 0;
			AnimToId.clear();
		}
	};

	/// Class to stock parameters for an actor.
	class CActor
	{
	public:
		sint			Id;
		string			Name;
		bool			Fly;
		string			Skeleton;
		list<string>	Meshes;
		list<string>	Anims;
		CVector			Pos;
		CQuat			Rot;
		sint			Actor;

		void reset()
		{
			Id = -1;
			Actor = -1;
			Fly = false;
			Meshes.clear();
			Anims.clear();
		}
	};

	/// Class to stock parameters for an actor in a sequence.
	class CParticleSeq
	{
	public:
		sint			Id;
		list<string>	Anims;

		void reset()
		{
			Id = -1;
			Anims.clear();
		}
	};

	/// Class to stock parameters for an actor in a sequence.
	class CActorSeq
	{
	public:
		sint			Id;
		list<string>	Anims;

		void reset()
		{
			Id = -1;
			Anims.clear();
		}
	};

	/// Class to stock parameters for a sequence.
	class CSequence
	{
	public:
		sint					Id;
		map<uint, CParticleSeq>	ParticlesSeq;
		map<uint, CActorSeq>	ActorsSeq;
		list<string>			CamAnims;

		void reset()
		{
			Id = -1;
			ActorsSeq.clear();
		}
	};

	/// ...
	list<string>			_CamAnims;
	/// Particle with the parse in progress.
	CParticle				_CurrentParticle;
	/// All particles in the scene.
	map<uint, CParticle>	_Particles;
	/// Actor with the parse in progress.
	CActor					_CurrentActor;
	/// All actors in the scene.
	map<uint, CActor>		_Actors;
	/// ...
	CParticleSeq			_CurParticleSeq;
	/// ...
	CActorSeq				_CurrentActorSeq;
	/// ...
	CSequence				_CurrentSequence;
	/// All Sequences in the scene.
	map<uint, CSequence>	_Sequences;

	/// ...
	UPlayListManager		*_PlayListManager;
	UAnimationSet			*_AnimationSet;
	UPlayList				*_PlayList;
	map<string, uint>		_AnimCamToId;

	set<UInstanceGroup *>	_IGInScene;

	// Scene.
	typedef list<pair<sint, double> > TScene;
	TScene					_Scene;
	TScene::iterator		_ItScene;
	double					_SceneStart;


	/// Update the camera (position, target, roll, fov).
	void updateCamera(double timeInSec);
	/// Update actor position (for actor on another actor).
	void updateActors();
	/// Update particles.
	void updateParticles(double timeInSec);
	void updateParticlesNoActor(float difTime, CParticle &particle, UAnimation &animation);
	void updateParticlesActor(float difTime, CParticle &particle, UAnimation &animation);

public:
	/// Constructor
	CSceneParser();

	inline bool Time() {return _Day;}

	/// Load the file with the scene.
	void load(const string &filename);

	/// Initialize the scene with the parameters loaded from the script.
	void apply();

	///
	void update(double timeInSec);

	/// Play the sequence with the ID 'seq'.
	void playSeq(uint seq, double timeInSec);

	inline double frameRate() {return _FrameRate;}
	inline void frameRate(double fr) {_FrameRate = fr;}

	/** \name SCENE
	 * Functions to manage a scene.
	 */
	//@{
	/**
	 * Load a scene from a file and put it in memory.
	 * \param filename : filename for the file that contains the scene.
	 * \warning This function clear the old scene.
	 */
	void loadScene(const string &filename);
	/**
	 * Play a scene in memory.
	 * \param timeInSec : current time in second.
	 */
	void playScene(double timeInSec);
	/**
	 * Update the scene currently playing.
	 * \param timeInSec : current time in second.
	 */
	void updateScene(double timeInSec);
	/**
	 * Stop the scene currently playing.
	 */
	void stopScene();
	/**
	 * Function to know if there is a scene currently playing.
	 * \return bool : 'true' if there is a scene currently playing.
	 */
	bool isScenePlaying();
	//@}
};


#endif // CL_SCENE_PARSER_H

/* End of scene_parser.h */

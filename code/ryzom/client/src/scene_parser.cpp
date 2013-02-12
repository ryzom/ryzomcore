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
// Misc.
#include "nel/misc/path.h"
#include "nel/misc/debug.h"
#include "nel/misc/matrix.h"
// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_track.h"
#include "nel/3d/u_camera.h"
// Client.
#include "scene_parser.h"
#include "entities.h"
#include "entity_cl.h"
#include "animated_scene_object.h"
#include "ig_client.h"
#include "user_controls.h"
#include "time_client.h"
// Game Share
#include "game_share/entity.h"


///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;
using namespace std;


////////////
// EXTERN //
////////////
extern UDriver		*Driver;
extern UScene		*Scene;
extern UScene		*SceneRoot;


/////////////
// METHODS //
/////////////
const uint CSceneParser::MAX_LINE_SIZE = 500;
const char *CSceneParser::delimiter = " \t";

//-----------------------------------------------
// CSceneParser :
// Constructor.
//-----------------------------------------------
CSceneParser::CSceneParser()
{
	_FrameRate = 30.0;
	_Line = 0;
	_AnimationSet = 0;
	_TimeStart = -1;
	_AnimationSet = 0;
	_Apply = false;
	_Day = true;

	_ItScene = _Scene.end();
	_SceneStart = -1.f;
}// CSceneParser //


//-----------------------------------------------
// load :
// Load the file with the scene.
//-----------------------------------------------
void CSceneParser::load(const string &filename)
{
	// Look for the file -> nlwarning and return if sceneFile is empty.
	string sceneFile = CPath::lookup(filename, false);
	if(sceneFile.empty())
	{
		nlwarning("CSceneParser::load : Can't find file \"%s\".", filename.c_str());
		return;
	}

	// Open the file.
	ifstream file(sceneFile.c_str(), ios::in);
	if(file.is_open())
	{
		// Load the capture speed.
		ifstream speedFile("capture_speed.txt", ios::in);
		if(speedFile.is_open())
		{
			char tmpBuff[MAX_LINE_SIZE];

			// Set the default fram rate (30fps).
			_FrameRate = 30.0;

			// Init Lines.
			_Line = 0;

			// While the end of the file is not reached -> parse the script.
			while(!speedFile.eof())
			{
				// Get next valid line.
				getNextValidLine(speedFile, tmpBuff);

				char *ptr = strtok(tmpBuff, delimiter);
				if(ptr != NULL)
					_FrameRate = atof(ptr);
			}

			// Close the speed file.
			speedFile.close();
		}
		else
			nlwarning("CSceneParser::load : 'capture_speed.txt' can't be open, default frame rate is %f.", _FrameRate);

		// Init Lines.
		_Line = 0;

		// Parse the File.
		parse(file);

		// Initialize Actors Position and rotation .
		initActors();

		// Close the File.
		file.close();
	}
	else
	{
		nlwarning("CSceneParser::load : File \"%s\" can't be open.", sceneFile.c_str());
		return;
	}
}// load //


//-----------------------------------------------
// parse :
// Parse the file.
//-----------------------------------------------
void CSceneParser::parse(ifstream &file)
{
	char tmpBuff[MAX_LINE_SIZE];

	// While the end of the file is not reached -> parse the script.
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, tmpBuff);

		char *ptr = strtok(tmpBuff, delimiter);
		if(ptr != NULL)
		{
			// Time (Day/Night)
			if(strcmp(ptr, "Time:") == 0)
				parseTime();

			// Filename of the File with all IG to load for the scene.
			else if(strcmp(ptr, "IG:") == 0)
				parseIG(file, tmpBuff);

			// Ig for the init.
			else if(strcmp(ptr, "IG_Init:") == 0)
				parseIGInit();

			// Particle.
			else if(strcmp(ptr, "Particle:") == 0)
				parseParticle(file, tmpBuff);

			// Actor.
			else if(strcmp(ptr, "Actor:") == 0)
				parseActor(file, tmpBuff);

			// Camera.
			else if(strcmp(ptr, "Camera:") == 0)
				parseCamAnims(file, tmpBuff);

			// Action.
			else if(strcmp(ptr, "Sequence:") == 0)
				parseSequence(file, tmpBuff);

			// Bad Keyword
			else
				nlwarning("Unknown keyword '%s' at line %d.", ptr, _Line);
		}
	}
}// parse //


//-----------------------------------------------
// getNextValidLine :
// Skip empty lines and comment lines.
//-----------------------------------------------
void CSceneParser::getNextValidLine(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get the line.
		file.getline(buff, MAX_LINE_SIZE);

		// Increase line nnumber.
		_Line++;

		// Skip comments.
		if(buff[0] != 0)
		{
			// First char is not the beginning of a comment.
			if(buff[0] != '/')
				break;
			// Second char is not the end of a coment.
			else if(buff[1] != '/')
				break;
		}
	}
}// getNextValidLine //


//-----------------------------------------------
// parseTime :
// Day or Night.
//-----------------------------------------------
void CSceneParser::parseTime()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
	{
		if(atoi(ptr))
			_Day = true;
		else
			_Day = false;
	}
}// parseTime //

//-----------------------------------------------
// parseIG :
// Parse the list of IG to load for the scene
//-----------------------------------------------
void CSceneParser::parseIG(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_IG") == 0)
				break;
			// IG Name.
			else
				_IG.push_back(ptr);
		}
	}
}// parseIG //

//-----------------------------------------------
// parseIGInit :
// Get the name of the IG to initialize entities.
//-----------------------------------------------
void CSceneParser::parseIGInit()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_IG_Init = ptr;
}// parseIGInit //


///////////////
// PARTICLES //
//-----------------------------------------------
// parseParticle :
// Parse Particle.
//-----------------------------------------------
void CSceneParser::parseParticle(ifstream &file, char *buff)
{
	// Reset the current particle.
	_CurrentParticle.reset();

	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Particle") == 0)
				break;

			// Particle's Id
			else if(strcmp(ptr, "Id:") == 0)
				parseParticleId();

			// Particle's IG
			else if(strcmp(ptr, "IG:") == 0)
				parseParticleIG();

			// Particle's Cluster
			else if(strcmp(ptr, "Cluster:") == 0)
				parseParticleCluster();

			// Actor father of the particle is there is one.
			else if(strcmp(ptr, "Actor:") == 0)
				parseParticleActor();

			// Particle's Anims
			else if(strcmp(ptr, "Anims:") == 0)
				parseParticleAnims(file, buff);

			// Bad Keyword
			else
				nlwarning("Unknown keyword '%s' at line %d.", ptr, _Line);
		}
	}

	// If the current particle Id is Valid -> create the particle.
	if(_CurrentParticle.Id >= 0)
	{
		map<uint, CParticle>::iterator it = _Particles.find((uint)_CurrentParticle.Id);
		// Id already exists.
		if(it != _Particles.end())
			nlwarning("Particle before line %d has the ID %d that already exists -> Particle Not Created.", _Line, _CurrentParticle.Id);
		// Insert the new Particle.
		else
			_Particles.insert(make_pair((uint)_CurrentParticle.Id, _CurrentParticle));
	}
	else
		nlwarning("Particle before line %d has no ID or a Bad one -> Particle Not Created.", _Line);
}// parseParticle //

//-----------------------------------------------
// parseParticleId :
// Parse Particle's Id.
//-----------------------------------------------
void CSceneParser::parseParticleId()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentParticle.Id = atoi(ptr);
}// parseParticleId //

//-----------------------------------------------
// parseParticleIG :
// Parse Particle's IG.
//-----------------------------------------------
void CSceneParser::parseParticleIG()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentParticle.IG = ptr;
}// parseParticleIG //

//-----------------------------------------------
// parseParticleCluster :
// Parse Particle's Cluster.
//-----------------------------------------------
void CSceneParser::parseParticleCluster()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentParticle.Cluster = ptr;
}// parseParticleCluster //

//-----------------------------------------------
// parseParticleActor :
// Parse Particle's Actor.
//-----------------------------------------------
void CSceneParser::parseParticleActor()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentParticle.Actor = atoi(ptr);
}// parseParticleActor //

//-----------------------------------------------
// parseParticleAnims :
// Parse Particle's anims.
//-----------------------------------------------
void CSceneParser::parseParticleAnims(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Anims") == 0)
				break;
			// Meshes Name.
			else
				_CurrentParticle.Anims.push_back(ptr);
		}
	}
}// parseParticleAnims //


////////////
// ACTORS //
//-----------------------------------------------
// parseActor :
// Parse an actor.
//-----------------------------------------------
void CSceneParser::parseActor(ifstream &file, char *buff)
{
	// Reset the current actor.
	_CurrentActor.reset();

	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Actor") == 0)
				break;

			// Actor's Id
			else if(strcmp(ptr, "Id:") == 0)
				parseId();

			// Actor's Name
			else if(strcmp(ptr, "Name:") == 0)
				parseName();

			// Is the Actor a flyer.
			else if(strcmp(ptr, "Fly:") == 0)
				parseFly();

			// Actor father of the particle is there is one.
			else if(strcmp(ptr, "Actor:") == 0)
				parseActorActor();

			// Actor's Skeleton
			else if(strcmp(ptr, "Skel:") == 0)
				parseSkel();

			// Actor's Meshes
			else if(strcmp(ptr, "Meshes:") == 0)
				parseMeshes(file, buff);

			// Actor's Anims
			else if(strcmp(ptr, "Anims:") == 0)
				parseAnims(file, buff);

			// Bad Keyword
			else
				nlwarning("Unknown keyword '%s' at line %d.", ptr, _Line);
		}
	}

	// If the current actor Id is Valid -> create the actor.
	if(_CurrentActor.Id >= 0)
	{
		map<uint, CActor>::iterator it = _Actors.find((uint)_CurrentActor.Id);
		// Id already exists.
		if(it != _Actors.end())
			nlwarning("Actor before line %d has the ID %d that already exists -> Actor Not Created.", _Line, _CurrentActor.Id);
		// Insert the new actor.
		else
			_Actors.insert(make_pair((uint)_CurrentActor.Id, _CurrentActor));
	}
	else
		nlwarning("Actor before line %d has no ID or a Bad one -> Actor Not Created.", _Line);
}// parseActor //

//-----------------------------------------------
// parseId :
// Parse the current actor Id.
//-----------------------------------------------
void CSceneParser::parseId()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentActor.Id = atoi(ptr);
}// parseId //

//-----------------------------------------------
// parseName :
// Parse the current actor Name.
//-----------------------------------------------
void CSceneParser::parseName()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentActor.Name = ptr;
}// parseName //

//-----------------------------------------------
// parseFly :
// Is the Actor a flyer.
//-----------------------------------------------
void CSceneParser::parseFly()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
	{
		if(atoi(ptr))
			_CurrentActor.Fly = true;
		else
			_CurrentActor.Fly = false;
	}
}// parseFly //

//-----------------------------------------------
// parseActorActor :
// Parse Actor's Actor.
//-----------------------------------------------
void CSceneParser::parseActorActor()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
	{
		_CurrentActor.Actor = atoi(ptr);
		nlwarning("Actor: %d Follow : %d", _CurrentActor.Id, _CurrentActor.Actor);
	}
}// parseActorActor //

//-----------------------------------------------
// parseSkel :
// Parse the current actor Skeleton Name.
//-----------------------------------------------
void CSceneParser::parseSkel()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentActor.Skeleton = ptr;
}// parseSkel //

//-----------------------------------------------
// parseMeshes :
// Parse meshes used for the current actor.
//-----------------------------------------------
void CSceneParser::parseMeshes(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Meshes") == 0)
				break;
			// Meshes Name.
			else
				_CurrentActor.Meshes.push_back(ptr);
		}
	}
}// parseMeshes //

//-----------------------------------------------
// parseAnims :
// Parse anims used for the current actor.
//-----------------------------------------------
void CSceneParser::parseAnims(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Anims") == 0)
				break;
			// Meshes Name.
			else
				_CurrentActor.Anims.push_back(ptr);
		}
	}
}// parseAnims //


////////////
// CAMERA //
//-----------------------------------------------
// parseCamAnims :
// Parse all anims used for the camera in all sequence.
//-----------------------------------------------
void CSceneParser::parseCamAnims(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of Anims for the camera.
			if(strcmp(ptr, "End_Camera") == 0)
				break;
			// Meshes Name.
			else
				_CamAnims.push_back(ptr);
		}
	}
}// parseCamAnims //


///////////////
// SEQUENCES //
//-----------------------------------------------
// parseSequence :
// Parse a sequence.
//-----------------------------------------------
void CSceneParser::parseSequence(ifstream &file, char *buff)
{
	// Reset the current sequence.
	_CurrentSequence.reset();

	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Sequence") == 0)
				break;

			// Sequence Id.
			else if(strcmp(ptr, "Id:") == 0)
				parseSeqId();

			// Particle for the sequence.
			else if(strcmp(ptr, "Particle:") == 0)
				parseSeqParticle(file, buff);

			// Actor for the sequence.
			else if(strcmp(ptr, "Actor:") == 0)
				parseSeqActor(file, buff);

			// Camera for the sequence.
			else if(strcmp(ptr, "Camera:") == 0)
				parseSeqCam(file, buff);

			// Bad Keyword
			else
				nlwarning("Unknown keyword '%s' at line %d.", ptr, _Line);
		}
	}

	// If the sequence Id is Valid -> create the sequence.
	if(_CurrentSequence.Id >= 0)
	{
		map<uint, CSequence>::iterator itSeq = _Sequences.find((uint)_CurrentSequence.Id);
		// Id already exists.
		if(itSeq != _Sequences.end())
			nlwarning("Sequence before line %d has the ID %d that already exists -> Sequence Not Created.", _Line, _CurrentSequence.Id);
		// Insert the new actor.
		else
			_Sequences.insert(make_pair((uint)_CurrentSequence.Id, _CurrentSequence));
	}
	else
		nlwarning("Sequence before line %d has no ID or a Bad one -> Sequence Not Created.", _Line);
}// parseSequence //

//-----------------------------------------------
// parseSeqId :
// Parse the current sequence Id.
//-----------------------------------------------
void CSceneParser::parseSeqId()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentSequence.Id = atoi(ptr);
}// parseSeqId //

//-----------------------------------------------
// parseSeqParticle :
// Parse particle in the sequence.
//-----------------------------------------------
void CSceneParser::parseSeqParticle(ifstream &file, char *buff)
{
	// Reset the current particle in the sequence.
	_CurParticleSeq.reset();

	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Particle") == 0)
				break;

			// Actor in Sequence Id.
			else if(strcmp(ptr, "Id:") == 0)
				parseSeqParticleId();

			// Actor in Sequence anims.
			else if(strcmp(ptr, "Anims:") == 0)
				parseSeqParticleAnims(file, buff);

			// Bad Keyword
			else
				nlwarning("Unknown keyword '%s' at line %d.", ptr, _Line);
		}
	}

	// If the particle's Id in the sequence is Valid -> create the particleSeq.
	if(_CurParticleSeq.Id >= 0)
	{
		map<uint, CParticleSeq>::iterator itPartSeq = _CurrentSequence.ParticlesSeq.find((uint)_CurParticleSeq.Id);
		// Id already exists.
		if(itPartSeq != _CurrentSequence.ParticlesSeq.end())
			nlwarning("Particle in Sequence before line %d has the ID %d that already exists -> Particle in the sequence Not Created.", _Line, _CurParticleSeq.Id);
		// Insert the new particle in the sequence.
		else
			_CurrentSequence.ParticlesSeq.insert(make_pair((uint)_CurParticleSeq.Id, _CurParticleSeq));
	}
	else
		nlwarning("Particle in Sequence before line %d has no ID or a Bad one -> Particle in the sequence Not Created.", _Line);
}// parseSeqParticle //

//-----------------------------------------------
// parseSeqParticleId :
// Parse the current particle's Id in the current sequence.
//-----------------------------------------------
void CSceneParser::parseSeqParticleId()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurParticleSeq.Id = atoi(ptr);
}// parseSeqParticleId //

//-----------------------------------------------
// parseSeqParticleAnims :
// Parse anims used for the current particle in the current sequence.
//-----------------------------------------------
void CSceneParser::parseSeqParticleAnims(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Anims") == 0)
				break;
			// Meshes Name.
			else
				_CurParticleSeq.Anims.push_back(ptr);
		}
	}
}// parseSeqParticleAnims //

//-----------------------------------------------
// parseSeqActor :
// Parse an actor in a sequence.
//-----------------------------------------------
void CSceneParser::parseSeqActor(ifstream &file, char *buff)
{
	// Reset the current sequence.
	_CurrentActorSeq.reset();

	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Actor") == 0)
				break;

			// Actor in Sequence Id.
			else if(strcmp(ptr, "Id:") == 0)
				parseSeqActorId();

			// Actor in Sequence anims.
			else if(strcmp(ptr, "Anims:") == 0)
				parseSeqActorAnims(file, buff);

			// Bad Keyword
			else
				nlwarning("Unknown keyword '%s' at line %d.", ptr, _Line);
		}
	}

	// If the actor's Id in the sequence is Valid -> create the actorSeq.
	if(_CurrentActorSeq.Id >= 0)
	{
		map<uint, CActorSeq>::iterator itActSeq = _CurrentSequence.ActorsSeq.find((uint)_CurrentActorSeq.Id);
		// Id already exists.
		if(itActSeq != _CurrentSequence.ActorsSeq.end())
			nlwarning("Actor in Sequence before line %d has the ID %d that already exists -> Actor in the sequence Not Created.", _Line, _CurrentActorSeq.Id);
		// Insert the new actor.
		else
			_CurrentSequence.ActorsSeq.insert(make_pair((uint)_CurrentActorSeq.Id, _CurrentActorSeq));
	}
	else
		nlwarning("Actor in Sequence before line %d has no ID or a Bad one -> Actor in the sequence Not Created.", _Line);
}// parseSeqActor //

//-----------------------------------------------
// parseSeqActorId :
// Parse the current actor's Id in the current sequence.
//-----------------------------------------------
void CSceneParser::parseSeqActorId()
{
	char *ptr = strtok(NULL, delimiter);
	if(ptr != NULL)
		_CurrentActorSeq.Id = atoi(ptr);
}// parseSeqActorId //

//-----------------------------------------------
// parseSeqActorAnims :
// Parse anims used for the current actor in the current sequence.
//-----------------------------------------------
void CSceneParser::parseSeqActorAnims(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Anims") == 0)
				break;
			// Meshes Name.
			else
				_CurrentActorSeq.Anims.push_back(ptr);
		}
	}
}// parseSeqActorAnims //

//-----------------------------------------------
// parseSeqCam :
//  Parse camera in the sequence.
//-----------------------------------------------
void CSceneParser::parseSeqCam(ifstream &file, char *buff)
{
	while(!file.eof())
	{
		// Get next valid line.
		getNextValidLine(file, buff);

		char *ptr = strtok(buff, delimiter);
		if(ptr != NULL)
		{
			// End of the actor definition.
			if(strcmp(ptr, "End_Camera") == 0)
				break;
			// Meshes Name.
			else
				_CurrentSequence.CamAnims.push_back(ptr);
		}
	}
}// parseSeqCam //


////////////////
// INIT AFTER //
//-----------------------------------------------
// initActors :
// Initialize Actors Position and rotation .
//-----------------------------------------------
void CSceneParser::initActors()
{
	if(_IG_Init.empty())
		return;

	// Create the instance group with all start positions and rotations.
	UInstanceGroup *IGInit = UInstanceGroup::createInstanceGroup(_IG_Init);
	if(IGInit)
	{
		int nbInst = IGInit->getNumInstance();
		map<uint, CActor>::iterator itActor;
		for(int i = 0; i < nbInst; ++i )
		{
			sint id = atoi(IGInit->getInstanceName(i).c_str());
			itActor = _Actors.find(id);
			if(itActor != _Actors.end())
			{
				CActor &actor = (*itActor).second;
				actor.Pos = IGInit->getInstancePos(i);
				actor.Rot = IGInit->getInstanceRot(i);
			}
			else
				nlwarning("CSceneParser::initActors : Actor %d is in the IG \"%s\" but is not in the script.", id, _IG_Init.c_str());
		}

		// Destroy the IG to initialize (all is already backup now).
		delete IGInit;
		IGInit = 0;
	}
	else
		nlwarning("CSceneParser::initActors : Can't create the Instance group to initialize actors.");
}// initActors ///


//////////////
// PLAY SEQ //
//-----------------------------------------------
// apply :
// Initialize the scene with the parameters loaded from the script.
//-----------------------------------------------
void CSceneParser::apply()
{
	// If the scene is already initialized -> reset the scene.
	if(_Apply)
	{
		reset();
		return;
	}

	// Create a play list manager for the scene.
	_PlayListManager = Scene->createPlayListManager();

	// Initialize the Camera and reset it the second time.
	applyCamera();

	// Initialize IG for the scenery (IG present from start to end like trees or torch.
	applyIG();

	// Initialize Particles.
	applyParticles();

	// Initialize Actors.
	applyActors();

	// Scene already aplly now.
	_Apply = true;
}// apply //

//-----------------------------------------------
// reset :
// Reset the scene.
//-----------------------------------------------
void CSceneParser::reset()
{
	// Camera in scene mode.
	UserControls.mode(CUserControls::InterfaceMode);

	// Reset all particles.
	resetParticles();

	// Reset all actors.
	resetActors();
}// reset //

//-----------------------------------------------
// resetParticles :
// Reset all particles.
//-----------------------------------------------
void CSceneParser::resetParticles()
{
	map<uint, CParticle>::iterator itPart = _Particles.begin();
	for(itPart = _Particles.begin(); itPart != _Particles.end(); ++itPart)
	{
		// Get a reference on the particule.
		CParticle &particle = (*itPart).second;

		// Check if the IG of particles is not already in the scene.
		if(particle.IGPtr && _IGInScene.find(particle.IGPtr) != _IGInScene.end())
		{
			// Reset the playlist.
			particle.PlayList->resetAllChannels();

			// Remove from the set with all particle in the scene.
			_IGInScene.erase(particle.IGPtr);
			// Remove particles from scene.
			particle.IGPtr->removeFromScene(*Scene);
			Scene->deleteInstanceGroup (particle.IGPtr);

			// Load the IG of the particle.
			particle.IGPtr = 0;

			try
			{
				particle.IGPtr = UInstanceGroup::createInstanceGroup((*itPart).second.IG);
			}
			catch(Exception &e){nlwarning("CSceneParser::resetParticles : %s", e.what());}
		}
	}
}// resetParticles //

//-----------------------------------------------
// resetActors :
// Reset all actors.
//-----------------------------------------------
void CSceneParser::resetActors()
{
	// Create new entities from actors
	map<uint, CActor>::iterator itActor;
	for(itActor = _Actors.begin(); itActor != _Actors.end(); ++itActor)
	{
		// Get a reference on the actor.
		CActor &actor = (*itActor).second;

		// Get the Sid.
		Sid entityId(Sid::npc, (uint64)actor.Id);

		// Find the actor.
		CEntityCL *entity = getEntity(entityId);
		if(entity)
		{
			// Position the entity.
			entity->pacsPos(actor.Pos);

			// Position the skeleton at the same position as the entity.
			if(entity->skeleton())
			{
				entity->skeleton()->setPos(actor.Pos);
				entity->skeleton()->setRotQuat(actor.Rot);
			}
		}

		// Reset entity animations.
		resetAnimatedSceneObject(entityId);
	}
}// resetActors //


//-----------------------------------------------
// applyParticles :
// Initialize the scene with the parameters loaded from the script for particles.
//-----------------------------------------------
void CSceneParser::applyParticles()
{
	map<uint, CParticle>::iterator itPart = _Particles.begin();
	for(itPart = _Particles.begin(); itPart != _Particles.end(); ++itPart)
	{
		// Get a reference on the particule.
		CParticle &particle = (*itPart).second;

		// Create the animation set for the particule.
		particle.AnimationSet = Driver->createAnimationSet();

		// Add animations to the animation set.
		list<string>::iterator itPartAnim;
		for(itPartAnim = particle.Anims.begin(); itPartAnim != particle.Anims.end(); ++itPartAnim)
		{
			uint idAnim = UAnimationSet::NotFound;
			try
			{
				idAnim = particle.AnimationSet->addAnimation((*itPartAnim + string(".anim")).c_str(), (*itPartAnim).c_str());
			}
			catch(Exception &e) {nlwarning("%s", e.what());}

			if(idAnim != UAnimationSet::NotFound)
			{
				particle.AnimToId.insert(make_pair((*itPartAnim).c_str(), idAnim));
			}
			else
				nlwarning("CSceneParser::applyParticles : Anim %s cannot be add in the animation set", (*itPartAnim).c_str());
		}

		// Build animation set
		particle.AnimationSet->build();

		// Create playlist.
		particle.PlayList = _PlayListManager->createPlayList(particle.AnimationSet);
		if(particle.PlayList)
		{
			// Load the IG of the particle.
			particle.IGPtr = 0;
			try
			{
				particle.IGPtr = UInstanceGroup::createInstanceGroup((*itPart).second.IG);
			}
			catch(Exception &e) {nlwarning("%s", e.what());}

			if(particle.IGPtr)
			{
				// Get the position of Instances in IG.
				for(uint i = 0; i < particle.IGPtr->getNumInstance(); ++i )
					particle.IGPos.push_back(particle.IGPtr->getInstancePos(i));
			}
		}
	}
}// applyParticles //

//-----------------------------------------------
// applyCamera :
// Initialize the Camera and reset it the second time.
//-----------------------------------------------
void CSceneParser::applyCamera()
{
	// Create the animation set for the camera if not already done.
	_AnimationSet = Driver->createAnimationSet();
	if(_AnimationSet)
	{
		list<string>::iterator itCamAnims;
		for(itCamAnims = _CamAnims.begin(); itCamAnims != _CamAnims.end(); ++itCamAnims)
		{
			uint idAnim = UAnimationSet::NotFound;
			try
			{
				idAnim = _AnimationSet->addAnimation((*itCamAnims + string(".anim")).c_str(), (*itCamAnims).c_str());
			}
			catch(Exception &e) {nlwarning("%s", e.what());}

			if(idAnim != UAnimationSet::NotFound)
				_AnimCamToId.insert(make_pair((*itCamAnims).c_str(), idAnim));
			else
				nlwarning("CSceneParser::apply : Camera Anim %s cannot be add in the animation set", (*itCamAnims).c_str());
		}

		// Build animation set
		_AnimationSet->build();

		// create playlist
		_PlayList = _PlayListManager->createPlayList( _AnimationSet );
	}
	else
		nlwarning("CSceneParser::apply : AnimationSet cannot be created.");
}// applyCamera //

//-----------------------------------------------
// applyIG :
// Initialize IG for the scenery (IG present from start to end like trees or torch.
//-----------------------------------------------
void CSceneParser::applyIG()
{
	// Load Instances Groups for the scene.
	for(list<string>::iterator itIG = _IG.begin(); itIG != _IG.end(); ++itIG)
	{
		UInstanceGroup *IGTemp = 0;
		try
		{
			IGTemp = UInstanceGroup::createInstanceGroup(*itIG);
		}
		catch(Exception &e) {nlwarning("CSceneParser::applyIG : %s", e.what());}

		// Add Instance Group in the scene.
		if(IGTemp)
			IGTemp->addToScene(*Scene, Driver);
	}
}// applyIG //

//-----------------------------------------------
// applyActors :
// Initialize actors.
//-----------------------------------------------
void CSceneParser::applyActors()
{
	// Create new entities from actors
	map<uint, CActor>::iterator itActor;
	for(itActor = _Actors.begin(); itActor != _Actors.end(); ++itActor)
	{
		// Get a reference on the actor.
		CActor &actor = (*itActor).second;

		// Create the entity Sid.
		Sid entityId(Sid::npc, (uint64)actor.Id);

		// Create the entity type.
		CTypeEntity typeEntity;
		typeEntity.TypeInfo.TypeEntity	= CTypeEntity::npc;
		typeEntity.TypeInfo.Kind		= CTypeEntity::zorai;
		typeEntity.TypeInfo.Age			= CTypeEntity::adult;
		typeEntity.TypeInfo.Sex			= CTypeEntity::male;

		// Create the entity.
		CEntityCL *entityTmp = createEntity(entityId, typeEntity);
		if(entityTmp)
		{
			// Get a reference on the entity.
			CEntityCL &entity = *entityTmp;

			// True if the entity fly.
			entity.flyer(actor.Fly);

			// Set the name of the entity
			entity.name(actor.Name);
			// Time for the current frame.
			entity.time(T1);
			// Set the entity position.
			entity.pacsPos(actor.Pos);
			// Set the Front.
			CMatrix m;
			m.identity();
			m.setRot(actor.Rot);
			entity.front((m * CVector::J).normed());
			// Set the direction like the front.
			entity.dir(entity.front());
			// Set the vector UP.
			entity.up(CVector(0.f,0.f,1.f));

			// Assign a skeleton.
			entity.skeleton(actor.Skeleton);

			// Position the entity.
			entity.pacsPos(actor.Pos);

			// Position the skeleton at the same position as the entity.
			if(entity.skeleton())
			{
				entity.skeleton()->setPos(actor.Pos);
				entity.skeleton()->setRotQuat(actor.Rot);
			}

			// Assign meshes used for the entity.
			uint count = 0;
			list<string>::iterator itMeshes;
			for(itMeshes = actor.Meshes.begin(); itMeshes != actor.Meshes.end(); ++itMeshes)
			{
				// If there are too many meshes.
				if(count >= (uint)CEntityCL::NB_SLOT)
				{
					nlwarning("CSceneParser::applyActors : Too many meshes for Actor %d.", actor.Id);
					break;
				}

				entity.slot((CEntityCL::ESlots)count, *itMeshes);
				++count;
			}

			// Create the playlist
			addEntityClAnimatedSceneObject(entityId, actor.Anims);
		}
		else
			nlwarning("CSceneParser::applyActors : Entity %d Not created", actor.Id);
	}
}// applyActors //


//-----------------------------------------------
// playSeq :
// Play the sequence with the ID 'seq'.
//-----------------------------------------------
void CSceneParser::playSeq(uint seq, double timeInSec)
{
	// If the scene is not initialized -> return.
	if(!_Apply)
		return;

	// Camera in scene mode.
	UserControls.mode(CUserControls::SceneMode);

	_TimeStart = timeInSec;

	map<uint, CSequence>::iterator itSeq = _Sequences.find(seq);
	if(itSeq != _Sequences.end())
	{
		// Get a reference on the sequence.
		CSequence &sequence = (*itSeq).second;

		// Log the time used for the sequence.
		nlwarning("Sequence: %d, Time: %f", seq, timeInSec);

		// Camera in the sequence.
		if(!sequence.CamAnims.empty())
		{
			if(_PlayList)
			{
				map<string, uint>::iterator itAnimCamId;
				for(itAnimCamId = _AnimCamToId.begin(); itAnimCamId != _AnimCamToId.end(); ++itAnimCamId)
				{
					_PlayList->setTimeOrigin(0, timeInSec);
					_PlayList->setAnimation(0, (*itAnimCamId).second);
				}
			}
		}

		// Particles in the sequence.
		map<uint, CParticleSeq>::iterator itPartSeq;
		for(itPartSeq = sequence.ParticlesSeq.begin(); itPartSeq != sequence.ParticlesSeq.end(); ++itPartSeq)
		{
			CParticleSeq &particleSeq = (*itPartSeq).second;
			map<uint, CParticle>::iterator itPart = _Particles.find(particleSeq.Id);
			if(itPart != _Particles.end())
			{
				// Get a reference on the particle.
				CParticle &particle = (*itPart).second;
				// Check if the IG has been created.
				if(particle.IGPtr)
				{
					// Check if the IG of particles is not already in the scene.
					if(_IGInScene.find(particle.IGPtr) == _IGInScene.end())
					{
						// Insert IG in the set of IG in the scene.
						_IGInScene.insert(particle.IGPtr);
						// Cluster.
						if(IGCity.find(particle.Cluster) == IGCity.end())
							Scene->setToGlobalInstanceGroup(particle.IGPtr);
						else
							particle.IGPtr->setClusterSystem(IGCity[particle.Cluster]);
						// Add particles to the scene.
						particle.IGPtr->addToScene(*Scene, Driver);
						// Unfreeze the IG.
						particle.IGPtr->unfreezeHRC();

						//  Register all instances in the IG to a playlist.
						for(uint i = 0; i < particle.IGPtr->getNumInstance(); ++i )
						{
							std::string iName = particle.IGPtr->getInstanceName( i );
							UInstance  instance = particle.IGPtr->getByName( iName );
							particle.PlayList->registerTransform(instance);
							particle.PlayList->registerTransform(instance, (iName + ".").c_str());
						}
					}

					// Start the particle animation.
					map<string, uint>::iterator itAnimId;
					for(itAnimId = particle.AnimToId.begin(); itAnimId != particle.AnimToId.end(); ++itAnimId)
					{
						particle.PlayList->setTimeOrigin(0, timeInSec);
						particle.PlayList->setAnimation(0, (*itAnimId).second);
					}
				}
			}
		}

		// Actors in the sequence.
		map<uint, CActorSeq>::iterator itActSeq;
		for(itActSeq = sequence.ActorsSeq.begin(); itActSeq != sequence.ActorsSeq.end(); ++itActSeq)
		{
			CActorSeq &actorSeq = (*itActSeq).second;
			updateAnimationSequence(Sid(Sid::npc, (uint64)actorSeq.Id), actorSeq.Anims, 0 );
		}
	}
}// playSeq //


//-----------------------------------------------
// update :
// Update the scene.
//-----------------------------------------------
void CSceneParser::update(double timeInSec)
{
	// Nothing to update if the scene is not applied.
	if(!_Apply)
		return;

	// Update the scene currently playing.
	updateScene(timeInSec);

	// Update particles.
	updateParticles(timeInSec);

	// Animate the camera.
	updateCamera(timeInSec);

	// Update particle anims.
	if(_PlayListManager)
		_PlayListManager->animate(timeInSec);

	// Update actor position (for actor on another actor).
	updateActors();
}// update //

//-----------------------------------------------
// updateCamera :
// Update the camera (position, target, roll, fov)
//-----------------------------------------------
void CSceneParser::updateCamera(double timeInSec)
{
	// If there is a play list for the camera.
	if(_PlayList)
	{
		// Get the Id of the animation in the slot 0.
		uint idAnim = _PlayList->getAnimation(0);
		if(idAnim != UPlayList::empty)
		{
			UAnimation *animation = _AnimationSet->getAnimation(idAnim);
			if(animation)
			{
				// Get Camera information from the animation (Pos, Target, Roll).
				UTrack* trackRollCam	= animation->getTrackByName("Camera.roll");
				UTrack* trackFovCam		= animation->getTrackByName("Camera.fov");
				UTrack* trackPosCam		= animation->getTrackByName("Camera.PathPos");
				UTrack* trackPosTarget	= animation->getTrackByName("Camera.Target.PathPos");
				if(trackPosCam && trackPosTarget)
				{
					float rollCam = 0.f;
					CVector posCam;
					CVector posTarget;
					float difTime = (float)(timeInSec-_TimeStart);

					if(trackRollCam)
						trackRollCam->interpolate(difTime, rollCam);
					trackPosCam->interpolate(difTime, posCam);
					trackPosTarget->interpolate(difTime, posTarget);

					// Update camera transformations.
					UCamera cam = Scene->getCam();
					if(cam)
					{
						cam->setTransformMode(UTransformable::RotQuat);
						cam->lookAt(posCam, posTarget, rollCam);
						if(trackFovCam)
						{
							float fov;
							trackFovCam->interpolate(difTime, fov);
							CFrustum	fr= cam->getFrustum();
							// change only the fov
							cam->setPerspective(fov, fr.getAspectRatio(), fr.Near, fr.Far);
						}
					}

					// Update camera transformations for the Root.
					cam = SceneRoot->getCam();
					if(cam)
					{
						cam->setTransformMode(UTransformable::RotQuat);
						cam->lookAt(posCam, posTarget, rollCam);
						if(trackFovCam)
						{
							float fov;
							trackFovCam->interpolate(difTime, fov);
							CFrustum	fr= cam->getFrustum();
							// change only the fov
							cam->setPerspective(fov, fr.getAspectRatio(), fr.Near, fr.Far);
						}
					}
				}
			}
		}
	}
}// updateCamera //

//-----------------------------------------------
// updateActors :
// Update Actors.
//-----------------------------------------------
void CSceneParser::updateActors()
{
	// All actors in the scene.
	for(map<uint, CActor>::iterator itActor = _Actors.begin(); itActor != _Actors.end(); ++itActor)
	{
		// Get a reference on the actor.
		CActor &actor = (*itActor).second;

		// If there is no actor to follow -> next actor.
		if(actor.Actor < 0)
			continue;

		// Get the entity pointer.
		CEntityCL *entity = getEntity(Sid(Sid::npc, (uint64)actor.Id));
		if(!entity)
		{
			nlwarning("CSceneParser::updateActors : Cannot get the actor %d.", actor.Id);
			continue;
		}

		// Get the target entity pointer.
		CEntityCL *entityTarget = getEntity(Sid(Sid::npc, (uint64)actor.Actor));
		if(!entityTarget)
		{
			nlwarning("CSceneParser::updateActors : Cannot get the targeted actor %d.", actor.Actor);
			continue;
		}

		// Changes the entity position.
		entity->pacsPos(entityTarget->pos());
	}
}// updateActors //

//-----------------------------------------------
// updateParticles :
// Update particles.
//-----------------------------------------------
void CSceneParser::updateParticles(double timeInSec)
{
	for(map<uint, CParticle>::iterator itPart = _Particles.begin(); itPart != _Particles.end(); ++itPart)
	{
		// Get a reference on the particule.
		CParticle &particle = (*itPart).second;

		// If the IG pointer is null -> Next particle.
		if(!(particle.IGPtr))
			continue;

		// If the play list is NULL -> Next particle
		if(!(particle.PlayList))
			continue;

		// Get the Id of the animation in the slot 0 -> if empty -> Next particle.
		uint idAnim = particle.PlayList->getAnimation(0);
		if(idAnim == UPlayList::empty)
			continue;

		// Get the animation pointer.
		UAnimation *animation = particle.AnimationSet->getAnimation(idAnim);
		if(!animation)
			continue;

		// Get the time difference.
		float difTime = (float)(timeInSec-_TimeStart);

		// Particle do not follow anything.
		if(particle.Actor < 0)
		{
			updateParticlesNoActor(difTime, particle, *animation);
		}
		// Particle follow an actor.
		else
		{
			updateParticlesActor(difTime, particle, *animation);
		}
	}
}// updateParticles //

//-----------------------------------------------
// updateParticlesNoActor :
//
//-----------------------------------------------
void CSceneParser::updateParticlesNoActor(float difTime, CParticle &particle, UAnimation &animation)
{
	// Animate all instances.
	for(uint i = 0; i < particle.IGPtr->getNumInstance(); ++i )
	{
		std::string iName = particle.IGPtr->getInstanceName(i);
		UInstance instance = particle.IGPtr->getByName(iName);

		if(!instance)
			continue;

		instance->setTransformMode(UTransformable::RotQuat);

		// If the animation has no track of position.
		UTrack* trackPos = animation.getTrackByName("PathPos");
		if(!trackPos)
			trackPos = animation.getTrackByName(string(iName + "." + "PathPos").c_str());
		if(trackPos)
		{
			CVector pos;
			trackPos->interpolate(difTime, pos);
			instance->setPos(pos);
		}

		// If the animation has no track of rotation.
		UTrack* trackRot = animation.getTrackByName("PathRotQuat");
		if(!trackRot)
			trackRot = animation.getTrackByName(string(iName + "." + "PathRotQuat").c_str());
		if(trackRot)
		{
			CQuat rot;
			if(trackRot->interpolate(difTime, rot))
				instance->setRotQuat(rot);
			else
				nlwarning("CSceneParser::updateParticles : Not a Quat!");
		}
	}
}// updateParticlesNoActor //


//-----------------------------------------------
// updateParticlesActor :
//
//-----------------------------------------------
void CSceneParser::updateParticlesActor(float difTime, CParticle &particle, UAnimation &animation)
{
	// Get the entity pointer.
	CEntityCL *entity = getEntity(Sid(Sid::npc, (uint64)particle.Actor));
	if(!entity)
	{
		nlwarning("CSceneParser::updateParticlesActor : cannot get the actor %d.", (uint64)particle.Actor);
		return;
	}

	// If the entity has no skeleton -> Next particle.
	if(!entity->skeleton())
	{
		nlwarning("The particle follow an entity %d without a skeleton.", (uint64)particle.Actor);
		return;
	}

	// Matrix 90 degrees
	CMatrix m90;
	m90.identity();
	m90.rotateZ((float)(Pi/2.0));

	// Matrix of the entity.
	CMatrix mChar = entity->skeleton()->getMatrix();
	mChar.setPos(entity->pos());

	// Animate all instances.
	for(uint i = 0; i < particle.IGPtr->getNumInstance(); ++i )
	{
		std::string iName = particle.IGPtr->getInstanceName(i);
		UInstance instance = particle.IGPtr->getByName(iName);

		if(!instance)
			continue;

		instance->setTransformMode(UTransformable::RotQuat);

		CMatrix mAnim;
		mAnim.identity();
		// If the animation has no track of position.
		UTrack* trackPos = animation.getTrackByName("PathPos");
		if(!trackPos)
			trackPos = animation.getTrackByName(string(iName + "." + "PathPos").c_str());
		if(trackPos)
		{
			CVector pos;
			trackPos->interpolate(difTime, pos);
			mAnim.setPos(pos);
		}

		// If the animation has no track of rotation.
		UTrack* trackRot = animation.getTrackByName("PathRotQuat");
		if(!trackRot)
			trackRot = animation.getTrackByName(string(iName + "." + "PathRotQuat").c_str());
		if(trackRot)
		{
			CQuat rot;
			trackPos->interpolate(difTime, rot);
			mAnim.setRot(rot);
		}

		CMatrix mFinal = mChar * m90 * mAnim;
		instance->setPos(mFinal.getPos());
		instance->setRotQuat(mFinal.getRot());
	}
}// updateParticlesActor //


//-----------------------------------------------
// loadScene :
// Load a scene from a file and put it in memory.
// \param filename : filename for the file that contains the scene.
// \warning This function clear the old scene.
//-----------------------------------------------
void CSceneParser::loadScene(const string &filename)
{
	// Look for the file -> nlwarning and return if sceneFile is empty.
	string sceneFile = CPath::lookup(filename, false);
	if(sceneFile.empty())
	{
		nlwarning("CSceneParser::loadScene : Can't find file \"%s\".", filename.c_str());
		return;
	}

	// Open the file.
	ifstream file(sceneFile.c_str(), ios::in);
	if(file.is_open())
	{
		// ...
		char tmpBuff[MAX_LINE_SIZE];

		// While the end of the file is not reached -> parse the script.
		while(!file.eof())
		{
			// Get next valid line.
			getNextValidLine(file, tmpBuff);

			pair<sint, double> seq;
			// ...
			char *ptr = strtok(tmpBuff, delimiter);
			if(ptr != NULL)
			{
				seq.first = atoi(ptr);
				ptr = strtok(NULL, delimiter);
				if(ptr != NULL)
				{
					seq.second = atof(ptr);
					_Scene.push_back(seq);
				}
			}
		}

		// Close the File.
		file.close();
	}
	else
	{
		nlwarning("CSceneParser::loadScene : File \"%s\" can't be open.", sceneFile.c_str());
		return;
	}
}// loadScene //

//-----------------------------------------------
// playScene :
// Play a scene in memory.
// \param timeInSec : current time in second.
//-----------------------------------------------
void CSceneParser::playScene(double timeInSec)
{
	// Initialize the Iterator for the scene.
	_ItScene = _Scene.begin();
	// Initialize the time for the scene.
	_SceneStart = timeInSec;

	// Initialize or Reset the scene.
	apply();

	// Play the first sequence to play for this scene.
	updateScene(timeInSec);
}// playScene //

//-----------------------------------------------
// updateScene :
// Update the scene currently playing.
// \param timeInSec : current time in second.
//-----------------------------------------------
void CSceneParser::updateScene(double timeInSec)
{
	// If there are still sequences in the scene.
	if(_ItScene != _Scene.end())
	{
		double time = timeInSec-_SceneStart;
		if((*_ItScene).second <= time)
		{
			// If the scene is finish -> Stop it.
			if((*_ItScene).first < 0)
			{
				stopScene();
			}
			else
			{
				// Play the sequence.
				playSeq((*_ItScene).first, timeInSec);

				// Next sequence.
				_ItScene++;
			}
		}
	}
}// updateScene //

//-----------------------------------------------
// stopScene :
// Stop the scene currently playing.
//-----------------------------------------------
void CSceneParser::stopScene()
{
	_ItScene = _Scene.end();
	_SceneStart = -1.f;
}// stopScene //

//-----------------------------------------------
// Function to know if there is a scene currently playing.
// \return bool : 'true' if there is a scene currently playing.
//-----------------------------------------------
bool CSceneParser::isScenePlaying()
{
	return (_ItScene != _Scene.end());
}// isScenePlaying //

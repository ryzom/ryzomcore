// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2012-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdsound.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
//#include "nel/3d/std3d.h"
#include "nel/3d/scene.h"
#include "nel/3d/scene_user.h"
#include "nel/3d/cluster.h"
#include "nel/3d/portal.h"
#include "nel/sound/driver/listener.h"
#include "nel/sound/audio_mixer_user.h"
#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/effect.h"
#include "nel/sound/clustered_sound.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace NLSOUND
{

#if EAX_AVAILABLE == 1
// An array to report all EAX predefined meterials
float EAX_MATERIAL_PARAM[][3] =
{
	{EAX_MATERIAL_SINGLEWINDOW}, {EAX_MATERIAL_SINGLEWINDOWLF}, {EAX_MATERIAL_SINGLEWINDOWROOMRATIO},
	{EAX_MATERIAL_DOUBLEWINDOW}, {EAX_MATERIAL_DOUBLEWINDOWHF}, {EAX_MATERIAL_DOUBLEWINDOWHF},
	{EAX_MATERIAL_THINDOOR}, {EAX_MATERIAL_THINDOORLF}, {EAX_MATERIAL_THINDOORROOMRATIO},
	{EAX_MATERIAL_THICKDOOR}, {EAX_MATERIAL_THICKDOORLF}, {EAX_MATERIAL_THICKDOORROOMRTATION},
	{EAX_MATERIAL_WOODWALL}, {EAX_MATERIAL_WOODWALLLF}, {EAX_MATERIAL_WOODWALLROOMRATIO},
	{EAX_MATERIAL_BRICKWALL}, {EAX_MATERIAL_BRICKWALLLF}, {EAX_MATERIAL_BRICKWALLROOMRATIO},
	{EAX_MATERIAL_STONEWALL}, {EAX_MATERIAL_STONEWALLLF}, {EAX_MATERIAL_STONEWALLROOMRATIO},
	{EAX_MATERIAL_CURTAIN}, {EAX_MATERIAL_CURTAINLF}, {EAX_MATERIAL_CURTAINROOMRATIO}
};
#else	// EAX_AVAILABLE
// No EAX, just have an array of gain factor to apply for each material type
float EAX_MATERIAL_PARAM[] =
{
	float(pow((double)10, (double)-2800/2000)),
	float(pow((double)10, (double)-5000/2000)),
	float(pow((double)10, (double)-1800/2000)),
	float(pow((double)10, (double)-4400/2000)),
	float(pow((double)10, (double)-4000/2000)),
	float(pow((double)10, (double)-5000/2000)),
	float(pow((double)10, (double)-6000/2000)),
	float(pow((double)10, (double)-1200/2000))
};
#endif	// EAX_AVAILABLE

// An utility class to handle packed sheet loading/saving/updating
class CSoundGroupSerializer
{
public:
	std::vector<std::pair<NLMISC::TStringId, NLMISC::TStringId> >	_SoundGroupAssoc;

	// load the values using the george sheet (called by GEORGE::loadForm)
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const std::string &/* name */)
	{
		try
		{
			NLGEORGES::UFormElm &root = form->getRootNode();
			NLGEORGES::UFormElm *items;
			uint size;
			root.getNodeByName(&items, ".Items");
			items->getArraySize(size);

			for (uint i=0; i<size; ++i)
			{
				std::string soundGroup;
				std::string sound;

				NLGEORGES::UFormElm *item;

				items->getArrayNode(&item, i);

				item->getValueByName(soundGroup, ".SoundGroup");
				item->getValueByName(sound, ".Sound");

				string::size_type n = sound.rfind(".sound");

				if (n != string::npos)
				{
					// remove the tailing .sound
					sound = sound.substr(0, n);
				}

				_SoundGroupAssoc.push_back(make_pair(CStringMapper::map(soundGroup), CStringMapper::map(sound)));
			}
		}
		catch(...)
		{
		}
	}

	// load/save the values using the serial system (called by GEORGE::loadForm)
	void serial (NLMISC::IStream &s)
	{
		uint32 size;
		if (!s.isReading())
		{
			size = (uint32)_SoundGroupAssoc.size();
		}
		s.serial(size);

		for (uint i=0; i<size; ++i)
		{
			if (s.isReading())
			{
				std::string soundGroup;
				std::string sound;

				s.serial(soundGroup);
				s.serial(sound);

				_SoundGroupAssoc.push_back(make_pair(CStringMapper::map(soundGroup), CStringMapper::map(sound)));
			}
			else
			{
				std::string soundGroup;
				std::string sound;

				soundGroup = CStringMapper::unmap(_SoundGroupAssoc[i].first);
				sound = CStringMapper::unmap(_SoundGroupAssoc[i].second);

				s.serial(soundGroup);
				s.serial(sound);
			}
		}
	}

	/** called by GEORGE::loadForm when a sheet read from the packed sheet is no more in
	 *	the directories.
	 */
	void removed()
	{
		// nothing to do
	}

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 1; }

};

CClusteredSound::CClusteredSound()
:	_Scene(0),
	_RootCluster(0),
	_LastEnv(CStringMapper::emptyId()),
	_LastEnvSize(-1.0f) // size goes from 0.0f to 100.0f
{
	
}

void CClusteredSound::buildSheets(const std::string &packedSheetPath)
{
	std::map<std::string, CSoundGroupSerializer> container;
	::loadForm("sound_group", packedSheetPath + "sound_groups.packed_sheets", container, true, false);
}

void CClusteredSound::init(NL3D::CScene *scene, float portalInterpolate, float maxEarDist, float minGain)
{
	// load the sound_group sheets
	std::map<std::string, CSoundGroupSerializer> container;
	::loadForm("sound_group", CAudioMixerUser::instance()->getPackedSheetPath()+"sound_groups.packed_sheets", container, CAudioMixerUser::instance()->getPackedSheetUpdate(), false);

	// copy the container data into internal structure
	std::map<std::string, CSoundGroupSerializer>::iterator first(container.begin()), last(container.end());
	for (; first != last; ++first)
	{
		_SoundGroupToSound.insert(first->second._SoundGroupAssoc.begin(), first->second._SoundGroupAssoc.end());
	}

	// and clear the temporary Container
	container.clear();


	_Scene = scene;
	_PortalInterpolate = portalInterpolate;
	_MaxEarDistance = maxEarDist;
	_MinGain = minGain;
	if(scene != 0)
	{
		_RootCluster = _Scene->getClipTrav().RootCluster;
	}
	else
		_RootCluster = 0;
}

void CClusteredSound::update(const CVector &listenerPos, const CVector &/* view */, const CVector &/* up */)
{
	H_AUTO(NLSOUND_ClusteredSoundUpdate)
	if (_Scene == 0)
	{
		// hum... what to do ?
		static bool bDisplayOnce = false;
		if (!bDisplayOnce)
		{
			nlinfo("CClusteredSound::update : no scene specified !");
			bDisplayOnce = true;
		}
		return;
	}

	CClipTrav	&clipTrav = _Scene->getClipTrav ();

	// Retreive the list of cluster where the listener is
	vector<CCluster*> vCluster;
	clipTrav.fullSearch (vCluster, listenerPos);


	// reset the audible cluster map
	_AudibleClusters.clear();

	// create the initial travesal context
	CSoundTravContext stc(listenerPos, false, false);

	// and start the cluster traversal to find out what cluster is audible and how we ear it
	soundTraverse(vCluster, stc);

	//-----------------------------------------------------
	// update the clustered sound (create and stop sound)
	//-----------------------------------------------------

//	std::hash_map<uint, CClusterSound>		newSources;
	TClusterSoundCont		newSources;

	{
		// fake the distance for all playing source
//		std::map<std::string, CClusterSound>::iterator first(_Sources.begin()), last(_Sources.end());
		TClusterSoundCont::iterator first(_Sources.begin()), last(_Sources.end());
		for (; first != last; ++first)
		{
			first->second.Distance = FLT_MAX;
		}
	}


	TClusterStatusMap::const_iterator first(_AudibleClusters.begin()), last(_AudibleClusters.end());
	for (; first != last; ++first )
	{
		static NLMISC::TStringId NO_SOUND_GROUP = CStringMapper::emptyId();
		const CClusterSoundStatus &css = first->second;
		CCluster *cluster = first->first;
		NLMISC::TStringId soundGroup;

		soundGroup = cluster->getSoundGroupId();


		if (soundGroup != NO_SOUND_GROUP)
		{
			// search an associated sound name
			TClusterSoundCont::iterator it(_Sources.find(soundGroup));
			if (it != _Sources.end())
			{
				// the source is already playing, check and replace if needed
				CClusterSound &cs = it->second;

				if (cs.Distance >= css.Dist)
				{
					// this one is better !
					cs.Distance = css.Dist;
					cs.Source->setPos(listenerPos + css.Direction * css.Dist + CVector(0,0,2));
					if (css.DistFactor < 1.0f)
						cs.Source->setRelativeGain(css.Gain * (1.0f - (css.DistFactor*css.DistFactor*css.DistFactor*css.DistFactor)));
					else
						cs.Source->setRelativeGain(css.Gain);
				}
				newSources.insert(make_pair(soundGroup, cs));
			}
			else
			{
				// create a new source

//				nldebug("Searching sound assoc for group [%s]", CStringMapper::unmap(soundGroup).c_str());

				TStringStringMap::iterator it2(_SoundGroupToSound.find(soundGroup));
				if (it2 != _SoundGroupToSound.end())
				{
					NLMISC::TStringId soundName = it2->second;
					CClusterSound cs;

//					nldebug("Found the sound [%s] for sound group [%s]", CStringMapper::unmap(soundName).c_str(), CStringMapper::unmap(soundGroup).c_str());

					cs.Distance = css.Dist;
					cs.Source = CAudioMixerUser::instance()->createSource(soundName, false, NULL, NULL, cluster);
					if (cs.Source != 0)
					{
						cs.Source->setPos(listenerPos + css.Direction * css.Dist + CVector(0,0,2));
						if (css.DistFactor < 1.0f)
							cs.Source->setRelativeGain(css.Gain * (1.0f - (css.DistFactor*css.DistFactor/**css.DistFactor*css.DistFactor*/)));
						else
							cs.Source->setRelativeGain(css.Gain);
						cs.Source->setLooping(true);
						newSources.insert(make_pair(soundGroup, cs));
					}
				}
			}
		}
	}
	// check for source to stop
	{
		TClusterSoundCont	oldSources;
		oldSources.swap(_Sources);

		TClusterSoundCont::iterator first(newSources.begin()), last(newSources.end());
		for (; first != last; ++first)
		{
			_Sources.insert(*first);
			if (!first->second.Source->isPlaying())
				first->second.Source->play();

			oldSources.erase(first->first);
		}

		while (!oldSources.empty())
		{
			CClusterSound &cs = oldSources.begin()->second;
			delete cs.Source;
			oldSources.erase(oldSources.begin());
		}
	}

	// update the environment effect (if any)
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	if (mixer->useEnvironmentEffects() && !vCluster.empty())
	{
		H_AUTO(NLSOUND_ClusteredSound_updateEnvFx)
		TStringId fxId = vCluster[0]->getEnvironmentFxId();
		const CAABBox &box = vCluster[0]->getBBox();
		CVector vsize = box.getHalfSize();
		float	size = NLMISC::minof(vsize.x, vsize.y, vsize.z) * 2;

		// special case for root cluster (ie, external)
		if (vCluster[0] == _RootCluster)
		{
			// this is the root cluster. This cluster have a size of 0 !
			size = 100.f;
		}
		else
		{
			// else, clip the env size to max eax supported size
			clamp(size, 1.f, 100.f);
		}

		// only update environment if there is some change.
		if (fxId != _LastEnv || size != _LastEnvSize)
		{
			nldebug("AM: CClusteredSound => setEnvironment %s %f", CStringMapper::unmap(fxId).c_str(), size);
			mixer->setEnvironment(fxId, size);
			_LastEnv = fxId;
			_LastEnvSize = size;
		}
	}
}

const CClusteredSound::CClusterSoundStatus *CClusteredSound::getClusterSoundStatus(NL3D::CCluster *cluster)
{
	TClusterStatusMap::iterator it(_AudibleClusters.find(cluster));

	if (it == _AudibleClusters.end())
	{
		return 0;
	}
	else
		return &(it->second);
}


NL3D::CCluster	*CClusteredSound::getRootCluster()
{
	if (_Scene == 0)
		return 0;

	return _Scene->getClipTrav().RootCluster;
}


void CClusteredSound::soundTraverse(const std::vector<CCluster *> &clusters, CSoundTravContext &travContext)
{
	H_AUTO(NLSOUND_soundTraverse)
//	std::map<CCluster*, CSoundTravContext>	nextTraverse;
	std::vector<std::pair<const CCluster*, CSoundTravContext> >	curClusters;
	CVector		realListener (travContext.ListenerPos);

	_AudioPath.clear();

	// fill the initial cluster liste
	CClusterSoundStatus css;
	css.Direction = CVector::Null;
	css.DistFactor = 0.0f;
	css.Dist = 0.0f;
	css.Gain = 1.0f;
	css.Occlusion = 0;
	css.OcclusionLFFactor = 1.0f;
	css.OcclusionRoomRatio = 1.0f;
	css.Obstruction = 0;
	css.PosAlpha = 0;
//	css.Position = CVector::Null;
	css.Position = realListener;

	for (uint i=0; i<clusters.size(); ++i)
	{
		bool valid = true;
		// eliminate cluster when listener is behind their portals AND inside the other cluster
		for (uint j=0; j<clusters[i]->getNbPortals(); j++)
		{
			CPortal *portal = clusters[i]->getPortal(j);
			const std::vector<CVector> &poly = portal->getPoly();

			if (poly.size() < 3)
			{
				// only warn once, avoid log flooding !
				static std::set<std::string>	warned;
				if (warned.find(clusters[i]->Name) == warned.end())
				{
					nlwarning("Cluster [%s] contains a portal [%s] with less than 3 vertex !",
						clusters[i]->Name.c_str(), portal->getName().empty() ? "no name" : portal->getName().c_str());
					warned.insert(clusters[i]->Name);
				}
				valid = false;
				continue;
			}
			CVector normal = (poly[0] - poly[1]) ^ (poly[2] - poly[1]);

			float dist = (realListener - poly[0]) * normal;
			float dist2 = (clusters[i]->getBBox().getCenter() - poly[0]) * normal;

			if ((dist < 0 && dist2 > 0) || (dist > 0 && dist2 < 0))
			{
				if (portal->getCluster(0) == clusters[i])
				{
					if (find(clusters.begin(), clusters.end(), portal->getCluster(1)) != clusters.end())
					{
						valid = false;
						continue;
					}
				}
				else if (find(clusters.begin(), clusters.end(), portal->getCluster(0)) != clusters.end())
				{
					valid = false;
					continue;
				}

			}

/*
			if (portal->getCluster(0) == clusters[i] && dist > 0)
//				if (!portal->isInFront(realListener))
			{
				valid = false;
				continue;
			}
			else if (portal->getCluster(1) == clusters[i] && dist < 0)
//				if (portal->isInFront(realListener))
			{
				valid = false;
				continue;
			}
*/
		}
		if( valid)
		{
			curClusters.push_back(make_pair(clusters[i], travContext));
			addAudibleCluster(clusters[i], css);
		}
	}

	do
	{
		// add the next traverse (if any)
		std::copy(_NextTraversalStep.begin(), _NextTraversalStep.end(), std::back_inserter(curClusters));
		_NextTraversalStep.clear();

		while (!curClusters.empty())
		{
			CCluster * cluster = const_cast<CCluster*>(curClusters.back().first);
			CSoundTravContext &travContext = curClusters.back().second;

			CClusterSoundStatus css;
			css.DistFactor = 0.0f;
			css.Position = CVector::Null;
			css.PosAlpha = 0.0f;
			css.Gain = travContext.Gain;
			css.Dist = travContext.Dist;
			css.Direction = travContext.Direction;
			css.Occlusion = travContext.Occlusion;
			css.OcclusionLFFactor = travContext.OcclusionLFFactor;
			css.OcclusionRoomRatio = travContext.OcclusionRoomRatio;
			css.Obstruction = travContext.Obstruction;

			// store this cluster and it's parameters
			_AudibleClusters.insert(make_pair(cluster, css));

			// 1st, look each portal
			uint i;
			for (i=0; i<cluster->getNbPortals(); ++i)
			{
				CPortal *portal = cluster->getPortal(i);
				// get the other cluster
				CCluster *otherCluster = portal->getCluster(0);
				bool	clusterInFront = true;
				if (otherCluster == cluster)
				{
					otherCluster = portal->getCluster(1);
					clusterInFront = false;
				}
				nlassert(otherCluster != cluster);

				if (otherCluster && travContext.PreviousCluster != otherCluster) // && (!travContext.FilterUnvisibleChild || otherCluster->AudibleFromFather))
				{
					const vector<CVector> &poly = portal->getPoly();

					// a security test
					if (poly.size() < 3)
					{
						// only warn once, avoid log flooding !
						static std::set<std::string>	warned;
						if (warned.find(cluster->Name) == warned.end())
						{
							nlwarning("Cluster [%s] contains a portal [%s] with less than 3 vertex !",
								cluster->Name.c_str(), portal->getName().empty() ? "no name" : portal->getName().c_str());
							warned.insert(cluster->Name);
						}
					}
					else
					{

						// Test to skip portal with suface > 40 m2 (aprox)
						float surface = ((poly[2]-poly[1]) ^ (poly[0]-poly[1])).norm();
						if (surface > 340 /* && otherCluster->isIn(travContext.ListenerPos, travContext.MaxDist-travContext.Dist)*/)
						{
							float minDist;
							CVector nearPos;
							CAABBox box = otherCluster->getBBox();

							minDist = getAABoxNearestPos(box, travContext.ListenerPos, nearPos);

							if (travContext.Dist + minDist < _MaxEarDistance)
							{
								CVector soundDir = (nearPos - travContext.ListenerPos).normed();
								CClusterSoundStatus css;
								css.Gain = travContext.Gain;
								css.Dist = travContext.Dist + minDist;
								css.Occlusion = travContext.Occlusion;
								css.OcclusionLFFactor = travContext.OcclusionLFFactor;
								css.Obstruction = travContext.Obstruction;
								css.OcclusionRoomRatio = travContext.OcclusionRoomRatio;
								css.DistFactor = css.Dist / _MaxEarDistance;
								css.Direction = travContext.Direction;

								float alpha = travContext.Alpha;
								CVector d1(travContext.Direction1), d2;

								css.Direction = interpolateSourceDirection(travContext, css.Dist+travContext.Dist, nearPos, travContext.ListenerPos /*realListener*/, d1, d2, alpha);
								css.Position = nearPos + css.Dist * css.Direction;
								css.PosAlpha = min(1.0f, css.Dist / _PortalInterpolate);

								if (addAudibleCluster(otherCluster, css))
								{
	//								debugLines.push_back(CLine(travContext.ListenerPos, nearPos));
									CSoundTravContext stc(travContext);
									stc.FilterUnvisibleChild = true;
									stc.Direction1 = d1;
									stc.Direction2 = d2;
									stc.Direction = css.Direction;
									stc.PreviousCluster = cluster;
									stc.Alpha = alpha;
									stc.PreviousVector = (nearPos - travContext.ListenerPos).normed();
									addNextTraverse(otherCluster, stc);
									_AudioPath.push_back(make_pair(travContext.ListenerPos, nearPos));
								}
							}
						}
						else
						{
							// find the nearest point of this portal (either on of the perimeter vertex or a point on the portal surface)
							float minDist;
							CVector nearPos;

							minDist = getPolyNearestPos(poly, travContext.ListenerPos, nearPos);

							if (travContext.Dist+minDist < _MaxEarDistance)
							{
								// note: this block of code is a mess and should be cleaned up and commented =)
								// TODO : compute relative gain according to portal behavior.
								CClusterSoundStatus css;
								css.Gain = travContext.Gain;
								CVector soundDir = (nearPos - travContext.ListenerPos).normed();
								/* ****** Todo: OpenAL EFX & XAudio2 implementation of Occlusion & Obstruction (not implemented for fmod anyways) !!! ******
								TStringId occId = portal->getOcclusionModelId();
								TStringIntMap::iterator it(_IdToMaterial.find(occId));
								   ****** Todo: OpenAL EFX & XAudio2 implementation of Occlusion & Obstruction (not implemented for fmod anyways) !!! ****** */

	#if EAX_AVAILABLE == 1 // EAX_AVAILABLE no longer used => TODO: implement with EFX and remove when new implementation OK.
								if (it != _IdToMaterial.end())
								{
									// found an occlusion material for this portal
									uint matId = it->second;
									css.Occlusion = max(sint32(EAXBUFFER_MINOCCLUSION), sint32(travContext.Occlusion + EAX_MATERIAL_PARAM[matId][0])); //- 1800); //EAX_MATERIAL_THINDOOR;
									css.OcclusionLFFactor = travContext.OcclusionLFFactor * EAX_MATERIAL_PARAM[matId][1]; //EAX_MATERIAL_THICKDOORLF; //0.66f; //0.0f; //min(EAX_MATERIAL_THINDOORLF, travContext.OcclusionLFFactor);
									css.OcclusionRoomRatio = EAX_MATERIAL_PARAM[matId][2] * travContext.OcclusionRoomRatio;
								}
								else
								{
									// the id does not match any know material
									css.Occlusion = travContext.Occlusion;
									css.OcclusionLFFactor = travContext.OcclusionLFFactor;
									css.OcclusionRoomRatio = travContext.OcclusionRoomRatio;
								}
	#else	// EAX_AVAILABLE
								/* ****** Todo: OpenAL EFX & XAudio2 implementation of Occlusion & Obstruction (not implemented for fmod anyways) !!! ******
								if (it != _IdToMaterial.end())
								{
									// found an occlusion material for this portal
									uint matId = it->second;
									css.Gain *= EAX_MATERIAL_PARAM[matId];
								}
								   ****** Todo: OpenAL EFX & XAudio2 implementation of Occlusion & Obstruction (not implemented for fmod anyways) !!! ****** */
	#endif	// EAX_AVAILABLE
	/*							if (portal->getOcclusionModel() == "wood door")
								{
	//								css.Gain *= 0.5f;
	#if EAX_AVAILABLE == 1
									css.Occlusion = max(EAXBUFFER_MINOCCLUSION, travContext.Occlusion + EAX_MATERIAL_THICKDOOR); //- 1800); //EAX_MATERIAL_THINDOOR;
									css.OcclusionLFFactor = 0.1f * travContext.OcclusionLFFactor; //EAX_MATERIAL_THICKDOORLF; //0.66f; //0.0f; //min(EAX_MATERIAL_THINDOORLF, travContext.OcclusionLFFactor);
									css.OcclusionRoomRatio = EAX_MATERIAL_THICKDOORROOMRATION * travContext.OcclusionRoomRatio;
	#else
									css.Gain *= 0.5f;
	#endif
								}
								else if (portal->getOcclusionModel() == "brick door")
								{
	#if EAX_AVAILABLE == 1
									css.Occlusion = max(EAXBUFFER_MINOCCLUSION, travContext.Occlusion + EAX_MATERIAL_BRICKWALL);
									css.OcclusionLFFactor = min(EAX_MATERIAL_BRICKWALLLF, travContext.OcclusionLFFactor);
									css.OcclusionRoomRatio = EAX_MATERIAL_BRICKWALLROOMRATIO * travContext.OcclusionRoomRatio;
	#else
									css.Gain *= 0.2f;
	#endif
								}
								else
								{
	#if EAX_AVAILABLE == 1
									css.Occlusion = travContext.Occlusion;
									css.OcclusionLFFactor = travContext.OcclusionLFFactor;
									css.OcclusionRoomRatio = travContext.OcclusionRoomRatio;
	#endif
								}

	*/							// compute obstruction
								if (travContext.NbPortal >= 1)
								{
									float h = soundDir * travContext.PreviousVector;
									float obst;

									if (h < 0)
									{
	//									obst = float(2000 + asinf(-(soundDir ^ travContext.PreviousVector).norm()) / (Pi/2) * 2000);
										obst = float(4000 - (soundDir ^ travContext.PreviousVector).norm() * 2000);
									}
									else
									{
	//									obst = float(asinf((soundDir ^ travContext.PreviousVector).norm()) / (Pi/2) * 2000);
										obst = float((soundDir ^ travContext.PreviousVector).norm() * 2000);
									}

	//								float sqrdist = (realListener - nearPoint).sqrnorm();
									if (travContext.Dist < 2.0f)	// interpolate a 2 m
										obst *= travContext.Dist / 2.0f;
	#if EAX_AVAILABLE == 1 // EAX_AVAILABLE no longer used => TODO: implement with EFX and remove when new implementation OK.
									css.Obstruction = max(sint32(EAXBUFFER_MINOBSTRUCTION), sint32(travContext.Obstruction - sint32(obst)));
									css.OcclusionLFFactor = 0.50f * travContext.OcclusionLFFactor;
	#else
									css.Gain *= float(pow(10, -(obst/4)/2000));
	#endif
								}
								else
									css.Obstruction = travContext.Obstruction;
	//							css.Dist = travContext.Dist + float(sqrt(minDist));
								css.Dist = travContext.Dist + minDist;
								css.DistFactor = css.Dist / _MaxEarDistance;
								float	portalDist = css.Dist;
								float	alpha = travContext.Alpha;
								CVector d1(travContext.Direction1), d2(travContext.Direction2);

								css.Direction = interpolateSourceDirection(travContext, portalDist+travContext.Dist, nearPos, travContext.ListenerPos /*realListener*/, d1, d2, alpha);
								css.Position = nearPos + css.Dist * css.Direction;
								css.PosAlpha = min(1.0f, css.Dist / _PortalInterpolate);

								if (addAudibleCluster(otherCluster, css))
								{
	//								debugLines.push_back(CLine(travContext.ListenerPos, nearPoint));
									CSoundTravContext tc(nearPos, travContext.FilterUnvisibleChild, !cluster->AudibleFromFather);
									tc.Dist = css.Dist;
									tc.Gain = css.Gain;
									tc.Occlusion = css.Occlusion;
									tc.OcclusionLFFactor = css.OcclusionLFFactor;
									tc.OcclusionRoomRatio = css.OcclusionRoomRatio;
									tc.Obstruction = css.Obstruction;
									tc.Direction1 = d1;
									tc.Direction2 = d2;
									tc.NbPortal = travContext.NbPortal+1;
									tc.Direction = css.Direction;
									tc.PreviousCluster = cluster;
									tc.Alpha = alpha;
									tc.PreviousVector = soundDir;

									addNextTraverse(otherCluster, tc);
									_AudioPath.push_back(make_pair(travContext.ListenerPos, nearPos));
								}
							}
						}
					}
				}
			}

			// 2nd, look each child cluster
			for (i=0; i<cluster->Children.size(); ++i)
			{
				CCluster *c = cluster->Children[i];

				// dont redown into an upstream
				if (c != travContext.PreviousCluster)
				{
					// clip on distance.
					if (c->AudibleFromFather && c->isIn(travContext.ListenerPos, _MaxEarDistance-travContext.Dist))
					{
						float minDist;
						CVector nearPos;
						CAABBox box = c->getBBox();

						minDist = getAABoxNearestPos(box, travContext.ListenerPos, nearPos);

						if (travContext.Dist + minDist < _MaxEarDistance)
						{
							CClusterSoundStatus css;
							css.Gain = travContext.Gain;
							css.Dist = travContext.Dist + minDist;
							css.DistFactor = css.Dist / _MaxEarDistance;
							css.Occlusion = travContext.Occlusion;
							css.OcclusionLFFactor = travContext.OcclusionLFFactor;
							css.OcclusionRoomRatio = travContext.OcclusionRoomRatio;
							css.Obstruction = travContext.Obstruction;
/*							if (travContext.NbPortal == 0)
								css.Direction = (nearPos - travContext.ListenerPos).normed();
							else
								css.Direction = travContext.Direction1;
*/
							float alpha = travContext.Alpha;
							CVector d1(travContext.Direction1), d2;

							css.Direction = interpolateSourceDirection(travContext, css.Dist+travContext.Dist, nearPos, travContext.ListenerPos /*realListener*/, d1, d2, alpha);
							css.Position = nearPos + css.Dist * css.Direction;
							css.PosAlpha = min(1.0f, css.Dist / _PortalInterpolate);

							if (addAudibleCluster(c, css))
							{
//								debugLines.push_back(CLine(travContext.ListenerPos, nearPos));
								CSoundTravContext stc(travContext);
								stc.FilterUnvisibleChild = true;
								stc.Direction1 = d1;
								stc.Direction2 = d2;
								stc.Direction = css.Direction;
								stc.PreviousCluster = cluster;
								stc.Alpha = alpha;
								stc.PreviousVector = (nearPos - travContext.ListenerPos).normed();
								addNextTraverse(c, stc);
								_AudioPath.push_back(make_pair(travContext.ListenerPos, nearPos));
							}
						}
					}
				}
			}

			// 3nd, look in father cluster
			if (cluster->Father && cluster->Father != travContext.PreviousCluster && cluster->FatherAudible)
			{
//				if (!travContext.FilterUnvisibleFather || ((1.0f-travContext.Alpha) > travContext.MinGain))
				{
					CCluster *c = cluster->Father;
					float minDist;
					CVector nearPos;
					CAABBox box = c->getBBox();

					if (c != _RootCluster)
						minDist = getAABoxNearestPos(box, travContext.ListenerPos, nearPos);
					else
					{
						// special case for root cluster coz it have a zero sized box and a zero position.
						nearPos = travContext.ListenerPos;
						minDist = 0;
					}

					CClusterSoundStatus css;
					css.Gain = travContext.Gain;
/*					if (travContext.FilterUnvisibleFather)
					{
						// compute a gain
						float alpha = 1-(travContext.Dist / _PortalInterpolate);
						alpha = alpha * alpha * alpha;
						css.Gain = max(0.0f, alpha);
					}
					else
						css.Gain = travContext.Gain;
*/
//					if (c->Name == "cluster_1")
//						nldebug("Cluster 1 : gain = %f", css.Gain);
					float alpha = travContext.Alpha;
					CVector d1(travContext.Direction1), d2;

					css.Direction = interpolateSourceDirection(travContext, travContext.Dist, nearPos, travContext.ListenerPos /*realListener*/, d1, d2, alpha);

					if (css.Gain > _MinGain)
					{
						css.Dist = travContext.Dist;
//						css.Direction = CVector::Null;
						css.DistFactor = css.Dist / _MaxEarDistance;
						css.Occlusion = travContext.Occlusion;
						css.OcclusionLFFactor = travContext.OcclusionLFFactor;
						css.OcclusionRoomRatio = travContext.OcclusionRoomRatio;
						css.Obstruction = travContext.Obstruction;
						css.Position = nearPos + css.Dist * css.Direction;
						css.PosAlpha = min(1.0f, css.Dist / _PortalInterpolate);

						if (addAudibleCluster(c, css))
						{
							CSoundTravContext stc(travContext);
							stc.FilterUnvisibleFather = true;
							stc.PreviousCluster = cluster;
							stc.Direction1 = d1;
							stc.Direction2 = d2;
							stc.Direction = css.Direction;
							stc.Alpha = alpha;
							stc.PreviousVector = (nearPos - travContext.ListenerPos).normed();
							_NextTraversalStep.insert(make_pair(c, stc));
						}
					}
				}
			}
			curClusters.pop_back();
		}
	}
	while (!_NextTraversalStep.empty());
}

void CClusteredSound::addNextTraverse(CCluster *cluster, CSoundTravContext &travContext)
{
	std::map<CCluster*, CSoundTravContext>::iterator it = _NextTraversalStep.find(cluster);

	if (it != _NextTraversalStep.end())
	{
		if (it->second.Dist > travContext.Dist)
		{
			it->second = travContext;
		}
	}
	else
		_NextTraversalStep.insert(make_pair(cluster, travContext));

}

bool CClusteredSound::addAudibleCluster(CCluster *cluster, CClusterSoundStatus &soundStatus)
{
	TClusterStatusMap::iterator it(_AudibleClusters.find(cluster));
	nlassert(soundStatus.Dist < _MaxEarDistance);
	nlassert(soundStatus.Direction.norm() <= 1.01f);

	if (it != _AudibleClusters.end())
	{
		// get the best one (for now, based on shortest distance)
		if (soundStatus.Dist < it->second.Dist)
		{
			it->second = soundStatus;

			return true;
		}
	}
	else
	{
		_AudibleClusters.insert(make_pair(cluster, soundStatus));
		return true;
	}

	return false;
}

CVector CClusteredSound::interpolateSourceDirection(const CClusteredSound::CSoundTravContext &context, float portalDist, const CVector &nearPoint, const CVector &realListener, CVector &d1, CVector &/* d2 */, float &alpha)
{
	CVector direction;// (context.Direction);

	if (portalDist > _PortalInterpolate || alpha >= 1.0f)
	{
		// the portal is too far, no interpolation.
		if (context.NbPortal == 0)
		{
			// it's the first portal, compute the initial virtual sound direction
			direction = d1 = (nearPoint-realListener).normed();
		}
		else
		{
			direction = (nearPoint-realListener).normed();
			direction = (direction * (1-alpha) + d1 * (alpha)).normed();
			d1 = direction;
		}
		alpha = 1;
	}
	else
	{
		// the portal is near the listener, interpolate the direction
		if (context.NbPortal == 0)
		{
			// It's the first portal, compute the initial direction
			alpha = (portalDist / _PortalInterpolate);
//			alpha = alpha*alpha*alpha;
			direction = d1 = (nearPoint-realListener).normed();
		}
/*		else if (context.NbPortal == 1)
		{
			float factor = (1-alpha);
//			factor = factor*factor*factor;
			direction = (nearPoint-realListener).normed();
			direction = d1 = (direction * factor + d1 * (1-factor)).normed();

//			alpha = 1-factor;
			alpha = factor;
		}
*/		else
		{
			// two or more portal
			float factor = (portalDist / _PortalInterpolate) * (1-alpha);
//			factor = factor*factor*factor;
			direction = (nearPoint-realListener).normed();
			direction = d1 = (direction * factor + d1 * alpha).normed();

//			alpha = 1-factor;
			alpha = factor;
		}
	}

	nlassert(direction.norm() <= 1.01f);
	return direction;
/*
	CVector direction (context.Direction);
	d1 = context.Direction1;
	d2 = context.Direction2;


	if (portalDist < _PortalInterpolate)
	{
		if (context.NbPortal == 0)
		{
			alpha = (portalDist / _PortalInterpolate);
			direction = d1 = (nearPoint-realListener).normed() * alpha;
			direction.normalize();
		}
		else if (context.NbPortal == 1)
		{
			alpha = alpha * (portalDist / _PortalInterpolate);
//			d2 = (nearPoint-realListener).normed() * alpha;
			d2 = (nearPoint-context.ListenerPos).normed() * (1-alpha);
			direction = d1 + d2;
			direction.normalize();
		}
		else
		{
			alpha = alpha * (portalDist / _PortalInterpolate);
//			d1 = d1+d2;
			d2 = (nearPoint-context.ListenerPos).normed() * (1-alpha);
//			direction = d1 + d2 + (nearPoint-context.ListenerPos).normed() * (1-alpha);
			direction = d1 + d2;
			direction.normalize();
		}
	}
	else
	{
//		alpha = 0.0f
		if (context.NbPortal == 0)
		{
			direction = d1 = (nearPoint-realListener).normed();
		}
		else if (context.NbPortal == 1)
		{
			d2 = (nearPoint-context.ListenerPos).normed() * (1-alpha);
//			d2 = d1; //(nearPoint-context.ListenerPos).normed(); // * (1-alpha);
			direction = d1+d2;
			direction.normalize();
		}
		else
		{
//			d2 = (nearPoint-context.ListenerPos).normed() * (1-alpha);
			d1 = d1+d2;
//			d2 = (nearPoint-realListener).normed();
//			direction = d1+d2+(nearPoint-context.ListenerPos).normed() * (1-alpha);
			direction.normalize();
		}
	}

	return direction;
*/
}


float CClusteredSound::getPolyNearestPos(const std::vector<CVector> &poly, const CVector &pos, CVector &nearPoint)
{
	CPlane plane;
	plane.make(poly[0], poly[1], poly[2]);
	CVector proj = plane.project(pos);
	float	minDist = FLT_MAX;
	bool	projIn = true;
	uint	nbVertex = (uint)poly.size();

	// loop throw all vertex
	for (uint j=0; j<nbVertex; ++j)
	{
		float d = (pos-poly[j]).sqrnorm();
		// check if the vertex is the nearest point
		if (d < minDist)
		{
			nearPoint = poly[j];
			minDist = d;
		}
//		if (projIn /*&& j<poly.size()-1*/)
		{
			// check each segment
			if (plane.getNormal()*((poly[(j+1)%nbVertex] - poly[j]) ^ (proj - poly[j])) < 0)
			{
				// the point is not inside the poly surface !
				projIn = false;
				// check if the nearest point is on this segment
				CVector v1 = (poly[(j+1)%nbVertex] - poly[j]);
				float v1Len = v1.norm();
				v1 = v1 / v1Len;
				CVector v2 = proj - poly[j];
				// project v2 on v1
				float p = v1 * v2;
				if (p>=0 && p<=v1Len)
				{
					// the nearest point is on the segment!
					nearPoint = poly[j] + v1 * p;
					minDist = (nearPoint-pos).sqrnorm();
					break;
				}
			}
		}
	}
	if (projIn)
	{
		float d = (proj-pos).sqrnorm();
		if (d < minDist)
		{
			// the nearest point is on the surface
			nearPoint = proj;
			minDist = d;
		}
	}

	return sqrtf(minDist);
}

float CClusteredSound::getAABoxNearestPos(const CAABBox &box, const CVector &pos, CVector &nearPos)
{
	CVector vMin, vMax;
	box.getMin(vMin);
	box.getMax(vMax);


	nearPos = pos;
	// X
	clamp(nearPos.x, vMin.x, vMax.x);
	// Y
	clamp(nearPos.y, vMin.y, vMax.y);
	// Z
	clamp(nearPos.z, vMin.z, vMax.z);

	return (pos-nearPos).norm();
}

}

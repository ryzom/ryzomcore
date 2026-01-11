// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
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

#ifndef NL_CLUSTERED_SOUND_H
#define NL_CLUSTERED_SOUND_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include <vector>
#include <map>

namespace NLMISC
{
	class CAABBox;
}
namespace NL3D
{
	class UScene;
	class CScene;
	class CCluster;
}

namespace NLSOUND
{

class USource;

/** This class will manage the clipping/positioning/occlusion of sound placed inside the
 *	cluster/portal system.
 *
 * \author Boris Boucher
 * \author Nevrax France
 * \date 2002
 */
class CClusteredSound
{
public:
	/// This structure contain data about sound status in a cluster
	struct CClusterSoundStatus
	{
		/// The relative gain of sound in the cluster
		float			Gain;
		/// The distance from listener.
		float			Dist;
		/// The ratio distance/max earing distance
		float			DistFactor;
		/// The sound virtual position (in fact Dist * Direction)
		NLMISC::CVector	Position;
		/// The blending factor between real sound pos and virtual pos (1 mean virtual pos, 0 mean real pos).
		float			PosAlpha;
		/// The direction vector for the virtual sound source.
		NLMISC::CVector	Direction;
		/// The occlusion att.
		sint32			Occlusion;
		/// The occlusion LF factor (see EAX spec)
		float			OcclusionLFFactor;
		/// The occlusion romm ration
		float			OcclusionRoomRatio;
		/// The obsctruction att db
		sint32			Obstruction;
	};

	/// Container for audible cluster status
	typedef std::map<NL3D::CCluster*, CClusterSoundStatus>	TClusterStatusMap;

	/// This structure is used when we traverse the cluster/portal graph.
	struct CSoundTravContext
	{
		/// The current gain.
		float			Gain;
		sint32			Occlusion;
		float			OcclusionLFFactor;
		float			OcclusionRoomRatio;
		sint32			Obstruction;
		/// The distance acumulator
		float			Dist;
		/// A flag that indicate if we need to filter the unvisible child.
		bool			FilterUnvisibleChild;
		/// A flag that iindicate if we need to filter the unvisible father.
		bool			FilterUnvisibleFather;
		/// The number of traversed portals
		uint			NbPortal;
		/// A blending factor to compute virtual source position.
		float			Alpha;
		/// The direction vector from listener to the first portal/cluster
		NLMISC::CVector	Direction1;
		/// The direction vector from the first portal/cluster to the second one.
		NLMISC::CVector	Direction2;
		/// The current blended direction used to place vitual source.
		NLMISC::CVector	Direction;
		/// The previously traversed cluster. Used to stop back traversal.
		NL3D::CCluster	*PreviousCluster;
		/// The previous sound propagation vector
		NLMISC::CVector	PreviousVector;

		/// The last pseudo listener position
		NLMISC::CVector	ListenerPos;

		/// Constructor. Init all default value.
		CSoundTravContext(const NLMISC::CVector &listenerPos,
			bool filterUnvisibleChild, bool filterUnvisibleFather)
			:	Gain(1.0f),
				Occlusion(0),
				OcclusionLFFactor(1.0f),
				OcclusionRoomRatio(1.0f),
				Obstruction(0),
				Dist(0),
				FilterUnvisibleChild(filterUnvisibleChild),
				FilterUnvisibleFather(filterUnvisibleFather),
				NbPortal(0),
				Alpha(0.0f),
				Direction1(NLMISC::CVector::Null),
				Direction2(NLMISC::CVector::Null),
				Direction(NLMISC::CVector::Null),
				PreviousCluster(0),
				PreviousVector(NLMISC::CVector::Null),
				ListenerPos(listenerPos)
		{}

		/// Assignment operator.
		CSoundTravContext &operator = (const CSoundTravContext &other)
		{
			Gain = other.Gain;
			Occlusion = other.Occlusion;
			OcclusionLFFactor = other.OcclusionLFFactor;
			OcclusionRoomRatio = other.OcclusionRoomRatio;
			Obstruction = other.Obstruction;
			Dist = other.Dist;
			FilterUnvisibleChild = other.FilterUnvisibleChild;
			FilterUnvisibleFather = other.FilterUnvisibleFather;
			NbPortal = other.NbPortal;
			Alpha = other.Alpha;
			Direction1 = other.Direction1;
			Direction2 = other.Direction2;
			Direction = other.Direction;
			PreviousCluster = other.PreviousCluster;
			ListenerPos = other.ListenerPos;
			PreviousVector = other.PreviousVector;
			return *this;
		}
	};

	///
	struct CClusterSound
	{
		USource		*Source;
		float		Distance;
	};


	/// Container for the next traversal step
	typedef std::map<NL3D::CCluster*, CSoundTravContext>	TClusterTravContextMap;


	/// Constructor
	CClusteredSound();

	/** Initialize the class
	 *	\param scene The scene of interest for cluster management.
	 *	\param portalInterpolate Ditance from listener to portal for interpolation.
	 *	\param maxEarDist The maximum traversal distance to limit graph traversal.
	 *	\param minGain The minimun gain to stop traversal.
	 */
	void		init(NL3D::CScene *scene, float portalInterpolate, float maxEarDistance, float minGain);

	/** Update the cluster sound system.
	 */
	void		update(const NLMISC::CVector &listenerPos, const NLMISC::CVector &view, const NLMISC::CVector &up);

	NL3D::CCluster	*getRootCluster();


	const CClusterSoundStatus *getClusterSoundStatus(NL3D::CCluster *cluster);

	const TClusterStatusMap &getAudibleClusters() {return _AudibleClusters;}

	const std::vector<std::pair<NLMISC::CVector, NLMISC::CVector> >	&getAudioPath() { return _AudioPath;}

	static void buildSheets(const std::string &packedSheetPath);

private:

	/// Traverse the cluster system to build/update the audible cluster set and theire respective status.
	void		soundTraverse(const std::vector<NL3D::CCluster *> &clusters, CSoundTravContext &travContext);
	/// Add a cluster for the next traversal step
	void		addNextTraverse(NL3D::CCluster *cluster, CSoundTravContext &travContext);
	/// Add a cluster into the list of audible cluster.
	bool		addAudibleCluster(NL3D::CCluster *cluster, CClusterSoundStatus &soundStatus);
	/// Compute a positional blending depending on context.
	NLMISC::CVector		interpolateSourceDirection(const CSoundTravContext &context, float portalDist, const NLMISC::CVector &nearPoint, const NLMISC::CVector &realListener, NLMISC::CVector &d1, NLMISC::CVector &d2, float &alpha);
	//\name Utility method
	//@{
	/** Compute the point on the poly that is the nearest from a given position.
	 *	This point can be on the surface of the poly, on a segment or on one of the vertex.
	 *	In addition, the method also return the distance from the nearest point to the
	 *	reference position.
	 *	\param poly The polygone description.
	 *	\param pos The reference position.
	 *	\param nearPoint The nearest point (out var).
	 *	\return The distance between pos and nearPoint.
	 */
	float		getPolyNearestPos(const std::vector<NLMISC::CVector> &poly, const NLMISC::CVector &pos, NLMISC::CVector &nearPoint);

	/** Compute the point in the bounding box that is the nearest from a given position.
	 *	This point can be in the volume of the box, on a segment of one of the vertex.
	 *	In addition, the method also return the distance from the nearest point to the
	 *	reference position.
	 *	\param poly The bounding box description.
	 *	\param pos The reference position.
	 *	\param nearPoint The nearest point (out var).
	 *	\return The distance between pos and nearPoint.
	 */
	float		getAABoxNearestPos(const NLMISC::CAABBox &box, const NLMISC::CVector &pos, NLMISC::CVector &nearPos);
	//@}

	/// The scene of interest
	NL3D::CScene	*_Scene;
	/// Interpolation distance when listener is near a portal.
	float			_PortalInterpolate;
	/// Maximum earing distance.
	float			_MaxEarDistance;
	/// Minimum gain.
	float			_MinGain;
	/// The root cluster of the scene.
	NL3D::CCluster	*_RootCluster;


	/// The current audible cluster
	TClusterStatusMap		_AudibleClusters;
	/// The cluster for the next travesal step
	TClusterTravContextMap	_NextTraversalStep;
	/// The last setted environement.
	NLMISC::TStringId				_LastEnv;
	/// The last set environment size.
	float					_LastEnvSize;
	/// The segment of all the audio path.
	std::vector<std::pair<NLMISC::CVector, NLMISC::CVector> >	_AudioPath;

	typedef CHashMap<NLMISC::TStringId, CClusterSound, NLMISC::CStringIdHashMapTraits>	TClusterSoundCont;
	/// The current cluster playing source indexed with sound group id
	TClusterSoundCont		_Sources;

	typedef CHashMap<NLMISC::TStringId, NLMISC::TStringId, NLMISC::CStringIdHashMapTraits> TStringStringMap;
	/// The sound_group to sound assoc
	TStringStringMap	_SoundGroupToSound;
};

} // NLSOUND

#endif // NL_CLUSTERED_SOUND_H

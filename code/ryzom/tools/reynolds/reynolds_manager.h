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



#ifndef NL_REYNOLDS_MANAGER_H
#define NL_REYNOLDS_MANAGER_H

// NeL
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"

// Game share
#include "game_share/continent_container.h"

// ReynoldsLib
#include "track.h"

// Stl
#include <map>




/**
 * Track manager
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CReynoldsManager
{
	friend CTrack::~CTrack();

public:

	/**
	 * User Motion controlled Callback type
	 * \param track is the base track
	 */
	typedef void	(*TUserCallback) (CTrackBase *track);

	/**
	 * User Motion Callback type
	 * Called each time a controlled track is updated
	 * \param track is the base track
	 * \param motion is the current motion to be applied -- WARNING do not crash this value, only add or substract value to it
	 */
	typedef void	(*TUserMotionCallback) (CTrackBase *track, NLMISC::CVectorD &motion);


	/// The command interface
	class ICommandInterface
	{
	public:
		/** 
		 * Called when manager needs a sheet for an entity.
		 * Reply using setSheet(CEntityId)
		 */
		virtual void	requestSheet(const NLMISC::CEntityId &id) = 0;

		/** 
		 * Called when manager needs a position for an entity.
		 * Reply using setPosition(CEntityId)
		 */
		virtual void	requestPosition(const NLMISC::CEntityId &id) = 0;

		/** 
		 * Called when manager needs a position to be updated evently
		 * Reply using setPosition(CEntityId) evently
		 */
		virtual void	requestPositionUpdates(const NLMISC::CEntityId &id) = 0;

		/** 
		 * Called when manager doesn't need more position updates
		 */
		virtual void	unrequestPositionUpdates(const NLMISC::CEntityId &id) = 0;

		/** 
		 * Called when manager needs an entity vision to be updated evently
		 * Reply using setVision() evently
		 */
		virtual void	requestVision(const NLMISC::CEntityId &id) = 0;

		/** 
		 * Called when manager doesn't need more entity vision updates
		 */
		virtual void	unrequestVision(const NLMISC::CEntityId &id) = 0;

		/**
		 * Called when a track position is updated
		 */
		virtual void	updatePosition(const NLMISC::CEntityId &id, const NLMISC::CVectorD &position, float &heading) = 0;

		/**
		 * Called when track state changed
		 */
		virtual void	stateChanged(const NLMISC::CEntityId &id, CTrackBase::TTrackState state) = 0;

		/** 
		 * Called when manager leave control of an entity
		 */
		virtual void	stopTrack(const NLMISC::CEntityId &id) = 0;
	};

public:

	/// @name Reynolds Manager inits/update/release
	//@{

	/// Init
	static void	init(const std::string &packSheetFile = "reynolds.packed_sheets");


	/// Load continent
	static void	loadContinent(const std::string &name, const std::string &file, sint index);

	/// Set Sheet request callback
	static void	setCommandInterface(ICommandInterface *commandInterface)	{ _CommandInterface = commandInterface; }

	/// Set User init callback
	static void	setUserInitCallback(TUserCallback cb)						{ _UserInitCallback = cb; }

	/// Set User motion callback
	static void	setUserMotionCallback(TUserMotionCallback cb)				{ _UserMotionCallback = cb; }

	/// Set User release callback
	static void	setUserReleaseCallback(TUserCallback cb)					{ _UserReleaseCallback = cb; }

	/// Update
	static void	update(double dt = 0.1);


	/// Release
	static void	release();

	//@}




	/// @name Control methods over tracks
	//@{

	/// Follow
	static void	follow(const NLMISC::CEntityId &entity, const NLMISC::CEntityId &target);

	/// Go to
	static void	goTo(const NLMISC::CEntityId &entity, const NLMISC::CVectorD &position);

	/// Stop
	static void	leaveMove(const NLMISC::CEntityId &entity);

	/// Destroy a track
	static void	destroy(const NLMISC::CEntityId &entity);

	//@}




	/// @name Tracks requests and state towards interface
	//@{

	/// Request Sheet
	static void	requestSheet(const NLMISC::CEntityId &entity);

	/// Request Position
	static void	requestPosition(const NLMISC::CEntityId &entity);

	/// Request Position Updates
	static void	requestPositionUpdates(const NLMISC::CEntityId &entity);

	/// Unrequest Position Updates
	static void	unrequestPositionUpdates(const NLMISC::CEntityId &entity);

	/// Request Vision
	static void	requestVision(const NLMISC::CEntityId &entity);

	/// Unrequest Vision
	static void	unrequestVision(const NLMISC::CEntityId &entity);

	/// Track stopped following
	static void	trackStop(CTrack *track);

	/// Updated position
	static void	updatedPosition(const NLMISC::CEntityId &entity, const NLMISC::CVectorD &position, float heading);

	/// Updated state
	static void	stateChanged(const NLMISC::CEntityId &entity, CTrackBase::TTrackState state);



	/// User init
	static void initUserMotion(CTrack *track)
	{
		if (_UserInitCallback != NULL)
			_UserInitCallback((CTrackBase*)track);
	}

	/// User move
	static void applyUserMotion(CTrack *track, NLMISC::CVectorD &motion)
	{
		if (_UserMotionCallback != NULL)
			_UserMotionCallback((CTrackBase*)track, motion);
	}

	/// User release
	static void releaseUserMotion(CTrack *track)
	{
		if (_UserReleaseCallback != NULL)
			_UserReleaseCallback((CTrackBase*)track);
	}



	/// Create Move primitive
	static void	createMovePrimitive(const NLMISC::CVectorD &pos, NLPACS::UMovePrimitive *&primitive, NLPACS::UMoveContainer *&container);

	/// Lookup a sheet
	static const CTrack::CSheet	*lookup(const NLMISC::CSheetId &sheet);

	//@}




	/// @name Answers to tracks requests
	//@{

	/// Set Sheet
	static void	setSheet(const NLMISC::CEntityId &id, const NLMISC::CSheetId &sheet);

	/// Set Position
	static void	setPosition(const NLMISC::CEntityId &id, const NLMISC::CVectorD &position, float heading);

	/// Update Vision
	static void	setVision(const NLMISC::CEntityId &id, const std::vector<NLMISC::CEntityId> &in, const std::vector<NLMISC::CEntityId> &out);

	/// Update Vision
	static void	setVision(const NLMISC::CEntityId &id, const std::vector<NLMISC::CEntityId> &vision);

	/// Get a track
	static CTrack	*getTrack(const NLMISC::CEntityId &entity)
	{
		TTrackMap::iterator	it = _TrackMap.find(entity);
		return (it == _TrackMap.end()) ? NULL : (*it).second;
	}

	/// Get cycle
	static uint32	getCycle()	{ return _Cycle; }

	//@}




	/// Create a non controlled track, referenced in _TrackMap
	static CTrack	*createTrack(const NLMISC::CEntityId &entity);


protected:

	/// Constructor. Not to be used.
	CReynoldsManager();

	/// Remove a track from the whole map -- only called by the CTrack destructor
	static void		removeTrackFromMap(const NLMISC::CEntityId &entity);

	/// Checks interface is ok
	static bool		checkInterface()
	{
		if (_CommandInterface == NULL)
		{
			nlwarning("CReynoldsLib:checkInterface(): CommandInterface not set");
			return false;
		}
		return true;
	}

	/// Init sheets
	static void		initSheets(const std::string &packSheetFilnamePrefix);


protected:

	/// Whole track container
	typedef std::map<NLMISC::CEntityId, CTrack* >						TTrackMap;

	/// Only controlled track container
	typedef std::map<NLMISC::CEntityId, NLMISC::CSmartPtr<CTrack> >		TControlledTrackMap;




protected:

	/// Continents
	static CContinentContainer		_Continents;

	/// Track Map, all tracks referenced, held by a simple pointer so when no one references a track, it is deleted
	static TTrackMap				_TrackMap;

	/// Controlled Track Map, only controlled tracks, held by a smart pointer
	static TControlledTrackMap		_ControlledTrackMap;


	/// Georges sheets
	static std::map<NLMISC::CSheetId, CTrack::CSheet> _Sheets;

	/// Sheets initialised ?
	static bool						_Initialised;


	/// Command interface
	static ICommandInterface		*_CommandInterface;

	/// User init callback
	static TUserCallback			_UserInitCallback;

	/// User motion callback
	static TUserMotionCallback		_UserMotionCallback;

	/// User release callback
	static TUserCallback			_UserReleaseCallback;


	/// Current internal cycle
	static uint32					_Cycle;
};


#endif // NL_REYNOLDS_MANAGER_H

/* End of reynolds_manager.h */

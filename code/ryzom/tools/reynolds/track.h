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



#ifndef NL_TRACK_H
#define NL_TRACK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"

#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"

#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"


/**
 * The base class for moving entity
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CTrackBase
{
public:
	/// State type
	enum TTrackState
	{
		Idle = 0,				// not controlled yet
		MovingTowardsTarget,	// first step of motion, moving towards target
		TargetLocked,			// close to target, locked
		TargetLost,				// target moved away, can't reach it any longer
		TargetUnreachable		// can't reach target, too far or too many obstacles on the way
	};

	/// Get Id
	virtual const NLMISC::CEntityId	&getId() const = 0;

	/// Get SheetId
	virtual const NLMISC::CSheetId	&getSheetId() const = 0;

	/// Has Id ?
	virtual bool	hasId() const = 0;

	/// Get track Position
	virtual void	getPosition(NLMISC::CVectorD &pos, float &heading) const = 0;

	/// Has Position ?
	virtual bool	hasPosition() const = 0;

	/// Set user data
	virtual void	setUserData(void *data) = 0;

	/// Get user data
	virtual void	*getUserData() = 0;

	/// Get Track state
	virtual TTrackState	getTrackState() const = 0;
};

/**
 * A track that represents a moving point
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CTrack : public CTrackBase, public NLMISC::CRefCount
{
public:
	/**
	 * The sheet type read by Georges
	 */
	class CSheet
	{
	public:
		CSheet(): WalkSpeed(1.3f), RunSpeed(6.0f), Radius(0.5f), Height(2.0f), Length(1.0), Width(1.0) {}

		/// The Walk speed of the entity
		float	WalkSpeed;
		/// The Run speed of the entity
		float	RunSpeed;
		/// The Pacs radius of the entity
		float	Radius;
		/// The Height radius of the entity
		float	Height;
		/// The Animation length of the entity
		float	Length;
		/// The Width length of the entity
		float	Width;

		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
		{
			// the form was found so read the true values from George
			form->getRootNode ().getValueByName (WalkSpeed, "Basics.MovementSpeeds.WalkSpeed");
			form->getRootNode ().getValueByName (RunSpeed, "Basics.MovementSpeeds.RunSpeed");
			form->getRootNode ().getValueByName (Radius, "Collision.CollisionRadius");
			form->getRootNode ().getValueByName (Height, "Collision.Height");
			form->getRootNode ().getValueByName (Width, "Collision.Width");
			form->getRootNode ().getValueByName (Length, "Collision.Length");
		}

		void serial (NLMISC::IStream &s)
		{
			s.serial (WalkSpeed, RunSpeed);
			s.serial (Radius, Height);
			s.serial (Length, Width);
		}

		static uint getVersion () { return 1; }

		/// The default sheet
		static CSheet	DefaultSheet;
	};

public:

	/// Constructor
	CTrack() :	_OwnControl(NULL), _MoveContainer(NULL), _MovePrimitive(NULL),
				_Id(NLMISC::CEntityId::Unknown), _SheetId(NLMISC::CSheetId::Unknown), _Sheet(NULL), _Followed(NULL),
				_HasPosition(false), _HasId(false), _IdRequested(false), _PositionUpdatesRequested(false), _IsStatic(false),
				_ForceRelease(false), _ReceiveVision(false), _UserData(NULL), _State(Idle),
				_SmoothedTargetDistanceDelta(3.0), _LastTargetDistance(-1.0)

	{
	}

	/// Destructor
	~CTrack();

	/// Init track
	void	setId(const NLMISC::CEntityId &id, const NLMISC::CSheetId &sheet);

	/// Get Id
	const NLMISC::CEntityId	&getId() const
	{
		return _Id;
	}

	/// Get SheetId
	const NLMISC::CSheetId	&getSheetId() const
	{
		return _SheetId;
	}

	/// Get SheetId
	const CSheet	*getSheet() const
	{
		return _Sheet;
	}

	/// Has Id ?
	bool	hasId() const
	{
		return _HasId;
	}



	/// Update track position
	void	setPosition(const NLMISC::CVectorD &pos, float heading)
	{
		// don't allow more than one position to be set when control is owned
		if (_HasPosition && _OwnControl)
			return;
		_Position = pos;
		_Heading = heading;
		_HasPosition = true;
	}

	/// Get track Position
	void	getPosition(NLMISC::CVectorD &pos, float &heading) const
	{
		if (_HasPosition)
		{
			pos = _Position;
			heading = _Heading;
		}
		else
		{
			nlwarning("ReynoldsLib:CTrack:getPosition(): Track %s position not yet set", _Id.toString().c_str());
		}
	}

	/// Set Static state
	void	setStatic(bool isstatic = true)
	{
		_IsStatic = isstatic;
	}



	/// Has Control Owned ?
	bool	hasControlOwned() const
	{
		return _OwnControl;
	}

	/// Has Position ?
	bool	hasPosition() const
	{
		return _HasPosition;
	}

	/// Invalid position
	void	invalidPosition()
	{
		_HasPosition = false;
	}

	/// Is static ?
	bool	isStatic() const
	{
		return _IsStatic;
	}



	/// Follow
	void	follow(CTrack *followed);

	/// Leave
	void	leave();

	/// Update
	void	update(double dt);

	/// Update vision
	void	updateVision(const std::vector<NLMISC::CEntityId> &in, const std::vector<NLMISC::CEntityId> &out);

	/// Update vision
	void	updateVision(const std::vector<NLMISC::CEntityId> &vision);

	/// Force release
	void	forceRelease()		{ _ForceRelease = true; }



	/// Get current state
	TTrackState	getTrackState()	const	{ return _State; }




	/// Set user data
	void	setUserData(void *data)	{ _UserData = data; }

	/// Get user data
	void	*getUserData()			{ return _UserData; }






	/// Get contact distance
	double	rawDistance(const CTrack *other, NLMISC::CVectorD &distance) const
	{
		distance = other->_Position - _Position;
		distance.z = 0.0;
		return distance.norm();
	}

	/// Get contact distance
	double	contactDistance(const CTrack *other, NLMISC::CVectorD &distance, double &rawdistance) const
	{
		rawdistance = rawDistance(other, distance);
		return contactDistanceWithRawDistance(other, distance, rawdistance);
	}

	/// Get contact distance
	double	contactDistanceWithRawDistance(const CTrack *other, NLMISC::CVectorD &distance, double &rawdistance) const
	{
		double	theta = atan2(distance.y, distance.x);
		double	theta1 = _Heading - theta;
		double	theta2 = other->_Heading - theta + 3.1415926535;

		float	l1 = _Sheet->Length,
				w1 = _Sheet->Width;
		float	l2 = other->_Sheet->Length,
				w2 = other->_Sheet->Width;

		double	r1 = 0.5 * sqrt( l1*l1 + (w1*w1-l1*l1)*NLMISC::sqr(sin(theta1)) );
		double	r2 = 0.5 * sqrt( l2*l2 + (w2*w2-l2*l2)*NLMISC::sqr(sin(theta2)) );

		return rawdistance - r1 - r2;
	}



protected:

	/// Own Control
	void	acquireControl();

	/// Release Control
	void	releaseControl();

	/// Acquire vision
	void	acquireVision();

	/// Release vision
	void	releaseVision();

	/// Create Move primitive
	void	createMovePrimitive();

	/// Delete Move primitive
	void	deleteMovePrimitive();

	/// Check has position (ask for it if necessary)
	bool	isValid()
	{
		if (!hasId())
			return false;

		if (!hasPosition())
		{
			if (!hasControlOwned() && !_PositionUpdatesRequested)
				requestPositionUpdates();
			return false;
		}
		return true;
	}

	/// Request Id
	void	requestId();

	/// Request Position
	void	requestPositionUpdates();

protected:


	/// @name Track Id
	//@{

	/// Has Id
	bool						_HasId;

	/// Id Requested
	bool						_IdRequested;

	/// Entity Id
	NLMISC::CEntityId			_Id;

	/// Sheet Id
	NLMISC::CSheetId			_SheetId;

	/// Sheet
	const CSheet				*_Sheet;

	/// Is static
	bool						_IsStatic;

	//@}



	/// @name Track Position Control
	//@{

	/// Own control
	bool						_OwnControl;

	/// Has Position
	bool						_HasPosition;

	/// Id Requested
	bool						_PositionUpdatesRequested;

	/// Position
	NLMISC::CVectorD			_Position;

	/// Heading
	float						_Heading;

	/// Followed track
	NLMISC::CSmartPtr<CTrack>	_Followed;

	//@}



	/// @name Track PACS 
	//@{

	/// Move Container
	NLPACS::UMoveContainer		*_MoveContainer;

	/// Move Primitive
	NLPACS::UMovePrimitive		*_MovePrimitive;

	//@}



	/// @name Misc
	//@{

	/// Force release
	bool						_ForceRelease;

	/// Vision container
	typedef std::map<NLMISC::CEntityId, NLMISC::CSmartPtr<CTrack> >	TVision;

	/// Vision
	TVision						_Vision;

	/// Receive vision
	bool						_ReceiveVision;

	/// User data
	void						*_UserData;

	/// Track state
	TTrackState					_State;

	/// Last move cycle
	uint32						_LastMoveCycle;

	/// Last target distance
	double						_LastTargetDistance;

	/// Smoothed target distance
	double						_SmoothedTargetDistanceDelta;

	//@}

public:
	/// @name Target attraction control
	//@{

	/// Required target spacing
	static double				TargetSpacing;

	/// Target attraction strength
	static double				TargetAttraction;

	/// Target attraction amplification
	static double				TargetAmp;

	//@}


	/// @name Obstacle repulsion control
	//@{

	/// Fast obstacle exclusion distance
	static double				ObstacleExcludeDistance;

	/// Required obstacle spacing
	static double				ObstacleSpacing;

	/// Obstacle repulsion strength
	static double				ObstacleRepulsion;

	/// Obstacle repulsion amplification
	static double				ObstacleAmp;

	//@}


	/// @name Track motion control
	//@{

	/// Minimum motion distance
	static double				MinimumMotion;

	/// Lock distance threshold
	static double				LockThreshold;

	/// Lose distance threshold
	static double				LoseThreshold;

	/// Stabilise cycle
	static uint32				StabiliseCycle;

	//@}
};


#endif // NL_TRACK_H

/* End of track.h */

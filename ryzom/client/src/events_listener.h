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



#ifndef CL_EVENTS_LISTENER_H
#define CL_EVENTS_LISTENER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/events.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/mouse_smoother.h"

#include <set>

using std::set;

using NLMISC::IEventListener;
using NLMISC::CEvent;
using NLMISC::CEventServer;

/**
 * <Class description>
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CEventsListener : public IEventListener
{
public:
	enum TWheelState
	{
		no_move = 0,
		foreward,
		backward
	};

	NLMISC::TMouseButton	mouseButtons;

	/// Constructor
	CEventsListener();
	/// Destructor
	~CEventsListener();


	/**
     * Register the listener to the server.
	 * \param server is the reference to the server
	 */
	void addToServer(CEventServer& server);

	/**
	 * Unregister the listener to the server.
	 * \param server is the reference to the server
	 */
	void removeFromServer(CEventServer& server);

	/**
	 * As soon as we call the function, _Wheel will change to noWheel.
	 * \return Wheel state.
	 */
	TWheelState getWheelState();

	/**
	 * As soon as we call the function, _MouseMoveX will change to false.
	 */
	bool isMouseMoveX();

	/**
	 * As soon as we call the function, _MouseMoveY will change to false.
	 */
	bool isMouseMoveY();

	/**
	 * As soon as we call the function, _MouseAngleX will change to false.
	 */
	bool isMouseAngleX();

	/**
	 * As soon as we call the function, _MouseAngleY will change to false.
	 */
	bool isMouseAngleY();

	// Return true if the button begin to be pushed
	bool isMouseButtonPushed (NLMISC::TMouseButton button) const;

	// Return true if the button is pushed
	bool isMouseButtonDown (NLMISC::TMouseButton button) const;

	// Return true if the button begin to be released
	bool isMouseButtonReleased (NLMISC::TMouseButton button) const;

	// prevent mouse release being handled twice if handled by ui
	void setUIHandledButtonMask(NLMISC::TMouseButton mask);
	void addUIHandledButtonMask(NLMISC::TMouseButton mask);


	// Return true if the button is up
	bool isMouseButtonUp (NLMISC::TMouseButton buttons) const;

	/**
	 * \return float the mousePosX.
	 */
	float getMousePosX();

	/**
	 * \return float the mousePosY.
	 */
	float getMousePosY();


	/**
	 * \return float the mouse Delta angle X in radian.
	 */
	float getMouseAngleX();

	/**
	 * \return float the mouse Delta angle Y in radian.
	 */
	float getMouseAngleY();

	// Update to call before pumping the events
	void update ();

	// update mouse smoothing
	void updateMouseSmoothing();

	void enableMouseSmoothing(bool on);

	// update free look pos
	void updateFreeLookPos(float x, float y);

protected:
	/*
	 * Call back of the listener.
	 * \param event is the event send to the listener
	 */
	virtual void operator ()(const CEvent& event);


	// update cursor pos
	void updateCursorPos(float x, float y);
	//
	void smoothMouseCoordinates(float &x, float &y, float smoothingPeriod);

private:
	TWheelState _Wheel;

	// Position
	float _MouseX;
	float _MouseY;
	float _OldMouseX;
	float _OldMouseY;

	bool _MouseMoveX;
	bool _MouseMoveY;

	bool _FreeLookSmoothed;

	// Angle
	float _MouseDeltaAX;
	float _MouseDeltaAY;

	float _WantedMouseAX;
	float _WantedMouseAY;
	float _CurrSmoothedMouseAX;
	float _CurrSmoothedMouseAY;
	float _OldSmoothedMouseAX;
	float _OldSmoothedMouseAY;




	bool _MouseAngleX;
	bool _MouseAngleY;

	NLMISC::TMouseButton	_HandledUIButtonMask;
	NLMISC::TMouseButton	_MouseButtonPushed;
	NLMISC::TMouseButton	_MouseButtonReleased;
	NLMISC::TMouseButton	_MouseButtonDown;

	double					_LastFreeLookUpdateDate;
	NLMISC::CMouseSmoother	_MouseSmoother;

	bool					_MouseSmoothingOn;
};


#endif // CL_EVENTS_LISTENER_H

/* End of events_listener.h */






















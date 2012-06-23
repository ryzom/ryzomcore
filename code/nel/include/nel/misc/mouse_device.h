// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_MOUSE_DEVICE_H
#define NL_MOUSE_DEVICE_H

#include "types_nl.h"
#include "input_device.h"



namespace NLMISC
{

class CRect;

/// An interface to a low level mouse device
struct IMouseDevice : public IInputDevice
{
	enum TAxisMode { Raw, Clamped, AxisModeLast };
	enum TAxis	{ XAxis = 0, YAxis = 1, AxisLast };
	enum TMessageMode { NormalMode, RawMode, MessageModeLast };

	///\name Messages
	//@{
		/** Tells what messages should be sent :
		  * DEFAULT is 'raw' messages
		  * Raw messages : - no clamping nor frames applied
		  *                - no speed applied
		  *				   - no factor applied
		  *                - CGDMouseMove messages are sent
		  *				   - Move expressed in mickeys
		  * Normal messages : - CEventMouseMove messages are sent
		  *					  - A frame may clamp one or both axis
		  *                   - The mouse speed can be changed
		  */
		virtual	void					setMessagesMode(TMessageMode mode) = 0;
		/// retrieve what kinds of messages are sent
		virtual TMessageMode			getMessagesMode() const = 0;
	//@}

	///\name Mouse MOVE, valid only
	//@{
		/** Set the mode of axis of the mouse. This can be raw, or clamped. In clamped mode, a frame is used to limit the move.
		  * NB : invalid in raw message mode
		  * \see setMouseFrame(const CRect &rect)
		  */
		virtual void					setMouseMode(TAxis axis, TAxisMode axisMode) = 0;
		/** returns the mode of the mouse for the given axis.
		  * NB : invalid in raw message mode
		  */
		virtual TAxisMode				getMouseMode(TAxis axis) const  = 0;
		/** Set the mouse speed. It must be in the ]0, +inf] range, 1 gives the natural mouse speed.
		  * NB : invalid in raw message mode
		  */
		virtual void					setMouseSpeed(float speed)  = 0;
		/** Get the mouse speed.
		  * NB : invalid in raw message mode
		  */
		virtual float					getMouseSpeed() const  = 0;
		/** Set the mouse acceleration. It is the threshold in mickey, when start the acceleration. 0 means not acceleration.
		  */
		virtual void					setMouseAcceleration(uint speed)  = 0;
		/** Get the mouse acceleration.
		  */
		virtual uint					getMouseAcceleration() const  = 0;
		/** Set the current frame in which the mouse can move, expressed in pixels.
		  * NB do not forget to call setMouseFactors if you want the results to be reported in the 0-1 range.
  		  * NB : invalid in raw message mode.
		  * \see setMouseFactors
		  */
		virtual void					setMouseFrame(const CRect &rect)  = 0;
		/** Gives factor by which the mouse coordinates must be multiplied before an event is sent.
		  * The default factor is 1.
  		  * NB : invalid in raw message mode.
		  *
		  * Example : this set a frame of 800x600 and reports event in the [0, 1] range.
		  * \code
		  * mouse->setMouseFrame(800, 600);
		  * mouse->setMouseMode(XAxis, IMouseDevice::Clamped);
		  * mouse->setMouseMode(YAxis, IMouseDevice::Clamped);
		  * mouse->setFactors(1.f / 800, 1.f / 600);
		  * \endcode
		  */
		virtual void					setFactors(float xFactor, float yFactor) = 0;
		/** Get the x factor, use to multiply the mouse position before an event is sent.
		  * NB : invalid in raw message mode.
		  * \see setFactors()
		  */
		virtual float					getXFactor() const = 0;
		/** Get the y factor, use to multiply the mouse position before an event is sent.
		  * NB : invalid in raw message mode.
		  * \see setFactors()
		  */
		virtual float					getYFactor() const = 0;
	//@}

	// Get the current frame used for limiting mouse movements
	virtual const CRect				&getMouseFrame() const = 0;
	// Set the maximum delay for a double click to be taken in account (in ms).
	virtual void					setDoubleClickDelay(uint ms) = 0;
	// Get the maximum delay for double click (in ms)
	virtual uint					getDoubleClickDelay() const = 0;
	// Force the position of the mouse, expressed in pixels
	virtual void					setMousePos(float x, float y) = 0;

	/// From a delta of a mouse position input (eg from CEventMouseMove), deduce delta in mickeys (eg: like received from a CGDMouseMove)
	virtual void					convertStdMouseMoveInMickeys(float &dx, float &dy) const = 0;
};


} // NLMISC


#endif // NL_MOUSE_DEVICE_H

/* End of u_mouse_device.h */

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

#ifndef NL_KEYBOARD_DEVICE_H
#define NL_KEYBOARD_DEVICE_H

#include "types_nl.h"
#include "events.h"
#include "input_device.h"



namespace NLMISC
{

/** Gives access to low level keyboard parameters
  * - 'Shift' messages are replaced by RShift and LShift msg.
  * - 'Control' messages are replaced by 'RControl' and 'LControl' msg.
  * - 'Menu' (alternate) messages are replaced by 'RMenu' and 'LMenu' msg.
  */
struct IKeyboardDevice	: public IInputDevice
{
	/// Max number of supported keys
	enum { NumKeys = 256 };
	/// Get the delay before key repeat, in milliseconds
	virtual		uint getKeyRepeatDelay() const = 0;
	/// Get the delay before key repeat, in milliseconds
	virtual		void setKeyRepeatDelay(uint delay) = 0;
	/// Get the period before key repeat, in milliseconds
	virtual		uint getKeyRepeatPeriod() const = 0;
	/// Get the period before key repeat, in milliseconds
	virtual		void setKeyRepeatPeriod(uint period) = 0;
	/// Set a set of keys for which repetition is disabled
	virtual		void disableRepetition(const TKey *keyTab, uint numKey) = 0;
	/// Get the number of disabled keys
	virtual		uint getNumDisabledRepetition() const = 0;
	/** Get the disabled keys and stores in the given tab.
	  * NB: must ensure the destination table has the right size
	  * \see getNumDisabledKeys()
	  */
	virtual		void getDisabledRepetitions(TKey *destTab) const = 0;
};


} // NLMISC


#endif // NL_KEYBOARD_DEVICE_H

/* End of keyboard_device.h */

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



#ifndef CL_IG_ENUM_H
#define CL_IG_ENUM_H

#include "game_share/time_weather_season/time_and_season.h"

namespace NL3D
{
	class UInstanceGroup;
}

/** This serves as a callback to enumerate ig in various places .
  * (loaded igs of the landscape, igs of villages
  */
struct IIGEnum
{
	/** Called by the enumerator for each ig of interest
	  * \return true if the enumeration must continue
	  */
	virtual bool enumIG(NL3D::UInstanceGroup *ig) = 0;
};


/** Tool fct : this enum all instanciated (added to the main scene) igs of the following categories :
  * - Zone igs
  * - ZC igs
  * - continents igs (villages)
  * \return false if the enumeration has been stopped
  */
bool enumAllIGs(IIGEnum *callaback);

/** A callback to know when an ig has been added.
  * (such callbacks are managed bu UInstanceGroup, but it doesn't tells which ig has been instanciated
  */
struct IIGObserver
{
	virtual void instanceGroupLoaded(NL3D::UInstanceGroup *ig) = 0;
	virtual void instanceGroupAdded(NL3D::UInstanceGroup *ig) = 0;
	virtual void instanceGroupRemoved(NL3D::UInstanceGroup *ig) = 0;
};

/** An helper class to register 'IIGAdded' observers
  */
class CIGNotifier
{
public:
	void registerObserver(IIGObserver *obs);
	void removeObserver(IIGObserver *obs);
	bool isObserver(IIGObserver *obs) const;
	//
	void notifyIGLoaded(NL3D::UInstanceGroup *ig);
	void notifyIGAdded(NL3D::UInstanceGroup *ig);
	void notifyIGRemoved(NL3D::UInstanceGroup *ig);
///////////////////
private:
	typedef std::vector<IIGObserver *> TObservers;
	TObservers	_Observers;
};

#endif





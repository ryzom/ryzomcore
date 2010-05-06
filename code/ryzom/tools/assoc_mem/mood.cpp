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

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details. 

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "mood.h"


CMood::CMood()
{
	_Fear = 0;
	_Agressivity = 0;
	_Empathy = 0;
	_Happiness = 0;
	_Hunger = 0;
}

CMood::CMood(float f,float a,float e,float ha,float hu)
{
	_Fear = f;
	_Agressivity = a;
	_Empathy = e;
	_Happiness = ha;
	_Hunger = hu;
}

CMood::CMood(const CMood &mood)
{
	_Fear = mood._Fear;
	_Agressivity = mood._Agressivity;
	_Empathy = mood._Empathy;
	_Happiness = mood._Happiness;
	_Hunger = mood._Hunger;

}

CMood::~CMood()
{
}



CMood CMood::operator=(const CMood &mood)
{
	_Fear = mood._Fear;
	_Agressivity = mood._Agressivity;
	_Empathy = mood._Empathy;
	_Happiness = mood._Happiness;
	_Hunger = mood._Hunger;
	return *this;
}

float CMood::getFear()
{
	return _Fear;
}

float CMood::getAgressivity()
{
	return _Agressivity;
}

float CMood::getEmpathy()
{
	return _Empathy;
}

float CMood::getHappiness()
{
	return _Happiness;
}

float CMood::getHunger()
{
	return _Hunger;
}

void CMood::setFear(float f)
{
	_Fear = f;
}

void CMood::setAgressivity(float a)
{
	_Agressivity = a;
}

void CMood::setEmpathy(float e)
{
	_Empathy = e;
}

void CMood::setHappiness(float ha)
{
	_Happiness = ha;
}

void CMood::setHunger(float hu)
{
	_Hunger = hu;
}


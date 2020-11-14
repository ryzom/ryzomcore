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
#include "player_manager/action_distance_checker.h"
#include "player_manager/player.h"

CActionDistanceChecker* CActionDistanceChecker::_Instance = NULL;

//---------------------------------------------------
// CActionDistanceChecker init
//---------------------------------------------------
void CActionDistanceChecker::init()
{
	nlassert(_Instance == NULL);
	_Instance = new CActionDistanceChecker;
}// init

//---------------------------------------------------
// CActionDistanceChecker release
//---------------------------------------------------
void CActionDistanceChecker::release()
{
	nlassert(_Instance != NULL);
	delete _Instance;
	_Instance = NULL;
}// init

//---------------------------------------------------
// CActionDistanceChecker ctor
//---------------------------------------------------
CActionDistanceChecker::CActionDistanceChecker()
:_LastCheck(0),_NbCheckPerTick(50)
{
}// ctor



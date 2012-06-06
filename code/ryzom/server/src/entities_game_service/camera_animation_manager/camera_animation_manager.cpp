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
// 

#include "stdpch.h"

#include "camera_animation_manager/camera_animation_manager.h"

using namespace std;

CCameraAnimationManager* CCameraAnimationManager::_Instance = NULL;

void CCameraAnimationManager::init()
{
	// Just asserts the instance is not already created and we create the manager that will load the camera animations
	nlassert(_Instance == NULL);
	_Instance = new CCameraAnimationManager();
}

void CCameraAnimationManager::release()
{
	// We delete the instance of the manager which will delete the allocated resources
	delete _Instance;
	_Instance = NULL;
}

CCameraAnimationManager::CCameraAnimationManager()
{

}

CCameraAnimationManager::~CCameraAnimationManager()
{

}

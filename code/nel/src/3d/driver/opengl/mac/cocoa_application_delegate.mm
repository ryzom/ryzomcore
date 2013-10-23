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

#include "../stdopengl.h"
#include "../driver_opengl.h"

#include "cocoa_application_delegate.h"
#include "cocoa_event_emitter.h"

static NSApplicationTerminateReply applicationShouldTerminate(CDriverGL* driver)
{
	// cancel if there is a driver and a custom exit handler set up
	if(driver && driver->ExitFunc)
	{
		driver->ExitFunc();
		return NSTerminateCancel;
	}

	NLMISC::CCocoaEventEmitter* eventEmitter = 
		NLMISC::safe_cast<NLMISC::CCocoaEventEmitter*>(&(driver->_EventEmitter));

	// cancel if there is a driver and cocoa event emitter handles the quit 
	if(driver && eventEmitter && eventEmitter->handleQuitRequest())
		return NSTerminateCancel;

	// just let the app terminate if no custom quit handling worked
	return NSTerminateNow;
}

@implementation CocoaApplicationDelegate

-(id)initWithDriver:(CDriverGL*)driver
{
	if((self = [super init]))
	{
		_driver = driver;
		return self;
	}
	return nil;
}

-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender 
{
	return applicationShouldTerminate(_driver);
}

@end

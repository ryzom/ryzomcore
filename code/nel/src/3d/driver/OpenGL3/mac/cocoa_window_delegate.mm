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

#import "cocoa_window_delegate.h"

static void windowDidMove(NSWindow* window, CDriverGL* driver)
{
	// get the rect (position, size) of the screen with menu bar
	NSRect screenRect = [[[NSScreen screens] objectAtIndex:0] frame];

	// get the rect (position, size) of the window
	NSRect windowRect = [window frame];

	// set x in driver
	driver->_WindowX = windowRect.origin.x;

	// map y from cocoa to NeL coordinates before setting in driver
	driver->_WindowY = 
		screenRect.size.height - windowRect.size.height - windowRect.origin.y;
}

@implementation CocoaWindowDelegate

-(id)initWithDriver:(CDriverGL*)driver 
{
	if((self = [super init])) 
	{
		_driver = driver;
		return self;
	}

	return nil;
}

-(void)windowDidMove:(NSNotification *)notification
{
	if(!_driver)
		return;

	windowDidMove([notification  object], _driver);
}

@end

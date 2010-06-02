/*
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
*/

#import "cocoa_opengl_view.h"

@implementation CocoaOpenGLView

-(BOOL)acceptsFirstResponder
{
	return YES;
}

-(BOOL)needsPanelToBecomeKey
{
	return NO;
}

-(void)keyDown:(NSEvent*)event
{
	// we handle the key here, so os x does not make a sound :)
	/*
		TODO do it in the event emitter? eg do not forward key down? 
			does command+q / command+m still work then?
	*/
}

@end

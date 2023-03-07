// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#import <Cocoa/Cocoa.h>
#include "AvailabilityMacros.h"

#ifdef NL_STATIC
#ifdef USE_OPENGLES
using NL3D::NLDRIVERGLES::CDriverGL;
#else
using NL3D::NLDRIVERGL::CDriverGL;
#endif
#else
using NL3D::CDriverGL;
#endif

@interface CocoaWindowDelegate : NSObject
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
<NSWindowDelegate>
#endif
{
	CDriverGL* _driver;
}

- (id)initWithDriver:(CDriverGL*)driver;
- (void)windowDidMove:(NSNotification*)notification;

@end

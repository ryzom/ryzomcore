// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Robert TIMM (rti) <mail@rtti.de>
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

namespace NL3D 
{

#ifdef NL_STATIC
#ifdef USE_OPENGLES
namespace NLDRIVERGLES {
#else
namespace NLDRIVERGL {
#endif
#endif

	class CDriverGL;

#ifdef NL_STATIC
} // NLDRIVERGL/ES
#endif

}

#ifdef NL_STATIC
#ifdef USE_OPENGLES
using NL3D::NLDRIVERGLES::CDriverGL;
#else
using NL3D::NLDRIVERGL::CDriverGL;
#endif
#else
using NL3D::CDriverGL;
#endif

@interface CocoaOpenGLView : NSOpenGLView<NSTextInputClient>
{
	NSMutableAttributedString* _characterStorage;
	NSRange _markedRange;
	CDriverGL* _driver;
}

-(id)initWithFrame:(NSRect)frame;
-(void)dealloc;
-(void)keyDown:(NSEvent*)event;
-(void)setDriver:(CDriverGL*)driver;
-(void)resizeWithOldSuperviewSize:(NSSize)oldBoundsSize;

@end

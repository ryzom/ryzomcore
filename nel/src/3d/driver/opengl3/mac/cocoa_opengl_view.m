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

#import "cocoa_opengl_view.h"

static void viewDidResize(NSView* view, CDriverGL* driver)
{
	NSRect rect = [[view superview] frame];
	driver->_CurrentMode.Height = rect.size.height;
	driver->_CurrentMode.Width  = rect.size.width;
}

@implementation CocoaOpenGLView

-(id)initWithFrame:(NSRect)frame
{
	if((self = [super initWithFrame:frame]))
	{
		_characterStorage = [[NSMutableAttributedString alloc] initWithString:@""];
		_driver           = nil;
		return self;
	}
	return nil;
}

-(void)dealloc
{
	[_characterStorage release];
	[super dealloc];
}

-(void)keyDown:(NSEvent*)event
{
#if defined(MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
	[[self inputContext] handleEvent:event];
#endif
}

-(void)setDriver:(CDriverGL*)driver
{
	_driver = driver;
}

-(void)resizeWithOldSuperviewSize:(NSSize)oldBoundsSize
{
	[super resizeWithOldSuperviewSize:oldBoundsSize];

	if(!_driver)
		return;

	viewDidResize(self, _driver);
}

/******************************************************************************/
/* NSTextInputClient Protocol */

-(BOOL)hasMarkedText
{
	return (_markedRange.location == NSNotFound ? NO : YES);
}

-(NSRange)markedRange
{
	return _markedRange;
}

-(NSRange)selectedRange
{
	return NSMakeRange(NSNotFound, 0);
}

-(void)setMarkedText:(id)aString
	selectedRange:(NSRange)newSelection
	replacementRange:(NSRange)replacementRange
{
	if(replacementRange.location == NSNotFound)
		replacementRange = _markedRange;

	if([aString length] == 0)
	{
		[_characterStorage deleteCharactersInRange:replacementRange];
		[self unmarkText];
	}
	else
	{
		_markedRange = NSMakeRange(replacementRange.location, [aString length]);
		[_characterStorage replaceCharactersInRange:replacementRange
			withString:aString];
	}
}

-(void)unmarkText
{
	_markedRange = NSMakeRange(NSNotFound, 0);
	[[self inputContext] discardMarkedText];
}

-(NSArray*)validAttributesForMarkedText
{
	return [NSArray arrayWithObjects:
		NSMarkedClauseSegmentAttributeName, NSGlyphInfoAttributeName, nil];
}

-(NSAttributedString*)attributedSubstringForProposedRange:(NSRange)aRange
	actualRange:(NSRangePointer)actualRange
{
	return [_characterStorage attributedSubstringFromRange:aRange];
}

-(void)insertText:(id)aString
	replacementRange:(NSRange)replacementRange
{
	if(replacementRange.location == NSNotFound)
		replacementRange = _markedRange;

	[_characterStorage replaceCharactersInRange:replacementRange
		withString:aString];
}

-(NSUInteger)characterIndexForPoint:(NSPoint)aPoint
{
	return 0;
}

-(NSRect)firstRectForCharacterRange:(NSRange)aRange
	actualRange:(NSRangePointer)actualRange
{
	return NSMakeRect(0, 0, 0, 0);
}

-(void)doCommandBySelector:(SEL)aSelector
{
	[super doCommandBySelector:aSelector];
}

@end

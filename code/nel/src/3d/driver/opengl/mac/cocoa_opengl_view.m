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

- (id)initWithFrame:(NSRect)frame 
{
	if(self = [super initWithFrame:frame]) 
	{
		characterStorage = [[NSMutableAttributedString alloc] initWithString:@""];
		return self;
	}
	return nil;
}

- (void)dealloc
{
	[characterStorage release];
	[super dealloc];
}

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
	[[self inputContext] handleEvent:event];
}

/******************************************************************************/
/* NSTextInputClient Protocol */

-(BOOL)hasMarkedText 
{
	return (markedRange.location == NSNotFound ? NO : YES);
}

-(NSRange)markedRange 
{
	return markedRange;
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
		replacementRange = markedRange;

	if([aString length] == 0) 
	{
		[characterStorage deleteCharactersInRange:replacementRange];
		[self unmarkText];
	} 
	else 
	{
		markedRange = NSMakeRange(replacementRange.location, [aString length]);
		[characterStorage replaceCharactersInRange:replacementRange withString:aString];
	}
}

-(void)unmarkText
{
	markedRange = NSMakeRange(NSNotFound, 0);
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
	return [characterStorage attributedSubstringFromRange:aRange];
}

-(void)insertText:(id)aString 
	replacementRange:(NSRange)replacementRange
{
	if(replacementRange.location == NSNotFound)
		replacementRange = markedRange;

	[characterStorage replaceCharactersInRange:replacementRange withString:aString];
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

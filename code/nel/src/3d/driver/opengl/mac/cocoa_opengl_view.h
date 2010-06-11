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

#import <Cocoa/Cocoa.h>

/**
 * derived to configure the NSOpenGLView
 */
@interface CocoaOpenGLView : NSOpenGLView<NSTextInputClient>
{
  NSMutableAttributedString* characterStorage;
  NSRange markedRange;
}

-(id)initWithFrame:(NSRect)frame;
-(void)dealloc;

-(BOOL)acceptsFirstResponder;
-(BOOL)needsPanelToBecomeKey;
-(void)keyDown:(NSEvent*)event;

/******************************************************************************/
/* NSTextInputClient Protocol */

-(BOOL)hasMarkedText;
-(NSRange)markedRange;
-(NSRange)selectedRange;
-(void)setMarkedText:(id)aString 
	selectedRange:(NSRange)newSelection 
	replacementRange:(NSRange)replacementRange;
-(void)unmarkText;
-(NSArray*)validAttributesForMarkedText;
-(NSAttributedString*)attributedSubstringForProposedRange:(NSRange)aRange 
	actualRange:(NSRangePointer)actualRange;
-(void)insertText:(id)aString replacementRange:(NSRange)replacementRange;
-(NSUInteger)characterIndexForPoint:(NSPoint)aPoint;
-(NSRect)firstRectForCharacterRange:(NSRange)aRange 
	actualRange:(NSRangePointer)actualRange;
-(void)doCommandBySelector:(SEL)aSelector;

@end

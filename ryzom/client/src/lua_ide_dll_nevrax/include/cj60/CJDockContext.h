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

// CJDockContext.h : header file
//
// Copyright (c) 1998-99 Kirk Stowell   
//		mailto:kstowell@codejockeys.com
//		http://www.codejockeys.com/kstowell/
//
// This source code may be used in compiled form in any way you desire. 
// Source file(s) may be redistributed unmodified by any means PROVIDING
// they are not sold for profit without the authors expressed written consent,
// and providing that this notice and the authors name and all copyright
// notices remain intact. If the source code is used in any commercial
// applications then a statement along the lines of:
//
// "Portions Copyright (c) 1998-99 Kirk Stowell" must be included in the
// startup banner, "About" box or printed documentation. An email letting
// me know that you are using it would be nice as well. That's not much to ask
// considering the amount of work that went into this.
//
// This software is provided "as is" without express or implied warranty. Use
// it at your own risk! The author accepts no liability for any damage/loss of
// business that this product may cause.
//
// ==========================================================================  
//
// Acknowledgements:
//	<>  Many thanks to all of you, who have encouraged me to update my articles
//		and code, and who sent in bug reports and fixes.
//  <>  Many thanks Zafir Anjum (zafir@codeguru.com) for the tremendous job that
//      he has done with codeguru, enough can not be said!
//	<>  Many thanks to Microsoft for making the source code availiable for MFC. 
//		Since most of this work is a modification from existing classes and 
//		methods, this library would not have been possible.
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	12 Jan 1999 - Initial creation to add side-by-side 
//								  docking support for CCJControlBar class.
// ==========================================================================
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CJDOCKCONTEXT_H__
#define __CJDOCKCONTEXT_H__

/////////////////////////////////////////////////////////////////////////////
// CCJDockContext class

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CCJSizeDockBar;
class CCJControlBar;
class AFX_EXT_CLASS CCJDockContext : public CDockContext
{
// Construction
public:
	CCJDockContext(CControlBar* pBar);

// Attributes
public:
protected:

// Operations
public:
	void EndDragDockBar();
	void MoveDockBar(CPoint pt);
	void StartDragDockBar(CPoint pt);
	DWORD CanDockDockBar();
	DWORD CanDockDockBar(CRect rect, DWORD dwDockStyle, CDockBar** ppDockBar = NULL);
	BOOL TrackDockBar();
	CCJSizeDockBar* GetDockBar(DWORD dwOverDockStyle);
	void DockSizeBar(CControlBar *,CCJSizeDockBar* =NULL,LPRECT =NULL);
	virtual void ToggleDocking();

// Implementation
public:
	virtual ~CCJDockContext();
};

/////////////////////////////////////////////////////////////////////////////

#endif // __CJDOCKCONTEXT_H__

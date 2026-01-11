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

// ScintillaBar.h: interface for the CScintillaBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCINTILLABAR_H__377FAA54_BEAC_412F_8A77_86A79CBF577C__INCLUDED_)
#define AFX_SCINTILLABAR_H__377FAA54_BEAC_412F_8A77_86A79CBF577C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScintillaView;
struct SCNotification;

class CScintillaBar : public CCJTabCtrlBar
{
public:
	CScintillaBar();
	virtual ~CScintillaBar();

	virtual int OnSci(CScintillaView* pView, SCNotification* pNotify) = 0;
};

#endif // !defined(AFX_SCINTILLABAR_H__377FAA54_BEAC_412F_8A77_86A79CBF577C__INCLUDED_)

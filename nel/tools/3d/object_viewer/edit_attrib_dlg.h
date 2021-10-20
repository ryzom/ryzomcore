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


#if !defined(AFX_EDIT_ATTRIB_DLG_H__0B2EFF2B_FA0E_4AC8_88B8_416605043BF9__INCLUDED_)
#define AFX_EDIT_ATTRIB_DLG_H__0B2EFF2B_FA0E_4AC8_88B8_416605043BF9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * a base class for attribute edition dialog in a particle system
 */

class CEditAttribDlg : public CDialog
{
public:
	virtual BOOL EnableWindow( BOOL bEnable = TRUE ) { return CDialog::EnableWindow(bEnable) ; } ;
	virtual void init(uint32 x, uint32 y, CWnd *pParent) = 0 ;
	virtual ~CEditAttribDlg() {}
} ;


#endif 

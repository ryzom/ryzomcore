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

// ProjectFile.h: interface for the CProjectFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROJECTFILE_H__737B3FC6_28E7_47DD_B9F6_D41D550C8E23__INCLUDED_)
#define AFX_PROJECTFILE_H__737B3FC6_28E7_47DD_B9F6_D41D550C8E23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

class CLuaEditor;


class CProjectFile
{
public:
	class CBreakPoint
	{
	public:
		int		m_Line;
		CString m_Condition;
		BOOL    m_Enabled;
		CBreakPoint(int line = -1) : m_Line(line), m_Enabled(TRUE) {}
	};
	void UpdateRelPathName();
	void DeleteIntermediateFiles();
	BOOL Compile();
	BOOL IsModified();
	BOOL Save(CArchive& ar);
	BOOL Load(CArchive& ar);
	void RemoveBreakPoint(int nLine);
	BOOL HasFile(CString strPathName);
	void SetBreakPointsIn(CLuaEditor* pEditor);
	BOOL HasBreakPoint(int nLine);
	BOOL GetBreakPoint(int nLine, CBreakPoint &bp);
	CProjectFile();
	virtual ~CProjectFile();

	void AddDebugLine(int nLine);
	void RemoveAllDebugLines();
	void AddBreakPoint(int nLine);
	void AddBreakPoint(const CBreakPoint &bp);
	void RemoveAllBreakPoints();

	BOOL PositionBreakPoints();
	int GetNearestDebugLine(int nLine);
	int GetPreviousDebugLine(int nLine);
	int GetNextDebugLine(int nLine);

	void SetPathName(CString strPathName);	
	CString GetPathName() { return m_strPathName; };
	CString GetName();
	CString GetNameExt();
	CString GetOutputNameExt() { return GetName()+".out"; }
	CString GetOutputPathNameExt();

	void	GetBreakPointLines(std::vector<int> &dest) const;
	void	GetBreakPoints(std::vector<CBreakPoint> &dest) const;

	void SetModifiedFlag(BOOL modified);

protected:
	CString m_strPathName, m_strRelPathName;
	CMap<int, int, CBreakPoint, CBreakPoint> m_breakPoints;
	int m_nMinBreakPoint, m_nMaxBreakPoint;
	CMap<int, int, BOOL, BOOL> m_debugLines;
	int m_nMinDebugLine, m_nMaxDebugLine;
	SYSTEMTIME	m_timeCompiled;
	BOOL		m_Modified;
public:
	HTREEITEM	m_TreeItem;
};

#endif // !defined(AFX_PROJECTFILE_H__737B3FC6_28E7_47DD_B9F6_D41D550C8E23__INCLUDED_)

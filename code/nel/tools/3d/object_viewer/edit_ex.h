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

#if !defined(AFX_EDIT_EX_H__5D675CB2_E40F_4031_856A_33B0B3BD362E__INCLUDED_)
#define AFX_EDIT_EX_H__5D675CB2_E40F_4031_856A_33B0B3BD362E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "nel/misc/types_nl.h"
#include <string>


/** A CEdit control, that is selected when the focus is on it, and that can notify a listener when return is pressed.
  * It can be setup to contains a given type.
  */
class CEditEx : public CEdit
{
public:
	enum TType { StringType, FloatType, SIntType, UIntType, LastType };
	/// ctor
	CEditEx();
	/// dtor
	virtual ~CEditEx();
	void	setType(TType type) { nlassert(type < LastType); _Type = type; }
	TType	getType() const { return _Type; }

	/// interface of a listener that can be notified when return is pressed
	struct IListener
	{
		virtual void editExValueChanged(CEditEx *ctrl) = 0;
	};
	void	setListener(IListener *l) { _Listener = l; }	
	IListener	*getListener() { return _Listener; }		

	/// should have the right type, otherwise it asserts. (except for getString)

	sint		getSInt() const;
	uint		getUInt() const;
	float		getFloat() const;
	std::string getString() const;

	void		setSInt(sint value);
	void		setUInt(uint value);
	void		setFloat(float value);
	void	    setString(const TCHAR *value);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditEx)
	//}}AFX_VIRTUAL

// Implementation
protected:
	TType _Type;
	IListener *_Listener;

	// Generated message map functions
	//{{AFX_MSG(CEditEx)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	bool isValid();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDIT_EX_H__5D675CB2_E40F_4031_856A_33B0B3BD362E__INCLUDED_)

// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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


#include "stdpch.h"
#include "nel/gui/view_pointer_base.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	CViewPointerBase::CViewPointerBase( const CViewBase::TCtorParam &param ) :
	CViewBase( param ),
	_Buttons( NLMISC::noButton )
	{
		_PointerX = _PointerY = _PointerOldX = _PointerOldY = _PointerDownX = _PointerDownY = InvalidCoord;
		_PointerDown = false;
		_PointerVisible = true;
	}

	CViewPointerBase::~CViewPointerBase()
	{
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::setPointerPos (sint32 x, sint32 y)
	{
		if (_PointerDown)
		{
			if (!_PointerDrag)
			{
				if (((_PointerX - _PointerDownX) != 0) ||
					((_PointerY - _PointerDownY) != 0))
				{
					_PointerDrag = true;
				}
			}
		}

		_PointerOldX = getX();
		_PointerOldY = getY();

		_PointerX = x;
		_PointerY = y;
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::setPointerDispPos (sint32 x, sint32 y)
	{
		setX (x);
		setY (y);
		updateCoords ();
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::resetPointerPos ()
	{
		_PointerOldX = _PointerX;
		_PointerOldY = _PointerY;
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::setPointerDown (bool pd)
	{
		_PointerDown = pd;

		if (_PointerDown)
		{
			_PointerDownX = _PointerX;
			_PointerDownY = _PointerY;
		}
		else
			_PointerDrag = false;
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::setPointerMiddleDown (bool pd)
	{
		_PointerMiddleDown = pd;

		if (_PointerMiddleDown)
		{
			_PointerMiddleDownX = _PointerX;
			_PointerMiddleDownY = _PointerY;
		}
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::setPointerRightDown (bool pd)
	{
		_PointerRightDown = pd;

		if (_PointerRightDown)
		{
			_PointerRightDownX = _PointerX;
			_PointerRightDownY = _PointerY;
		}
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::setPointerDownString (const std::string &s)
	{
		_PointerDownString = s;
	}

	// +++ GET +++

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::getPointerPos (sint32 &x, sint32 &y)
	{
		x = _PointerX;
		y = _PointerY;
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::getPointerDispPos (sint32 &x, sint32 &y)
	{
		x = getX();
		y = getY();
	}

	// --------------------------------------------------------------------------------------------------------------------
	void CViewPointerBase::getPointerOldPos (sint32 &x, sint32 &y)
	{
		x = _PointerOldX;
		y = _PointerOldY;
	}

	// --------------------------------------------------------------------------------------------------------------------
	bool CViewPointerBase::getPointerDown (sint32 &x, sint32 &y)
	{
		x = _PointerDownX;
		y = _PointerDownY;

		return _PointerDown;
	}

	// --------------------------------------------------------------------------------------------------------------------
	bool CViewPointerBase::getPointerMiddleDown (sint32 &x, sint32 &y)
	{
		x = _PointerMiddleDownX;
		y = _PointerMiddleDownY;

		return _PointerMiddleDown;
	}

	// --------------------------------------------------------------------------------------------------------------------
	bool CViewPointerBase::getPointerRightDown (sint32 &x, sint32 &y)
	{
		x = _PointerRightDownX;
		y = _PointerRightDownY;

		return _PointerRightDown;
	}

	// --------------------------------------------------------------------------------------------------------------------
	bool CViewPointerBase::getPointerDrag ()
	{
		return _PointerDrag;
	}

	// --------------------------------------------------------------------------------------------------------------------
	std::string CViewPointerBase::getPointerDownString ()
	{
		return _PointerDownString;
	}

}


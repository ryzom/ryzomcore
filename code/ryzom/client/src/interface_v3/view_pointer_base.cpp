#include "view_pointer_base.h"

CViewPointerBase::CViewPointerBase( const CViewBase::TCtorParam &param ) :
CViewBase( param )
{
	_PointerX = _PointerY = _PointerOldX = _PointerOldY = _PointerDownX = _PointerDownY = 0;
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

	if (_PointerDown == true)
	{
		_PointerDownX = _PointerX;
		_PointerDownY = _PointerY;
	}

	if (_PointerDown == false)
		_PointerDrag = false;
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
void CViewPointerBase::getPointerDownPos (sint32 &x, sint32 &y)
{
	x = _PointerDownX;
	y = _PointerDownY;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointerBase::getPointerDown ()
{
	return _PointerDown;
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




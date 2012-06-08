#ifndef VIEW_POINTER_BASE_H
#define VIEW_POINTER_BASE_H

#include "nel/gui/view_base.h"

class CViewPointerBase : public CViewBase
{
public:
	DECLARE_UI_CLASS( CViewPointerBase )

	CViewPointerBase( const TCtorParam &param );
	virtual ~CViewPointerBase();

	// Set the pointer position.
	void setPointerPos (sint32 x, sint32 y);
	void setPointerDispPos (sint32 x, sint32 y);

	void resetPointerPos ();
	void setPointerDown (bool pd);
	void setPointerDownString (const std::string &s);

	void getPointerPos (sint32 &x, sint32 &y);
	void getPointerDispPos (sint32 &x, sint32 &y);

	void getPointerOldPos (sint32 &x, sint32 &y);
	void getPointerDownPos (sint32 &x, sint32 &y);
	bool getPointerDown ();
	std::string getPointerDownString ();
	bool getPointerDrag ();

	/// Is the pointer visible ?
	bool show() const {return _PointerVisible;}

	void draw(){}

protected:
	// (x,y) is from the TopLeft corner of the window
	sint32		_PointerX;				// Current pointer position (raw, before snapping)
	sint32		_PointerY;
	sint32		_PointerOldX;			// Previous frame pointer position
	sint32		_PointerOldY;
	bool		_PointerDown;			// Is the pointer down ?
	sint32		_PointerDownX;			// Pointer down position
	sint32		_PointerDownY;
	std::string	_PointerDownString;		// What is under the pointer at the down position
	bool		_PointerDrag;			// Is the pointer down and we have moved ?
	bool		_PointerVisible;		// Is the pointer visible or hidden ?

private:


};

#endif


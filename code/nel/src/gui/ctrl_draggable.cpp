#include "nel/gui/ctrl_draggable.h"

CCtrlDraggable* CCtrlDraggable::_LastDraggedSheet = NULL;

CCtrlDraggable::CCtrlDraggable(const TCtorParam &param) :
CCtrlBase( param )
{
	dragged   = false;
	draggable = false;
}


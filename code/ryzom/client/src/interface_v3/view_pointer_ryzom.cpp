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

#include "stdpch.h"
#include "view_pointer_ryzom.h"
#include "group_map.h"
#include "nel/gui/group_html.h"
#include "interface_3d_scene.h"
#include "../r2/editor.h"
#include "nel/gui/ctrl_col_pick.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/group_paragraph.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/group_container.h"


NLMISC_REGISTER_OBJECT( CViewBase, CViewPointerRyzom, std::string, "pointer");

CViewPointerRyzom::CViewPointerRyzom( const TCtorParam &param ) :
CViewPointer( param )
{
}

CViewPointerRyzom::~CViewPointerRyzom()
{
}

void CViewPointerRyzom::forceLinking()
{
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointerRyzom::drawResizer(CCtrlBase* pCB, CRGBA col)
{
	CCtrlResizer *pCR = dynamic_cast<CCtrlResizer*>(pCB);
	if (pCR != NULL)
	{
		CGroupContainer *parent = dynamic_cast<CGroupContainer *>(pCR->getParent());
		if (parent && !parent->isLocked())
		{
			sint32 texID= -1;
			switch(pCR->getRealResizerPos())
			{
				case Hotspot_BR:
				case Hotspot_TL:
					texID = _TxIdResizeBRTL;
				break;
				case Hotspot_BL:
				case Hotspot_TR:
					texID = _TxIdResizeBLTR;
				break;
				case Hotspot_MR:
				case Hotspot_ML:
					texID = _TxIdResizeLR;
				break;
				case Hotspot_TM:
				case Hotspot_BM:
					texID = _TxIdResizeTB;
				break;
				default:
					return false;
				break;
			}
			drawCursor(texID, col, false);
			return true;
		}
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointerRyzom::drawRotate (CCtrlBase* pCB, CRGBA col)
{
	CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene *>(pCB);
	if (pI3DS != NULL)
	{
		drawCursor(_TxIdRotate, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointerRyzom::drawScale (CCtrlBase* pCB, CRGBA col)
{
	CInterface3DScene *pI3DS = dynamic_cast<CInterface3DScene *>(pCB);
	if (pI3DS != NULL)
	{
		drawCursor(_TxIdScale, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointerRyzom::drawColorPicker (CCtrlBase* pCB, CRGBA col)
{
	CCtrlColPick *pCCP = dynamic_cast<CCtrlColPick*>(pCB);
	if (pCCP != NULL)
	{
		drawCursor(_TxIdColPick, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointerRyzom::drawLink (CCtrlBase* pCB, CRGBA col)
{
	CCtrlLink *pCCP = dynamic_cast<CCtrlLink*>(pCB);
	if (pCCP != NULL)
	{
		drawCursor(_TxIdColPick, col, 0);
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointerRyzom::drawBrowse (CCtrlBase* pCB, CRGBA col)
{
	CGroupHTML *pCGH = dynamic_cast<CGroupHTML *>(pCB);
	if (pCGH != NULL)
	{
		if (pCGH->isBrowsing())
		{
			static uint8 rot =0;
			drawCursor(_TxIdRotate, col, rot>>3);
			rot = (rot+1) & 0x1f;
			return true;
		}
	}
	return false;
}

// --------------------------------------------------------------------------------------------------------------------
bool CViewPointerRyzom::drawPan(CCtrlBase* pCB, NLMISC::CRGBA col)
{
	CGroupMap *gm = dynamic_cast<CGroupMap *>(pCB);
	if (gm)
	{
		sint32 texId;
		if (ClientCfg.R2EDEnabled && R2::getEditor().getCurrentTool())
		{
			/** If cursor is set to anything other than the default cursor, use that cursor (because action can be performed on the map
			  * by the current tool
			  */
			if (_TxDefault == "curs_default.tga")
			{
				texId = gm->isPanning() ? _TxIdPanR2 : _TxIdCanPanR2;
			}
			else return false;
		}
		else
		{
			texId = gm->isPanning() ? _TxIdPan : _TxIdCanPan;
		}
		drawCursor(texId, col, 0);
		return true;
	}
	return false;
}



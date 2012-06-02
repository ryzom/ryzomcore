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


#include "dbgroup_select_number.h"
#include "view_text.h"
#include "view_bitmap.h"
#include "ctrl_button.h"
#include "interface_property.h"
#include "interface_manager.h"
#include "action_handler.h"


using namespace std;
using namespace NL3D;
using namespace NLMISC;

NLMISC_REGISTER_OBJECT(CViewBase, CDBGroupSelectNumber, std::string, "select_number");

// ***************************************************************************
CDBGroupSelectNumber::CDBGroupSelectNumber(const TCtorParam &param)
:	CInterfaceGroup(param)
{
	_SlotNumber= NULL;
	_TextNumber= NULL;
	_ButtonUp= NULL;
	_ButtonDown= NULL;
	_LoopMode= true;
	_MinValue= 0;
	_MaxValue= 9;
	_DeltaMultiplier= 1;
}

// ***************************************************************************
CDBGroupSelectNumber::~CDBGroupSelectNumber()
{
}

// ***************************************************************************
bool CDBGroupSelectNumber::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	if(!CInterfaceGroup::parse(cur, parentGroup))
		return false;

	// read params
	CXMLAutoPtr ptr;
	ptr = xmlGetProp (cur, (xmlChar*)"value");
	if ( ptr )
		_Number.link ( ptr );
	else
	{
		nlinfo ("no value in %s", _Id.c_str());
		return false;
	}

	// Loop, min and max
	_LoopMode= false;
	ptr= xmlGetProp (cur, (xmlChar*)"loop");
	if(ptr)	_LoopMode= convertBool(ptr);
	_MinValue = 0;
	ptr= xmlGetProp (cur, (xmlChar*)"min");
	if(ptr)	fromString((const char*)ptr, _MinValue);
	_MaxValue = 0;
	ptr= xmlGetProp (cur, (xmlChar*)"max");
	if(ptr)	fromString((const char*)ptr, _MaxValue);
	ptr= xmlGetProp (cur, (xmlChar*)"delta");
	if(ptr)	fromString((const char*)ptr, _DeltaMultiplier);

	// set min val in DB
	_Number.setSInt32(_MinValue);

	return true;
}

// ***************************************************************************
void CDBGroupSelectNumber::setup()
{
	if (_SlotNumber != NULL)
		return;

	// bind to the controls.
	_SlotNumber= dynamic_cast<CViewBitmap*>(CInterfaceGroup::getView("slot_number"));
	_TextNumber= dynamic_cast<CViewText*>(CInterfaceGroup::getView("number"));
	_ButtonUp= dynamic_cast<CCtrlBaseButton*>(CInterfaceGroup::getCtrl("arrow_up"));
	_ButtonDown= dynamic_cast<CCtrlBaseButton*>(CInterfaceGroup::getCtrl("arrow_down"));

	// checks
	if(_SlotNumber==NULL)
		nlwarning("Interface: SelectNumberGroup: bitmap 'slot_number' missing or bad type");
	if(_TextNumber==NULL)
		nlwarning("Interface: SelectNumberGroup: text view 'number' missing or bad type");
	if(_ButtonUp==NULL)
		nlwarning("Interface: SelectNumberGroup: button 'arrow_up' missing or bad type");
	if(_ButtonDown==NULL)
		nlwarning("Interface: SelectNumberGroup: button 'arrow_down' missing or bad type");
	if(_SlotNumber==NULL || _TextNumber==NULL || _ButtonUp==NULL || _ButtonDown==NULL)
		return;

	// actions
	_ButtonUp->setActionOnLeftClick("sn_up");
	_ButtonDown->setActionOnLeftClick("sn_down");
}


// ***************************************************************************
void CDBGroupSelectNumber::updateCoords ()
{
	setup();
	CInterfaceGroup::updateCoords();
}



void CDBGroupSelectNumber::checkCoords()
{
	if(_TextNumber)
		_TextNumber->setText( toString(_Number.getSInt32()) );
	CInterfaceGroup::checkCoords();
}

// ***************************************************************************
void CDBGroupSelectNumber::draw ()
{
	CInterfaceGroup::draw();
}

// ***************************************************************************
void CDBGroupSelectNumber::clearViews ()
{
	CInterfaceGroup::clearViews();
}

// ***************************************************************************
bool CDBGroupSelectNumber::handleEvent (const CEventDescriptor &event)
{
	if (event.getType() == CEventDescriptor::mouse)
	{
		const CEventDescriptorMouse &eventDesc = (const CEventDescriptorMouse &)event;
		if (isIn(eventDesc.getX(), eventDesc.getY()))
		{
			if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mousewheel)
			{
				changeValue (eventDesc.getWheel());
				return true;
			}
		}
	}
	if (CInterfaceGroup::handleEvent(event)) return true;
	return false;
}


// ***************************************************************************
void CDBGroupSelectNumber::changeValue(sint delta)
{
	delta*= _DeltaMultiplier;

	// get DB and add
	sint32	val= _Number.getSInt32();
	val+= delta;

	// Loop or clamp
	if(_LoopMode)
	{
		sint32	dval= _MaxValue+1 - _MinValue;
		if (dval <= 0)
		{
			val = 0;
			return;
		}
		val -= _MinValue;
		val = val% dval; val = (val+dval)% dval;
		val += _MinValue;
	}
	else
	{
		clamp(val, _MinValue, _MaxValue);
	}

	// set in DB
	_Number.setSInt32(val);
	if(_TextNumber)
		_TextNumber->setText( toString(_Number.getSInt32()) );
}


// ***************************************************************************
// ***************************************************************************
// Actions Handlers
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
class CSNUp : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
	{
		CDBGroupSelectNumber *pSN = dynamic_cast<CDBGroupSelectNumber*>(pCaller->getParent());
		if (pSN == NULL) return;
		pSN->changeValue(+1);
	}
};
REGISTER_ACTION_HANDLER (CSNUp, "sn_up");

// ***************************************************************************
class CSNDown : public IActionHandler
{
public:
	virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
	{
		CDBGroupSelectNumber *pSN = dynamic_cast<CDBGroupSelectNumber*>(pCaller->getParent());
		if (pSN == NULL) return;
		pSN->changeValue(-1);
	}
};
REGISTER_ACTION_HANDLER (CSNDown, "sn_down");


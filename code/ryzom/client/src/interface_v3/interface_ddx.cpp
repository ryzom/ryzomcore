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

#include "interface_manager.h"
#include "interface_ddx.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/ctrl_button.h"
#include "nel/gui/group_modal.h"
#include "../client_cfg.h"

#include "nel/misc/xml_auto_ptr.h"
//

using namespace std;
using namespace NLMISC;

// ***************************************************************************
// CParam
// ***************************************************************************

// ***************************************************************************
void CInterfaceDDX::CParam::DBToWidget()
{
	if (Type != DataBase) return;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (Widget == ColorButton)
	{
		CRGBA col = CRGBA::White;
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
		{
			uint32 intCol = (uint32)pNL->getValue32();
			col.R = (uint8) (intCol & 0xff);
			col.G = (uint8) ((intCol >> 8) & 0xff);
			col.B = (uint8) ((intCol >> 16) & 0xff);
			col.A = (uint8) ((intCol >> 24) & 0xff);
		}
		CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(Elt.getPtr());
		if (pBut != NULL)
		{
			pBut->setColor(col);
			pBut->setColorPushed(col);
			pBut->setColorOver(col);
		}
	}
	else if (Widget == BoolButton)
	{
		bool bVal = 0;
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
		{
			bVal = pNL->getValue32()==0?false:true;
		}
		CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(Elt.getPtr());
		if (pBut != NULL)
		{
			pBut->setPushed(bVal);
		}
	}
	else if (Widget == ScrollBarInt)
	{
		sint32 nVal = 0;
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
		{
			nVal = pNL->getValue32();
		}
		CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
		if (pCS != NULL)
		{
			pCS->setValue (nVal);
			pCS->setMinMax (Min, Max);
		}
		updateScrollView(nVal);
	}
}


// ***************************************************************************
void CInterfaceDDX::CParam::CFGToWidget()
{
	if (Type != ConfigFile) return;

	if (Widget == BoolButton)
	{
		bool bVal = ClientCfg.readBool(Link);
		CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(Elt.getPtr());
		if (pBut != NULL)
		{
			pBut->setPushed(bVal);
		}
	}
	else if (Widget == ScrollBarInt)
	{
		sint32 nVal = ClientCfg.readInt(Link);
		sint32 nMin = ClientCfg.readInt(Link+"_min");
		sint32 nMax = ClientCfg.readInt(Link+"_max");
		sint32 step = ClientCfg.readIntNoWarning(Link+"_step");
		CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
		if (pCS != NULL)
		{
			pCS->setMinMax (nMin, nMax);
			pCS->setStepValue(step);
			pCS->setValue(nVal);
		}
		updateScrollView(nVal);
	}
	else if (Widget == ScrollBarFloat)
	{
		double rVal= ClientCfg.readDouble(Link);
		tryRound(rVal);
		sint32 nVal = (sint32)(10000*rVal);
		sint32 nMin = (sint32)(10000*ClientCfg.readDouble(Link+"_min"));
		sint32 nMax = (sint32)(10000*ClientCfg.readDouble(Link+"_max"));
		sint32 step = (sint32)(10000*ClientCfg.readDoubleNoWarning(Link+"_step"));
		CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
		if (pCS != NULL)
		{
			pCS->setMinMax (nMin, nMax);
			pCS->setStepValue(step);
			pCS->setValue (nVal);
		}
		updateScrollView(rVal);
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::WidgetToDB()
{
	if (Type != DataBase) return;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (Widget == ColorButton)
	{
		CRGBA col = CRGBA::White;
		CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(Elt.getPtr());
		if (pBut != NULL)
			col = pBut->getColor();
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
			pNL->setValue32(col.R+(col.G<<8)+(col.B<<16)+(col.A<<24));
	}
	else if (Widget == BoolButton)
	{
		bool bVal = false;
		CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(Elt.getPtr());
		if (pBut != NULL)
			bVal = pBut->getPushed();
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
		{
			pNL->setValue32(bVal?1:0);
		}
	}
	else if (Widget == ScrollBarFloat)
	{
		// TODO
	}
	else if (Widget == ScrollBarInt)
	{
		sint32 nVal = 0;
		CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
		if (pCS != NULL)
			nVal = pCS->getValue();
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
			pNL->setValue32(nVal);
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::WidgetToCFG()
{
	if (Type != ConfigFile) return;

	if (Widget == BoolButton)
	{
		bool bVal = false;
		CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(Elt.getPtr());
		if (pBut != NULL)
			bVal = pBut->getPushed();
		ClientCfg.writeBool(Link, bVal);
	}
	else if (Widget == ScrollBarInt)
	{
		sint32 nVal = 0;
		CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
		if (pCS != NULL)
			nVal = pCS->getValue();
		ClientCfg.writeInt(Link, nVal);
	}
	else if (Widget == ScrollBarFloat)
	{
		sint32 nVal = 0;
		CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
		if (pCS != NULL)
			nVal = pCS->getValue();
		double	rVal= nVal/10000.0;
		tryRound(rVal);
		ClientCfg.writeDouble(Link, rVal);
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::WidgetToResultView()
{
	if (Widget == ScrollBarInt)
	{
		sint32 nVal = 0;
		CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
		if (pCS != NULL)
			nVal = pCS->getValue();
		updateScrollView(nVal);
	}
	else if (Widget == ScrollBarFloat)
	{
		sint32 nVal = 0;
		CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
		if (pCS != NULL)
			nVal = pCS->getValue();
		double	rVal= nVal/10000.0;
		tryRound(rVal);
		updateScrollView(rVal);
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::backupDB()
{
	if (Type != DataBase) return;
	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (Widget == ColorButton)
	{
		CRGBA col = CRGBA::White;
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
		{
			uint32 intCol = (uint32)pNL->getValue32();
			RTBackupValue = intCol;
		}
	}
	else if (Widget == ScrollBarInt)
	{
		sint32 nVal = 0;
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
		{
			nVal = pNL->getValue32();
		}
		RTBackupValue = nVal;
	}
}


// ***************************************************************************
void CInterfaceDDX::CParam::backupCFG()
{
	if (Type != ConfigFile) return;

	if (Widget == BoolButton)
	{
		bool bVal = ClientCfg.readBool(Link);
		RTBackupValue = bVal;
	}
	else if (Widget == ScrollBarInt)
	{
		sint32 nVal = ClientCfg.readInt(Link);
		RTBackupValue = nVal;
	}
	else if (Widget == ScrollBarFloat)
	{
		double	rVal= ClientCfg.readDouble(Link);
		tryRound(rVal);
		sint32 nVal = (sint32)(10000*rVal);
		RTBackupValue = nVal;
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::restoreDB()
{
	if (Type != DataBase) return;
	if (RealTimeMode==RTModeFalse) return;

	CInterfaceManager *pIM = CInterfaceManager::getInstance();

	if (Widget == ColorButton)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
			pNL->setValue32(RTBackupValue);
	}
	else if (Widget == ScrollBarFloat)
	{
		// TODO
	}
	else if (Widget == ScrollBarInt)
	{
		CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(Link,false);
		if (pNL != NULL)
			pNL->setValue32(RTBackupValue);
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::restoreCFG()
{
	if (Type != ConfigFile) return;
	if (RealTimeMode==RTModeFalse) return;

	if (Widget == BoolButton)
	{
		ClientCfg.writeBool(Link, (RTBackupValue == 0) ? false : true);
	}
	else if (Widget == ScrollBarInt)
	{
		ClientCfg.writeInt(Link, RTBackupValue);
	}
	else if (Widget == ScrollBarFloat)
	{
		double	rVal= RTBackupValue/10000.0;
		tryRound(rVal);
		ClientCfg.writeDouble(Link, rVal);
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::updateScrollView(sint32 nVal)
{
	if(ResultView)
	{
		ResultView->setText(ucstring(toString(nVal)) + ResultUnit);
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::updateScrollView(double nVal)
{
	if(ResultView)
	{
		// allow N digits
		string	fmt= toString("%%.%df", ResultDecimal);
		ResultView->setText(ucstring(toString(fmt.c_str(), nVal)) + ResultUnit);
	}
}

// ***************************************************************************
void CInterfaceDDX::CParam::tryRound(double rVal)
{
	// Round?
	if(RoundMode)
		rVal= floor(rVal+0.5);
}

// ***************************************************************************
void CInterfaceDDX::CParam::restoreCFGPreset(uint presetVal)
{
	if (Type != ConfigFile) return;

	if (Widget == BoolButton)
	{
		// try to get the preset from cfg
		CConfigFile::CVar *varPtr = ClientCfg.ConfigFile.getVarPtr(Link+"_ps"+toString(presetVal));
		if(varPtr)
		{
			bool bVal = varPtr->asInt() != 0;
			CCtrlBaseButton *pBut = dynamic_cast<CCtrlBaseButton*>(Elt.getPtr());
			if (pBut != NULL)
			{
				pBut->setPushed(bVal);
			}
		}
	}
	else if (Widget == ScrollBarInt)
	{
		// try to get the preset from cfg
		CConfigFile::CVar *varPtr = ClientCfg.ConfigFile.getVarPtr(Link+"_ps"+toString(presetVal));
		if(varPtr)
		{
			sint32 nVal = varPtr->asInt();
			CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
			if (pCS != NULL)
			{
				pCS->setValue(nVal);
			}
			updateScrollView(nVal);
		}
	}
	else if (Widget == ScrollBarFloat)
	{
		// try to get the preset from cfg
		CConfigFile::CVar *varPtr = ClientCfg.ConfigFile.getVarPtr(Link+"_ps"+toString(presetVal));
		if(varPtr)
		{
			double rVal = varPtr->asDouble();
			tryRound(rVal);
			sint32 nVal = (sint32)(10000*rVal);
			CCtrlScroll *pCS = dynamic_cast<CCtrlScroll*>(Elt.getPtr());
			if (pCS != NULL)
			{
				pCS->setValue (nVal);
			}
			updateScrollView(rVal);
		}
	}
}

// ***************************************************************************
uint32	CInterfaceDDX::CParam::getPresetPossibleBF()
{
	if (Type != ConfigFile) return 0;
	uint32	ret= 0;

	if (Widget == BoolButton)
	{
		// get the current value from cfg
		bool	curVal= ClientCfg.readBool(Link);

		// compare to each preset from cfg
		for(uint i=0;i<CInterfaceDDX::NumPreset;i++)
		{
			CConfigFile::CVar *varPtr = ClientCfg.ConfigFile.getVarPtr(Link+"_ps"+toString(i));
			if(varPtr)
			{
				bool bVal = varPtr->asInt() != 0;
				// if same value, then the current value is compatible with the preset
				if(bVal == curVal)
					ret|= 1<<i;
			}
		}
	}
	else if (Widget == ScrollBarInt)
	{
		// get the current value from cfg
		sint32	curVal= ClientCfg.readInt(Link);

		// compare to each preset from cfg
		for(uint i=0;i<CInterfaceDDX::NumPreset;i++)
		{
			CConfigFile::CVar *varPtr = ClientCfg.ConfigFile.getVarPtr(Link+"_ps"+toString(i));
			if(varPtr)
			{
				sint32 nVal = varPtr->asInt();
				// if same value, then the current value is compatible with the preset
				if(nVal == curVal)
					ret|= 1<<i;
			}
		}
	}
	else if (Widget == ScrollBarFloat)
	{
		// get the current value from cfg
		double	curVal= ClientCfg.readDouble(Link);
		tryRound(curVal);

		// compare to each preset from cfg
		for(uint i=0;i<CInterfaceDDX::NumPreset;i++)
		{
			CConfigFile::CVar *varPtr = ClientCfg.ConfigFile.getVarPtr(Link+"_ps"+toString(i));
			if(varPtr)
			{
				double rVal = varPtr->asDouble();
				tryRound(rVal);
				// if same value, then the current value is compatible with the preset
				if(rVal == curVal)
					ret|= 1<<i;
			}
		}
	}

	return ret;
}


// ***************************************************************************
// CInterfaceDDX
// ***************************************************************************


// ***************************************************************************
CInterfaceDDX::CInterfaceDDX()
{
	_PresetObs.Owner= this;
	_ApplyButton= NULL;
}

// ***************************************************************************
CInterfaceDDX::~CInterfaceDDX()
{
	// unregister observer
	std::set<CCDBNodeLeaf*>::iterator	it;
	for(it=_PresetNodes.begin();it!=_PresetNodes.end();it++)
	{
		ICDBNode::CTextId	id;
		(*it)->removeObserver(&_PresetObs, id);
	}

	_Parameters.clear();
	_PresetNodes.clear();
}

// ***************************************************************************
bool CInterfaceDDX::parse (xmlNodePtr cur, CInterfaceGroup *parentGroup)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	if (!CInterfaceElement::parse(cur, parentGroup))
		return false;


//		bool ok = true;
	cur = cur->children;
	while (cur)
	{
		// Check that this is a param node
		if ( stricmp((char*)cur->name,"param") == 0 )
		{
			CParam p;
			bool bOK = true;

			CXMLAutoPtr ptrUI((const char*)xmlGetProp (cur, (xmlChar*)"ui"));
			p.Elt = _Parent->getId()+":"+string((const char*)ptrUI);
			if (p.Elt == NULL) bOK = false;

			CXMLAutoPtr ptrType((const char*)xmlGetProp (cur, (xmlChar*)"type"));
			if (!ptrType) bOK = false;
			if (stricmp((const char*)ptrType,"db") == 0)
				p.Type = CParam::DataBase;
			else if (stricmp((const char*)ptrType,"cfg") == 0)
				p.Type = CParam::ConfigFile;

			CXMLAutoPtr ptrWidget((const char*)xmlGetProp (cur, (xmlChar*)"widget"));
			if (!ptrWidget) bOK = false;
			if (stricmp((const char*)ptrWidget,"colbut") == 0)
				p.Widget = CParam::ColorButton;
			else if (stricmp((const char*)ptrWidget,"sbfloat") == 0)
				p.Widget = CParam::ScrollBarFloat;
			else if (stricmp((const char*)ptrWidget,"sbint") == 0)
				p.Widget = CParam::ScrollBarInt;
			else if (stricmp((const char*)ptrWidget,"boolbut") == 0)
				p.Widget = CParam::BoolButton;
			else if (stricmp((const char*)ptrWidget,"sbfloat_round") == 0)
			{
				p.Widget = CParam::ScrollBarFloat;
				p.RoundMode = true;
			}

			CXMLAutoPtr ptrLink((const char*)xmlGetProp (cur, (xmlChar*)"link"));
			if (!ptrLink) bOK = false;
			else p.Link = (const char*)ptrLink;

			if (p.Widget == CParam::ScrollBarInt)
			{
				CXMLAutoPtr ptrMin((const char*)xmlGetProp (cur, (xmlChar*)"min"));
				if (!ptrMin) p.Min = 0;
				else fromString((const char*)ptrMin, p.Min);
				CXMLAutoPtr ptrMax((const char*)xmlGetProp (cur, (xmlChar*)"max"));
				if (!ptrMax) p.Max = 255;
				else fromString((const char*)ptrMax, p.Max);
			}

			CXMLAutoPtr ptrRealtime((const char*)xmlGetProp (cur, (xmlChar*)"realtime"));
			if (!ptrRealtime) p.RealTimeMode = CParam::RTModeFalse;
			else
			{
				if( !strcmp((const char*)ptrRealtime, "true"))
					p.RealTimeMode= CParam::RTModeTrue;
				else if( !strcmp((const char*)ptrRealtime, "end_scroll"))
					p.RealTimeMode = CParam::RTModeEndScroll;
				else
					p.RealTimeMode = CParam::RTModeFalse;
			}

			// try to get the ui_view related
			CXMLAutoPtr ptrUIView((const char*)xmlGetProp (cur, (xmlChar*)"ui_view"));
			if(ptrUIView)
				p.ResultView = _Parent->getId()+":"+string((const char*)ptrUIView);
			CXMLAutoPtr ptrUIUnit((const char*)xmlGetProp (cur, (xmlChar*)"ui_unit"));
			if(ptrUIUnit && ((const char*)ptrUIUnit)[0]!=0 )
				p.ResultUnit = CI18N::get((const char*)ptrUIUnit);
			CXMLAutoPtr ptrUIDecimal((const char*)xmlGetProp (cur, (xmlChar*)"ui_decimal"));
			if(ptrUIDecimal)
				fromString((const char*)ptrUIDecimal, p.ResultDecimal);

			// try to get the preset db entry
			if(p.Type==CParam::ConfigFile && bOK)
			{
				CXMLAutoPtr ptrPreset((const char*)xmlGetProp (cur, (xmlChar*)"preset"));
				if(ptrPreset)
				{
					p.PresetDB = NLGUI::CDBManager::getInstance()->getDbProp((const char*)ptrPreset, false);
					if(p.PresetDB)
					{
						// if not exist in the set, add it and register callback
						if(_PresetNodes.insert(p.PresetDB).second)
						{
							ICDBNode::CTextId	id;
							p.PresetDB->addObserver(&_PresetObs, id);
						}
					}
				}
			}

			// if ok, add this parameter!
			if (bOK)
				_Parameters.push_back(p);
		}
		// apply node?
		else if( stricmp((char*)cur->name,"apply") == 0 )
		{
			CXMLAutoPtr ptrUI((const char*)xmlGetProp (cur, (xmlChar*)"ui"));
			_ApplyButton = dynamic_cast<CCtrlBaseButton*>(CWidgetManager::getInstance()->getElementFromId(_Parent->getId(),(const char*)ptrUI));
		}

		cur = cur->next;
	}

	CDDXManager::getInstance()->add(this);

	return true;
}

// ***************************************************************************
// Initialize parameters from external stuff (database, client.cfg ...)
void CInterfaceDDX::init()
{
	// **** start values from CFG, and backup
	for (uint i = 0; i < _Parameters.size(); ++i)
	{
		CParam &p = _Parameters[i];

		if (p.Type == CParam::DataBase)
		{
			p.DBToWidget();
			p.backupDB();
		}
		else if (p.Type == CParam::ConfigFile)
		{
			p.CFGToWidget();
			p.backupCFG();
		}
	}

	// **** initialize the preset
	resetPreSet();

	// **** Start apply frozen
	if(_ApplyButton)
	{
		_ApplyButton->setFrozen(true);
	}
}

// ***************************************************************************
void CInterfaceDDX::update()
{
	for (uint i = 0; i < _Parameters.size(); ++i)
	{
		CParam &p = _Parameters[i];
		if (p.Type == CParam::DataBase)
		{
			p.WidgetToDB();
			p.backupDB();
		}
		else if (p.Type == CParam::ConfigFile)
		{
			p.WidgetToCFG();
			p.backupCFG();
		}
	}

	// Write the modified client.cfg
	ClientCfg.IsInvalidated = true;

	// **** set apply frozen
	if(_ApplyButton)
	{
		_ApplyButton->setFrozen(true);
	}
}

// ***************************************************************************
void CInterfaceDDX::cancel()
{
	// Update scroll bars that are realtime
	bool bMustSave = false;
	uint i;
	for (i = 0; i < _Parameters.size(); ++i)
	{
		CParam &p = _Parameters[i];

		if (p.RealTimeMode==CParam::RTModeFalse) continue;

		if (p.Type == CParam::DataBase)
		{
			p.restoreDB();
		}
		else if (p.Type == CParam::ConfigFile)
		{
			bMustSave = true;
			p.restoreCFG();
		}
	}

	if (bMustSave)
	{
		ClientCfg.IsInvalidated = true;
	}

	for (i = 0; i < _Parameters.size(); ++i)
	{
		CParam &p = _Parameters[i];

		if (p.Type == CParam::DataBase)
		{
			p.DBToWidget();
		}
		else if (p.Type == CParam::ConfigFile)
		{
			p.CFGToWidget();
		}
	}

	// **** reset the preset
	resetPreSet();

	// **** set apply frozen
	if(_ApplyButton)
	{
		_ApplyButton->setFrozen(true);
	}
}

// ***************************************************************************
void CInterfaceDDX::updateRealtime(CCtrlBase *pSB, bool updateOnScrollEnd)
{
	// Update scroll bars that are realtime
	bool bMustSave = false;
	uint i;
	for (i = 0; i < _Parameters.size(); ++i)
	{
		CParam &p = _Parameters[i];

		if (p.Elt != pSB) continue;

		if( (!updateOnScrollEnd && p.RealTimeMode==CParam::RTModeTrue) ||
			(updateOnScrollEnd && p.RealTimeMode==CParam::RTModeEndScroll) )
		{
			if (p.Type == CParam::DataBase)
			{
				p.WidgetToDB();
			}
			else if (p.Type == CParam::ConfigFile)
			{
				bMustSave = true;
				p.WidgetToCFG();
			}
		}

		// even if not realtime, if has a preset setuped, set it to custom!
		if(p.PresetDB)
		{
			p.PresetDB->setValue32(CustomPreset);
		}

		// even if not realtime, must update text view
		p.WidgetToResultView();
	}

	if (bMustSave)
	{
		ClientCfg.IsInvalidated = true;
	}

	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	for (i = 0; i < _Parameters.size(); ++i)
	{
		CParam &p = _Parameters[i];

		if( (!updateOnScrollEnd && p.RealTimeMode==CParam::RTModeTrue) ||
			(updateOnScrollEnd && p.RealTimeMode==CParam::RTModeEndScroll) )
		{
			if (p.Type == CParam::DataBase)
			{
				p.DBToWidget();
			}
			else if (p.Type == CParam::ConfigFile)
			{
				p.CFGToWidget();
			}
		}
	}

	// **** something changed, allow apply!
	if(_ApplyButton)
	{
		_ApplyButton->setFrozen(false);
	}
}

// ***************************************************************************
void CInterfaceDDX::updateParamPreset(CCDBNodeLeaf *presetChanged)
{
	if(!presetChanged)
		return;
	// if not custom or bad id
	uint	presetVal= presetChanged->getValue32();
	if(presetVal>=NumPreset)
		return;

	// **** check each parameter
	for(uint i=0;i<_Parameters.size();i++)
	{
		CParam	&param= _Parameters[i];

		// if can be set by this preset modified
		if(param.PresetDB == presetChanged)
		{
			// must be a config file param (see parse())
			nlassert(param.Type==CParam::ConfigFile);

			// restore preset
			param.restoreCFGPreset(presetVal);

			// if realTime, then must update CFG
			if (param.RealTimeMode!=CParam::RTModeFalse)
			{
				param.WidgetToCFG();
				ClientCfg.IsInvalidated= true;
			}
		}
	}

	// **** something changed, allow apply!
	if(_ApplyButton)
	{
		_ApplyButton->setFrozen(false);
	}
}

// ***************************************************************************
void CInterfaceDDX::validateApplyButton()
{
	if(_ApplyButton)
	{
		_ApplyButton->setFrozen(false);
	}
}

// ***************************************************************************
void CInterfaceDDX::resetPreSet()
{
	nlassert(NumPreset<32);
	// for each preset, assign a bitfield, where bit==1 <=> preset possible
	std::map<CCDBNodeLeaf*, uint32>		bfPreset;
	std::set<CCDBNodeLeaf*>::iterator	itSet;
	// init as "preset possible"
	for(itSet=_PresetNodes.begin();itSet!=_PresetNodes.end();itSet++)
	{
		bfPreset[*itSet]= (1<<NumPreset)-1;
	}

	// then for each parameter, invalidate preset according to value
	for (uint i = 0; i < _Parameters.size(); ++i)
	{
		CParam &p = _Parameters[i];

		if (p.Type == CParam::ConfigFile && p.PresetDB)
		{
			std::map<CCDBNodeLeaf*, uint32>::iterator	it= bfPreset.find(p.PresetDB);
			if(it!=bfPreset.end())
			{
				it->second&= p.getPresetPossibleBF();
			}
		}
	}

	// for each preset DB, assign the lowest preset possible
	std::map<CCDBNodeLeaf*, uint32>::iterator	itMap;
	for(itMap= bfPreset.begin();itMap!=bfPreset.end();itMap++)
	{
		CCDBNodeLeaf	*node= itMap->first;
		uint32			bf= itMap->second;

		// if no common preset is possible, then assign custom!
		if(bf==0)
		{
			node->setValue32(CustomPreset);
		}
		// else assign the lowest bit
		else
		{
			uint32	presetId= 0;
			while( (bf&(1<<presetId)) == 0)
				presetId++;
			node->setValue32(presetId);
		}
	}
}

// ***************************************************************************
void CInterfaceDDX::CPresetObs::update(ICDBNode* node)
{
	CCDBNodeLeaf	*leaf= dynamic_cast<CCDBNodeLeaf*>(node);
	if(leaf)
		Owner->updateParamPreset(leaf);
}

// ***************************************************************************
// CDDXManager
// ***************************************************************************

// ***************************************************************************
CDDXManager* CDDXManager::_Instance = NULL;

// ***************************************************************************
CDDXManager::CDDXManager()
{
}

// ***************************************************************************
void CDDXManager::releaseInstance()
{
	if( _Instance )
		delete _Instance;
	_Instance = NULL;
}

// ***************************************************************************
void CDDXManager::release()
{
	std::map<std::string, CInterfaceDDX*>::iterator ite = _DDXes.begin();
	while (ite != _DDXes.end())
	{
		delete ite->second;

		ite++;
	}
	_DDXes.clear();
}

// ***************************************************************************
void CDDXManager::add(CInterfaceDDX *pDDX)
{
	_DDXes.insert(pair<string,CInterfaceDDX*>(pDDX->getId(),pDDX));
}

// ***************************************************************************
CInterfaceDDX *CDDXManager::get(const std::string &ddxName)
{
	map<string, CInterfaceDDX*>::iterator it = _DDXes.find(ddxName);
	if (it == _DDXes.end())
	{
		it = _DDXes.find("ui:interface:"+ddxName);
	}
	if (it != _DDXes.end())
	{
		return it->second;
	}
	return NULL;
}

// ***************************************************************************
CInterfaceDDX *CDDXManager::getFromParent(const std::string &ddxParentName)
{
	map<string, CInterfaceDDX*>::iterator it = _DDXes.begin();
	while (it != _DDXes.end())
	{
		string parentName = it->first;
		parentName = parentName.substr(0,parentName.rfind(":"));
		if (parentName == ddxParentName)
			return it->second;
		it++;
	}
	return NULL;
}

// ***************************************************************************
// Update database or config file from the parameters (ddx memory zone)
class CHandlerDDXUpdate : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CDDXManager *pDM = CDDXManager::getInstance();
		CInterfaceDDX *pDDX = pDM->get(sParams);
		if (pDDX != NULL)
			pDDX->update();
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerDDXUpdate, "ddx_update");

// ***************************************************************************
// Update the parameters (ddx mem zone) from database or config file
class CHandlerDDXInit : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CDDXManager *pDM = CDDXManager::getInstance();
		CInterfaceDDX *pDDX = pDM->get(sParams);
		if (pDDX != NULL)
			pDDX->init();
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerDDXInit, "ddx_init");

// ***************************************************************************
// Cancel all realtime values
class CHandlerDDXCancel : public IActionHandler
{
public:
	void execute (CCtrlBase * /* pCaller */, const std::string &sParams)
	{
		CDDXManager *pDM = CDDXManager::getInstance();
		CInterfaceDDX *pDDX = pDM->get(sParams);
		if (pDDX != NULL)
			pDDX->cancel();
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerDDXCancel, "ddx_cancel");

// ***************************************************************************
class CHandlerDDXScroll : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CDDXManager *pDM = CDDXManager::getInstance();
		CInterfaceDDX *pDDX = pDM->get(sParams);
		if (pDDX != NULL)
			pDDX->updateRealtime(pCaller, false);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerDDXScroll, "ddx_scroll");

// ***************************************************************************
class CHandlerDDXScrollEnd : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &sParams)
	{
		CDDXManager *pDM = CDDXManager::getInstance();
		CInterfaceDDX *pDDX = pDM->get(sParams);
		if (pDDX != NULL)
			pDDX->updateRealtime(pCaller, true);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerDDXScrollEnd, "ddx_scroll_end");

// ***************************************************************************
// Automatically search for the ddx and launch an update
class CHandlerDDXColor : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CDDXManager *pDM = CDDXManager::getInstance();
		if (pCaller == NULL) return;
		CInterfaceManager *pIM = CInterfaceManager::getInstance();
		CCtrlBase *pCB = CWidgetManager::getInstance()->getCtrlLaunchingModal();
		// Search for a ddx in the parents
		CInterfaceGroup *pIG = pCB->getParent();
		bool found = false;
		CInterfaceDDX *pDDX = NULL;
		while (!found)
		{
			pDDX = pDM->getFromParent(pIG->getId());
			if (pDDX != NULL) found = true;
			else pIG = pIG->getParent();
			if (pIG == NULL) return;
		}
		if (pDDX != NULL)
			pDDX->updateRealtime(pCB, false);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerDDXColor , "ddx_color");


// ***************************************************************************
// Automatically search for the ddx and launch an update
class CHandlerDDXBoolButton : public IActionHandler
{
public:
	void execute (CCtrlBase *pCaller, const std::string &/* sParams */)
	{
		CDDXManager *pDM = CDDXManager::getInstance();
		if (pCaller == NULL) return;
		// Search for a ddx in the parents
		CInterfaceGroup *pIG = pCaller->getParent();
		bool found = false;
		CInterfaceDDX *pDDX = NULL;
		while (!found)
		{
			pDDX = pDM->getFromParent(pIG->getId());
			if (pDDX != NULL) found = true;
			else pIG = pIG->getParent();
			if (pIG == NULL) return;
		}
		if (pDDX != NULL)
			pDDX->updateRealtime(pCaller, false);
	}
protected:
};
REGISTER_ACTION_HANDLER( CHandlerDDXBoolButton , "ddx_bool_button");

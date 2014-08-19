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
#include "nel/gui/action_handler.h"
#include "nel/gui/group_container_base.h"
#include "nel/gui/interface_property.h"
#include "nel/gui/interface_expr.h"
#include "nel/gui/db_manager.h"
#include "nel/gui/interface_link.h"
#include "nel/gui/widget_manager.h"

using namespace std;
using namespace NLMISC;

namespace NLGUI
{

	// ------------------------------------------------------------------------------------------------
	CAHManager	*CAHManager::_GlobalInstance = NULL;
	bool CAHManager::editorMode = false;
	// ------------------------------------------------------------------------------------------------

	// ------------------------------------------------------------------------------------------------
	static void skipBlankAtStart (string &start)
	{
		while (!start.empty())
		{
			if ((start[0] == ' ' || start[0] == '\t' || start[0] == '\r' || start[0] == '\n'))
				start = start.substr(1,start.size());
			else
				break;
		}
	}

	// ------------------------------------------------------------------------------------------------
	static void skipBlankAtEnd (string &end)
	{
		while (!end.empty())
		{
			if ((end[end.size()-1] == ' ' || end[end.size()-1] == '\t' || end[end.size()-1] == '\r' || end[end.size()-1] == '\n'))
				end = end.substr(0,end.size()-1);
			else
				break;
		}
	}

	// ------------------------------------------------------------------------------------------------
	std::string IActionHandler::getParam (const string &Params, const string &ParamName)
	{
		string allparam = Params;
		skipBlankAtStart (allparam);
		string param = toLower (ParamName);
		while (allparam.size() > 0)
		{
			std::string::size_type e = allparam.find('=');
			if (e == std::string::npos || e == 0) break;
			std::string::size_type p = allparam.find('|');
			string tmp = NLMISC::toLower(allparam.substr(0,e));
			skipBlankAtEnd(tmp);
			if (tmp == param)
			{
				string tmp2 = allparam.substr(e+1,p-e-1);
				skipBlankAtStart(tmp2);
				skipBlankAtEnd(tmp2);
				return tmp2;
			}
			if (p == std::string::npos || p == 0) break;
			allparam = allparam.substr(p+1,allparam.size());
			skipBlankAtStart (allparam);
		}
		return "";
	}

	// ------------------------------------------------------------------------------------------------
	void IActionHandler::getAllParams (const string &Params, vector< pair<string,string> > &vAllParams)
	{
		string allparam = Params;
		skipBlankAtStart (allparam);
		while (allparam.size() > 0)
		{
			std::string::size_type e = allparam.find('=');
			if (e == std::string::npos || e == 0) break;
			std::string::size_type p = allparam.find('|');
			string tmp = NLMISC::toLower(allparam.substr(0,e));
			skipBlankAtEnd(tmp);

			string tmp2 = allparam.substr(e+1,p-e-1);
			skipBlankAtStart(tmp2);
			skipBlankAtEnd(tmp2);

			vAllParams.push_back(pair<string,string>(tmp,tmp2));

			if (p == std::string::npos || p == 0) break;
			allparam = allparam.substr(p+1,allparam.size());
			skipBlankAtStart (allparam);
		}
	}

	void CAHManager::getActionHandlers( std::vector< std::string > &handlers )
	{
		handlers.clear();

		std::map< string, IActionHandler* >::iterator itr = FactoryMap.begin();
		while( itr != FactoryMap.end() )
		{
			handlers.push_back( itr->first );
			++itr;
		}
	}

	// ------------------------------------------------------------------------------------------------
	IActionHandler* CAHManager::getAH(const std::string &name, std::string &params)
	{
		// Special AH form?
		string::size_type	i= name.find(':');
		if(i!=string::npos)
		{
			string	ahName= name.substr(0, i);
			params= name.substr(i+1);
			return getActionHandler(ahName);
		}
		// standalone form
		else
			return getActionHandler(name);
	}

	// ------------------------------------------------------------------------------------------------
	IActionHandler* CAHManager::getAH(const std::string &name, CStringShared &params)
	{
		// Special AH form?
		string::size_type	i= name.find(':');
		if(i!=string::npos)
		{
			string	ahName= name.substr(0, i);
			params= name.substr(i+1);
			return getActionHandler(ahName);
		}
		// standalone form
		else
			return getActionHandler(name);
	}

	// ------------------------------------------------------------------------------------------------
	void CAHManager::parseAH(xmlNodePtr cur, const char *ahId, const char *paramId, IActionHandler *&ahRet, std::string &paramRet)
	{
		CXMLAutoPtr	prop;

		// Read the action handler and any param he defines
		bool	paramSpecifiedInAH= false;
		if(ahId)
		{
			prop = (char*) xmlGetProp( cur, (xmlChar*)ahId );
			if (prop)
			{
				string	ahVal= (const char*)prop;
				if(ahVal.find(':')!= string::npos)
					paramSpecifiedInAH= true;
				ahRet = getAH(ahVal, paramRet);
			}
		}

		// Read parameter (if specified)
		if(paramId)
		{
			prop = (char*) xmlGetProp( cur, (xmlChar*)paramId );
			/* Precise stuff here (for legacy rules):
				If the param is not specified in the ahId, then replace params.
				But if it is specified, don't replace it if the prop is empty!!
				Because this cause problems with template and parameter replacement.
			*/
			if ((const char *)prop && (!paramSpecifiedInAH || strlen((const char*)prop)>0) )
				paramRet = string((const char*)prop);
		}
	}

	void CAHManager::parseAH(xmlNodePtr cur, const char *ahId, const char *paramId, IActionHandler *&ahRet, CStringShared &paramRet)
	{
		CXMLAutoPtr	prop;

		// Read the action handler and any param he defines
		bool	paramSpecifiedInAH= false;
		if(ahId)
		{
			prop = (char*) xmlGetProp( cur, (xmlChar*)ahId );
			if (prop)
			{
				string	ahVal= (const char*)prop;
				if(ahVal.find(':')!= string::npos)
					paramSpecifiedInAH= true;
				ahRet = getAH(ahVal, paramRet);
			}
		}

		// Read parameter (if specified)
		if(paramId)
		{
			prop = (char*) xmlGetProp( cur, (xmlChar*)paramId );
			/* Precise stuff here (for legacy rules):
				If the param is not specified in the ahId, then replace params.
				But if it is specified, don't replace it if the prop is empty!!
				Because this cause problems with template and parameter replacement.
			*/
			if ((const char *)prop && (!paramSpecifiedInAH || strlen((const char*)prop)>0) )
				paramRet = string((const char*)prop);
		}
	}

	// ------------------------------------------------------------------------------------------------
	void CAHManager::runActionHandler (const string &ahCmdLine, CCtrlBase *pCaller, const string &ahUserParams)
	{
		if (ahCmdLine.empty()) return;

		if( editorMode )
			return;

		// Special AH form ("ah:params") ?
		string::size_type i = ahCmdLine.find(':');
		string	ahName;
		string	ahParams;
		if(i!=string::npos)
		{
			ahName= ahCmdLine.substr(0, i);
			ahParams= ahCmdLine.substr(i+1);
		}
		else
		{
			ahName= ahCmdLine;
		}

		// Replace params if defined
		if(!ahUserParams.empty())
			ahParams= ahUserParams;

		// Execute the action handler
		map<string, IActionHandler*>::iterator it = FactoryMap.find (ahName);
		if (it == FactoryMap.end())
		{
			nlwarning ("not found action handler : %s",ahName.c_str());
			return;
		}
		IActionHandler *pAH = it->second;
		pAH->execute (pCaller, ahParams);

		// Quick Help
		const string submitQuickHelp = "submit_quick_help";
		it = FactoryMap.find(submitQuickHelp);
		if(it == FactoryMap.end())
		{
			nlwarning ("not found action handler : %s", submitQuickHelp.c_str());
			return;
		}
		pAH = it->second;
		const std::string event = ahName + ":" + ahParams;
		pAH->execute(NULL, event);
	}

	// ------------------------------------------------------------------------------------------------
	void CAHManager::runActionHandler (IActionHandler *pAH, CCtrlBase *pCaller, const std::string &Params)
	{
		if (pAH == NULL)
		{
			nlwarning ("no action handler");
			return;
		}

		if( editorMode )
			return;

		pAH->execute (pCaller, Params);
		string AHName = CAHManager::getInstance()->getAHName(pAH);

		// Quick Help
		const string submitQuickHelp = "submit_quick_help";
		map<string, IActionHandler*>::iterator it = FactoryMap.find (AHName);
		it = FactoryMap.find(submitQuickHelp);
		if(it == FactoryMap.end())
		{
			nlwarning ("not found action handler : %s", submitQuickHelp.c_str());
			return;
		}
		pAH = it->second;
		const std::string event = AHName + ":" + Params;
		pAH->execute(NULL, event);
	}
	
	void CAHManager::submitEvent( const std::string &evt )
	{
		// Submit the event to the quick help system
		runActionHandler( "submit_quick_help", NULL, evt );
	}

	
	// ------------------------------------------------------------------------------------------------
	class CAHSet : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const string &Params)
		{
			string dblink   = getParam (Params, "dblink");
			string property = getParam (Params, "target_property");
			string propertyToEval = getParam (Params, "target");
			string expr = getParam (Params, "value");
			//nlinfo("set %s %s %s %s", dblink.c_str(), property.c_str(), propertyToEval.c_str(), expr.c_str());
			CInterfaceExprValue value;
			if (CInterfaceExpr::eval(expr, value, NULL))
			{
				if (!dblink.empty())
				{
					// Do not allow Write on SERVER: or LOCAL:
					static const std::string	dbServer= "SERVER:";
					static const std::string	dbLocal= "LOCAL:";
					static const std::string	dbLocalR2= "LOCAL:R2";
					if( (0==dblink.compare(0,    dbServer.size(),    dbServer)) ||
						(0==dblink.compare(0,    dbLocal.size(),    dbLocal))
						)
					{
						if (0!=dblink.compare(0,    dbLocalR2.size(),    dbLocalR2))
						{
							//nlwarning("You are not allowed to write on 'SERVER:...' or 'LOCAL:...' database");
							nlstop;
							return;
						}
					}

					string dblinkeval;
					CInterfaceExpr::unpackDBentry(dblink.c_str(), NULL, dblinkeval);
					if (!value.toInteger())
					{
						nlwarning("<CAHSet:execute> expression doesn't evaluate to a numerical value");
					}
					CInterfaceProperty ip;

					if (!value.toInteger())
					{
						nlwarning("<CAHSet:execute> expression doesn't evaluate to a numerical value");
					}
					if (ip.link (dblinkeval.c_str()))
					{
						ip.setSInt64(value.getInteger());
					}
				}

				if (!propertyToEval.empty())
				{
					CInterfaceExprValue res;
					if (!CInterfaceExpr::eval(propertyToEval, res, NULL)) return;
					res.toString();
					property = res.getString();
				}


				if (!property.empty())
				{
					std::vector<CInterfaceLink::CTargetInfo> targets;
					// find first enclosing group
					CCtrlBase *currCtrl = pCaller;
					CInterfaceGroup *ig = NULL;
					while (currCtrl)
					{
						ig = dynamic_cast<CInterfaceGroup *>(currCtrl);
						if (ig != NULL) break;
						currCtrl = currCtrl->getParent();
					}
					if (ig == NULL)
					{
						string elt = property.substr(0,property.rfind(':'));
						CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(elt);
						ig = dynamic_cast<CInterfaceGroup*>(pIE);
						if (ig == NULL && pIE != NULL)
							ig = pIE->getParent();
					}

					if (ig != NULL)
					{
						CInterfaceLink::splitLinkTargets(property, ig, targets);
						for(uint k = 0; k < targets.size(); ++k)
						{
							if (targets[k].Elem) targets[k].affect(value);
						}
					}
				}
			}
			else
			{
				nlwarning("<CAHSet::execute> Couldn't evaluate expression to affect, expr = %s", expr.c_str());
			}
		}
	};
	REGISTER_ACTION_HANDLER (CAHSet, "set");


	// ------------------------------------------------------------------------------------------------
	class CAHCopy : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase * /* pCaller */, const string &Params)
		{
			string dbdst = getParam (Params, "dbdst");
			string dbsrc = getParam (Params, "dbsrc");
			CCDBNodeBranch *pNBdst = NLGUI::CDBManager::getInstance()->getDbBranch(dbdst);
			CCDBNodeBranch *pNBsrc = NLGUI::CDBManager::getInstance()->getDbBranch(dbsrc);

			// Branch copy

			if ((pNBdst != NULL) && (pNBsrc != NULL))
			{
				//nlinfo("copying from %s to %s",pNBsrc->getName()->c_str(), pNBdst->getName()->c_str());

				// Parse all children of the src branch
				uint nbLeaves = pNBsrc->countLeaves();
				for (uint i = 0; i < nbLeaves; ++i)
				{
					uint count = i;
					CCDBNodeLeaf *pNLsrc = pNBsrc->findLeafAtCount(count);
					// Find its access name
					string sTmp = *pNLsrc->getName();
					CCDBNodeBranch *pParent = pNLsrc->getParent();
					while (pParent != pNBsrc)
					{
						sTmp = *pParent->getName() + ":" + sTmp;
						pParent = pParent->getParent();
					}
					// Find the correspondant node in the dst branch
					CCDBNodeLeaf *pNLdst = dynamic_cast<CCDBNodeLeaf*>(pNBdst->getNode(ICDBNode::CTextId(sTmp)));
					if (pNLdst == NULL)
					{
						nlwarning ("cannot find destination leaf %s",sTmp.c_str());
					}
					else
					{
						pNLdst->setValue64(pNLsrc->getValue64());

						//sint32 nVal = pNLsrc->getValue64();
						//nlinfo("set value %d for node %s", nVal, sTmp.c_str());
					}
				}
				return;
			}

			// Not branch copy so leaf copy

			CInterfaceProperty ipsrc;
			CInterfaceProperty ipdst;
			if (!ipsrc.link (dbsrc.c_str()))
			{
				nlwarning("cannot find leaf %s",dbsrc.c_str());
				return;
			}
			if (!ipdst.link (dbdst.c_str()))
			{
				nlwarning("cannot find leaf %s",dbdst.c_str());
				return;
			}
			// copy
			ipdst.setSInt64 (ipsrc.getSInt64());
		}
	};
	REGISTER_ACTION_HANDLER (CAHCopy, "copy");


	// ------------------------------------------------------------------------------------------------
	class CAHResizeW : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const string &Params)
		{
			string elt = getParam (Params, "elt");

			sint32 value;
			fromString(getParam(Params, "value"), value);

			sint32 limit;
			fromString(getParam(Params, "limit"), limit);

			CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId (pCaller->getId(), elt);
			if (pIE == NULL) return;

			sint32 newW = pIE->getW();
			newW += value;
			if (value < 0)
			{
				if (newW < limit)
					newW = limit;
			}
			else
			{
				if (newW > limit)
					newW = limit;
			}
			pIE->setW (newW);
			pIE->invalidateCoords();
		}
	};
	REGISTER_ACTION_HANDLER (CAHResizeW, "resize_w");

	////////////////////////////////
	// EDITION OF CONTAINER ALPHA //
	////////////////////////////////

	// the container whose alpha is being edited
	static CGroupContainerBase *AlphaChooserTarget = NULL;
	static bool  OldUseGlobalAlpha;
	static uint8 OldContentAlpha;
	static uint8 OldBgAlpha;
	static uint8 OldRolloverAlphaBG;
	static uint8 OldRolloverAlphaContent;

	// observer to change the container alpha
	class CContainerAlphaObserver : public ICDBNode::IPropertyObserver
	{
	public:
		bool		 On;
		enum TTargetAlpha { ContentAlpha = 0, BgAlpha, RolloverAlphaContent, RolloverAlphaBG };
		TTargetAlpha Target;
		virtual void update(ICDBNode *node)
		{
			if (!On) return;
			if (!AlphaChooserTarget) return;
			CCDBNodeLeaf *leaf = safe_cast<CCDBNodeLeaf *>(node);
			switch(Target)
			{
				case ContentAlpha:  AlphaChooserTarget->setContentAlpha((uint8) leaf->getValue32()); break;
				case BgAlpha:	    AlphaChooserTarget->setContainerAlpha((uint8) leaf->getValue32()); break;
				case RolloverAlphaContent: AlphaChooserTarget->setRolloverAlphaContent((uint8) (255 - (uint8) leaf->getValue32())); break;
				case RolloverAlphaBG: AlphaChooserTarget->setRolloverAlphaContainer((uint8) (255 - (uint8) leaf->getValue32())); break;
			}
		}
	};



	// ------------------------------------------------------------------------------------------------
	class CAHChooseUIAlpha : public IActionHandler
	{
	public:
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CGroupContainerBase *gc = NULL;
			CCtrlBase *cb = pCaller;
			while (cb)
			{
				gc = dynamic_cast<CGroupContainerBase*>(cb);
				if (gc) break;
				cb = cb->getParent();
			}
			if (!gc) return;
			AlphaChooserTarget = gc;
			if (!_AlphaObserversAdded)
			{
				_UiVariableBGAlpha = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ALPHA_BG");
				_UiVariableContentAlpha = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ALPHA_CONTENT");
				_UiVariableRolloverAlphaBG = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ALPHA_ROLLOVER_BG");
				_UiVariableRolloverAlphaContent = NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:ALPHA_ROLLOVER_CONTENT");
				ICDBNode::CTextId textIdBGAlpha, textIdContentAlpha, textIdRolloverAlphaBG, textIdRolloverAlphaContent;
				_UiVariableBGAlpha->addObserver(&_BgAlphaObs, textIdBGAlpha);
				_UiVariableContentAlpha->addObserver(&_ContentAlphaObs, textIdContentAlpha);
				_UiVariableRolloverAlphaBG->addObserver(&_RolloverAlphaBGObs, textIdRolloverAlphaBG);
				_UiVariableRolloverAlphaContent->addObserver(&_RolloverAlphaContentObs, textIdRolloverAlphaContent);
				_AlphaObserversAdded = true;
			}
			// disable observers
			_ContentAlphaObs.On = false;
			_BgAlphaObs.On      = false;
			_RolloverAlphaBGObs.On		= false;
			_RolloverAlphaContentObs.On	= false;
			// set alpha of current chosen container
			_UiVariableBGAlpha->setValue32(gc->getContainerAlpha());
			_UiVariableContentAlpha->setValue32(gc->getContentAlpha());
			_UiVariableRolloverAlphaBG->setValue32(255 - gc->getRolloverAlphaContainer());
			_UiVariableRolloverAlphaContent->setValue32(255 - gc->getRolloverAlphaContent());
			// enable observers
			_ContentAlphaObs.On  = true;
			_BgAlphaObs.On       = true;
			_RolloverAlphaBGObs.On		= true;
			_RolloverAlphaContentObs.On	= true;
			// backup current alpha (if the user cancel)
			OldContentAlpha = gc->getContentAlpha();
			OldBgAlpha = gc->getContainerAlpha();
			OldRolloverAlphaBG = gc->getRolloverAlphaContainer();
			OldRolloverAlphaContent = gc->getRolloverAlphaContent();
			OldUseGlobalAlpha = gc->isUsingGlobalAlpha();
			// Enable 'use global alpha' button
			NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER_ALPHA")->setValue64(gc->isUsingGlobalAlpha() ? 0 : 1);
			// show the modal box
			CWidgetManager::getInstance()->enableModalWindow(gc, "ui:interface:define_ui_transparency");

		}


		CAHChooseUIAlpha()
		{
			_UiVariableContentAlpha = NULL;
			_UiVariableBGAlpha = NULL;
			_UiVariableRolloverAlphaBG = NULL;
			_UiVariableRolloverAlphaContent = NULL;
			_AlphaObserversAdded = false;
			_BgAlphaObs.Target = CContainerAlphaObserver::BgAlpha;
			_ContentAlphaObs.Target = CContainerAlphaObserver::ContentAlpha;
			_RolloverAlphaBGObs.Target = CContainerAlphaObserver::RolloverAlphaBG;
			_RolloverAlphaContentObs.Target = CContainerAlphaObserver::RolloverAlphaContent;
		}
	private:
		// instance of observer to copy alpha from db to a container
		CContainerAlphaObserver _ContentAlphaObs;
		CContainerAlphaObserver _BgAlphaObs;
		CContainerAlphaObserver _RolloverAlphaContentObs;
		CContainerAlphaObserver _RolloverAlphaBGObs;
		// flag to know if observer have been added
		bool					_AlphaObserversAdded;
		// db leaf that contains alpha for the current container
		CCDBNodeLeaf			*_UiVariableContentAlpha;
		CCDBNodeLeaf			*_UiVariableBGAlpha;
		CCDBNodeLeaf			*_UiVariableRolloverAlphaContent;
		CCDBNodeLeaf			*_UiVariableRolloverAlphaBG;
	};
	REGISTER_ACTION_HANDLER (CAHChooseUIAlpha, "choose_ui_alpha");

	// ------------------------------------------------------------------------------------------------
	class CAHCancelChooseUIAlpha : public IActionHandler
	{
		virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* Params */)
		{
			if (AlphaChooserTarget)
			{
				AlphaChooserTarget->setUseGlobalAlpha(OldUseGlobalAlpha);
				AlphaChooserTarget->setContainerAlpha(OldBgAlpha);
				AlphaChooserTarget->setContentAlpha(OldContentAlpha);
				AlphaChooserTarget->setRolloverAlphaContainer(OldRolloverAlphaBG);
				AlphaChooserTarget->setRolloverAlphaContent(OldRolloverAlphaContent);
			}
		}
	};
	REGISTER_ACTION_HANDLER (CAHCancelChooseUIAlpha, "cancel_choose_ui_alpha");

	// ------------------------------------------------------------------------------------------------
	class CAHUseGlobalAlphaSettings : public IActionHandler
	{
		virtual void execute (CCtrlBase * /* pCaller */, const std::string &/* Params */)
		{
			if (AlphaChooserTarget)
			{
				AlphaChooserTarget->setUseGlobalAlpha(!AlphaChooserTarget->isUsingGlobalAlpha());
				NLGUI::CDBManager::getInstance()->getDbProp("UI:VARIABLES:USER_ALPHA")->setValue64(AlphaChooserTarget->isUsingGlobalAlpha() ? 0 : 1);
			}
		}
	};
	REGISTER_ACTION_HANDLER (CAHUseGlobalAlphaSettings, "use_global_alpha_settings");


	// ------------------------------------------------------------------------------------------------
	class CAHLockUnlock : public IActionHandler
	{
		virtual void execute (CCtrlBase *pCaller, const std::string &/* Params */)
		{
			CGroupContainerBase *gc = NULL;
			CCtrlBase *cb = pCaller;
			while (cb)
			{
				gc = dynamic_cast< CGroupContainerBase* >(cb);
				if (gc) break;
				cb = cb->getParent();
			}
			if (!gc) return;
			//gc->setMovable(!gc->isMovable());
			gc->setLocked(!gc->isLocked());
		}
	};
	REGISTER_ACTION_HANDLER (CAHLockUnlock, "lock_unlock");

	// ------------------------------------------------------------------------------------------------
	class CAHSetTransparent : public IActionHandler
	{
		virtual void execute (CCtrlBase * /* pCaller */, const std::string &Params)
		{
			CGroupContainerBase *pGC = dynamic_cast< CGroupContainerBase* >(CWidgetManager::getInstance()->getElementFromId(Params));
			if (pGC != NULL)
			{
				pGC->setUseGlobalAlpha(false);
				pGC->setContainerAlpha((uint8) 0);
				pGC->setContentAlpha((uint8) 255);
				pGC->setRolloverAlphaContainer((uint8) 255);
				pGC->setRolloverAlphaContent((uint8) 0);
			}
		}
	};
	REGISTER_ACTION_HANDLER (CAHSetTransparent, "set_transparent");

	// ------------------------------------------------------------------------------------------------
	class CAHSetAlpha : public IActionHandler
	{
		virtual void execute (CCtrlBase * /* pCaller */, const std::string &Params)
		{
			string ui	= getParam (Params, "target");

			uint8 alpha;
			fromString(getParam (Params, "alpha"), alpha);

			CGroupContainerBase *pGC = dynamic_cast<CGroupContainerBase*>(CWidgetManager::getInstance()->getElementFromId(ui));
			if (pGC != NULL)
			{
				pGC->setUseGlobalAlpha(false);
				pGC->setContainerAlpha((uint8) alpha);
				pGC->setContentAlpha((uint8) 255);
				pGC->setRolloverAlphaContainer((uint8) 0);
				pGC->setRolloverAlphaContent((uint8) 0);
			}
		}
	};
	REGISTER_ACTION_HANDLER (CAHSetAlpha, "set_alpha");

	// ------------------------------------------------------------------------------------------------
	class CAHUnlockAllContainer : public IActionHandler
	{
		virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
		{
			const vector<CWidgetManager::SMasterGroup> &rVMG = CWidgetManager::getInstance()->getAllMasterGroup();
			for (uint32 nMasterGroup = 0; nMasterGroup < rVMG.size(); nMasterGroup++)
			{
	//			const CInterfaceManager::SMasterGroup &rMG = rVMG[nMasterGroup];
				CWidgetManager::getInstance()->getMasterGroup((uint8)nMasterGroup).unlockAllContainers();
			}
		}
	};
	REGISTER_ACTION_HANDLER (CAHUnlockAllContainer, "unlock_all_container");

}
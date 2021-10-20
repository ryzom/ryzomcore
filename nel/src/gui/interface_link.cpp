// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2013-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/interface_expr.h"
#include "nel/gui/interface_expr_node.h"
#include "nel/gui/reflect.h"
#include "nel/gui/db_manager.h"
#include "nel/misc/cdb_branch.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/interface_link.h"
#include "nel/gui/interface_element.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "nel/misc/algo.h"

#include <iterator>

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	/////////////
	// GLOBALS //
	/////////////

	CInterfaceLink::TLinkList CInterfaceLink::_LinkList;
	CInterfaceLink::TLinkVect CInterfaceLink::_LinksWithNoTarget;
	//
	CInterfaceLink *CInterfaceLink::_FirstTriggeredLink[2] = { NULL, NULL };
	CInterfaceLink *CInterfaceLink::_LastTriggeredLink[2] = { NULL, NULL };
	CInterfaceLink *CInterfaceLink::_CurrUpdatedLink = NULL;
	CInterfaceLink *CInterfaceLink::_NextUpdatedLink = NULL;
	uint CInterfaceLink::_CurrentTriggeredLinkList = 0;

	bool CInterfaceLink::_UpdateAllLinks = false;


	///////////////
	// FUNCTIONS //
	///////////////

	/** Tool fct : affect a value to a reflected property of an interface element.
	  */
	static bool affect(const CInterfaceExprValue &value, CInterfaceElement &destElem, const CReflectedProperty &property)
	{

		CInterfaceExprValue valueToAffect = value;
		switch(property.Type)
		{
			case CReflectedProperty::Boolean:
				if (valueToAffect.toBool())
				{
					(destElem.*(property.SetMethod.SetBool))(valueToAffect.getBool());
				}
				else
				{

					return false;
				}
			break;
			case CReflectedProperty::SInt32:
				if (valueToAffect.toInteger())
				{
					(destElem.*(property.SetMethod.SetSInt32))((sint32) valueToAffect.getInteger());
				}
				else
				{

					return false;
				}
			break;
			case CReflectedProperty::Float:
				if (valueToAffect.toDouble())
				{
					(destElem.*(property.SetMethod.SetFloat))((float) valueToAffect.getDouble());
				}
				else
				{

					return false;
				}
			break;
			case CReflectedProperty::String:
			case CReflectedProperty::StringRef:
				if (valueToAffect.toString())
				{
					(destElem.*(property.SetMethod.SetString))(valueToAffect.getString());
				}
				else
				{

					return false;
				}
			break;
#ifdef RYZOM_LUA_UCSTRING
			case CReflectedProperty::UCString:
			case CReflectedProperty::UCStringRef:
				if (valueToAffect.toString())
				{
					(destElem.*(property.SetMethod.SetUCString))(ucstring::makeFromUtf8(valueToAffect.getString()));
				}
				else
				{
					return false;
				}
#endif
			break;
			case CReflectedProperty::RGBA:
				if (valueToAffect.toRGBA())
				{
					(destElem.*(property.SetMethod.SetRGBA))(valueToAffect.getRGBA());
				}
				else
				{

					return false;
				}
			break;
			default:
			break;
		}

		return true;
	}

	CInterfaceLink::CInterfaceLinkUpdater::CInterfaceLinkUpdater()
	{
		NLGUI::CDBManager::getInstance()->addFlushObserver( this );
	}

	CInterfaceLink::CInterfaceLinkUpdater::~CInterfaceLinkUpdater()
	{
	}

	void CInterfaceLink::CInterfaceLinkUpdater::onObserverCallFlush()
	{
		CInterfaceLink::updateTrigeredLinks();
	}

	/////////////
	// MEMBERS //
	/////////////

	//===========================================================
	CInterfaceLink::CInterfaceLink() : _On(true)
	{
		// add an entry in the links list
		_LinkList.push_front(this);
		_ListEntry = _LinkList.begin();
		_PrevTriggeredLink[0] = _PrevTriggeredLink[1] = NULL;
		_NextTriggeredLink[0] = _NextTriggeredLink[1] = NULL;
		_Triggered[0] = _Triggered[1] = false;
		_ParseTree = NULL;
		_AHCondParsed = NULL;
	}

	//===========================================================
	CInterfaceLink::~CInterfaceLink()
	{
		if (this == _CurrUpdatedLink)
		{
			_CurrUpdatedLink = NULL;
		}
		if (this == _NextUpdatedLink)
		{
			_NextUpdatedLink = _NextUpdatedLink->_NextTriggeredLink[1 - _CurrentTriggeredLinkList];
		}
		// unlink observer from both update lists
		unlinkFromTriggerList(0);
		unlinkFromTriggerList(1);

		removeObservers(_ObservedNodes);
		// removes from the link list
		_LinkList.erase(_ListEntry);

		delete _ParseTree;
		_ParseTree = NULL;
		delete _AHCondParsed;
		_AHCondParsed = NULL;
	}

	//===========================================================
	bool CInterfaceLink::init(const std::vector<CTargetInfo> &targets, const std::vector<CCDBTargetInfo> &cdbTargets, const std::string &expr, const std::string &actionHandler, const std::string &ahParams, const std::string &ahCond, CInterfaceGroup *parentGroup)
	{
		CInterfaceExprValue result;
		// Build the parse tree
		nlassert(!_ParseTree);
		_ParseTree = CInterfaceExpr::buildExprTree(expr);
		if (!_ParseTree)
		{
			// wrong expression : release the link
			nlwarning("CInterfaceLink::init : can't parse expression %s", expr.c_str());
			_LinksWithNoTarget.push_back(TLinkSmartPtr(this));
			return false;
		}
		// Examine the expression, to see which database nodes it depends on
		_ParseTree->getDepends(_ObservedNodes);
		// must keep observerd node sorted (for std::set_xx ops)
		std::sort(_ObservedNodes.begin(), _ObservedNodes.end());
		if (!targets.empty())
		{
			// retrieve properties from the element
			_Targets.resize(targets.size());
			for (uint k = 0; k < targets.size(); ++k)
			{
				CInterfaceElement *elem = targets[k].Elem;
				if (!elem)
				{
					nlwarning("<CInterfaceLink::init> : Element %d is NULL", k);
					_Targets[k]._InterfaceElement = NULL;
					continue;
				}
				_Targets[k]._Property = elem->getReflectedProperty(targets[k].PropertyName);
				if (!_Targets[k]._Property)
				{
					nlwarning("<CInterfaceLink::init> : Can't retrieve property %s for element %d.", targets[k].PropertyName.c_str(), k);
					_Targets[k]._InterfaceElement = NULL;
					continue;
				}
				_Targets[k]._InterfaceElement = elem;
				elem->addLink(this);
			}
		}
		else
		{
			// There are no target for this link, so, put in a dedicated list to ensure that the link will be destroyed at exit
			_LinksWithNoTarget.push_back(TLinkSmartPtr(this));
		}
		_CDBTargets = cdbTargets;

		// create observers
		createObservers(_ObservedNodes);
		_Expr = expr;
		//
		_ActionHandler = actionHandler;
		_AHParams = ahParams;
		nlassert(!_AHCondParsed);
		_AHCond = ahCond;
		if (!ahCond.empty())
		{
			_AHCondParsed = CInterfaceExpr::buildExprTree(ahCond);
		}
		_AHParent = parentGroup;
		return true;
	}

	//===========================================================
	void CInterfaceLink::uninit()
	{
		for (uint32 i = 0; i < _LinksWithNoTarget.size(); i++)
		{
			CInterfaceLink *pLink = _LinksWithNoTarget[i];
			if (pLink == this)
			{
				_LinksWithNoTarget.erase(_LinksWithNoTarget.begin()+i);
				return;
			}
		}
	}

	//===========================================================
	void CInterfaceLink::update(ICDBNode * /* node */)
	{
		// mark link as triggered
		linkInTriggerList(_CurrentTriggeredLinkList);
	}

	//===========================================================
	void CInterfaceLink::createObservers(const TNodeVect &nodes)
	{
		for(std::vector<ICDBNode *>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			if ((*it)->isLeaf())
			{
				ICDBNode::CTextId textId;
				(*it)->addObserver(this, textId);
			}
			else
			{
				CCDBNodeBranch *br = static_cast<CCDBNodeBranch *>(*it);
				NLGUI::CDBManager::getInstance()->addBranchObserver( br, this );
			}
		}
	}

	//===========================================================
	void CInterfaceLink::removeObservers(const TNodeVect &nodes)
	{
		for(std::vector<ICDBNode *>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			if ((*it)->isLeaf())
			{
				ICDBNode::CTextId textId;
				(*it)->removeObserver(this, textId);
			}
			else
			{
				CCDBNodeBranch *br = static_cast<CCDBNodeBranch *>(*it);
				NLGUI::CDBManager::getInstance()->removeBranchObserver( br, this );
			}
		}
	}

	//===========================================================
	void CInterfaceLink::updateAllLinks()
	{
		nlassert(!_UpdateAllLinks);
		_UpdateAllLinks = true;
		for(TLinkList::iterator it = _LinkList.begin(); it != _LinkList.end(); ++it)
		{
			CInterfaceLink *pLink = *it;
			pLink->update();
		}
		_UpdateAllLinks = false;
	}

	//===========================================================
	void CInterfaceLink::update()
	{
		//_On = false; // prvent recursive calls
		if (!_ParseTree) return;
		static TNodeVect observedNodes; // should not be too big, so ok to keep it static.
		static TNodeVect tmpNodes;
		observedNodes.clear();
		CInterfaceExprValue result;
		_ParseTree->evalWithDepends(result, observedNodes);
		// std::set_xxx require ordered container
		std::sort(observedNodes.begin(), observedNodes.end());
		// we assume that _ObservedNode are also sorted
		// compute new observers
		tmpNodes.clear();
		std::set_difference(observedNodes.begin(), observedNodes.end(), _ObservedNodes.begin(), _ObservedNodes.end(), std::back_inserter(tmpNodes));
		// add these observers
		createObservers(tmpNodes);
		// compute lost observers
		tmpNodes.clear();
		std::set_difference(_ObservedNodes.begin(), _ObservedNodes.end(), observedNodes.begin(), observedNodes.end(), std::back_inserter(tmpNodes));
		// remove these observers
		removeObservers(tmpNodes);
		// subsitute new (sorted) list to current
		_ObservedNodes.swap(observedNodes);
		//
		for (uint k = 0; k < _Targets.size(); ++k)
		{
			CInterfaceElement *elem = _Targets[k]._InterfaceElement;
			const CReflectedProperty    *prop = _Targets[k]._Property;
			if (elem && prop)
			{
				if (!affect(result, *elem, *prop))
				{
					nlwarning("CInterfaceLink::update: Result conversion failed");
				}
			}
		}
		if (_CDBTargets.size())
		{
			CInterfaceExprValue resultCopy = result;
			if (resultCopy.toInteger())
			{
				sint64 resultValue = resultCopy.getInteger();
				for (uint k = 0; k < _CDBTargets.size(); ++k)
				{
					NLMISC::CCDBNodeLeaf *node = _CDBTargets[k].Leaf;
					if (!node) 
					{
						node = _CDBTargets[k].Leaf = NLGUI::CDBManager::getInstance()->getDbProp(_CDBTargets[k].LeafName, false);
					}
					if (node)
					{
						// assuming setvalue64 always works
						node->setValue64(resultValue);
					}
					else
					{
						nlwarning("CInterfaceLink::update: Node does not exist: '%s'", _CDBTargets[k].LeafName.c_str());
					}
				}
			}
			else
			{
				nlwarning("CInterfaceLink::update: Result conversion to db target failed");
			}
		}
		// if there's an action handler, execute it
		if (!_ActionHandler.empty())
		{
			// If there is a condition, test it.
			bool launch = _AHCond.empty();
			if (_AHCondParsed) // todo: maybe makes more sense to make condition also cover target
			{
				CInterfaceExprValue result;
				_AHCondParsed->eval(result);
				launch = result.getBool();
			}
			if (launch)
			{
				CAHManager::getInstance()->runActionHandler(_ActionHandler, _AHParent, _AHParams);
				// do not add any code after this line because this can be deleted !!!!
			}
		}

		//_On = true; // prevent recursive calls
	}

	// predicate to remove an element from a list
	struct CRemoveTargetPred
	{
		CInterfaceElement *Target;
		bool operator()(const CInterfaceLink::CTarget &target) const { return target._InterfaceElement == Target; }
	};

	//===========================================================
	void CInterfaceLink::removeTarget(CInterfaceElement *elem)
	{
		CRemoveTargetPred pred;
		pred.Target = elem;
		_Targets.erase(std::remove_if(_Targets.begin(), _Targets.end(), pred), _Targets.end());
	}

	//===========================================================
	bool CInterfaceLink::CTargetInfo::affect(const CInterfaceExprValue &value)
	{

		if (!Elem)
		{
			nlwarning("<CInterfaceLink::CTargetInfo::affect> : Target element is NULL");

			return false;
		}
		const CReflectedProperty    *property = Elem->getReflectedProperty(PropertyName);
		if (!property)
		{
			nlwarning("<CInterfaceLink::CTargetInfo::affect> : Can't retrieve property %s for element %s.", PropertyName.c_str(), Elem->getId().c_str());

			return false;
		}
		// try to affect the property
		if (!NLGUI::affect(value, *Elem, *property))
		{
			nlwarning("<CInterfaceLink::CTargetInfo::affect> Couldn't convert the value to affect to %s", Elem->getId().c_str());

			return false;
		}

		return true;
	}

	//===========================================================
	void CInterfaceLink::removeAllLinks()
	{
		_LinksWithNoTarget.clear();
		TLinkList::iterator curr = _LinkList.begin();
		TLinkList::iterator nextIt;
		uint k;
		while(curr != _LinkList.end())
		{
			nextIt = curr;
			++ nextIt;
			CInterfaceLink &link = **curr;
			link.checkNbRefs();
			NLMISC::CSmartPtr<CInterfaceLink> linkPtr = *curr; // must keep a smart ptr because of the test in the loop
			for (k = 0; k < link._Targets.size(); ++k)
			{
				if (link._Targets[k]._InterfaceElement)
				{
					nlinfo("InterfaceElement %s was not remove.", link._Targets[k]._InterfaceElement->getId().c_str());
					link._Targets[k]._InterfaceElement->removeLink(&link);
				}
			}
			linkPtr = NULL; // effectively destroy link
			curr = nextIt;
		}
		nlassert(_LinkList.empty());
	}

	// ***************************************************************************
	bool CInterfaceLink::splitLinkTarget(const std::string &target,  CInterfaceGroup *parentGroup, std::string &propertyName, CInterfaceElement *&targetElm)
	{
		// the last token of the target gives the name of the property
		std::string::size_type lastPos = target.find_last_of(':');
		if (lastPos == (target.length() - 1))
		{
			// todo hulud interface syntax error
			nlwarning("The target should at least contains a path and a property as follow 'path:property'");
			return false;
		}
		std::string elmPath;
		std::string elmProp;
		CInterfaceElement *elm = NULL;
		if (parentGroup)
		{
			if (lastPos == std::string::npos)
			{
				elmProp = target;
				elm = parentGroup;
				elmPath = "current";
			}
			else
			{
				elmProp = target.substr(lastPos + 1);
				elmPath = parentGroup->getId() + ":" + target.substr(0, lastPos);
				elm = parentGroup->getElement(elmPath);
			}
		}
		if (!elm)
		{
			// try the absolute adress of the element
			elmPath = target.substr(0, lastPos);
			elm = CWidgetManager::getInstance()->getElementFromId(elmPath);
			elmProp = target.substr(lastPos + 1);
		}

		if (!elm)
		{
			// todo hulud interface syntax error
			nlwarning("<splitLinkTarget> can't find target link %s", elmPath.c_str());
			return false;
		}
		targetElm = elm;
		propertyName = elmProp;
		return true;
	}


	// ***************************************************************************
	bool CInterfaceLink::splitLinkTargets(const std::string &targets, CInterfaceGroup *parentGroup,std::vector<CInterfaceLink::CTargetInfo> &targetsVect)
	{
		std::vector<std::string> targetNames;
		NLMISC::splitString(targets, ",", targetNames);
		targetsVect.clear();
		targetsVect.reserve(targetNames.size());
		bool everythingOk = true;
		for (uint k = 0; k < targetNames.size(); ++k)
		{
			CInterfaceLink::CTargetInfo ti;
			std::string::size_type startPos = targetNames[k].find_first_not_of(" ");
			if(startPos == std::string::npos)
			{
				// todo hulud interface syntax error
				nlwarning("<splitLinkTargets> empty target encountered");
				continue;
			}
			std::string::size_type lastPos = targetNames[k].find_last_not_of(" ");
			if (startPos >= lastPos)
			{
				nlwarning("<splitLinkTargets> empty target encountered");
				continue;
			}

			if (!splitLinkTarget(targetNames[k].substr(startPos, lastPos - startPos+1), parentGroup, ti.PropertyName, ti.Elem))
			{
				// todo hulud interface syntax error
				nlwarning("<splitLinkTargets> Can't get link target");
				everythingOk = false;
				continue;
			}
			targetsVect.push_back(ti);
		}
		return everythingOk;
	}


	// ***************************************************************************
	bool CInterfaceLink::splitLinkTargetsExt(const std::string &targets, CInterfaceGroup *parentGroup,std::vector<CInterfaceLink::CTargetInfo> &targetsVect, std::vector<CInterfaceLink::CCDBTargetInfo> &cdbTargetsVect)
	{
		std::vector<std::string> targetNames;
		NLMISC::splitString(targets, ",", targetNames);
		targetsVect.clear();
		targetsVect.reserve(targetNames.size());
		cdbTargetsVect.clear(); // no reserve because less used
		bool everythingOk = true;
		for (uint k = 0; k < targetNames.size(); ++k)
		{
			std::string::size_type startPos = targetNames[k].find_first_not_of(" ");
			if(startPos == std::string::npos)
			{
				// todo hulud interface syntax error
				nlwarning("<splitLinkTargets> empty target encountered");
				continue;
			}
			std::string::size_type lastPos = targetNames[k].find_last_not_of(" ");
			if (startPos >= (lastPos+1))
			{
				nlwarning("<splitLinkTargets> empty target encountered");
				continue;
			}

			if (targetNames[k][startPos] == '@')
			{
				CInterfaceLink::CCDBTargetInfo ti;
				ti.LeafName = targetNames[k].substr((startPos+1), (lastPos+1) - (startPos+1));
				// Do not allow Write on SERVER: or LOCAL:
				static const std::string	dbServer= "SERVER:";
				static const std::string	dbLocal= "LOCAL:";
				static const std::string	dbLocalR2= "LOCAL:R2";
				if( (0==ti.LeafName.compare(0,    dbServer.size(),    dbServer)) ||
					(0==ti.LeafName.compare(0,    dbLocal.size(),    dbLocal))
					)
				{
					if (0!=ti.LeafName.compare(0,    dbLocalR2.size(),    dbLocalR2))
					{
						//nlwarning("You are not allowed to write on 'SERVER:...' or 'LOCAL:...' database");
						nlstop;
						return false;
					}
				}
				ti.Leaf = NLGUI::CDBManager::getInstance()->getDbProp(ti.LeafName, false);
				cdbTargetsVect.push_back(ti);
			}
			else
			{
				CInterfaceLink::CTargetInfo ti;
				if (!splitLinkTarget(targetNames[k].substr(startPos, lastPos - startPos+1), parentGroup, ti.PropertyName, ti.Elem))
				{
					// todo hulud interface syntax error
					nlwarning("<splitLinkTargets> Can't get link target");
					everythingOk = false;
					continue;
				}
				targetsVect.push_back(ti);
			}
		}
		return everythingOk;
	}


	//===========================================================
	void    CInterfaceLink::checkNbRefs()
	{
		uint numValidPtr = 0;
		for(uint k = 0; k < _Targets.size(); ++k)
		{
			if (_Targets[k]._InterfaceElement != NULL) ++numValidPtr;
		}
		nlassert(numValidPtr == (uint) crefs);
	}


	//===========================================================
	void    CInterfaceLink::setTargetProperty (const std::string &Target, const CInterfaceExprValue &val)
	{
		// Eval target !
		string elt = Target.substr(0,Target.rfind(':'));
		CInterfaceElement *pIE = CWidgetManager::getInstance()->getElementFromId(elt);
		CInterfaceGroup *pIG = dynamic_cast<CInterfaceGroup*>(pIE);
		nlassert(pIE);
		if (pIG == NULL)
			pIG = pIE->getParent();

		if (pIG != NULL)
		{
			std::vector<CTargetInfo> vTargets;
			splitLinkTargets(Target, pIG, vTargets);
			if (!vTargets.empty() && (vTargets[0].Elem))
			{
				vTargets[0].affect(val);
			}
		}
	}

	//-----------------------------------------------
	void CInterfaceLink::linkInTriggerList(uint list)
	{
		nlassert(list < 2);
		if (_Triggered[list]) return; // already inserted in list
		_Triggered[list] = true;
		nlassert(!_PrevTriggeredLink[list]);
		nlassert(!_NextTriggeredLink[list]);
		if (!_FirstTriggeredLink[list])
		{
			_FirstTriggeredLink[list] = _LastTriggeredLink[list] = this;
		}
		else
		{
			nlassert(!_LastTriggeredLink[list]->_NextTriggeredLink[list]);
			_LastTriggeredLink[list]->_NextTriggeredLink[list] = this;
			_PrevTriggeredLink[list] = _LastTriggeredLink[list];
			_LastTriggeredLink[list] = this;
		}
	}

	//-----------------------------------------------
	void CInterfaceLink::unlinkFromTriggerList(uint list)
	{
		nlassert(list < 2);
		if (!_Triggered[list])
		{
			// not linked in this list
			nlassert(_PrevTriggeredLink[list] == NULL);
			nlassert(_NextTriggeredLink[list] == NULL);
			return;
		}
		if (_PrevTriggeredLink[list])
		{
			_PrevTriggeredLink[list]->_NextTriggeredLink[list] = _NextTriggeredLink[list];
		}
		else
		{
			// this was the first node
			_FirstTriggeredLink[list] = _NextTriggeredLink[list];
		}
		if (_NextTriggeredLink[list])
		{
			_NextTriggeredLink[list]->_PrevTriggeredLink[list] = _PrevTriggeredLink[list];
		}
		else
		{
			// this was the last node
			_LastTriggeredLink[list] = _PrevTriggeredLink[list];
		}
		_PrevTriggeredLink[list] = NULL;
		_NextTriggeredLink[list] = NULL;
		_Triggered[list] = false;
	}





	//-----------------------------------------------
	void CInterfaceLink::updateTrigeredLinks()
	{
		static bool called = false;
		nlassert(!called);
		called = true;
		_CurrUpdatedLink = _FirstTriggeredLink[_CurrentTriggeredLinkList];
		while (_CurrUpdatedLink)
		{
			// modified node should now store them in other list when they're modified
			_CurrentTriggeredLinkList = 1 - _CurrentTriggeredLinkList;
			// switch list so that modified node are stored in the other list
			while (_CurrUpdatedLink)
			{
				_NextUpdatedLink = _CurrUpdatedLink->_NextTriggeredLink[1 - _CurrentTriggeredLinkList];
				_CurrUpdatedLink->update();
				if (_CurrUpdatedLink) // this may be modified by the call (if current observer is removed)
				{
					_CurrUpdatedLink->unlinkFromTriggerList(1 - _CurrentTriggeredLinkList);
				}
				_CurrUpdatedLink = _NextUpdatedLink;
			}
			nlassert(_FirstTriggeredLink[1 - _CurrentTriggeredLinkList] == NULL);
			nlassert(_LastTriggeredLink[1 - _CurrentTriggeredLinkList] == NULL);
			// examine other list to see if nodes have been registered
			_CurrUpdatedLink = _FirstTriggeredLink[_CurrentTriggeredLinkList];
		}
		called = false;
	}


}


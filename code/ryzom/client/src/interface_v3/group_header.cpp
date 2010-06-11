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
//
#include "interface_manager.h"
#include "group_container.h"
#include "group_header.h"
#include "lua_ihm.h"
//
#include "nel/misc/smart_ptr.h"


using namespace NLMISC;

//////////////////
// CGroupHeader //
//////////////////

// *****************************************************************************************************************
CGroupHeader::CGroupHeader(const TCtorParam &param) : CGroupList(param), _HeaderMaxSize(32767)
{
}

// *****************************************************************************************************************
void CGroupHeader::enlargeColumns(sint32 margin)
{
	std::vector<CGroupHeaderEntry *> entries;
	getEntries(entries);
	sint32 totalWidth = 0;
	for (uint k = 0; k < entries.size(); ++k)
	{
		CInterfaceGroup *colEnclosing = entries[k]->getTargetColumn();
		if (colEnclosing && !colEnclosing->getGroups().empty())
		{
			CInterfaceGroup *col = colEnclosing->getGroups()[0];
			if (col)
			{
				// enlarge to the max to be able to measure the sub text (they may clamp themselves based
				// on their first non-"child resizing" parent (see CViewText::updateCoords)
				colEnclosing->setW(16384);
				colEnclosing->invalidateCoords();
				colEnclosing->updateCoords();

				// assume that first child is resizing from its children width (either 'child_resize_w=true' or a CGroupList)
				entries[k]->setW(std::max(entries[k]->getMinSize(), col->getW() + margin));
				entries[k]->invalidateCoords();
				totalWidth += entries[k]->getW();
			}
		}
	}
	// if total width bigger than allowed, reduce proportionnally
	if (totalWidth > _HeaderMaxSize)
	{
		while (totalWidth > _HeaderMaxSize)
		{
			bool adjusted = false;
			// stupid algo here, but ponctual ...
			for (uint k = 0; k < entries.size() && totalWidth > _HeaderMaxSize; ++k)
			{
				if (entries[k]->getW() > entries[k]->getMinSize())
				{
					entries[k]->setW(entries[k]->getW() - 1);
					entries[k]->invalidateCoords();
					--totalWidth;
					adjusted = true;
				}
			}
			// if all at min size, just exit ...
			if (!adjusted) break;
		}
	}
	else
	{
		// search first parent that limit size, if it is larger then enlarge to fit size
		CInterfaceGroup *limitingParent = getParent();
		while (limitingParent && (limitingParent->getResizeFromChildW() || dynamic_cast<CGroupList *>(limitingParent)))
		{
			// NB nico : the dynamic_cast for CGroupList is bad!!
			// can't avoid it for now, because, CGroupList implicitly does a "resize from child" in its update coords
			// ...
			limitingParent = limitingParent->getParent();
		}
		if (limitingParent && limitingParent->getWReal() > totalWidth)
		{
			while (limitingParent->getWReal() > totalWidth && totalWidth < _HeaderMaxSize)
			{
				// enlarge to matche parent size
				// stupid algo here, but ponctual ...
				for (uint k = 0; k < entries.size(); ++k)
				{
					entries[k]->setW(entries[k]->getW() + 1);
					entries[k]->invalidateCoords();
					++totalWidth;
					if (limitingParent->getWReal() <= totalWidth || totalWidth >= _HeaderMaxSize) break;
				}
			}
		}
	}
	invalidateCoords();
}

// *****************************************************************************************************************
void CGroupHeader::resizeColumnsAndContainer(sint32 margin)
{
	std::vector<CGroupHeaderEntry *> entries;
	getEntries(entries);
	sint32 totalWidth = 0;
	for (uint k = 0; k < entries.size(); ++k)
	{
		CInterfaceGroup *colEnclosing = entries[k]->getTargetColumn();
		if (colEnclosing && !colEnclosing->getGroups().empty())
		{
			CInterfaceGroup *col = colEnclosing->getGroups()[0];
			if (col)
			{
				// enlarge to the max to be able to measure the sub text (they may clamp themselves based
				// on their first non-"child resizing" parent (see CViewText::updateCoords)
				colEnclosing->setW(16384);
				colEnclosing->invalidateCoords();
				colEnclosing->updateCoords();

				// assume that first child is resizing from its children width (either 'child_resize_w=true' or a CGroupList)
				entries[k]->setW(std::max(entries[k]->getMinSize(), col->getW() + margin));
				entries[k]->invalidateCoords();
				totalWidth += entries[k]->getW();
			}
		}
	}

	// resize W
	if (totalWidth <= _HeaderMaxSize)
	{
		// search first parent that limit size, if it is larger then enlarge to fit size
		CInterfaceGroup *limitingParent = getParent();
		while (limitingParent && (limitingParent->getResizeFromChildW() || dynamic_cast<CGroupList *>(limitingParent)))
		{
			// NB nico : the dynamic_cast for CGroupList is bad!!
			// can't avoid it for now, because, CGroupList implicitly does a "resize from child" in its update coords
			// ...
			limitingParent = limitingParent->getParent();
		}

		getParentContainer()->setW(totalWidth + getParentContainer()->getWReal() - limitingParent->getWReal());
	}

	// resize H
	if(entries.size()>0)
	{
		CInterfaceGroup *colEnclosing = entries[0]->getTargetColumn();
		if (colEnclosing && !colEnclosing->getGroups().empty())
		{
			CInterfaceGroup *col = colEnclosing->getGroups()[0];
			if (col)
			{
				// search first parent that limit size, if it is larger then enlarge to fit size
				CInterfaceGroup *limitingParent = colEnclosing->getParent();
				while (limitingParent && (limitingParent->getResizeFromChildH() || dynamic_cast<CGroupList *>(limitingParent)))
					limitingParent = limitingParent->getParent();

				getParentContainer()->setH(col->getH() + getParentContainer()->getHReal() - limitingParent->getHReal());
			}
		}
	}


	invalidateCoords();
}

// *****************************************************************************************************************
void CGroupHeader::getEntries(std::vector<CGroupHeaderEntry *> &dest)
{
	dest.clear();
	const std::vector<CInterfaceGroup*> &groups = getGroups();
	for (uint k = 0; k < groups.size(); ++k)
	{
		CGroupHeaderEntry *entry = dynamic_cast<CGroupHeaderEntry *>(groups[k]);
		if (entry)
		{
			dest.push_back(entry);
		}
	}
}

// *****************************************************************************************************************
int CGroupHeader::luaEnlargeColumns(CLuaState &ls)
{
	const char *funcName = "enlargeColumns";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
	enlargeColumns((sint32) ls.toNumber(1));
	return 0;
}

// *****************************************************************************************************************
int CGroupHeader::luaResizeColumnsAndContainer(CLuaState &ls)
{
	const char *funcName = "resizeColumnsAndContainer";
	CLuaIHM::checkArgCount(ls, funcName, 1);
	CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
	resizeColumnsAndContainer((sint32) ls.toNumber(1));
	return 0;
}

// *****************************************************************************************************************
bool CGroupHeader::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	if(!CGroupList::parse(cur, parentGroup)) return false;
	CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"header_max_size" ));
	if (prop) fromString((const char*)prop, _HeaderMaxSize);
	return true;
}

/////////////////////////
// CHeaderEntryResizer //
/////////////////////////

class CHeaderEntryResizer : public CCtrlBase
{
public:
	CHeaderEntryResizer(bool rightSide, sint32 wMin) : CCtrlBase(TCtorParam()),
										   _RightSide(rightSide),
										   _Moving(false),
										   _StartX(0),
										   _OffsetX(0),
										   _WMin(wMin)
	{}
	void release()
	{
		CInterfaceManager	*im = CInterfaceManager::getInstance();
		if (im->getCapturePointerLeft() == this)
		{
			_Moving = false;
			im->setCapturePointerLeft(NULL);
		}
	}
	virtual uint		getDeltaDepth() const { return 100; }
	CInterfaceGroup *getTargetGroup()
	{
		if (_RightSide) return _Parent;

		if (getParent()->getParent() == _Parent->getParentPos()) return NULL; // leftmost header
		return dynamic_cast<CInterfaceGroup *>(getParent()->getParentPos());
	}
	bool handleEvent (const CEventDescriptor &event)
	{
		CInterfaceManager	*im = CInterfaceManager::getInstance();
		if (_Parent)
		{
			if (event.getType() == CEventDescriptor::system)
			{
				const CEventDescriptorSystem &eds = (const CEventDescriptorSystem &) event;
				if (eds.getEventTypeExtended() == CEventDescriptorSystem::setfocus)
				{
					const CEventDescriptorSetFocus &edsf = (const CEventDescriptorSetFocus &) eds;
					if (edsf.hasFocus() == false)
					{
						release();
						return CCtrlBase::handleEvent(event);
					}
				}
			}
			if (event.getType() == CEventDescriptor::mouse)
			{
				const CEventDescriptorMouse &eventDesc = (const CEventDescriptorMouse &)event;
				if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftdown)
				{
					if (!this->isIn(eventDesc.getX(), eventDesc.getY())) return false;
					_TargetGroup = getTargetGroup();
					if (!_TargetGroup) return false;
					im->setCapturePointerLeft(this);
					_Moving = true;
					_OffsetX = _TargetGroup->getW() - eventDesc.getX();
					return true;
				}
				if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mouseleftup)
				{
					release();
				}
				if (eventDesc.getEventTypeExtended() == CEventDescriptorMouse::mousemove)
				{
					if (_Moving && im->getCapturePointerLeft() == this)
					{
						if (!_TargetGroup)
						{
							release();
							return false;
						}
						sint32 newW = eventDesc.getX() + _OffsetX;
						// compute width of all header entries but this one
						CGroupHeader *header = dynamic_cast<CGroupHeader *>(getParent()->getParent());
						if (header)
						{
							sint32 w = 0;
							for (uint k = 0; k < header->getNumChildren(); ++k)
							{
								if (header->getChild(k) != _TargetGroup)
								{
									w += header->getChild(k)->getW();
								}
							}
							sint32 excess = w + newW - header->getHeaderMaxSize();
							if (excess)
							{
								// try to diminish the size of all headers starting from the last
								for (sint k = header->getNumChildren() - 1; k >= 0 && excess > 0; --k)
								{
									if (header->getChild(k) == _TargetGroup) break;
									CGroupHeaderEntry *ghe = dynamic_cast<CGroupHeaderEntry *>(header->getChild(k));
									sint32 wGain = std::min(excess, std::max((sint32) 0, ghe->getW() - ghe->getMinSize()));
									if (wGain > 0)
									{
										ghe->setW(ghe->getW() - wGain);
										ghe->invalidateCoords();
										excess -= wGain;
									}
								}
							}
							newW -= std::max((sint32) 0, excess);
						}
						_TargetGroup->setW(std::max(_WMin, newW));
						_TargetGroup->invalidateCoords();
						CGroupHeaderEntry *ghe = dynamic_cast<CGroupHeaderEntry *>((CInterfaceGroup *) _TargetGroup);
						if (ghe)
						{
							ghe->setW(_TargetGroup->getW());
							ghe->invalidateCoords();
							im->runActionHandler(ghe->getAHOnResize(), ghe, ghe->getAHOnResizeParams());
						}
						return true;
					}
					_Moving = false;
				}
			}
		}
		return CCtrlBase::handleEvent(event);
	}
	virtual void draw ()
	{
		// no-op
	}
	virtual bool getMouseOverShape(std::string &texName, uint8 &rot, NLMISC::CRGBA &col)
	{

		if (!getTargetGroup()) return false;
		texName = "curs_resize_LR.tga";
		rot = 0;
		col = CRGBA::White;
		return true;
	}

private:
	NLMISC::CRefPtr<CInterfaceGroup> _TargetGroup; // group for which w is modified
	bool	_RightSide; // right or left side mover ?
	bool	_Moving;
	sint32  _StartX; // value to add to mouse to get local x pos of target group
	sint32  _OffsetX;
	sint32	_WMin;
};

// *****************************************************************************************************************
CGroupHeaderEntry::CGroupHeaderEntry(const TCtorParam &param) : CInterfaceGroup(param)
{
	_MinSize = 4;
}

// *****************************************************************************************************************
bool CGroupHeaderEntry::parse(xmlNodePtr cur, CInterfaceGroup * parentGroup)
{
	if (!CInterfaceGroup::parse(cur, parentGroup)) return false;
	// left mover
	CXMLAutoPtr prop((const char*) xmlGetProp( cur, (xmlChar*)"wmin" ));
	if (prop) fromString((const char*)prop, _MinSize);
	sint32 resizerSize = 4;
	prop = (char*) xmlGetProp( cur, (xmlChar*)"resizer_size" );
	if (prop) fromString((const char*)prop, resizerSize);
	prop = (char*) xmlGetProp(cur, (xmlChar*) "target");
	if (prop) _TargetColumnId = (const char *) prop;

	prop = (char*) xmlGetProp(cur, (xmlChar*) "on_resize");
	if (prop) _AHOnResize = (const char *) prop;
	prop = (char*) xmlGetProp(cur, (xmlChar*) "on_resize_params");
	if (prop) _AHOnResizeParams = (const char *) prop;

	CHeaderEntryResizer *hm = new CHeaderEntryResizer(false, _MinSize);
	addCtrl(hm);
	hm->setW(resizerSize);
	hm->setSizeRef(2);
	hm->setParent(this);
	hm->setParentPosRef(Hotspot_TL);
	hm->setPosRef(Hotspot_TL);
	// right mover
	hm = new CHeaderEntryResizer(true, _MinSize);
	addCtrl(hm);
	hm->setW(resizerSize);
	hm->setSizeRef(2);
	hm->setParent(this);
	hm->setParentPosRef(Hotspot_TR);
	hm->setPosRef(Hotspot_TR);
	//
	return true;
}

// *****************************************************************************************************************
CInterfaceGroup *CGroupHeaderEntry::getTargetColumn() const
{
	CInterfaceManager	*im = CInterfaceManager::getInstance();
	return dynamic_cast<CInterfaceGroup*>(im->getElementFromId(_TargetColumnId));
}

// *****************************************************************************************************************
void	CGroupHeaderEntry::updateCoords()
{
	CInterfaceGroup::updateCoords();
	CInterfaceGroup *targetColumn = getTargetColumn();
	if (targetColumn)
	{
		if (targetColumn->getW() != getW())
		{
			targetColumn->setW(getW());
			targetColumn->invalidateCoords();
		}
	}
}


NLMISC_REGISTER_OBJECT(CViewBase, CGroupHeader, std::string, "header");
NLMISC_REGISTER_OBJECT(CViewBase, CGroupHeaderEntry, std::string, "header_entry");

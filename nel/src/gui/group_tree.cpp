// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/gui/group_tree.h"
#include "nel/gui/interface_element.h"
#include "nel/gui/view_bitmap.h"
#include "nel/gui/view_text.h"
#include "nel/gui/group_container_base.h"
#include "nel/gui/action_handler.h"
#include "nel/gui/lua_ihm.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/i18n.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/gui/widget_manager.h"
#include "nel/gui/view_renderer.h"
#include "nel/gui/view_pointer_base.h"

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{

	// ----------------------------------------------------------------------------
	// SNode
	// ----------------------------------------------------------------------------
	// TestYoyo
	//uint	SNodeCount= 0;
	// ----------------------------------------------------------------------------
	CGroupTree::SNode::SNode()
	{
		Opened = false;
		Father = NULL;
		FontSize = -1;
		YDecal = 0;
		DisplayText = true;
		Template = NULL;
		Show= true;
		NodeAddedCallback = NULL;
		Color = CRGBA::White;
		ParentTree = NULL;
		LastVisibleSon = NULL;
		// TestYoyo
		//nlinfo("SNode(): %8x, c%d", this, SNodeCount++);
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::updateLastVisibleSon()
	{
		LastVisibleSon = NULL;
		if (!Show || !Opened) return;
		for (sint sonIndex = (sint)Children.size() - 1; sonIndex >= 0; -- sonIndex)
		{
			if (Children[sonIndex]->Show)
			{
				LastVisibleSon = Children[sonIndex];
				break;
			}
		}
		for(uint k = 0; k < Children.size(); ++k)
		{
			Children[k]->updateLastVisibleSon();
		}
	}

	// ----------------------------------------------------------------------------
	CGroupTree::SNode::~SNode()
	{
		makeOrphan();
		// IMPORTANT : must delete in reverse order because "makeOrphan" is called when deleting sons, thus changing vector size...
		for (sint i = (sint)Children.size() - 1; i >= 0; --i)
			delete Children[i];
		Children.clear();
		// TestYoyo
		//nlinfo("~SNode(): %8x, c%d", this, --SNodeCount);
	}

	void CGroupTree::SNode::setParentTree(CGroupTree *parent)
	{
		ParentTree = parent;
		for (uint k = 0; k < Children.size(); ++k)
		{
			Children[k]->setParentTree(parent);
		}
	}

	void CGroupTree::SNode::setFather(SNode *father)
	{
		Father = father;
		setParentTree(father ? father->ParentTree : NULL);
	}


	CGroupTree::SNode *CGroupTree::SNode::getNodeFromId(const std::string &id)
	{
		if (Id == id) return this;
		// breadth first
		for (uint k = 0; k < Children.size(); ++k)
		{
			if (Children[k]->Id == id)
			{
				return Children[k];
			}
		}
		for (uint k = 0; k < Children.size(); ++k)
		{
			SNode *found = Children[k]->getNodeFromId(id);
			if (found) return found;
		}
		return NULL;
	}


	void CGroupTree::SNode::makeOrphan()
	{
		if (ParentTree)
		{
			ParentTree->forceRebuild();
			setParentTree(NULL);
		}
		//
		if (Template != NULL)
		{
			if (Template->getParent())
			{
				// don't delete because may want to keep it. NB: deleted by smartptr at dtor
				Template->getParent()->delGroup(Template, true);
			}
		}
		//
		if (Father)
		{
			Father->detachChild(this);
			Father = NULL;
		}
	}

	// ----------------------------------------------------------------------------
	struct CNodeSorter
	{
		bool operator()(const CGroupTree::SNode *lhs, const CGroupTree::SNode *rhs) const
		{
			return lhs->Text < rhs->Text;
		}
	};

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::sort()
	{
		std::sort(Children.begin(), Children.end(), CNodeSorter());
		for(uint k = 0; k < Children.size(); ++k)
		{
			Children[k]->sort();
		}
	}


	// ----------------------------------------------------------------------------
	struct CNodeSorterByBitmap
	{
		bool operator()(const CGroupTree::SNode *lhs, const CGroupTree::SNode *rhs) const
		{
			if (lhs->Bitmap != rhs->Bitmap) return lhs->Bitmap < rhs->Bitmap;
			return lhs->Text < rhs->Text;
		}
	};

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::sortByBitmap()
	{
		std::sort(Children.begin(), Children.end(), CNodeSorterByBitmap());
		for(uint k = 0; k < Children.size(); ++k)
		{
			Children[k]->sortByBitmap();
		}
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::addChild (SNode *pNode)
	{
		if(!pNode) return;
		pNode->makeOrphan();
		Children.push_back(pNode);
		pNode->setFather(this);
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::addChildSorted(SNode *pNode)
	{
		if (!pNode) return;
		pNode->makeOrphan();
		std::vector<SNode*>::iterator it = std::lower_bound(Children.begin(), Children.end(), pNode, CNodeSorter());
		Children.insert(it, pNode);
		pNode->setFather(this);
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::addChildSortedByBitmap(SNode *pNode)
	{
		if (!pNode) return;
		pNode->makeOrphan();
		std::vector<SNode*>::iterator it = std::lower_bound(Children.begin(), Children.end(), pNode, CNodeSorterByBitmap());
		Children.insert(it, pNode);
		pNode->setFather(this);
	}

	// ----------------------------------------------------------------------------
	bool CGroupTree::SNode::isChild(SNode *pNode) const
	{
		return std::find(Children.begin(),  Children.end(),  pNode) != Children.end();
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::detachChild(SNode *pNode)
	{
		nlassert(pNode);
		nlassert(isChild(pNode));
		Children.erase(std::remove(Children.begin(),  Children.end(),  pNode),  Children.end());
		pNode->setFather(NULL);
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::deleteChild(SNode *pNode)
	{
		delete pNode;
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::addChildFront (SNode *pNode)
	{
		if(!pNode) return;
		pNode->makeOrphan();
		Children.insert(Children.begin(),  pNode);
		pNode->setFather(this);
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::addChildAtIndex(SNode *pNode, sint index)
	{
		if(!pNode) return;
		if (index < 0 || index > (sint) Children.size())
		{
			nlwarning("<CGroupTree::SNode::addChildAtIndex> bad index %d (%d elements in the vector)", index, Children.size());
			return;
		}
		pNode->makeOrphan();
		if (pNode->Father)
		{
			pNode->Father->detachChild(pNode);
		}
		Children.insert(Children.begin() + index,  pNode);
		pNode->setFather(this);
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::openAll()
	{
		Opened = true;
		for (uint i = 0; i < Children.size(); ++i)
			Children[i]->openAll();
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::SNode::closeAll()
	{
		Opened = false;
		for (uint i = 0; i < Children.size(); ++i)
			Children[i]->closeAll();
	}

	// ----------------------------------------------------------------------------
	bool CGroupTree::SNode::parse (xmlNodePtr cur,  CGroupTree * parentGroup)
	{
		if (stricmp((char*)cur->name,  "node") == 0)
		{
			CXMLAutoPtr id((const char*) xmlGetProp (cur,   (xmlChar*)"id"));
			if (id)
				Id = (const char*)id;
			else
				Id = toString(parentGroup->getIdNumber());

			CXMLAutoPtr name((const char*) xmlGetProp (cur,   (xmlChar*)"name"));

			if (name)
			{
				const char *ptrName = (const char*)name;
				if (NLMISC::startsWith(ptrName, "ui"))
					Text = CI18N::get(ptrName);
				else
					Text = ptrName;
			}

			CXMLAutoPtr color((const char*) xmlGetProp (cur,   (xmlChar*)"color"));
			if (color)
			{
				Color = convertColor(color);
			}

			CXMLAutoPtr open((const char*) xmlGetProp (cur,   (xmlChar*)"opened"));
			if (open)	Opened = convertBool(open);

			CXMLAutoPtr show((const char*) xmlGetProp (cur,   (xmlChar*)"show"));
			if (open)	Show = convertBool(show);

			CXMLAutoPtr ah((const char*) xmlGetProp (cur,   (xmlChar*)"handler"));
			if (ah)		AHName = (const char*)ah;
			CXMLAutoPtr cond((const char*) xmlGetProp (cur,   (xmlChar*)"cond"));
			if (cond)	AHCond = (const char*)cond;
			CXMLAutoPtr params((const char*) xmlGetProp (cur,   (xmlChar*)"params"));
			if (params)	AHParams = (const char*)params;

			CXMLAutoPtr ahRight((const char*) xmlGetProp (cur,   (xmlChar*)"handler_right"));
			if (ahRight)		AHNameRight = (const char*)ahRight;
			CXMLAutoPtr paramsRight((const char*) xmlGetProp (cur,   (xmlChar*)"params_right"));
			if (paramsRight)	AHParamsRight = (const char*)paramsRight;

			CXMLAutoPtr bitmap((const char*) xmlGetProp (cur,   (xmlChar*)"bitmap"));
			if (bitmap)	Bitmap = (const char*)bitmap;

			FontSize = parentGroup->getFontSize();
			CXMLAutoPtr fs((const char*) xmlGetProp (cur,   (xmlChar*)"fontsize"));
			if (fs)	fromString((const char*)fs, FontSize);

			YDecal = parentGroup->getYDecal();
			CXMLAutoPtr yDecalPtr((const char*) xmlGetProp (cur,   (xmlChar*)"y_decal"));
			if (yDecalPtr)	fromString((const char*)yDecalPtr, YDecal);


			xmlNodePtr child = cur->children;

			while (child != NULL)
			{
				SNode *pNode = new SNode;
				pNode->parse (child,   parentGroup);
				addChild(pNode);
				child = child->next;
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------------
	// CGroupTree
	// ----------------------------------------------------------------------------

	NLMISC_REGISTER_OBJECT(CViewBase, CGroupTree, std::string, "tree");

	// ----------------------------------------------------------------------------
	CGroupTree::CGroupTree(const TCtorParam &param)
	:CInterfaceGroup(param)
	{
		_IdGenerator = 0;
		_XExtend= 0;
		_BmpW = 14;
		_BmpH = 14;
		_FontSize = 12;
		_YDecal = 0;
		_MustRebuild = true;
		_OverColor = CRGBA(255, 255, 255, 128);
		_OverColorBack = CRGBA(64, 64, 64, 255);
		_SelectedNode = NULL;
		_SelectedLine = -1;
		_SelectedColor = CRGBA(255, 128, 128, 128);
		_RootNode = NULL;
		_OverLine = -1;
		_SelectAncestorOnClose= false;
		_NavigateOneBranch= false;

		_ArboOpenFirst= "arbo_open_first.tga";
		_ArboCloseJustOne= "arbo_close_just_one.tga";
		_ArboSonWithoutSon= "arbo_son_without_son.tga";
		_ArboSonLast= "arbo_son_last.tga";
		_ArboSon= "arbo_son.tga";
		_ArboLevel= "arbo_level.tga";

		_RectangleOutlineMode= false;
		_RectangleX= 0;
		_RectangleY= 0;
		_RectangleW= 10;
		_RectangleH= 10;
		_RectangleDeltaRL= 0;

		_AvoidSelectNodeByIdIR= false;
	}

	// ----------------------------------------------------------------------------
	CGroupTree::~CGroupTree()
	{
		removeAll();
		if (_RootNode != NULL) delete _RootNode;
	}

	std::string CGroupTree::getProperty( const std::string &name ) const
	{
		if( name == "col_over" )
		{
			return toString( _OverColor );
		}
		else
		if( name == "col_select" )
		{
			return toString( _SelectedColor );
		}
		else
		if( name == "col_over_back" )
		{
			return toString( _OverColorBack );
		}
		else
		if( name == "fontsize" )
		{
			return toString( _FontSize );
		}
		else
		if( name == "select_ancestor_on_close" )
		{
			return toString( _SelectAncestorOnClose );
		}
		else
		if( name == "navigate_one_branch" )
		{
			return toString( _NavigateOneBranch );
		}
		else
		if( name == "arbo_open_first" )
		{
			return _ArboOpenFirst;
		}
		else
		if( name == "arbo_close_just_one" )
		{
			return _ArboCloseJustOne;
		}
		else
		if( name == "arbo_son_without_son" )
		{
			return _ArboSonWithoutSon;
		}
		else
		if( name == "arbo_son_last" )
		{
			return _ArboSonLast;
		}
		else
		if( name == "arbo_son" )
		{
			return _ArboSon;
		}
		else
		if( name == "arbo_x_extend" )
		{
			return _ArboXExtend;
		}
		else
		if( name == "arbo_level" )
		{
			return _ArboLevel;
		}
		if( name == "rectangle_outline" )
		{
			return toString( _RectangleOutlineMode );
		}
		else
		if( name == "rectangle_x" )
		{
			return toString( _RectangleX );
		}
		else
		if( name == "rectangle_y" )
		{
			return toString( _RectangleY );
		}
		else
		if( name == "rectangle_w" )
		{
			return toString( _RectangleW );
		}
		else
		if( name == "rectangle_h" )
		{
			return toString( _RectangleH );
		}
		else
		if( name == "rectangle_drl" )
		{
			return toString( _RectangleDeltaRL );
		}
		else
			return CInterfaceGroup::getProperty( name );
	}

	void CGroupTree::setProperty( const std::string &name, const std::string &value )
	{
		if( name == "col_over" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_OverColor = c;
			return;
		}
		else
		if( name == "col_select" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_SelectedColor = c;
			return;
		}
		else
		if( name == "col_over_back" )
		{
			CRGBA c;
			if( fromString( value, c ) )
				_OverColorBack = c;
			return;
		}
		else
		if( name == "fontsize" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_FontSize = i;
			return;
		}
		else
		if( name == "select_ancestor_on_close" )
		{
			bool b;
			if( fromString( value, b ) )
				_SelectAncestorOnClose = b;
			return;
		}
		else
		if( name == "navigate_one_branch" )
		{
			bool b;
			if( fromString( value, b ) )
				_NavigateOneBranch = b;
			return;
		}
		else
		if( name == "arbo_open_first" )
		{
			_ArboOpenFirst = value;
			setupArbo();
			return;
		}
		else
		if( name == "arbo_close_just_one" )
		{
			_ArboCloseJustOne = value;
			return;
		}
		else
		if( name == "arbo_son_without_son" )
		{
			_ArboSonWithoutSon = value;
			return;
		}
		else
		if( name == "arbo_son_last" )
		{
			_ArboSonLast = value;
			return;
		}
		else
		if( name == "arbo_son" )
		{
			_ArboSon = value;
			return;
		}
		else
		if( name == "arbo_x_extend" )
		{
			_ArboXExtend = value;
			setupArbo();
			return;
		}
		else
		if( name == "arbo_level" )
		{
			_ArboLevel = value;
			return;
		}
		if( name == "rectangle_outline" )
		{
			bool b;
			if( fromString( value, b ) )
				_RectangleOutlineMode = b;
			return;
		}
		else
		if( name == "rectangle_x" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_RectangleX = i;
			return;
		}
		else
		if( name == "rectangle_y" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_RectangleY = i;
			return;
		}
		else
		if( name == "rectangle_w" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_RectangleW = i;
			return;
		}
		else
		if( name == "rectangle_h" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_RectangleH = i;
			return;
		}
		else
		if( name == "rectangle_drl" )
		{
			sint32 i;
			if( fromString( value, i ) )
				_RectangleDeltaRL = i;
			return;
		}
		else
			CInterfaceGroup::setProperty( name, value );
	}

	xmlNodePtr CGroupTree::serialize( xmlNodePtr parentNode, const char *type ) const
	{
		xmlNodePtr node = CInterfaceGroup::serialize( parentNode, type );
		if( node == NULL )
			return NULL;

		xmlSetProp( node, BAD_CAST "type", BAD_CAST "tree" );
		xmlSetProp( node, BAD_CAST "col_over", BAD_CAST toString( _OverColor ).c_str() );
		xmlSetProp( node, BAD_CAST "col_select", BAD_CAST toString( _SelectedColor ).c_str() );
		xmlSetProp( node, BAD_CAST "col_over_back", BAD_CAST toString( _OverColorBack ).c_str() );
		xmlSetProp( node, BAD_CAST "fontsize", BAD_CAST toString( _FontSize ).c_str() );
		xmlSetProp( node, BAD_CAST "select_ancestor_on_close", BAD_CAST toString( _SelectAncestorOnClose ).c_str() );
		xmlSetProp( node, BAD_CAST "navigate_one_branch", BAD_CAST toString( _NavigateOneBranch ).c_str() );
		xmlSetProp( node, BAD_CAST "arbo_open_first", BAD_CAST _ArboOpenFirst.c_str() );
		xmlSetProp( node, BAD_CAST "arbo_close_just_one", BAD_CAST _ArboCloseJustOne.c_str() );
		xmlSetProp( node, BAD_CAST "arbo_son_without_son", BAD_CAST _ArboSonWithoutSon.c_str() );
		xmlSetProp( node, BAD_CAST "arbo_son_last", BAD_CAST _ArboSonLast.c_str() );
		xmlSetProp( node, BAD_CAST "arbo_son", BAD_CAST _ArboSon.c_str() );
		xmlSetProp( node, BAD_CAST "arbo_x_extend", BAD_CAST _ArboXExtend.c_str() );
		xmlSetProp( node, BAD_CAST "arbo_level", BAD_CAST _ArboLevel.c_str() );
		xmlSetProp( node, BAD_CAST "rectangle_outline", BAD_CAST toString( _RectangleOutlineMode ).c_str() );
		xmlSetProp( node, BAD_CAST "rectangle_x", BAD_CAST toString( _RectangleX ).c_str() );
		xmlSetProp( node, BAD_CAST "rectangle_y", BAD_CAST toString( _RectangleY ).c_str() );
		xmlSetProp( node, BAD_CAST "rectangle_w", BAD_CAST toString( _RectangleW ).c_str() );
		xmlSetProp( node, BAD_CAST "rectangle_h", BAD_CAST toString( _RectangleH ).c_str() );
		xmlSetProp( node, BAD_CAST "rectangle_drl", BAD_CAST toString( _RectangleDeltaRL ).c_str() );

		return node;
	}

	// ----------------------------------------------------------------------------
	bool CGroupTree::parse (xmlNodePtr cur,  CInterfaceGroup * parentGroup)
	{
		if (!CInterfaceGroup::parse(cur,  parentGroup))
			return false;

		CXMLAutoPtr ptr;

		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"col_over");
		if (ptr) _OverColor = convertColor(ptr);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"col_select");
		if (ptr) _SelectedColor = convertColor(ptr);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"col_over_back");
		if (ptr) _OverColorBack = convertColor(ptr);

		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"fontsize");
		if (ptr) fromString((const char*)ptr, _FontSize);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"select_ancestor_on_close");
		if (ptr) _SelectAncestorOnClose = convertBool(ptr);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"navigate_one_branch");
		if (ptr) _NavigateOneBranch	 = convertBool(ptr);

		// read optional arbo bmps
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"arbo_open_first");
		if (ptr) _ArboOpenFirst= (const char*)ptr;
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"arbo_close_just_one");
		if (ptr) _ArboCloseJustOne= (const char*)ptr;
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"arbo_son_without_son");
		if (ptr) _ArboSonWithoutSon= (const char*)ptr;
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"arbo_son_last");
		if (ptr) _ArboSonLast= (const char*)ptr;
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"arbo_son");
		if (ptr) _ArboSon= (const char*)ptr;
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"arbo_level");
		if (ptr) _ArboLevel= (const char*)ptr;
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"arbo_x_extend");
		if (ptr) _ArboXExtend= (const char*)ptr;

		// Rectangle selection style
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"rectangle_outline");
		if (ptr) _RectangleOutlineMode= convertBool(ptr);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"rectangle_x");
		if (ptr) fromString((const char*)ptr, _RectangleX);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"rectangle_y");
		if (ptr) fromString((const char*)ptr, _RectangleY);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"rectangle_w");
		if (ptr) fromString((const char*)ptr, _RectangleW);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"rectangle_h");
		if (ptr) fromString((const char*)ptr, _RectangleH);
		ptr = (char*) xmlGetProp (cur,   (xmlChar*)"rectangle_drl");
		if (ptr) fromString((const char*)ptr, _RectangleDeltaRL);



		_RootNode = new SNode;
	//	bool ok = true;
		cur = cur->children;
		while (cur)
		{
			// Check that this is a camera node
			if ( stricmp((char*)cur->name, "node") == 0 )
			{
				SNode *pNode = new SNode;
				if (!pNode->parse(cur, this))
				{
					delete pNode;
					nlwarning("cannot parse node");
				}
				else
				{
					_RootNode->addChild(pNode);
				}
			}
			cur = cur->next;
		}
		_RootNode->Opened = true;
		_ResizeFromChildW = _ResizeFromChildH = true;

		setupArbo();

		return true;
	}

	void CGroupTree::setupArbo()
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();
		sint32 id = rVR.getTextureIdFromName(_ArboOpenFirst);
		if (id != -1)
			rVR.getTextureSizeFromId(id,  _BmpW,  _BmpH);
		sint32	dummy;
		id = rVR.getTextureIdFromName(_ArboXExtend);
		if (id != -1)
			rVR.getTextureSizeFromId(id,  _XExtend,  dummy);
		else
			// if not found,  reset,  to avoid errors
			_ArboXExtend.clear();
	}

	// ----------------------------------------------------------------------------
	sint32	CGroupTree::getHrcIconXStart(sint32 depth)
	{
		return depth*(_BmpW+_XExtend);
	}

	// ----------------------------------------------------------------------------
	sint32	CGroupTree::getHrcIconXEnd(sint32 depth)
	{
		return depth*(_BmpW+_XExtend) + _BmpW;
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::checkCoords()
	{
		CInterfaceGroup::checkCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::updateCoords()
	{
		if (_MustRebuild)
			rebuild();
		CInterfaceGroup::updateCoords();
	}

	// ----------------------------------------------------------------------------
	void	CGroupTree::drawSelection(sint x,  sint y,  sint w,  CRGBA col)
	{
		CViewRenderer &rVR = *CViewRenderer::getInstance();

		if(!_RectangleOutlineMode)
		{
			rVR.drawRotFlipBitmap (_RenderLayer,  _XReal+_OffsetX+x,  _YReal+_OffsetY+y,
								   w,  _BmpH,  0,  false,  rVR.getBlankTextureId(),
								   col );
		}
		else
		{
			// draw the outline
			x+= _XReal+_OffsetX+_RectangleX;
			y+= _YReal+_OffsetY+_RectangleY;
			w= _RectangleW;
			sint32	h= _RectangleH;
			sint32	rl= _RenderLayer + _RectangleDeltaRL;

			rVR.drawRotFlipBitmap (rl,  x,  y,  1,  h,  0,  false,  rVR.getBlankTextureId(),  col );
			rVR.drawRotFlipBitmap (rl,  x+w-1,  y,  1,  h,  0,  false,  rVR.getBlankTextureId(),  col );
			rVR.drawRotFlipBitmap (rl,  x,  y,  w,  1,  0,  false,  rVR.getBlankTextureId(),  col );
			rVR.drawRotFlipBitmap (rl,  x,  y+h-1,  w,  1,  0,  false,  rVR.getBlankTextureId(),  col );
		}
	}

	// ----------------------------------------------------------------------------
	CGroupTree::SNode *CGroupTree::getNodeUnderMouse() const
	{
		if (_OverLine == -1) return NULL;
		return _Lines[_OverLine].Node;
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::draw()
	{
		// get the clip area
		sint32 clipx, clipy, clipw, cliph;
		getClip(clipx, clipy, clipw, cliph);

		// change the over
		bool bDisplayOver = true;

		if (!CWidgetManager::getInstance()->isMouseHandlingEnabled())
		{
			bDisplayOver = false;
		}
		else
		if (CWidgetManager::getInstance()->getModalWindow() == NULL)
		{
			sint32 x = CWidgetManager::getInstance()->getPointer()->getX();
			sint32 y = CWidgetManager::getInstance()->getPointer()->getY();

			CInterfaceGroup	*pIG = CWidgetManager::getInstance()->getWindowUnder(x, y);
			CInterfaceGroup	*pParent = this;
			bool bFound = false;
			while (pParent != NULL)
			{
				if (pParent == pIG)
				{
					bFound = true;
					break;
				}
				pParent = pParent->getParent();
			}

			// if the mouse is not in the clipped area
			if ((x < clipx) ||
				(x > (clipx + clipw)) ||
				(y < clipy) ||
				(y > (clipy + cliph)) || !bFound)
			{
				_OverLine = -1;
			}
			else
			{
				x = x - _OffsetX;
				y = y - _OffsetY;
				for (sint32 i = 0; i < (sint32)_Lines.size(); ++i)
				{
					if ((y >= (_YReal+(sint32)(_Lines.size()-i-1)*_BmpH)) &&
						(y < (_YReal+(sint32)(_Lines.size()-i)*_BmpH)))
					{
						_OverLine = i;
					}
				}
				if (_OverLine != -1)
				{
					if (x < _XReal + getHrcIconXEnd(_Lines[_OverLine].Depth))
						bDisplayOver = false;
				}
			}
		}

		// some over to display?
		if ((_OverLine != -1) && bDisplayOver)
		{
			// Find the first container
			CInterfaceGroup *pIG = _Parent;
			CGroupContainerBase *pGC = dynamic_cast<CGroupContainerBase*>(pIG);
			while (pIG != NULL)
			{
				pIG = pIG->getParent();
				if (pIG == NULL) break;
				if (dynamic_cast<CGroupContainerBase*>(pIG) != NULL)
					pGC = dynamic_cast<CGroupContainerBase*>(pIG);
			}

			// avoid if window grayed
			if (pGC != NULL)
			{
				if (pGC->isGrayed())
					bDisplayOver = false;
			}

			// Has to display the over?
			if (bDisplayOver)
			{
				// !NULL if the text over must displayed across all windows
				CViewText	*viewTextExtend= NULL;

				// If the line is a simple Text line (not template)
				if(_Lines[_OverLine].Node && _Lines[_OverLine].Node->DisplayText)
				{
					// Get the view text
					viewTextExtend= safe_cast<CViewText*>(_Lines[_OverLine].TextOrTemplate);
					// If this viewText is not too big,  no need
					if(viewTextExtend->getXReal() + viewTextExtend->getWReal() <= (clipx+clipw) )
						viewTextExtend= NULL;
				}

				// draw simple over
				if(!viewTextExtend)
				{
					CRGBA col = _OverColor;
					if(getModulateGlobalColor())
					{
						col.modulateFromColor (_OverColor,  CWidgetManager::getInstance()->getGlobalColorForContent());
					}
					else
					{
						col= _OverColor;
						col.A = (uint8)(((sint32)col.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
					}

					drawSelection( getHrcIconXEnd(_Lines[_OverLine].Depth + _Lines[_OverLine].getNumAdditionnalBitmap()),  ((sint)_Lines.size()-_OverLine-1)*_BmpH,
						_WReal-getHrcIconXEnd(_Lines[_OverLine].Depth + _Lines[_OverLine].getNumAdditionnalBitmap()),  col);
				}
				// Draw extended over
				else
				{
					CRGBA col = _OverColorBack;
					// must add the selection color
					if(_SelectedLine == _OverLine)
					{
						// simulate alpha blend of the selection bitmap
						CRGBA	sel= _SelectedColor;
						sel.A= (uint8)((sel.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
						col.blendFromuiRGBOnly(col,  sel,  sel.A);
					}

					// will be drawn over all the interface
					CWidgetManager::getInstance()->setOverExtendViewText(viewTextExtend,  col);
				}
			}
		}

		// some selection to display
		if (_SelectedLine != -1)
		{
			CRGBA col = _SelectedColor;
			if(getModulateGlobalColor())
			{
				col.modulateFromColor (_SelectedColor,  CWidgetManager::getInstance()->getGlobalColorForContent());
			}
			else
			{
				col= _SelectedColor;
				col.A = (uint8)(((sint32)col.A*((sint32)CWidgetManager::getInstance()->getGlobalColorForContent().A+1))>>8);
			}

			drawSelection( getHrcIconXEnd(_Lines[_SelectedLine].Depth + _Lines[_SelectedLine].getNumAdditionnalBitmap()),  ((sint)_Lines.size()-_SelectedLine-1)*_BmpH,
										_WReal-getHrcIconXEnd(_Lines[_SelectedLine].Depth + _Lines[_SelectedLine].getNumAdditionnalBitmap()),  col );
		}

		CInterfaceGroup::draw();
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::selectLine(uint line,  bool runAH /*= true*/)
	{
		if(line>=_Lines.size())
			return;

		if (!_Lines[line].Node)
		{
			// just deleted : must wait next draw to know the new line under mouse
			return;
		}
		if (!_Lines[line].Node->AHName.empty() && runAH)
		{
			_CancelNextSelectLine = false;
			/*
			CAHManager::getInstance()->runActionHandler (	_Lines[line].Node->AHName,
				this,
				_Lines[line].Node->AHParams );
			*/
			if (!_CancelNextSelectLine)
			{
				_SelectedLine = line;
				_SelectedNode = _Lines[line].Node;
			}
			CAHManager::getInstance()->runActionHandler (	_Lines[line].Node->AHName,
				this,
				_Lines[line].Node->AHParams );
			_CancelNextSelectLine = false;
		}
	}

	// ----------------------------------------------------------------------------
	bool CGroupTree::rightButton(uint line)
	{
		if(line>=_Lines.size())
			return false;

		if (!_Lines[_OverLine].Node || _Lines[_OverLine].Node->AHNameRight.empty())
			return false;

		if (line != (uint) _SelectedLine) selectLine(line,  false);

		CAHManager::getInstance()->runActionHandler (	_Lines[line].Node->AHNameRight,
				this,
				_Lines[line].Node->AHParamsRight );
		return true;
	}



	// ----------------------------------------------------------------------------
	const std::string	&CGroupTree::getSelectedNodeId() const
	{
		if(_SelectedLine>=0 && _SelectedLine<(sint)_Lines.size() && _Lines[_SelectedLine].Node)
		{
			return _Lines[_SelectedLine].Node->Id;
		}
		else
		{
			static string empty;
			return empty;
		}
	}

	// ----------------------------------------------------------------------------
	bool CGroupTree::handleEvent (const NLGUI::CEventDescriptor& event)
	{
		if (!_Active) return false;
		if (CInterfaceGroup::handleEvent(event)) return true;
		// The line must be over (pre-selected)
		if (event.getType() == NLGUI::CEventDescriptor::mouse && _OverLine>=0)
		{
			const NLGUI::CEventDescriptorMouse &eventDesc = (const NLGUI::CEventDescriptorMouse &)event;

			if (!isIn(eventDesc.getX(),  eventDesc.getY()))
				return false;

			sint32 x = eventDesc.getX() - _OffsetX;
	//		sint32 y = eventDesc.getY() - _OffsetY;
			bool bText = false;
			if (x >= (_XReal+getHrcIconXEnd(_Lines[_OverLine].Depth + _Lines[_OverLine].getNumAdditionnalBitmap())))
				bText = true;
			bool bIcon = false;
			if ((x > (_XReal+getHrcIconXStart(_Lines[_OverLine].Depth)-_XExtend)) &&
				(x < (_XReal+getHrcIconXEnd(_Lines[_OverLine].Depth + _Lines[_OverLine].getNumAdditionnalBitmap()))))
				bIcon = true;

			if (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown)
			{
				if (bText)
				{
					return rightButton(_OverLine != -1 ? _OverLine : _SelectedLine);
				}
			}

			bool toggleOne = (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouseleftdown);
			bool toggleAll = (eventDesc.getEventTypeExtended() == NLGUI::CEventDescriptorMouse::mouserightdown);
			if (toggleOne || toggleAll)
			{
				// line selection
				if (bText)
				{
					selectLine(_OverLine);
				}
				// close of node
				if (bIcon)
				{
					SNode	*changedNode= _Lines[_OverLine].Node;

					if (changedNode)
					{
						// if "SelectAncestorOnClose" feature wanted,  if it was opened before,  and if some node selected
						if(_SelectAncestorOnClose && changedNode->Opened && _SelectedNode)
						{
							// check that the selected node is a son of the closing node
							SNode	*parent= _SelectedNode->Father;
							while(parent)
							{
								if(parent==changedNode)
								{
									// Then change selection to this parent first
									selectLine(_OverLine);
									break;
								}
								parent= parent->Father;
							}
						}

						// standard hrc
						if(!_NavigateOneBranch)
						{
							// open/close the node
							changedNode->Opened = !changedNode->Opened;
							if (toggleAll)
							{
								if (changedNode->Opened)
									changedNode->openAll();
								else
									changedNode->closeAll();
							}
						}
						// else must close all necessary nodes.
						else
						{
							if(changedNode->Opened)
								changedNode->closeAll();
							else
							{
								// must closeAll all his brothers first.
								if(changedNode->Father)
								{
									changedNode->Father->closeAll();
									changedNode->Father->Opened= true;
								}
								changedNode->Opened= true;
							}
						}

						CAHManager::getInstance()->runActionHandler(changedNode->AHNameClose, this, changedNode->AHParamsClose);
					}
					forceRebuild();
				}
				return true;
			}
		}
		return false;
	}


	// ----------------------------------------------------------------------------
	void CGroupTree::unselect()
	{
		_SelectedLine = -1;
		_SelectedNode = NULL;
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::reset()
	{
		unselect();
		if (_RootNode)
		{
			_RootNode->closeAll();
			_RootNode->Opened = true;
		}
		forceRebuild();
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::forceRebuild()
	{
		_MustRebuild = true;
		invalidateCoords();
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::rebuild()
	{
		// Remove all
		removeAll();

		if (_RootNode)
			_RootNode->Opened = true;

		// Rebuild all depending on the logic
		if(_RootNode)
			addTextLine (0,  _RootNode);

		// Reformating
		sint32 sizeH = (sint32)_Lines.size()*_BmpH;
		for (uint i = 0; i < _Lines.size(); ++i)
			_Lines[i].TextOrTemplate->setY (_Lines[i].Node->YDecal + sizeH - ((1+_Lines[i].TextOrTemplate->getY())*_BmpH));
		// Add the hierarchy bitmaps
		addHierarchyBitmaps();
		// Find if we can display selection
		if (_SelectedNode != NULL)
		{
			_SelectedLine = -1;
			for (uint i = 0; i < _Lines.size(); ++i)
				if (_Lines[i].Node == _SelectedNode)
				{
					_SelectedLine = i;
					break;
				}
		}

		// Ok no more need to rebuild all this
		_MustRebuild = false;
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::setRootNode (SNode *pNewRoot)
	{
		// reset selection
		_SelectedLine= -1;
		_SelectedNode= NULL;

		CRefPtr<SNode> refPtrNewRoot = pNewRoot;
		// clear old
		delete _RootNode;

		// If the node was deleted
		if (pNewRoot && !refPtrNewRoot)
		{
			// NB nico : if anyone need that a day, please feel free to modify ...
			nlwarning("Trying to set a node that is part of the tree as root node (not supported yet ...)");
		}

		// set new (may be NULL)
		_RootNode = pNewRoot;
		if(pNewRoot)
			pNewRoot->setParentTree(this);
		rebuild();
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::removeAll()
	{
		// Remove (but not delete) the groups template if any
		for(uint i=0;i<_Lines.size();i++)
		{
			if (_Lines[i].Node)
			{
				if(_Lines[i].Node->Template)
				{
					delGroup(_Lines[i].Node->Template,  true);
				}
			}
		}

		// Clear all lines,  but don't delete templates
		_Lines.clear();

		// Delete all BmpViews and/or all text views
		clearViews();
	}



	// ----------------------------------------------------------------------------
	void CGroupTree::addTextLine (uint8 nDepth,  CGroupTree::SNode *pNode)
	{
		pNode->setParentTree(this);
		if (nDepth > 0)
		{
			SLine line;
			line.Depth = nDepth-1;
			line.Node = pNode;
			if (pNode->DisplayText)
			{
				CViewText *pVT = new CViewText(TCtorParam());
				line.TextOrTemplate = pVT;
				pVT->setId("t"+toString(_Lines.size()));
				pVT->setText(pNode->Text);
				pVT->setColor(pNode->Color);
				if(pNode->FontSize==-1)
					pVT->setFontSize(_FontSize);
				else
					pVT->setFontSize(pNode->FontSize);
			}
			else
			{
				line.TextOrTemplate = pNode->Template;
			}
			line.TextOrTemplate->setPosRef (Hotspot_BL);
			line.TextOrTemplate->setParentPosRef (Hotspot_BL);
			line.TextOrTemplate->setParent (this);
			line.TextOrTemplate->setParentPos (NULL);
			line.TextOrTemplate->setX (getHrcIconXEnd(nDepth-1 + line.getNumAdditionnalBitmap()));
			line.TextOrTemplate->setY ((sint32)_Lines.size());
			line.TextOrTemplate->setModulateGlobalColor(this->getModulateGlobalColor());
			if (pNode->DisplayText)
				addView (line.TextOrTemplate);
			else
				addGroup ((CInterfaceGroup*)line.TextOrTemplate);
			if (pNode->NodeAddedCallback)
			{
				pNode->NodeAddedCallback->nodeAdded(pNode,  line.TextOrTemplate);
			}
			_Lines.push_back(line);
		}

		// recurs
		if (pNode->Opened)
		{
			// **** standard hierarchy display,  or if root
			if(!_NavigateOneBranch || nDepth==0)
			{
				for (uint i = 0; i < pNode->Children.size(); ++i)
				{
					// add the branch only if want to show it
					if(pNode->Children[i]->Show)
						addTextLine (nDepth+1,  pNode->Children[i]);
				}
			}
			// **** display only the branch navigated
			else
			{
				// find the first child opened
				sint	childOpen= -1;
				for (uint i = 0; i < pNode->Children.size(); ++i)
				{
					// don't take hid ones
					if(pNode->Children[i]->Show && pNode->Children[i]->Opened)
					{
						childOpen= i;
						break;
					}
				}

				// If some chidl opened,  add just this line
				if(childOpen>=0)
				{
					addTextLine (nDepth+1,  pNode->Children[childOpen]);
				}
				// else add all closed,  but showable lines
				else
				{
					for (uint i = 0; i < pNode->Children.size(); ++i)
					{
						if(pNode->Children[i]->Show)
							addTextLine (nDepth+1,  pNode->Children[i]);
					}
				}
			}
		}
	}

	// ----------------------------------------------------------------------------
	CViewBitmap	*CGroupTree::createViewBitmap(uint line, const std::string &idPrefix,  const std::string &texture)
	{
		CViewBitmap *pVB = new CViewBitmap(TCtorParam());
		pVB->setId(idPrefix+toString(_Lines.size())+"_"+toString(_Lines[line].Bmps.size()));
		pVB->setParent (this);
		pVB->setParentPos (NULL);
		pVB->setModulateGlobalColor(this->getModulateGlobalColor());
		pVB->setTexture(texture);
		return pVB;
	}

	// ----------------------------------------------------------------------------
	void CGroupTree::addHierarchyBitmaps ()
	{
		if (_RootNode) _RootNode->updateLastVisibleSon();
		for (uint nLayer = 0; nLayer < 256; nLayer++)
		{
			sint32 nCurRootLine = -1;
			bool bCurRootLineLast = false;
			bool bCurRootLineLastChild = false;
			for (uint nLine = 0; nLine < _Lines.size(); nLine++)
			if (nLayer <= _Lines[nLine].Depth)
			{
				// A Bitmap must be created
				CViewBitmap *pVB = createViewBitmap(nLine, "t",  "blank.tga");
				pVB->setX (getHrcIconXStart(nLayer));
				pVB->setY ((sint32)_Lines.size()*_BmpH - ((1+nLine)*_BmpH));

				bool bAddBitmap = true;
				bool bAddXExtendBitmap =  false;
				// Choose a bitmap
				// Are we on the last depth in the line ?
				if (_Lines[nLine].Depth == nLayer)
				{
					nCurRootLine = nLine;
					if (_Lines[nLine].Node == _Lines[nCurRootLine].Node->Father->LastVisibleSon)
						bCurRootLineLast = true;
					else
						bCurRootLineLast = false;
					bCurRootLineLastChild = false;

					// do i have some child shown?
					bool	haveSomeVisibleChild= false;
					for(uint k=0;k<_Lines[nLine].Node->Children.size();k++)
					{
						if(_Lines[nLine].Node->Children[k]->Show)
						{
							haveSomeVisibleChild= true;
							break;
						}
					}

					// if so
					if (haveSomeVisibleChild)
					{
						// Yes am I opened ?
						if (_Lines[nLine].Node->Opened)
						{
							pVB->setTexture(_ArboOpenFirst);
						}
						else
						{
							// No I am closed
							pVB->setTexture(_ArboCloseJustOne);
						}
					}
					else
					{
						/*
						// No child
						// If there's a bitmap on this line , left an empty bitmap
						if (!_Lines[nLine].Node->Bitmap.empty())
						{
							pVB->setTexture("blank.tga");		// create a transparent bitmap to have correct "child_resize_w"
							pVB->setColor(CRGBA(0, 0, 0, 0));
						}
						else
						{
							pVB->setTexture(_ArboSonWithoutSon);
						}
						*/
						pVB->setTexture(_ArboSonWithoutSon);
					}

					// if not the root line,  must add Extend Bitmap
					if(nLayer)
						bAddXExtendBitmap= true;
				}
				else
				{
					// No we are before the last depth,  Do we have any current root ?
					if (nCurRootLine != -1)
					{
						// Yes,  do the current line is child of current root line ?
						bool bFound = false;
						for (uint i = 0; i < _Lines[nCurRootLine].Node->Children.size(); ++i)
							if (_Lines[nLine].Node == _Lines[nCurRootLine].Node->Children[i])
							{
								bFound = true;
								break;
							}
						if (bFound)
						{
							// is it the last child ?
							bool	lastSonDisplay= _Lines[nLine].Node == _Lines[nCurRootLine].Node->LastVisibleSon;


							// Special for _NavigateOneBranch mode
							if(_NavigateOneBranch)
							{
								// if node opened,  then his brother are hid! => he behaves like a "last son"
								if(_Lines[nLine].Node->Opened)
									lastSonDisplay= true;
							}

							// if must display like last child
							if (lastSonDisplay)
							{
								// Yes this is the last child
								pVB->setTexture(_ArboSonLast);
								bCurRootLineLastChild = true;
							}
							else
							{
								// No so we have brothers
								pVB->setTexture(_ArboSon);
							}
						}
						else
						{
							// Not found,  display a line
							pVB->setTexture(_ArboLevel);

							// We have to not display a line if we have passed the last child of this root
							// We never have to display a line also if we are in _NavigateOneBranch mode
							if (bCurRootLineLastChild || _NavigateOneBranch)
								bAddBitmap = false;
						}
					}
				}

				// Add the bitmap
				if (bAddBitmap)
				{
					addView (pVB);
					_Lines[nLine].Bmps.push_back(pVB);

					// if must add the special extend bitmap,  and if exist
					if(bAddXExtendBitmap && !_ArboXExtend.empty())
					{
						CViewBitmap *pVB = createViewBitmap(nLine, "ext_t",  _ArboXExtend);
						pVB->setX (getHrcIconXStart(nLayer) - _XExtend);
						pVB->setY ((sint32)_Lines.size()*_BmpH - ((1+nLine)*_BmpH));
						addView (pVB);
						_Lines[nLine].Bmps.push_back(pVB);
					}
				}
				else
				{
					delete pVB;
				}
			}
		}
		// add additionnal bitmap for each line
		for (uint nLine = 0; nLine < _Lines.size(); nLine++)
		{
			if (!_Lines[nLine].Node->Bitmap.empty())
			{
				CViewBitmap *pVB = createViewBitmap(nLine,  "custom_bm",  _Lines[nLine].Node->Bitmap);
				pVB->setX (getHrcIconXStart(_Lines[nLine].Depth + 1));
				pVB->setY ((sint32)_Lines.size()*_BmpH - ((1+nLine)*_BmpH));
				_Lines[nLine].Bmps.push_back(pVB);
				addView (pVB);
			}
		}
	}

	// ***************************************************************************
	CGroupTree::SNode *CGroupTree::selectNodeByIdRecurse(SNode *pNode,  const std::string &nodeId)
	{
		// select this node?
		if(pNode!=_RootNode)
		{
			if(pNode->Id == nodeId)
				return pNode;
		}

		// try with sons
		for(uint i=0;i<pNode->Children.size();i++)
		{
			SNode	*ret= selectNodeByIdRecurse(pNode->Children[i],  nodeId);
			if(ret)
				return ret;
		}

		// not found => NULL
		return NULL;
	}

	// ***************************************************************************
	bool	CGroupTree::selectNodeById(const std::string &nodeId, bool triggerAH)
	{
		SNode		*selNode= NULL;

		// Avoid infinite recurs
		if(_AvoidSelectNodeByIdIR)
			return true;

		// first find in the hierarchy
		selNode= selectNodeByIdRecurse(_RootNode,  nodeId);

		// if found
		if(selNode)
		{
			// Opens the hierarchy
			SNode *pFather = selNode->Father;
			while(pFather != NULL)
			{
				pFather->Opened = true;
				pFather = pFather->Father;
			}

			if (triggerAH)
			{
				// runAH may infinite recurs (HTML browse...)
				_AvoidSelectNodeByIdIR= true;

				// launch the action handler
				CAHManager::getInstance()->runActionHandler (	selNode->AHName,
					this,
					selNode->AHParams );
			}

			// runAH may infinite recurs (HTML browse...)
			_AvoidSelectNodeByIdIR= false;

			// mark as selected
			_SelectedNode = selNode;

			forceRebuild();

			return true;
		}
		else
		{
			return false;
		}
	}

	// ***************************************************************************
	class CHandlerTreeReset : public IActionHandler
	{
	public:
		void execute (CCtrlBase * /* pCaller */,  const std::string &sParams)
		{
			CGroupTree *pTree = dynamic_cast<CGroupTree*>(CWidgetManager::getInstance()->getElementFromId(sParams));
			if (pTree != NULL)
				pTree->reset();
		}
	protected:
	};
	REGISTER_ACTION_HANDLER( CHandlerTreeReset,  "tree_reset");

	// ***************************************************************************
	void	CGroupTree::changeNavigateOneBranch(bool newState)
	{
		if(newState!=_NavigateOneBranch)
		{
			_NavigateOneBranch= newState;
			// if new is true,  then must reset both open state and selection
			if(_NavigateOneBranch)
			{
				reset();
				// reselect the first line
				selectLine(0);
			}
			// else just rebuild
			else
			{
				forceRebuild();
			}
		}
	}

	// ***************************************************************************
	void CGroupTree::cancelNextSelectLine()
	{
		_CancelNextSelectLine = true;
	}

	// ***************************************************************************
	int CGroupTree::luaGetRootNode(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "getRootNode", 0);
		CLuaIHM::pushReflectableOnStack(ls, getRootNode());
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::luaSetRootNode(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "setRootNode", 1);
		if (ls.isNil())
		{
			setRootNode(NULL);
			return 0;
		}
		setRootNode(SNode::luaGetNodeOnStack(ls, "CGroupTree::setRootNode"));
		return 0;
	}


	// ***************************************************************************
	int CGroupTree::luaForceRebuild(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "forceRebuild", 0);
		forceRebuild();
		return 0;
	}

	// ***************************************************************************
	CGroupTree::SNode *CGroupTree::SNode::luaGetNodeOnStack(CLuaState &ls, const char * /* funcName */)
	{
		SNode *node = dynamic_cast<CGroupTree::SNode *>(CLuaIHM::getReflectableOnStack(ls, 1));
		CLuaIHM::check(ls, node != NULL, "SNode expected");
		return node;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaDetachChild(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaDetachChild";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		detachChild(luaGetNodeOnStack(ls, funcName));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaSort(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaSort";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		sort();
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaSortByBitmap(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaSort";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		sortByBitmap();
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaDeleteChild(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaDeleteChild";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		deleteChild(luaGetNodeOnStack(ls, funcName));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaAddChild(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaAddChild";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		addChild(luaGetNodeOnStack(ls, funcName));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaAddChildSorted(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaAddChildSorted";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		addChildSorted(luaGetNodeOnStack(ls, funcName));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaAddChildSortedByBitmap(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaAddChildSortedByBitmap";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		addChildSorted(luaGetNodeOnStack(ls, funcName));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaGetNodeFromId(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaGetNodeFromId";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		SNode *result = getNodeFromId(ls.toString(1));
		if (result)
		{
			CLuaIHM::pushReflectableOnStack(ls, result);
		}
		else
		{
			ls.pushNil();
		}
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaGetParentTree(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "getParentTree", 0);
		if (ParentTree)
		{
			CLuaIHM::pushUIOnStack(ls, ParentTree);
		}
		else
		{
			ls.pushNil();
		}
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::luaGetNodeUnderMouse(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "getNodeUnderMouse", 0);
		SNode *node = getNodeUnderMouse();
		if (node)
		{
			CLuaIHM::pushReflectableOnStack(ls, node);
		}
		else
		{
			ls.pushNil();
		}
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::luaCancelNextSelectLine(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "cancelNextSelectLine", 0);
		cancelNextSelectLine();
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaAddChildFront(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaAddChildFront";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		addChild(luaGetNodeOnStack(ls, funcName));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaIsChild(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaIsChild";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		ls.push(isChild(luaGetNodeOnStack(ls, funcName)));
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaAddChildAtIndex(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaAddChildAtIndex";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TNUMBER);
		addChildAtIndex(luaGetNodeOnStack(ls, funcName), (sint) ls.toInteger(2));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaGetFather(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaGetFather";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		if(Father)
			CLuaIHM::pushReflectableOnStack(ls, Father);
		else
			ls.pushNil();
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaGetNumChildren(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaGetNumChildren";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		ls.push((uint)Children.size());
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaGetChild(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaGetChild";
		CLuaIHM::checkArgCount(ls, funcName, 1);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TNUMBER);
		//
		sint index = (sint) ls.toInteger(1);
		if (index < 0 || index >= (sint) Children.size())
		{
			std::string range = Children.empty() ? "<empty>" : toString("[0, %d]", Children.size() - 1);
			CLuaIHM::fails(ls, "Bad index of tree node child : %d, range is %s", (int) index, range.c_str());
		}
		CLuaIHM::pushReflectableOnStack(ls, Children[index]);
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::SNode::luaCloseAll(CLuaState &ls)
	{
		const char *funcName = "CGroupTree::SNode::luaGetFather";
		CLuaIHM::checkArgCount(ls, funcName, 0);
		closeAll();
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::luaSelectNodeById(CLuaState &ls)
	{
		const char *funcName = "selectNodeById";
		CLuaIHM::checkArgCount(ls, funcName, 2);
		CLuaIHM::checkArgType(ls, funcName, 1, LUA_TSTRING);
		CLuaIHM::checkArgType(ls, funcName, 2, LUA_TBOOLEAN);
		selectNodeById(ls.toString(1), ls.toBoolean(2));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::luaGetSelectedNodeId(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "getSelectedNodeId", 0);
		ls.push(getSelectedNodeId());
		return 1;
	}

	// ***************************************************************************
	int CGroupTree::luaSelectLine(CLuaState &ls)
	{
		CLuaIHM::checkArgType(ls, "CGroupTree::selectLine", 1, LUA_TNUMBER);
		CLuaIHM::checkArgType(ls, "CGroupTree::selectLine", 2, LUA_TBOOLEAN);
		selectLine((uint) ls.toInteger(1), ls.toBoolean(2));
		return 0;
	}

	// ***************************************************************************
	int CGroupTree::luaUnselect(CLuaState &ls)
	{
		CLuaIHM::checkArgCount(ls, "unselect", 0);
		unselect();
		return 0;
	}


	// ***************************************************************************
	uint CGroupTree::SLine::getNumAdditionnalBitmap() const
	{
		if (!Node) return 0;
		return Node->getNumBitmap();
	}

}



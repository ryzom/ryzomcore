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



#ifndef NL_GROUP_TREE_H
#define NL_GROUP_TREE_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_frame.h"
#include "nel/misc/smart_ptr.h"

namespace NLGUI
{
	class CViewText;
	class CViewBitmap;

	// ----------------------------------------------------------------------------
	class CGroupTree : public CInterfaceGroup
	{

	public:
        DECLARE_UI_CLASS( CGroupTree )

		struct SNode;
		// optional callback that is called when a node has been added
		struct INodeAddedCallback
		{
			/** A node has just been added in the CGroupTree object
			  *  \param node The logic node from which the ui node was built
			  *  \param interfaceElement The ui node that was built
			  */
			virtual void nodeAdded(SNode *node, CInterfaceElement *interfaceElement) = 0;
		};

		// Logic structure to initialize the group tree (root node is not displayed and is always opened)
		struct SNode : public CReflectableRefPtrTarget
		{
			typedef NLMISC::CRefPtr<SNode> TRefPtr;
			// Common
			std::string			Id;				// If not present auto-generated
			bool				Opened;
			bool				DisplayText;	// If false instanciate a template
			bool				Show;			// If false, the node is not displayed (true default, Root ignored)
			sint32				YDecal;
			// Text
			ucstring			Text;			// Internationalized displayed text
			sint32				FontSize;		// If -1 (default), then take the groupTree one
			NLMISC::CRGBA		Color;
			// Template
			NLMISC::CSmartPtr<CInterfaceGroup>	Template;
			// Actions Handlers (for left button)
			std::string			AHName;
			std::string			AHCond;
			std::string			AHParams;
			// Actions Handlers (for right button)
			std::string			AHNameRight;
			std::string			AHParamsRight;
			// Actions Handlers (close/open node)
			std::string			AHNameClose;
			std::string			AHParamsClose;
			// bitmap at this level of hierarchy
			std::string			Bitmap; // additionnal bitmap
			// Hierarchy
			std::vector<SNode*> Children;
			SNode				*Father;
			// updated at display
			SNode				*LastVisibleSon; // filled at build time, meaningfull only if son is shown and opened, undefined otherwise
			// Node added callback
			INodeAddedCallback  *NodeAddedCallback;
			//
			CGroupTree			*ParentTree;
			// ----------------------------
			SNode();
			~SNode();
			void updateLastVisibleSon();
			void detachChild(SNode *pNode);
			void deleteChild(SNode *pNode);
			void addChild (SNode *pNode);
			bool isChild(SNode *pNode) const;
			void addChildFront (SNode *pNode);
			void addChildAtIndex (SNode *pNode, sint index);
			void addChildSorted(SNode *pNode);
			void addChildSortedByBitmap(SNode *pNode);
			void setParentTree(CGroupTree *parent);
			void setFather(SNode *father);
			void closeAll();
			void makeOrphan();
			bool parse (xmlNodePtr cur, CGroupTree *parentGroup);
			uint getNumBitmap() const { return Bitmap.empty() ? 0 : 1; }
			SNode *getNodeFromId(const std::string &id);

			// accessors
			void setBitmap(const std::string &bitmap) { Bitmap = bitmap; }
			std::string getBitmap() const { return Bitmap; }
			void setOpened(bool opened) { Opened = opened; }
			bool getOpened() const { return Opened; }
			void setText(const ucstring &text) { Text = text; }
			const ucstring& getText() const { return Text; }
			sint32 getFontSize() const { return FontSize; }
			void setFontSize(sint32 value) { FontSize = value; }
			sint32 getYDecal() const { return YDecal; }
			void setYDecal(sint32 value) { YDecal = value; }


			std::string			getId() const { return Id; }
			void				setId(const std::string &value) { Id = value; }
			bool				getShow() const { return Show; }
			void				setShow(bool value) { Show = value; }
			std::string			getAHName() const { return AHName; }
			void				setAHName(const std::string &value) { AHName = value; }
			std::string			getAHCond() const { return AHCond; }
			void				setAHCond(const std::string &value) { AHCond = value; }
			std::string			getAHParams() const { return AHParams; }
			void				setAHParams(const std::string &value) { AHParams = value; }
			std::string			getAHNameRight() const { return AHNameRight; }
			void				setAHNameRight(const std::string &value) { AHNameRight = value; }
			std::string			getAHParamsRight() const { return AHParamsRight; }
			void				setAHParamsRight(const std::string &value) { AHParamsRight = value; }
			std::string			getAHNameClose() const { return AHNameClose; }
			void				setAHNameClose(const std::string &value) { AHNameClose = value; }
			std::string			getAHParamsClose() const { return AHParamsClose; }
			void				setAHParamsClose(const std::string &value) { AHParamsClose = value; }
			NLMISC::CRGBA		getColor() const { return Color; }
			void				setColor(NLMISC::CRGBA color) { Color = color; }
			// sort branch & sons alphabetically
			void				sort();
			// sort branch & sons alphabetically & by bitmap name (blank bitmap being the first)
			void				sortByBitmap();

			// lua bindings
			int luaGetNumChildren(CLuaState &ls);
			int luaGetChild(CLuaState &ls);
			int luaDetachChild(CLuaState &ls);
			int luaDeleteChild(CLuaState &ls);
			int luaAddChild(CLuaState &ls);
			int luaAddChildSorted(CLuaState &ls);
			int luaAddChildSortedByBitmap(CLuaState &ls);
			int luaIsChild(CLuaState &ls);
			int luaAddChildFront (CLuaState &ls);
			int luaAddChildAtIndex (CLuaState &ls);
			int luaCloseAll(CLuaState &ls);
			int luaGetFather(CLuaState &ls);
			int luaSort(CLuaState &ls);
			int luaSortByBitmap(CLuaState &ls);
			int luaGetNodeFromId(CLuaState &ls);
			int luaGetParentTree(CLuaState &ls);

			// get node from first parameter on lua stack and throw necessary exception if not present
			static SNode *luaGetNodeOnStack(CLuaState &ls, const char *funcName);

			REFLECT_EXPORT_START(CGroupTree::SNode, CReflectable)
				REFLECT_STRING("Id", getId, setId);
				REFLECT_STRING("Bitmap", getBitmap, setBitmap);
				REFLECT_SINT32("FontSize", getFontSize, setFontSize);
				REFLECT_SINT32("YDecal", getYDecal, setYDecal);
				REFLECT_STRING("AHName", getAHName, setAHName);
				REFLECT_STRING("AHCond", getAHCond, setAHCond);
				REFLECT_RGBA("Color", getColor, setColor);
				REFLECT_STRING("AHParams", getAHParams, setAHParams);
				REFLECT_STRING("AHNameRight", getAHNameRight, setAHNameRight);
				REFLECT_STRING("AHParamsRight", getAHParamsRight, setAHParamsRight);
				REFLECT_STRING("AHNameClose", getAHNameClose, setAHNameClose);
				REFLECT_STRING("AHParamsClose", getAHParamsClose, setAHParamsClose);
				REFLECT_BOOL("Opened", getOpened, setOpened);
				REFLECT_BOOL("Show", getShow, setShow);
				REFLECT_UCSTRING("Text", getText, setText);
				// lua
				REFLECT_LUA_METHOD("getNumChildren", luaGetNumChildren);
				REFLECT_LUA_METHOD("getChild", luaGetChild);
				REFLECT_LUA_METHOD("detachChild", luaDetachChild);
				REFLECT_LUA_METHOD("deleteChild", luaDeleteChild);
				REFLECT_LUA_METHOD("addChild", luaAddChild);
				REFLECT_LUA_METHOD("addChildSorted", luaAddChildSorted);
				REFLECT_LUA_METHOD("addChildSortedByBitmap", luaAddChildSortedByBitmap);
				REFLECT_LUA_METHOD("addChildFront", luaAddChildFront);
				REFLECT_LUA_METHOD("addChildAtIndex", luaAddChildAtIndex);
				REFLECT_LUA_METHOD("isChild", luaIsChild);
				REFLECT_LUA_METHOD("closeAll", luaCloseAll);
				REFLECT_LUA_METHOD("getFather", luaGetFather);
				REFLECT_LUA_METHOD("sort", luaSort);
				REFLECT_LUA_METHOD("sortByBitmap", luaSortByBitmap);
				REFLECT_LUA_METHOD("getNodeFromId", luaGetNodeFromId);
				REFLECT_LUA_METHOD("getParentTree", luaGetParentTree);
			REFLECT_EXPORT_END


		};

	public:

		///constructor
		CGroupTree(const TCtorParam &param);

		// dtor
		virtual ~CGroupTree();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup * parentGroup);

		virtual void checkCoords();

		virtual void updateCoords();

		virtual void draw();

		virtual bool handleEvent (const NLGUI::CEventDescriptor& eventDesc);

		void reset();

		// force rebuild the tree at next updateCoords()
		void forceRebuild();

		// For SNode
		sint32 getIdNumber() { _IdGenerator++; return _IdGenerator; }
		sint32 getFontSize() { return _FontSize; }
		sint32 getYDecal() { return _YDecal; }

		// Set root node and delete the last one the user must not delete all allocated sub node. Nb: selection is reseted
		void setRootNode (SNode *);
		// Remove all lines bitmaps and templates or texts
		void removeAll();

		// unselect current node in tree
		void unselect();

		// Select a node by its line index (depends on opened nodes). no-op if not possible (AH not here, line>size)
		void selectLine(uint line, bool runAH = true);
		// simulate right button click on the given line (this also select the line)
		bool rightButton(uint line);

		// Get the Selected Node Id. empty if none selected
		const std::string	&getSelectedNodeId() const;

		// Select by the node Id. return false if not found (selection is not reseted)
		// NB: if the node was already selected, no-op (no action handler launched)
		bool			selectNodeById(const std::string &nodeId, bool triggerAH = true);

		// Get the root node (Opened State represent the current state)
		SNode			*getRootNode () const {return _RootNode;}

		// get current SNode under the mouse (possibly NULL)
		SNode *getNodeUnderMouse() const;

		// Get/Change the NavigateOneBrnahc option. if false, then perform a reset before
		bool			getNavigateOneBranch() const {return _NavigateOneBranch;}
		void			changeNavigateOneBranch(bool newState);

		// should be called by action handler when thy want to cancel the selection of the line that triggered them
		void			cancelNextSelectLine();

		// Get selected node
		SNode * getSelectedNode() { return _SelectedNode;}

		// lua bindings
		int luaGetRootNode(CLuaState &ls);
		int luaSetRootNode(CLuaState &ls);
		int luaForceRebuild(CLuaState &ls);
		int luaSelectNodeById(CLuaState &ls);
		int luaGetSelectedNodeId(CLuaState &ls);
		int luaSelectLine(CLuaState &ls);
		int luaUnselect(CLuaState &ls);
		int	luaGetNodeUnderMouse(CLuaState &ls);
		int	luaCancelNextSelectLine(CLuaState &ls);

		// Reflection
		REFLECT_EXPORT_START(CGroupTree, CInterfaceGroup)
			REFLECT_BOOL ("navigate_one_branch", getNavigateOneBranch, changeNavigateOneBranch);
			REFLECT_LUA_METHOD("getRootNode", luaGetRootNode);
			REFLECT_LUA_METHOD("setRootNode", luaSetRootNode);
			REFLECT_LUA_METHOD("forceRebuild", luaForceRebuild);
			REFLECT_LUA_METHOD("getSelectedNodeId", luaGetSelectedNodeId);
			REFLECT_LUA_METHOD("selectNodeById", luaSelectNodeById);
			REFLECT_LUA_METHOD("selectLine", luaSelectLine);
			REFLECT_LUA_METHOD("unselect", luaUnselect);
			REFLECT_LUA_METHOD("getNodeUnderMouse", luaGetNodeUnderMouse);
			REFLECT_LUA_METHOD("cancelNextSelectLine", luaCancelNextSelectLine);
		REFLECT_EXPORT_END

	private:

		void setupArbo();
		sint32	_BmpW, _BmpH, _FontSize, _YDecal;
		sint32	_XExtend;

	private:

		// Display structure
		struct SLine
		{
			CViewBase					*TextOrTemplate;
			std::vector<CViewBitmap*>	Bmps;
			SNode::TRefPtr				Node;
			uint8						Depth;

			SLine()
			{
				TextOrTemplate = NULL;
			}

			~SLine()
			{
				Bmps.clear();
			}
			uint getNumAdditionnalBitmap() const;
		};

	protected:

		void rebuild();
		void addTextLine (uint8 nDepth, SNode *pNode);
		void addHierarchyBitmaps();

		SNode *selectNodeByIdRecurse(SNode *pNode, const std::string &nodeId);

		SNode				*_RootNode;
		sint32				_IdGenerator;
		bool				_MustRebuild;
		std::vector<SLine>	_Lines;
		sint32				_OverLine;
		NLMISC::CRGBA		_OverColor;
		NLMISC::CRGBA		_OverColorBack;
		SNode				*_SelectedNode;
		sint32				_SelectedLine;
		NLMISC::CRGBA		_SelectedColor;

		// If a node is closed and a son of this node was selected, then this option force the ancestor being the new selection
		bool				_SelectAncestorOnClose;
		bool				_NavigateOneBranch;
		bool				_AvoidSelectNodeByIdIR;

		// when an action handler is run, it can call 'cancelSelectLine' if no selection should be done for real
		bool				_CancelNextSelectLine;


		// Bitmap For arbo
		std::string			_ArboOpenFirst;
		std::string			_ArboCloseJustOne;
		std::string			_ArboSonWithoutSon;
		std::string			_ArboSonLast;
		std::string			_ArboSon;
		std::string			_ArboLevel;
		std::string			_ArboXExtend;

		// Special rectangle
		bool				_RectangleOutlineMode;
		sint32				_RectangleX, _RectangleY;
		sint32				_RectangleW, _RectangleH;
		sint32				_RectangleDeltaRL;

		sint32			getHrcIconXStart(sint32 depth);
		sint32			getHrcIconXEnd(sint32 depth);

		void			drawSelection(sint x, sint y, sint w, NLMISC::CRGBA col);

		CViewBitmap		*createViewBitmap(uint line, const std::string &idPrefix, const std::string &texture);
	};

}


#endif // NL_GROUP_TREE_H

/* End of group_tree.h */



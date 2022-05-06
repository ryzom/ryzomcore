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



#ifndef NL_GROUP_LIST_H
#define NL_GROUP_LIST_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_frame.h"
#include "nel/gui/view_text.h"

namespace NLGUI
{

	// ----------------------------------------------------------------------------
	class CGroupList : public CInterfaceGroup
	{
	public:
        DECLARE_UI_CLASS( CGroupList )

		enum EAlign
		{
			Bottom = 0,
			Top,
			Left,
			Right
		};

		///constructor
		CGroupList(const TCtorParam &param);

		// dtor
		~CGroupList();
		/**
		* add a child element to the group at the last position
		* 'order' of the element is set to the last order + 1
		* \param child : pointer to the child element
		*/
		void addChild (CViewBase* child, bool deleteOnRemove=true);


		/** add a child before the element at the given index.
		  * 'order' of the element is set to 0
		  * \return true if there was enough room for that child
		  */
		bool addChildAtIndex(CViewBase *child, uint index, bool deleteOnRemove = true);

		/**
		* add a text child element to the group, using the text template
		* \param line : text to be added
		* \param color : text color
		*/
		void addTextChild (const std::string& line,const NLMISC::CRGBA &textColor, bool multiLine = true);

		/**
		* add a text child element to the group, using the text template
		* \param line : text to be added
		*/
		void addTextChild (const std::string& line, bool multiLine = true);

		/// Same as adding a text child but the text will be taken from the string manager
		void addTextChildID (uint32 id, bool multiLine = true);
		// the same, but with id taken from the database
		void addTextChildID (const std::string &dbPath, bool multiLine = true);


		bool delChild (CViewBase* child, bool noWarning=false, bool forceDontDelete = false);

		bool delChild(uint index, bool forceDontDelete = false);

		CViewBase *getChild(uint index) const { return _Elements[index].Element; }
		int luaGetChild(CLuaState &ls);

		void deleteAllChildren();

		void removeHead();

		// Get the number of children
		uint getNumChildren() const { return (uint)_Elements.size(); }

		// Get the number of active children
		uint getNumActiveChildren() const;

		/**
		* set the template that will be used to add text;
		* \templ : a CViewText object. Only its font size, color and shadow are required.
		*/
		void setTextTemplate(const CViewText& templ);

		/**
		* set the template that will be used to add text;
		* \templ : a CViewText object. Only its font size, color and shadow are required.
		*/
		CViewText * getTextTemplatePtr()
		{
			return &_Templ;
		}

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		/**
		 * parse the element and initalize it
		 * \paral cur : pointer to the node describing this element
		 * \param parentGroup : the parent group of this element
		 * \return true if success
		 */
		virtual bool parse (xmlNodePtr cur, CInterfaceGroup * parentGroup);
		//virtual uint32 getMemory();
		/**
		 * init  or reset the children element coords. Orverloaded from CInterfaceGroup because we begin with the last inserted element here
		 */
		virtual void updateCoords();

		virtual void draw();

		virtual bool handleEvent (const NLGUI::CEventDescriptor& eventDesc);

		virtual void clearViews();
		virtual void clearControls();
		virtual void clearGroups();

		void setSpace (sint32 s) { _Space = s; }

		virtual CInterfaceElement* getElement (const std::string &id)
		{ return CInterfaceGroup::getElement (id); }

		sint32 getNbElement() { return (sint32)_Elements.size(); }
		sint32 getSpace() { return _Space; }

		void	setDynamicDisplaySize (bool dds) { _DynamicDisplaySize = dds; }
		bool	getDynamicDisplaySize() { return _DynamicDisplaySize; }

		void	forceSizeW (sint32 newSizeW);
		void	forceSizeH (sint32 newSizeH);

		void	setMinW(sint32 minW);
		void	setMinH(sint32 minH);
		sint32	getMinW() const {return _MinW;}
		sint32	getMinH() const {return _MinH;}

		// set the rank for the element at the given index (used to reinsert container after they have been turned into popups)
		void setOrder(uint index, uint order) { _Elements[index].Order = order; }
		uint getOrder(uint index) const { return _Elements[index].Order; }

		// get element of index or -1 if not found
		sint32 getElementIndex(CViewBase* child) const;
		int    luaGetElementIndex(CLuaState &ls);

		// swap 2 entries in the list (and also their orders)
		void   swapChildren(uint index1, uint index2);
		int	   luaUpChild(CLuaState &ls);
		int	   luaDownChild(CLuaState &ls);

		// deleteOnRemove flag
		void   setDelOnRemove(uint index, bool delOnRemove);
		bool   getDelOnRemove(uint index) const;

		// children number
		void			setChildrenNb(sint32 /* val */){}
		sint32			getChildrenNb() const {return (sint32)_Elements.size();}

		int luaAddChild(CLuaState &ls);
		int luaAddChildAtIndex(CLuaState &ls);
		int luaDetachChild(CLuaState &ls);
		int luaClear(CLuaState &ls); // synonimous for deleteAllChildren
		int luaAddTextChild(CLuaState &ls);
		int luaAddColoredTextChild(CLuaState &ls);
		int luaDelChild(CLuaState &ls);

		REFLECT_EXPORT_START(CGroupList, CInterfaceGroup)
			REFLECT_LUA_METHOD("addTextChild", luaAddTextChild)
			REFLECT_LUA_METHOD("addColoredTextChild", luaAddColoredTextChild)
			REFLECT_LUA_METHOD("addChild", luaAddChild);
			REFLECT_LUA_METHOD("addChildAtIndex", luaAddChildAtIndex);
			REFLECT_LUA_METHOD("detachChild", luaDetachChild);
			REFLECT_LUA_METHOD("clear", luaClear);
			REFLECT_LUA_METHOD("delChild", luaDelChild);
			REFLECT_LUA_METHOD("upChild", luaUpChild);
			REFLECT_LUA_METHOD("downChild", luaDownChild);
			REFLECT_LUA_METHOD("getChild", luaGetChild);
			REFLECT_LUA_METHOD("getElementIndex", luaGetElementIndex);
			REFLECT_SINT32 ("childrenNb", getChildrenNb, setChildrenNb);
		REFLECT_EXPORT_END


	protected:
		std::string _HardText;
		uint32 _TextId;

		//max number of elements
		sint32	_MaxElements;

		// Where to add next element
		EAlign	_AddElt;

		// Where to align the newly added element
		EAlign	_Align;

		// Space between two elements in pixel
		sint32	_Space;

		// Text template
		CViewText _Templ;

		// Current id of the view
		sint32	_IdCounter;

		// Used for context menu to display the same size as the whole content
		bool _DynamicDisplaySize;

		// Do we have a color under the element pointed by the mouse
		bool _Over;

		// If over is true so we have a color
		NLMISC::CRGBA _OverColor;

		// Current elt over the pointer
		sint32 _OverElt;

		struct CElementInfo
		{
			uint				Order; // Used to sort the window by their insertion order.
									   // This is used to put back a window at the right place if it was turned into a popup.
			CViewBase			*Element;
			bool				EltDeleteOnRemove;
		};
		friend struct CRemoveViewPred;
		friend struct CRemoveCtrlPred;
		friend struct CRemoveGroupPred;

		// The list is forced to be at least this size in updateCoords().
		sint32		_MinW;
		sint32		_MinH;


		// To conserve elements in the order they have been added
		// (the element drawn are stored in _views, _contrlos or _childrengroups of cinterfacegroup
		std::vector<CElementInfo> _Elements;

	private:

		void setHSGroup (CViewBase *child, EAlign addElt, EAlign align);
		void setHSParent(CViewBase *view, EAlign addElt, EAlign align, uint space);
		void setupSizes();
		void onTextChanged();

	};


}

#endif // NL_GROUP_LIST_H

/* End of group_list.h */



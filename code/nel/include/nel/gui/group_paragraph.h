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



#ifndef NL_GROUP_PARAGRAPH_H
#define NL_GROUP_PARAGRAPH_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_frame.h"
#include "nel/gui/view_text.h"
#include "nel/gui/view_link.h"
#include "nel/gui/ctrl_button.h"

namespace NLGUI
{

	class CCtrlLink : public CCtrlButton
	{
	public:
        DECLARE_UI_CLASS( CCtrlLink )

		CCtrlLink (const TCtorParam &param) : CCtrlButton(param)
		{}
	};

	// ----------------------------------------------------------------------------
	class CGroupParagraph : public CInterfaceGroup
	{
	public:
        DECLARE_UI_CLASS( CGroupParagraph )

		enum EAlign
		{
			Bottom = 0,
			Top,
			Left,
			Right
		};

		///constructor
		CGroupParagraph(const TCtorParam &param);

		// dtor
		~CGroupParagraph();

		/**
		* add a child element to the group at the last position
		* 'order' of the element is set to the last order + 1
		* \param child : pointer to the child element
		*/
		void addChild (CViewBase* child, bool deleteOnRemove=true);

		/**
		* add a link element to the group at the last position
		* \param child : pointer to the child element
		*/
		void addChildLink (CViewLink* child, bool deleteOnRemove=true);

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
		void addTextChild (const ucstring& line,const NLMISC::CRGBA &textColor, bool multiLine = true);

		/**
		* add a text child element to the group, using the text template
		* \param line : text to be added
		*/
		void addTextChild (const ucstring& line, bool multiLine = true);

		/// Same as adding a text child but the text will be taken from the string manager
		void addTextChildID (uint32 id, bool multiLine = true);
		// the same, but with id taken from the database
		void addTextChildID (const std::string &dbPath, bool multiLine = true);

	protected:

		void delChild (CViewBase* child);

		void delChild(uint index);

	public:

		CViewBase *getChild(uint index) const { return _Elements[index].Element; }

		void deleteAllChildren();

		// void removeHead();

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

		virtual void checkCoords();

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

		// swap 2 entries in the list (and also their orders)
		// void   swapChildren(uint index1, uint index2);

		// deleteOnRemove flag
		void   setDelOnRemove(uint index, bool delOnRemove);
		bool   getDelOnRemove(uint index) const;

		void	setTopSpace(uint topSpace)
		{
			_TopSpace = topSpace;
			setResizeFromChildHMargin(topSpace);
			invalidateContent();
		};

		uint	getTopSpace()
		{
			return _TopSpace;
		};

		void	setIndent(uint indent) { _Indent = indent; }

		void	setFirstViewIndent(sint indent)
		{
			_FirstViewIndentView = indent;
			invalidateContent();
		}

		/// temporarily enable mouse over effect
		//  will be automatically disabled when mouse leaves element
		void	enableTempOver() { _TempOver = true; }
		void	disableTempOver() { _TempOver = false; }

		/// \from CInterfaceElement
		void onInvalidateContent();
		sint32	getMaxUsedW() const;
		sint32	getMinUsedW() const;

	protected:

		// Content validated
		bool	_ContentValidated;

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

		// Do we have a color under the element pointed by the mouse
		bool _Over;
		// Temporarily force mouse over effect. Deactivated when mouse moves away
		bool _TempOver;

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

		// Last parent width
		sint32		_LastW;

		// Top space
		uint		_TopSpace;

		// Indent
		uint		_Indent;	// Left margin
		sint		_FirstViewIndentView;	// Additionnal left margin for the first view

		// A link structure
		class CLink
		{
		public:

			CLink(CViewLink *link);

			// The link view
			CViewLink		*Link;

			// The three control button
			CCtrlLink		*CtrlLink[3];
		};

		// The links
		std::vector<CLink>	_Links;

	private:
		std::string _HardText;
		uint32 _TextId;

		void setupSizes();
		void onTextChanged();

		// void setHSGroup (CViewBase *child, EAlign addElt, EAlign align);
		// void setHSParent(CViewBase *view, EAlign addElt, EAlign align, uint space);

	};

}

#endif // NL_GROUP_PARAGRAPH_H

/* End of group_paragraph.h */



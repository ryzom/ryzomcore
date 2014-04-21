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



#ifndef NL_INTERFACE_GROUP_H
#define NL_INTERFACE_GROUP_H

#include "nel/gui/ctrl_base.h"
#include "nel/gui/action_handler.h"

namespace NLGUI
{

	class CInterfaceGroup : public CCtrlBase
	{
	public:
		DECLARE_UI_CLASS(CInterfaceGroup)

		/// Constructor
		CInterfaceGroup(const TCtorParam &param);

		/// Destructor
		virtual ~CInterfaceGroup();

		virtual void setIdRecurse(const std::string &id);

		/// Coming from CInterfaceElement
		virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		xmlNodePtr serializeGroup( xmlNodePtr parentNode, const char *type ) const;
		xmlNodePtr serializeSubGroups( xmlNodePtr parentNode ) const;
		xmlNodePtr serializeControls( xmlNodePtr parentNode ) const;
		xmlNodePtr serializeViews( xmlNodePtr parentNode ) const;
		virtual xmlNodePtr serializeTreeData( xmlNodePtr parentNode ) const;
		bool serializeLinks( xmlNodePtr parentNode ) const;

		virtual uint32 getMemory ();

		virtual CInterfaceElement* getElement (const std::string &id);
		CInterfaceElement* findFromShortId(const std::string &id);

		/// Dynamic creation
		virtual void addView  (CViewBase *child , sint eltOrder = -1);
		virtual void addCtrl  (CCtrlBase *child, sint eltOrder = -1);
		virtual void addGroup (CInterfaceGroup *child, sint eltOrder = -1);

		CViewBase*		 getView (const std::string &id);
		CCtrlBase*		 getCtrl (const std::string &id);
		CInterfaceGroup* getGroup(const std::string &id) const;

		// Delete know type by ptr (return true if found and removed)
		virtual bool delView  (CViewBase *child, bool dontDelete = false);
		virtual bool delCtrl  (CCtrlBase *child, bool dontDelete = false);
		virtual bool delGroup (CInterfaceGroup * child, bool dontDelete = false);

		// Delete know type by name (return true if found and removed)
		virtual bool delView  (const std::string &id, bool dontDelete = false);
		virtual bool delCtrl  (const std::string &id, bool dontDelete = false);
		virtual bool delGroup (const std::string &id, bool dontDelete = false);

		// Delete unknow type by name or ptr. NB: additionaly, if it's a group, unmakeWindow() is called as necessary
		bool delElement (const std::string &id, bool noWarning=false);
		bool delElement (CInterfaceElement *pIE, bool noWarning=false);

		// Take the element from the group, but don't delete it!
		CInterfaceElement* takeElement( CInterfaceElement *e );

		uint getNumGroup() const { return (uint)_ChildrenGroups.size(); }
		CInterfaceGroup *getGroup(uint index) const;

		sint32	getMaxUsedW() const;
		sint32	getMinUsedW() const;

		/// Coming from CCtrlBase
		virtual bool handleEvent (const NLGUI::CEventDescriptor &event);

		void executeControl (const std::string &sControlName);

		const std::vector<CInterfaceGroup*> & getGroups () { return _ChildrenGroups; }
		const std::vector<CCtrlBase*> & getControls() { return _Controls; }
		const std::vector<CViewBase*> & getViews() { return _Views; }

		// test is a group is a direct child of this interface group
		bool isChildGroup(const CInterfaceGroup *group) const;

		virtual bool isWindowUnder (sint32 x, sint32 y); // Virtual for menu that is not square
		CInterfaceGroup *getGroupUnder (sint32 x, sint32 y);
		virtual bool getViewsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CViewBase*> &vVB); // Return true if x,y under the group
		virtual bool getCtrlsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CCtrlBase*> &vICL);
		virtual bool getGroupsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CInterfaceGroup *> &vIGL);

		void absoluteToRelative (sint32 &x, sint32 &y);

		/// Coming from CViewBase
		virtual void draw ();
		// Draw with no clip (if clip is done by parent)
		virtual void drawNoClip();

		/// Tool function to draw a single Element that should exist in the group (clipped by the group)
		void	drawElement (CViewBase *el);


		/**
		 * update the elements coords
		 */
		virtual void checkCoords();
		virtual void updateCoords();

		/// remove all views
		virtual void clearViews();

		/// remove all controls
		virtual void clearControls();

		/// remove all groups
		virtual void clearGroups();

		void setParentSizeMax(CInterfaceElement *pIE) { _ParentSizeMax = pIE; }
		void setMaxW (sint32 maxw)	{ _MaxW = maxw; }
		void setMaxH (sint32 maxh)	{ _MaxH = maxh; }
		void setOfsX (sint32 x)		{ _OffsetX = x; }
		void setOfsY (sint32 y)		{ _OffsetY = y; }
		bool moveSBTrackY (CInterfaceGroup *target, sint32 dy);
		bool moveSBTargetY (CInterfaceGroup *target, sint32 dy);
		void setResizeFromChildW(bool resize) { _ResizeFromChildW = resize; }
		void setResizeFromChildH(bool resize) { _ResizeFromChildH = resize; }

		// Valid only for windows InterfaceGroup.
		// escapable
		void setEscapable(bool b) { _Escapable= b; }
		bool getEscapable() const { return _Escapable; }
		void setAHOnEscape(const std::string &ah) { _AHOnEscape = CAHManager::getInstance()->getAH(ah, _AHOnEscapeParams); }
		const std::string &getAHOnEscape() const { return CAHManager::getInstance()->getAHName(_AHOnEscape); }
		void setAHOnEscapeParams(const std::string &ah) { _AHOnEscapeParams = ah; }
		const std::string &getAHOnEscapeParams() const { return _AHOnEscapeParams; }
		// enterable
		void setAHOnEnter(const std::string &ah) { _AHOnEnter = CAHManager::getInstance()->getAH(ah, _AHOnEnterParams); }
		const std::string &getAHOnEnter() const { return CAHManager::getInstance()->getAHName(_AHOnEnter); }
		void setAHOnEnterParams(const std::string &ah) { _AHOnEnterParams = ah; }
		const std::string &getAHOnEnterParams() const { return _AHOnEnterParams; }
		uint8 getPriority() const { return _Priority; }
		void setPriority(uint8 nprio);


		sint32 getMaxW () const { return _MaxW; }
		sint32 getMaxH () const { return _MaxH; }
		sint32 getMaxWReal () const { return _Active ? _MaxWReal : 0; }
		sint32 getMaxHReal () const { return _Active ? _MaxHReal : 0; }
		sint32 getOfsX () const { return _OffsetX; }
		sint32 getOfsY () const { return _OffsetY; }
		bool   getResizeFromChildW() const { return _ResizeFromChildW; }
		bool   getResizeFromChildH() const { return _ResizeFromChildH; }
		sint32 getResizeFromChildWMargin() const { return _ResizeFromChildWMargin; }
		sint32 getResizeFromChildHMargin() const { return _ResizeFromChildHMargin; }
		void   setResizeFromChildWMargin(sint32 margin) { _ResizeFromChildWMargin = margin; }
		void   setResizeFromChildHMargin(sint32 margin) { _ResizeFromChildHMargin = margin; }
		bool   getOverlappable() const { return _Overlappable; }

		virtual void setActive (bool state);

		// eval dimension of children bbox
		void evalChildrenBBox(bool resizeFromChildW, bool resizeFromChildH, sint &width, sint &height) const;

		virtual void	launch ();


		// right & left clicks handler
		void	setLeftClickHandler(const std::string &handler);
		void	setRightClickHandler(const std::string &handler);
		void	setLeftClickHandlerParams(const std::string &params) { _AHOnLeftClickParams = params; }
		void	setRightClickHandlerParams(const std::string &params) { _AHOnRightClickParams = params; }
		void	setOnActiveHandler(const std::string &h) { _AHOnActive = CAHManager::getInstance()->getAH(h,_AHOnActiveParams); }
		void	setOnActiveParams(const std::string &p) { _AHOnActiveParams = p; }
		void	setOnDeactiveHandler(const std::string &h) { _AHOnDeactive = CAHManager::getInstance()->getAH(h,_AHOnDeactiveParams); }
		void	setOnDeactiveParams(const std::string &p) { _AHOnDeactiveParams = p; }

		const std::string &getLeftClickHandler() const { return CAHManager::getInstance()->getAHName(_AHOnLeftClick); }
		const std::string &getLeftClickHandlerParams() const { return _AHOnLeftClickParams; }
		const std::string &getRightClickHandler() const { return CAHManager::getInstance()->getAHName(_AHOnRightClick); }
		const std::string &getRightClickHandlerParams() const { return _AHOnRightClickParams; }
		const std::string &getOnActiveHandler() const { return CAHManager::getInstance()->getAHName(_AHOnActive); }
		const std::string &getOnActiveParams() const { return _AHOnActiveParams; }
		const std::string &getOnDeactiveHandler() const { return CAHManager::getInstance()->getAHName(_AHOnDeactive); }
		const std::string &getOnDeactiveParams() const { return _AHOnDeactiveParams; }

		// find a sub view/ctrl/group in this group from its id
		int luaFind(CLuaState &ls);
		int luaGetEnclosingContainer(CLuaState &ls);
		int luaDeleteLUAEnvTable(CLuaState &ls);
		int luaAddGroup(CLuaState &ls);
		int luaDelGroup(CLuaState &ls);
		int luaGetNumGroups(CLuaState &ls);
		int luaGetGroup(CLuaState &ls);

		void setMaxSizeRef(const std::string &maxSizeRef);
		std::string getMaxSizeRefAsString() const;


		REFLECT_EXPORT_START(CInterfaceGroup, CCtrlBase)
			REFLECT_LUA_METHOD("find", luaFind);
			REFLECT_LUA_METHOD("deleteLUAEnvTable", luaDeleteLUAEnvTable);
			REFLECT_LUA_METHOD("getEnclosingContainer", luaGetEnclosingContainer);
			REFLECT_LUA_METHOD("addGroup", luaAddGroup);
			REFLECT_LUA_METHOD("delGroup", luaDelGroup);
			REFLECT_LUA_METHOD("getNumGroups", luaGetNumGroups);
			REFLECT_LUA_METHOD("getGroup", luaGetGroup);
			REFLECT_STRING ("left_click", getLeftClickHandler, setLeftClickHandler);
			REFLECT_STRING ("right_click", getRightClickHandler, setRightClickHandler);
			REFLECT_STRING ("left_click_params", getLeftClickHandlerParams, setLeftClickHandlerParams);
			REFLECT_STRING ("right_click_params", getRightClickHandlerParams, setRightClickHandlerParams);
			REFLECT_STRING ("on_active", getOnActiveHandler, setOnActiveHandler);
			REFLECT_STRING ("on_active_params", getOnActiveParams, setOnActiveParams);
			REFLECT_STRING ("on_deactive", getOnDeactiveHandler, setOnDeactiveHandler);
			REFLECT_STRING ("on_deactive_params", getOnDeactiveParams, setOnDeactiveParams);
			REFLECT_STRING ("on_enter", getAHOnEnter, setAHOnEnter);
			REFLECT_STRING ("on_enter_params", getAHOnEnterParams, setAHOnEnterParams);
			REFLECT_STRING ("on_escape", getAHOnEscape, setAHOnEscape);
			REFLECT_STRING ("on_escape_params", getAHOnEscapeParams, setAHOnEscapeParams);
			REFLECT_SINT32 ("ofsx", getOfsX, setOfsX);
			REFLECT_SINT32 ("ofsy", getOfsY, setOfsY);
			REFLECT_BOOL("child_resize_w", getResizeFromChildW, setResizeFromChildW);
			REFLECT_SINT32("child_resize_wmargin", getResizeFromChildWMargin, setResizeFromChildWMargin);
			REFLECT_BOOL("child_resize_h", getResizeFromChildH, setResizeFromChildH);
			REFLECT_SINT32("child_resize_hmargin", getResizeFromChildHMargin, setResizeFromChildHMargin);
			REFLECT_SINT32 ("ofsy", getOfsY, setOfsY);
			REFLECT_STRING("max_sizeref", getMaxSizeRefAsString, setMaxSizeRef);
			REFLECT_SINT32 ("max_w", getMaxW, setMaxW);
			REFLECT_SINT32 ("max_h", getMaxH, setMaxH);
			REFLECT_SINT32 ("max_w_real", getMaxWReal, dummySet);
			REFLECT_SINT32 ("max_h_real", getMaxHReal, dummySet);
		REFLECT_EXPORT_END



		// From CCtrlBase
		virtual void updateAllLinks();

		/// return true for some containers. false by default
		virtual bool	isMovable() const {return false;}

		virtual sint32 getAlpha() const;
		virtual void setAlpha (sint32 a);

		/// Eval current clip coords. This is not incremental as with makeNewClip, and thus more slow. This also doesn't change the current clip window.
		void getClip(sint32 &x, sint32 &y, sint32 &w, sint32 &h) const;

		// quick way to know if the group is a CGroupContainer
		bool isGroupContainer() const { return _IsGroupContainer; }
		bool isGroupScrollText() const{ return _IsGroupScrollText; }
		bool isGroupInScene() const{ return _IsGroupInScene; }
		bool isGroupList() const{ return _IsGroupList; }

		CInterfaceGroup* getEnclosingContainer();

		sint getInsertionOrder(CViewBase *vb) const;

		// for debug only
		void dumpGroups();
		void dumpEltsOrder();

		virtual void renderWiredQuads(CInterfaceElement::TRenderWired type, const std::string &uiFilter);

		virtual bool isGroup() const { return true; }

		// clear all edit box in the ui
		virtual void    clearAllEditBox();
		// restore all backuped positions for containers
		virtual void    restoreAllContainersBackupPosition();

		virtual void	dumpSize(uint depth = 0) const;

		// From CInterfaceElement
		virtual void visit(CInterfaceElementVisitor *visitor);

		/// Visits only this group's sub-groups and then the group itself
		virtual void visitGroupAndChildren( CInterfaceElementVisitor *visitor );

		// Check cursor
		void	setUseCursor(bool use);
		bool	getUseCursor() const { return _UseCursor; }


		// From CInterfaceElement
		virtual void	onFrameUpdateWindowPos(sint dx, sint dy);
		// true for CGroupInScene for instance
		bool	isNeedFrameUpdatePos() const {return _NeedFrameUpdatePos;}


		/// \name LUA specific
		// @{
		// Create a LUA Environement if don't exist, then push it on the LUA stack
		void	pushLUAEnvTable();
		// Free the LUA Env Table
		void	deleteLUAEnvTable(bool recurse = false);
		// Set the LUA script to execute at checkCoords time (empty to reset)
		void	setLuaScriptOnDraw(const std::string &script);
		//
		void	executeLuaScriptOnDraw();
		// Set the LUA script to execute when a list of DB change (of forms: "@DB1,@DB2" ....). The dbList is the key
		void	addLuaScriptOnDBChange(const std::string &dbList, const std::string &script);
		// Remove the LUA script to execute when a list of DB change
		void	removeLuaScriptOnDBChange(const std::string &dbList);
		// @}

		virtual CInterfaceElement *clone();
		virtual void serial(NLMISC::IStream &f);

		// Return the current Depth, with no ZBias applied.
		float getDepthForZSort() const { return _DepthForZSort; }

		void onWidgetDeleted( CInterfaceElement *e );

	protected:

		void makeNewClip (sint32 &oldClipX, sint32 &oldClipY, sint32 &oldClipW, sint32 &oldClipH);
		void restoreClip (sint32 oldSciX, sint32 oldSciY, sint32 oldSciW, sint32 oldSciH);

		// Compute clip contribution for current window, and a previous clipping rectangle. This doesn't change the clip window in the driver.
		void computeCurrentClipContribution(sint32 prevX, sint32 prevY, sint32 prevW, sint32 prevH,
								sint32 &newX, sint32 &newY, sint32 &newW, sint32 &newH) const;

		void delEltOrder (CViewBase *pElt);

		// update coords one time
		void doUpdateCoords();

		// notify children controls & groups that 'active' has been called on one of their parent
		void notifyActiveCalled(const NLGUI::CEventDescriptorActiveCalledOnParent &desc);

	protected:

		/// children interface elements
		std::vector<CInterfaceGroup*>	_ChildrenGroups;
		std::vector<CCtrlBase*>			_Controls;
		std::vector<CViewBase*>			_Views;

		std::vector<CViewBase*>			_EltOrder;

		/// Scroll properties
		NLMISC::CRefPtr<CInterfaceElement>	_ParentSizeMax;	 // RefPtr in case of group destroyed in a parent group with posref on it
		sint32 _MaxW, _MaxH;
		sint32 _MaxWReal, _MaxHReal;
		sint32 _OffsetX, _OffsetY;

		uint8 _Priority;

		// Misc prop
		bool	_Overlappable		: 1;
		bool	_ResizeFromChildW	: 1;
		bool	_ResizeFromChildH	: 1;
		bool	_Escapable			: 1;
		bool	_UseCursor			: 1;
		bool	_IsGroupContainer	: 1;	// faster than a virual call
		bool	_IsGroupScrollText  : 1;
		bool    _IsGroupInScene     : 1;
		bool	_IsGroupList		: 1;
		bool	_NeedFrameUpdatePos	: 1;	// typically For CGroupInScene
		sint32	_ResizeFromChildWMargin;
		sint32	_ResizeFromChildHMargin;
		sint32  _GroupSizeRef;

		// Projected Depth with no ZBias applied
		float				_DepthForZSort;

		// handler for activation
		IActionHandler	*_AHOnActive;
		CStringShared	_AHOnActiveParams;
		IActionHandler	*_AHOnDeactive;
		CStringShared	_AHOnDeactiveParams;

		// right & left clicks
		IActionHandler	*_AHOnLeftClick;
		CStringShared	_AHOnLeftClickParams;
		IActionHandler	*_AHOnRightClick;
		CStringShared	_AHOnRightClickParams;

		// enter params.
		IActionHandler	*_AHOnEnter;
		CStringShared	_AHOnEnterParams;

		// escape AH
		IActionHandler	*_AHOnEscape;
		CStringShared	_AHOnEscapeParams;

	private:

		void addToEltOrder(CViewBase *view, sint order);

		/// \name LUA specific
		// @{
		// Lua Env Table created. Table is in the LUA_REGISTRYINDEX, with key as this CInterfaceGroup* userdata
		bool			_LUAEnvTableCreated;
		// The LUA script to be executed on Draw (checkCoords)
		CStringShared	_LUAOnDraw;
		// The InterfaceLink created specialy for Lua Script to be executed at some DB change
		typedef std::map<std::string, NLMISC::CSmartPtr<CInterfaceLink> >	TLUAOnDbChange;
		TLUAOnDbChange	_LUAOnDbChange;
		void	removeAllLUAOnDbChange();
	protected:
		void parseMaxSizeRef(const char *ptr);
		// @}
	};

}

#endif // NL_INTERFACE_GROUP_H

/* End of interface_group.h */



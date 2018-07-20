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



#ifndef RZ_GROUP_CONTAINER_H
#define RZ_GROUP_CONTAINER_H

#include "nel/gui/interface_group.h"
#include "nel/gui/group_container_base.h"
#include "nel/misc/smart_ptr.h"

namespace NLGUI
{
	class CEventDescriptorLocalised;
	class CCtrlButton;
	class CCtrlScroll;
	class CViewText;
	class CViewBitmap;
	class CGroupList;
	class COptionsContainerInsertion;
	class COptionsContainerMove;
	class COptionsLayer;
	class CGroupContainer;



	// ***************************************************************************
	/**
	 * class describing a resizer for the container
	 * \author Matthieu 'TrapII' Besson
	 * \date 2003
	 */
	class CCtrlResizer : public CCtrlBase
	{

	public:
        DECLARE_UI_CLASS( CCtrlResizer )

		CCtrlResizer(const TCtorParam &param);
		virtual void draw ();
		virtual bool handleEvent (const NLGUI::CEventDescriptor &event);
		// Add a big delta so when the user is over the Resizer, always take it whatever other controls under
		virtual uint		getDeltaDepth() const { return 100; }

		// get real resizer pos : if parent has pop_min_w == pop_max_w, then horizontal resizer will be discarded
		//                        if parent has pop_min_h == pop_max_h, then vertical resizer will be discarded
		THotSpot			getRealResizerPos() const;
		THotSpot			getResizerPos() const { return _ResizerPos; }
		void				setResizerPos(THotSpot	resizerPos) { _ResizerPos = resizerPos; }

		bool		IsMaxH; // Do this resizer is a MaxH resizer ?

		// Max sizes for the parent
		sint32 WMin, WMax;
		sint32 HMin, HMax;

		// from CCtrlBase
		virtual bool		canChangeVirtualDesktop() const { return !_MouseDown; }

	private:

		sint32 resizeW (sint32 dx);
		sint32 resizeH (sint32 dy);

	private:
		THotSpot	_ResizerPos; // how the resizer should resize its parent
		bool _MouseDown;
		sint32 _MouseDownX;
		sint32 _MouseDownY;
		sint32 _XBias;
		sint32 _YBias;
	};


	// ***************************************************************************
	/**
	 * Class describing a Mover for the container
	 * Clicking on it can also open the container
	 * This can be used to move a container if it is movable.
	 * If the container is popable, it will first pull it of the hierarchy, then it becomes movable.
	 * It can also be used to change the position of a group container that is inserted in the list of another container.
	 * \author Lionel Berenguier
	 * \date 2003
	 */
	class CCtrlMover : public CCtrlBase
	{
	public:
        DECLARE_UI_CLASS( CCtrlMover )

        CCtrlMover(const TCtorParam &param, bool canMove = true, bool canOpen = true );
		~CCtrlMover();
		virtual void draw ();
		virtual bool handleEvent (const NLGUI::CEventDescriptor &event);
		bool canMove() { return _CanMove; }

		bool isMoving() const {return _Moving;}
		bool isMovingInParentList() const { return _MovingInParentList; }

		// from CCtrlBase
		virtual bool		canChangeVirtualDesktop() const { return !_Moving; }

	private:
		sint32				_MoveStartX, _MoveStartY;
		sint32				_MoveDeltaXReal, _MoveDeltaYReal;
		sint64              _ScrollTime;
		sint32              _StartIndex;
		sint32              _InsertionIndex;
		// clip window from parent list
		sint32              _ParentListTop;
		sint32              _ParentListBottom;
		//
		sint64				_WaitToOpenCloseDate;
		//
		bool				_CanMove              : 1;
		bool				_CanOpen              : 1;
		bool				_Moving               : 1;
		bool				_MovingInParentList   : 1;
		bool                _HasMoved             : 1;
		bool                _ParentScrollingUp    : 1;
		bool                _ParentScrollingDown  : 1;
		bool				_StopScrolling        : 1; // stop scrolling at next draw
		bool				_WaitToOpenClose      : 1;
		//
		static COptionsContainerInsertion *getInsertionOptions();
	private:
		void				setPoped(CGroupContainer *gc, sint32 x, sint32 y, const NLGUI::CEventDescriptorMouse &eventDesc);
		void				setMovingInParent(CGroupContainer *gc, sint32 x, sint32 y, const NLGUI::CEventDescriptorMouse &eventDesc);
		void				updateInsertionIndex(const CGroupList *gl, sint32 posY);
		void				stopMove();
		bool				runTitleActionHandler();
		void				handleScrolling();

	};


	// ***************************************************************************
	/**
	 * class describing a group of views controls and other groups
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */
	class CGroupContainer : public CGroupContainerBase
	{
	public:
		enum { NumResizers = 8 };
	public:
		// observer to know when children have moved. This can be used to keep external datas in sync
		struct IChildrenObs
		{
			virtual void childrenMoved(uint srcIndex, uint destIndex, CGroupContainer *children) = 0;
		};
	public:
        DECLARE_UI_CLASS( CGroupContainer )
		CGroupContainer(const TCtorParam &param);
		~CGroupContainer();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;
		xmlNodePtr serializeTreeData( xmlNodePtr parentNode ) const;

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

		virtual void updateCoords ();

		virtual void draw ();

		virtual void clearViews ();

		virtual bool handleEvent (const NLGUI::CEventDescriptor &eventDesc);

		virtual void launch ();

		virtual void setActive (bool state);

		virtual bool getViewsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CViewBase*> &vVB); // Return true if x,y under the group

		virtual bool getCtrlsUnder (sint32 x, sint32 y, sint32 clipX, sint32 clipY, sint32 clipW, sint32 clipH, std::vector<CCtrlBase*> &vICL);

		void open();

		void close();

		void setup(); // Create the container

		/** If insertion order is -1, pIC is added at the end of the container
		  * otherwise it is inserted after containers of a lower order
		  */
		void attachContainer (CGroupContainer *pIC, sint insertionOrder = -1);
		// Insert a container at the given index.
		bool attachContainerAtIndex(CGroupContainer *pIC, uint index);

		// Before a container is detached from parent, it should be pop in
		void detachContainer (CGroupContainer *pIC);
		void removeAllContainers();

		void setOpen(bool opened)
		{
			if (opened)
			{
				open();
			}
			else
			{
				close();
			}
		}
		bool isOpen() const { return _Opened; }

		// Force Open for container setActive and open()
		virtual void forceOpen();

		/// Set the title open and close
		virtual bool isMovable() const {return _Movable;}
		void setMovable(bool b);

		void	setContent (CInterfaceGroup *pC);

		std::string		getTitle () const;
		void			setTitle (const std::string &title);
		std::string		getTitleOpened () const;
		void			setTitleOpened (const std::string &title);
		std::string		getTitleClosed () const;
		void			setTitleClosed (const std::string &title);
		std::string		getTitleColorAsString() const;
		void			setTitleColorAsString(const std::string &col);

		void			setHeaderColor (const std::string &ptr) { _HeaderColor.link(ptr.c_str()); }

		// Get the header color draw. NB: depends if grayed, and if active.
		NLMISC::CRGBA	getDrawnHeaderColor () const;

		ucstring		getUCTitleOpened () const;
		void			setUCTitleOpened (const ucstring &title);
		ucstring		getUCTitleClosed () const;
		void			setUCTitleClosed (const ucstring &title);
		ucstring		getUCTitle () const;
		void			setUCTitle (const ucstring &title);

		void			setPopable(bool popable) { _Popable = popable; }
		bool			isPopable() const { return _Popable; }
		bool			isPopuped() const { return _Poped; }


		void			setMovableInParentList(bool /* movable */) { _MovableInParentList = true; }
		bool			isMovableInParentList() const { return _MovableInParentList; }

		// high light the border of the container
		void			setHighLighted(bool hightlighted, uint8 alpha=255) { _HighLighted = hightlighted; _HighLightedAlpha = alpha; }
		bool			isHighLighted() const { return	_HighLighted; }

		// y offset for content of container
		sint32			getContentYOffset() const { return (sint32) _ContentYOffset; }
		void			setContentYOffset(sint32 value);

		// Window requires attention
		void requireAttention();

		// Lua exports
		int luaBlink(CLuaState &ls);
		int luaSetHeaderColor(CLuaState &ls);
		int luaSetModalParentList(CLuaState &ls);

		REFLECT_EXPORT_START(CGroupContainer, CGroupContainerBase)
			REFLECT_LUA_METHOD("blink", luaBlink);
			REFLECT_LUA_METHOD("setHeaderColor", luaSetHeaderColor);
			REFLECT_LUA_METHOD("setModalParentList", luaSetModalParentList);
			
			REFLECT_STRING("title", getTitle, setTitle);
			REFLECT_STRING("title_opened", getTitleOpened, setTitleOpened);
			REFLECT_STRING("title_closed", getTitleClosed, setTitleClosed);
			REFLECT_UCSTRING("uc_title_opened", getUCTitleOpened, setUCTitleOpened);
			REFLECT_UCSTRING("uc_title_closed", getUCTitleClosed, setUCTitleClosed);
			REFLECT_UCSTRING("uc_title", getUCTitle, setUCTitle);
			REFLECT_STRING("title_color", getTitleColorAsString, setTitleColorAsString);
			REFLECT_SINT32("pop_min_h", getPopupMinH, setPopupMinH);
			REFLECT_SINT32("pop_max_h", getPopupMaxH, setPopupMaxH);
			REFLECT_SINT32("pop_min_w", getPopupMinW, setPopupMinW);
			REFLECT_SINT32("pop_max_w", getPopupMaxW, setPopupMaxW);
			REFLECT_SINT32("title_delta_max_w", getTitleDeltaMaxW, setTitleDeltaMaxW);
			REFLECT_SINT32("content_y_offset", getContentYOffset, setContentYOffset);
			REFLECT_BOOL("openable", isOpenable, setOpenable);
			REFLECT_BOOL("opened", isOpen, setOpen);
			REFLECT_BOOL("lockable", isLockable, setLockable);
			REFLECT_BOOL("locked", isLocked, setLocked);

			REFLECT_BOOL("header_active", getHeaderActive, setHeaderActive);
			REFLECT_BOOL("right_button_enabled", getRightButtonEnabled, setRightButtonEnabled);
		REFLECT_EXPORT_END

		sint32			getLayerSetup() const { return _LayerSetup; }

		// if this window is popable, pop it at its actual position
		void			popupCurrentPos();
		// Popup at previous memorized position
		void			popup();
		/** Popin the window and possibly put it back in its father container, using the order defined in the list of the container.
		  * \param putBackInFather When true, put the window back in its former father container, otherwise, the container is unliked from the hierachy (parents are NULL)
		  * \param insertPos If this is equal to -1, then the window is inserted at its previous position. Otherwise it is inserted before the given position in the list
		  */
		void			popin(sint32 insertPos = -1, bool putBackInFatherContainer = true);

		// get the mover control associated with that control, or NULL if none
		CCtrlMover	   *getCtrlMover() const { return _Mover; }

		// true if there is a mover and if the window is being moved
		bool		   isMoving() const { return _Mover && _Mover->isMoving(); }

		/** Force the container to blink (to tell the user that an event has happened).
		  * This  uses the global color, so the container must use it
		  * This state is automatically disabled if the container is opened
		  * \param numBlinks 0 If the container should blink endlessly, the number of blink otherwise
		  */
		virtual void	enableBlink(uint numBlinks = 0);
		virtual void    disableBlink();
		virtual bool	isBlinking() const { return _Blinking; }

		CGroupList      *getList() const { return _List; }

		CInterfaceGroup *getHeaderOpened()	const { return _HeaderOpened; }
		CInterfaceGroup *getHeaderClosed()	const { return _HeaderClosed; }
		CInterfaceGroup *getContent()		const { return _Content; }

		void			setChildrenObs(IChildrenObs *obs) { _ChildrenObs = obs; }
		IChildrenObs   *getChildrenObs() const { return _ChildrenObs; }

		// Get current father container (if any).
		CGroupContainer  *getFatherContainer() const;
		// Get current father container (if any). If the container is popup, it gives the proprietary container
		CGroupContainer  *getProprietaryContainer() const;


		bool			isOpenable() const { return _Openable; }
		void			setOpenable(bool openable);

		bool			getHeaderActive() const { return _HeaderActive; }
		void			setHeaderActive(bool active) { _HeaderActive = active; }

		bool			getRightButtonEnabled() const { return _EnabledRightButton; }
		void			setRightButtonEnabled(bool enabled);

		CCtrlScroll     *getScroll() const { return _ScrollBar; }

		bool             isSavable() const { return _Savable; }
		void             setSavable(bool savable) { _Savable = savable; }
		bool             isActiveSavable() const { return _ActiveSavable; }

		bool isLocalize() const { return _Localize; }
		void setLocalize(bool localize) { _Localize = localize; }

		void setPopupX(sint32 x) { _PopupX = x; }
		void setPopupY(sint32 y) { _PopupY = y; }
		void setPopupW(sint32 w) { _PopupW = w; }
		void setPopupH(sint32 h) { _PopupH = h; }

		sint32 getPopupX() const { return _PopupX; }
		sint32 getPopupY() const { return _PopupY; }
		sint32 getPopupW() const { return _PopupW; }
		sint32 getPopupH() const { return _PopupH; }

		sint32 getRefW() const { return _RefW; }

		/** Increase the rollover alpha for the current frame.
		  * Example of use : an edit box that has focus in a group container
		  */
		void	rollOverAlphaUp();
		// force the rollover alpha to its max value, depending on there's keyboard focus or not
		void    forceRolloverAlpha();

		bool	isOpenWhenPopup() const { return _OpenWhenPopup; }

		/// Locking of window (prevent it from being moved)
		void	setLockable(bool lockable);
		bool	isLockable() const { return _Lockable; }
		void	setLocked(bool locked);

		// to be called by the 'deactive check' handler
		static  void validateCanDeactivate(bool validate) { _ValidateCanDeactivate = validate; }
		const	std::string &getAHOnDeactiveCheck() const { return CAHManager::getInstance()->getAHName(_AHOnDeactiveCheck); }
		const	std::string &getAHOnDeactiveCheckParams() const { return _AHOnDeactiveCheckParams; }
		//
		const	std::string &getAHOnCloseButton() const { return CAHManager::getInstance()->getAHName(_AHOnCloseButton); }
		const	std::string &getAHOnCloseButtonParams() const { return _AHOnCloseButtonParams; }
		//
		IActionHandler	*getAHOnMovePtr() const { return _AHOnMove; }
		const	std::string &getAHOnMove() const { return CAHManager::getInstance()->getAHName(_AHOnMove); }
		const	std::string &getAHOnMoveParams() const { return _AHOnMoveParams; }
		//
		IActionHandler	*getAHOnResizePtr() const { return _AHOnResize; }
		const	std::string &getAHOnResize() const { return CAHManager::getInstance()->getAHName(_AHOnResize); }
		const	std::string &getAHOnResizeParams() const { return _AHOnResizeParams; }
		//
		IActionHandler	*getAHOnBeginMovePtr() const { return _AHOnBeginMove; }
		const	std::string &getAHOnBeginMove() const { return CAHManager::getInstance()->getAHName(_AHOnBeginMove); }
		const	std::string &getAHOnBeginMoveParams() const { return _AHOnBeginMoveParams; }

		//
		void	setOnCloseButtonHandler(const std::string &h) { _AHOnCloseButton = CAHManager::getInstance()->getAH(h,_AHOnCloseButtonParams); }
		void	setOnCloseButtonParams(const std::string &p) { _AHOnCloseButtonParams = p; }

		void setModalParentList (const std::string &name);
		bool checkIfModal(const NLGUI::CEventDescriptor& event); // Return true if we can handle the event (and prevent from selecting a window)
		bool isGrayed() const;
		bool blinkAllSons();

		// true if the resizer is enabled.
		bool getEnabledResizer() const {return _EnabledResizer;}

		sint32				getPopupMinW() const {return _PopupMinW;}
		sint32				getPopupMaxW() const {return _PopupMaxW;}
		sint32				getPopupMinH() const {return _PopupMinH;}
		sint32				getPopupMaxH() const {return _PopupMaxH;}
		sint32				getMinW() const {return _MinW;}
		void				setMinW(sint32 minW) { _MinW = minW;}
		void				setMaxW(sint32 maxW) { _MaxW = maxW;}
		sint32				getMaxW() const {return _MaxW;}
		void				setPopupMinW(sint32 minW);
		void				setPopupMaxW(sint32 maxW);
		void				setPopupMinH(sint32 minW);
		void				setPopupMaxH(sint32 maxW);


		// backup the current position of this container
		void				backupPosition();
		// restore the current position of this container
		void				restorePosition();
		// get x for backup position
		sint32				getBackupX() const { return _BackupX; }
		sint32				getBackupY() const { return _BackupY; }
		// Set backup position
		void				setBackupPosition(sint32 x, sint32 y);
		// clear backup
		void				clearBackup() {	_PositionBackuped = false; }
		// Test if position has been backuped (flag cleared by 'restorePosition()')
		bool				isPositionBackuped() const { return _PositionBackuped; }
		// check if the container has been moved, resized, or popuped by the user (and eventually clear that flag)
		bool                getTouchFlag(bool clearFlag) const;
		// from CInterfaceGroup
		virtual void		restoreAllContainersBackupPosition() { restorePosition(); }

		// when isModal() is true, the whole interface cannot switch desktop
		bool				isModal() const { return _Modal; }
		void				setModal(bool modal) { _Modal = modal; }

		// return true if the container has a modal parent window setuped => the whole interface cannot switch desktop
		bool				isModalSon() const { return !_ModalParents.empty(); }

		// return the help web page of this container. "" if none
		const	std::string &getHelpPage() const { return _HelpPage; }
		// set the help web page of this container. "" if none. NB: the help button is not updated
		void				setHelpPage(const std::string &newPage);

		void				setTitleDeltaMaxW(sint32 delta) { _TitleDeltaMaxW = delta; }
		sint32				getTitleDeltaMaxW() const { return _TitleDeltaMaxW; }

	protected:
		uint8				_ICurrentRolloverAlphaContainer;
		uint8				_HighLightedAlpha;
		float				_CurrentRolloverAlphaContainer;
		float				_CurrentRolloverAlphaContent;
		sint32				_LayerSetup;
		ucstring			_TitleTextOpened;
		ucstring			_TitleTextClosed;
		CViewText			*_TitleOpened;
		CViewText			*_TitleClosed;
		sint32				_TitleDeltaMaxW;
		CViewBitmap			*_ViewOpenState;	// Arrow showing if we are opened or not (if we are openable)
		CCtrlButton			*_RightButton;		// Multi usage button : deactive or popup or popin
		CCtrlButton			*_HelpButton;		// Help button

		CGroupList			*_List;
		CCtrlScroll			*_ScrollBar;
		CGroupContainer     *_OldFatherContainer;

		// NB: _ModalParentNames is a list of modal parent, separated by '|'
		std::string			_ModalParentNames;			// Modal handling between container (container can be linked together,
		std::vector<CGroupContainer*>	_ModalSons;		// when the son is active the parent is not active
		std::vector<CGroupContainer*>	_ModalParents;	// (but the rest of the interface is))

		uint				 _InsertionOrder;
		uint                 _BlinkDT;
		uint				 _NumBlinks;

		CInterfaceGroup		*_Content;			// Read From Script
		CInterfaceGroup		*_HeaderOpened;		// Read From Script
		CInterfaceGroup		*_HeaderClosed;		// Read From Script

		CCtrlResizer		*_Resizer[NumResizers]; // up to 8 resizers are available

		//
		CCtrlMover			*_Mover;

		IChildrenObs		*_Obs;

		// If layer==0 constraint on resize
		sint32				_PopupMinW;
		sint32				_PopupMaxW;
		sint32				_PopupMinH;
		sint32				_PopupMaxH;
		// If layer>0 constraint on resize
		sint32				_MinW;
		sint32				_MaxW;

		// backuped position
		sint32				_BackupX;
		sint32				_BackupY;

		// old position at which the window was popup, -1 values means that the window hasn't been turned into a popup yet
		sint32              _PopupX;
		sint32              _PopupY;
		sint32              _PopupW;
		sint32              _PopupH;
		//
		sint32				_RefW;


		sint32				_MoverDeltaW;

		// action handler
		IActionHandler		*_AHOnOpen;
		CStringShared		_AHOnOpenParams;
		IActionHandler		*_AHOnClose;
		CStringShared		_AHOnCloseParams;
		IActionHandler		*_AHOnCloseButton;
		CStringShared		_AHOnCloseButtonParams;
		IActionHandler		*_AHOnMove;
		CStringShared		_AHOnMoveParams;
		IActionHandler		*_AHOnResize;
		CStringShared		_AHOnResizeParams;
		IActionHandler		*_AHOnBeginMove;
		CStringShared		_AHOnBeginMoveParams;

		// action handler to test whether the windows can be deactivated (when the close button is pressed)
		IActionHandler		*_AHOnDeactiveCheck;
		CStringShared		_AHOnDeactiveCheckParams;


		// Observer to know when children have moved
		IChildrenObs	   *_ChildrenObs;

		// list of container that are poped up
		std::vector<CGroupContainer *> _PopedCont;

		// Open management
		bool                _Openable			: 1; // Is the container can be manually opened or closed ?
		bool				_Opened				: 1; // Is the container currently opened or closed ?
		bool				_OpenWhenPopup		: 1; // Does the container must open when poped up ? (layer>0)
													 // and close when poped in...
		bool				_OpenAtStart		: 1; // Mgt : to setup _Opened state at start
		bool				_OpenedBeforePopup	: 1; // Mgt : Is the container opened before poped up ? (layer>0)

		// Move management
		bool				_Movable			: 1; // Is the container movable ?
		bool				_MovableInParentList: 1;
		bool				_Lockable			: 1;
		bool				_MovingInParentList	: 1; // Mgt : currently moving ?

		// Pop up / pop in
		bool                _Popable			: 1;
		bool                _Poped				: 1;

		bool				_EnabledResizer		: 1;

		bool				_HighLighted		: 1;
		bool				_Blinking			: 1;
		bool				_BlinkState			: 1;

		bool                _Savable			: 1;
		bool                _ActiveSavable		: 1;

		// Display title background or not
		bool				_HeaderActive		: 1;
		bool				_EnabledRightButton	: 1; // Is the Button Deactive/Popup/Popin is enabled ?
		//
		enum	TTileClass {TitleText=0, TitleTextFormated, TitleTextId, TitleTextDynString};
		uint8				_TitleClass			: 2;
		//
		mutable bool		_TouchFlag          : 1;
		bool				_PositionBackuped   : 1;
		bool				_Modal              : 1; // the container is modal and prevent from switching virtual desktop
		//
		bool				_EnabledHelpButton	: 1; // Is the Button Help is enabled ?
		//
		bool				_TitleOverExtendViewText : 1;	// Does the title over extend view text
		bool				_Localize		    : 1;

		CInterfaceProperty	_HeaderColor;



		sint8				_ContentYOffset;

		// Special Top Resizer Height (for Inventory and ChatGroup). <0 (default) => take default option value
		sint8				_ResizerTopSize;
		uint8				_ICurrentRolloverAlphaContent;


		static bool			_ValidateCanDeactivate;

		CStringShared		_OptionsName;

		// Web Page used for help
		CStringShared		_HelpPage;

	private:

		sint32	getLayer();
		void	updateResizerSize(CCtrlResizer *cr);
		void	updateRightButton();
		void	updateHelpButton();
		void	updateMover();
		void	updateViewOpenState();
		void	updateTitle();

		void	createResizer(uint index, THotSpot posRef, THotSpot type, sint32 offsetX, sint32 offsetY, bool bMaxH);
		void	createResizerMaxH();
		void	removeResizerMaxH();

		TTileClass	convertTitleClass(const char *ptr);

		static COptionsContainerMove *getMoveOptions();

		COptionsLayer *getContainerOptions(sint32 ls=-1); // Depends if overload by OptionsName or default used

		bool hasKeyboardFocus() const;

		// private for modal system
		void addModalParent (CGroupContainer *pParent);
		void addModalSon (CGroupContainer *pSon);

		// Avoid each frame setup layer0, layer1 etc...
		enum	{NumLayerName=10};
		static	const std::string		_OptionLayerName[NumLayerName];

	public:
		// for use by CCtrlMover
		// Tell that this group is moving in its parent list
		void            setMovingInParentList(bool enable);
		CGroupList     *getPreviousParentList() const { return _OldFatherContainer ? _OldFatherContainer->_List : NULL; }
		CCtrlScroll    *getPreviousParentScrollBar() const { return _OldFatherContainer ? _OldFatherContainer->_ScrollBar : NULL; }
		CGroupContainer *getPreviousContainer() const { return 	_OldFatherContainer; }
		// set the 'hasMoved' flag
		void			touch(bool touched = true) { _TouchFlag = touched; }

		friend class CICDeactive;
	};

}

#endif // RZ_INTERFACE_CONTAINER_H

/* End of interface_container.h */

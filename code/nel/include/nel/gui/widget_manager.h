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


#ifndef WIDGET_MANAGER_H
#define WIDGET_MANAGER_H

#include <string>
#include <vector>
#include <list>
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/types_nl.h"
#include "nel/gui/interface_common.h"
#include "nel/gui/interface_options.h"
#include "nel/gui/event_descriptor.h"
#include "nel/3d/u_camera.h"
#include "nel/gui/parser.h"
#include "nel/gui/input_event_listener.h"

namespace NLMISC
{
	class CCDBNodeLeaf;
}

namespace NLGUI
{

	class CInterfaceElement;
	class CCtrlBase;
	class CViewBase;
	class CInterfaceGroup;
	class CViewPointerBase;
	class CInterfaceOptions;
	class CInterfaceAnim;
	class CProcedure;
	class IEditorSelectionWatcher;
	class IWidgetAdditionWatcher;

	/**
	 GUI Widget Manager

	 Manages the GUI widgets, asks them to draw themselves, etc.
	 */
	class CWidgetManager : public IInputEventListener{

	public:

		/// Interface for event handlers that can be called when the screen is resized.
		class INewScreenSizeHandler
		{
		public:
			virtual ~INewScreenSizeHandler(){}
			virtual void process( uint32 w, uint32 h ) = 0;
		};

		/// Interface for event handlers that can be called when the widgets finished drawing.
		class IOnWidgetsDrawnHandler
		{
		public:
			virtual ~IOnWidgetsDrawnHandler(){};
			virtual void process() = 0;
		};

		// Interface for event handlers that can be called when widgets are added or moved
		class IWidgetWatcher
		{
		public:
			IWidgetWatcher(){}
			virtual ~IWidgetWatcher(){}
			virtual void onWidgetAdded( const std::string &name ) = 0;
			virtual void onWidgetMoved( const std::string &oldid, const std::string &newid ) = 0;
		};

		/// Frame render times
		struct SInterfaceTimes
		{
		public:
			/// Time when the last frame was rendered in ms.
			sint64 lastFrameMs;
			/// Time when the current frame was rendered in ms.
			sint64 thisFrameMs;
			/// Difference between the two times in ms.
			sint64 frameDiffMs;

			SInterfaceTimes()
			{
				lastFrameMs = 0;
				thisFrameMs = 0;
				frameDiffMs = 0;
			}
		};

		// Master groups encapsulate all windows
		struct SMasterGroup
		{
			SMasterGroup()
			{
				Group = NULL;
				LastTopWindowPriority = WIN_PRIORITY_NORMAL;
			}

			CInterfaceGroup *Group;
			std::list< CInterfaceGroup* > PrioritizedWindows[ WIN_PRIORITY_MAX ];

			void addWindow( CInterfaceGroup *pIG, uint8 nPrio = WIN_PRIORITY_NORMAL );
			void delWindow( CInterfaceGroup *pIG );
			CInterfaceGroup *getWindowFromId( const std::string &winID );
			bool isWindowPresent( CInterfaceGroup *pIG );
			// Set a window top in its priority queue
			void setTopWindow( CInterfaceGroup *pIG );
			void setBackWindow( CInterfaceGroup *pIG );
			void deactiveAllContainers();
			void centerAllContainers();
			void unlockAllContainers();

			// Sort the world space group
			void sortWorldSpaceGroup ();

			uint8 LastTopWindowPriority;
		};


		// Infos about a modal window.
		struct SModalWndInfo
		{
			// Yoyo: store as CRefPtr in case they are deleted (can happen for instance if menu right click on a guild memeber, and guild members are udpated after)
			NLMISC::CRefPtr< CInterfaceGroup > ModalWindow; // the current modal window
			NLMISC::CRefPtr< CCtrlBase > CtrlLaunchingModal;
			bool ModalClip;
			bool ModalExitClickOut;
			bool ModalExitClickL;
			bool ModalExitClickR;
			bool ModalExitKeyPushed;
			std::string ModalHandlerClickOut;
			std::string ModalClickOutParams;

			SModalWndInfo()
			{
				ModalWindow = NULL;
				CtrlLaunchingModal = NULL;
				ModalExitClickOut = false;
				ModalExitClickL = false;
				ModalExitClickR = false;
				ModalExitKeyPushed = false;
			}
		};


		static CWidgetManager* getInstance();
		static void release();
		
		CInterfaceGroup* getMasterGroupFromId( const std::string &MasterGroupName );
		std::vector< SMasterGroup > &getAllMasterGroup(){ return _MasterGroups; }
		SMasterGroup& getMasterGroup( uint8 i ) { return _MasterGroups[ i ]; }
		CInterfaceGroup* getWindowFromId( const std::string &groupId );
		void addWindowToMasterGroup( const std::string &sMasterGroupName, CInterfaceGroup *pIG );
		void removeWindowFromMasterGroup( const std::string &sMasterGroupName, CInterfaceGroup *pIG );
		void removeAllMasterGroups();

		void activateMasterGroup (const std::string &sMasterGroupName, bool bActive);

		CInterfaceElement* getElementFromId( const std::string &sEltId );
		CInterfaceElement* getElementFromId( const std::string &sStart, const std::string &sEltId );

		/**
		 * get a window from its  Id of its group.
		 *	NB: "ctrl_launch_modal" is a special Id which return the last ctrl which has launch a modal. NULL if modal closed.
		 * \param groupId : the Id of the window group
		 */
		/// get an element from a define ID. shortcut for getElementFromId(getDefine(define))
		CInterfaceElement* getElementFromDefine( const std::string &defineId );

		/// Get the window from an element (ui:interface:###)
		CInterfaceGroup* getWindow(CInterfaceElement*);


		/**
		 * set the top window
		 * \param win : pointer to the window to be set on top
		 */
		void setTopWindow (CInterfaceGroup *pWin);

		/**
		 * set the back window
		 * \param win : pointer to the window to be set on top
		 */
		void setBackWindow (CInterfaceGroup *pWin);

		/** get the top window in the first activated masterGroup
		 */
		CInterfaceGroup* getTopWindow (uint8 nPriority = WIN_PRIORITY_NORMAL) const;

		/** get the back window in the first activated masterGroup
		 */
		CInterfaceGroup* getBackWindow (uint8 nPriority = WIN_PRIORITY_NORMAL) const;

		/** get the last escapable top window in the first activated masterGroup
		 */
		CInterfaceGroup* getLastEscapableTopWindow() const;

		void setWindowPriority (CInterfaceGroup *pWin, uint8 nPriority);

		/** return the priority of the Last Window setTopWindow()-ed.
		 */
		uint8 getLastTopWindowPriority() const;

		bool hasModal() const;

		SModalWndInfo& getModal();

		bool isPreviousModal( CInterfaceGroup *wnd ) const;

		void enableModalWindow (CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG);
		void enableModalWindow (CCtrlBase *ctrlLaunchingModal, const std::string &groupName);
		// Disable all modals windows
		void disableModalWindow ();

		/** Push a modal window that becomes the current modal window
		  */
		void pushModalWindow(CCtrlBase *ctrlLaunchingModal, CInterfaceGroup *pIG);
		void pushModalWindow (CCtrlBase *ctrlLaunchingModal, const std::string &groupName);
		void popModalWindow();
		// pop all top modal windows with the given category (a string stored in the modal)
		void popModalWindowCategory(const std::string &category);
		
		void hideAllWindows();
		void hideAllNonSavableWindows();

		CCtrlBase *getCtrlLaunchingModal ()
		{
			if (_ModalStack.empty()) return NULL;
			return _ModalStack.back().CtrlLaunchingModal;
		}
		/// get the currently active modal window, or NULL if none
		CInterfaceGroup *getModalWindow() const
		{
			if (_ModalStack.empty()) return NULL;
			return _ModalStack.back().ModalWindow;
		}

		void setCurContextHelp( CCtrlBase *curContextHelp ){ this->curContextHelp = curContextHelp; }
		CCtrlBase* getCurContextHelp(){ return curContextHelp; }

		float _DeltaTimeStopingContextHelp;
		float _MaxTimeStopingContextHelp;
		sint _LastXContextHelp;
		sint _LastYContextHelp;

		CViewPointerBase* getPointer(){ return _Pointer; }
		void setPointer( CViewPointerBase *pointer ){ _Pointer = pointer; }

		/**
		 * get the window under a spot
		 * \param : X coord of the spot
		 * \param : Y coord of the spot
		 * \return : pointer to the window
		 */
		CInterfaceGroup* getWindowUnder (sint32 x, sint32 y);
		CInterfaceGroup* getCurrentWindowUnder() { return _WindowUnder; }
		void setCurrentWindowUnder( CInterfaceGroup *group ){ _WindowUnder = group; }
		CInterfaceGroup* getGroupUnder (sint32 x, sint32 y);

		void getViewsUnder( sint32 x, sint32 y, std::vector< CViewBase* > &vVB );
		void getCtrlsUnder( sint32 x, sint32 y, std::vector< CCtrlBase* > &vICL );
		void getGroupsUnder (sint32 x, sint32 y, std::vector< CInterfaceGroup* > &vIGL );

		const std::vector< CViewBase* >& getViewsUnderPointer(){ return _ViewsUnderPointer; }
		const std::vector< CInterfaceGroup* >& getGroupsUnderPointer() { return _GroupsUnderPointer; }
		const std::vector< CCtrlBase* >& getCtrlsUnderPointer() { return _CtrlsUnderPointer; }
		
		//
		void clearViewUnders(){ _ViewsUnderPointer.clear(); }
		void clearGroupsUnders() { _GroupsUnderPointer.clear(); }
		void clearCtrlsUnders() { _CtrlsUnderPointer.clear(); }

		// Remove all references on a view (called when the ctrl is destroyed)
		void removeRefOnView( CViewBase *ctrlBase );
		
		// Remove all references on a ctrl (called when the ctrl is destroyed)
		void removeRefOnCtrl (CCtrlBase *ctrlBase);

		// Remove all references on a group (called when the group is destroyed)
		void removeRefOnGroup (CInterfaceGroup *group);

		void reset();

		void checkCoords();
		
		CInterfaceGroup* getWindowForActiveMasterGroup( const std::string &windowName );
		
		void drawOverExtendViewText();

		// Internal : adjust a tooltip with respect to its parent. Returns the number of coordinate that were clamped
		// against the screen border
		uint adjustTooltipPosition( CCtrlBase *newCtrl, CInterfaceGroup *win, THotSpot ttParentRef,
									THotSpot ttPosRef, sint32 xParent, sint32 yParent,
									sint32 wParent, sint32 hParent );
		
		void updateTooltipCoords();
		
		// Update tooltip coordinate if they need to be (getInvalidCoords() returns a value != 0)
		void updateTooltipCoords(CCtrlBase *newCtrl);
		
		/// for ContextHelp action handler only: set the result name
		void setContextHelpText( const ucstring &text ){ _ContextHelpText = text; }
		ucstring& getContextHelpText(){ return _ContextHelpText; }
		
		/// force disable the context help
		void disableContextHelp();
		
		/// force disable the context help, if it is issued from the given control
		void disableContextHelpForControl(CCtrlBase *pCtrl);
		
		CCtrlBase* getNewContextHelpCtrl();

		void drawContextHelp();
		
		void setContextHelpActive(bool active);
		
		void getNewWindowCoordToNewScreenSize( sint32 &x, sint32 &y, sint32 w, sint32 h,
												sint32 newW, sint32 newH) const;
		
		// move windows according to new screen size
		void moveAllWindowsToNewScreenSize(sint32 newScreenW, sint32 newScreenH, bool fixCurrentUI );
		
		void updateAllLocalisedElements();

		void drawViews( NL3D::UCamera camera );

		bool handleEvent( const CEventDescriptor &evnt );

		bool handleSystemEvent( const CEventDescriptor &evnt );

		bool handleKeyboardEvent( const CEventDescriptor &evnt );

		bool handleMouseEvent( const CEventDescriptor &evnt );

		bool handleMouseMoveEvent( const CEventDescriptor &eventDesc );

		// Relative move of pointer
		void movePointer (sint32 dx, sint32 dy);
		// Set absolute coordinates of pointer
		void movePointerAbs(sint32 px, sint32 py);

		/**
		 * Capture
		 */
		CViewBase *getCapturedView(){ return _CapturedView; }
		CCtrlBase *getCapturePointerLeft() { return _CapturePointerLeft; }
		CCtrlBase *getCapturePointerRight() { return _CapturePointerRight; }
		CCtrlBase *getCaptureKeyboard() { return _CaptureKeyboard; }
		CCtrlBase *getOldCaptureKeyboard() { return _OldCaptureKeyboard; }
		CCtrlBase *getDefaultCaptureKeyboard() { return _DefaultCaptureKeyboard; }

		void setCapturePointerLeft(CCtrlBase *c);
		void setCapturePointerRight(CCtrlBase *c);
		void setOldCaptureKeyboard(CCtrlBase *c){ _OldCaptureKeyboard = c; }
		// NB: setCaptureKeyboard(NULL) has not the same effect as resetCaptureKeyboard(). it allows the capture
		// to come back to the last captured window (resetCaptureKeyboard() not)
		void setCaptureKeyboard(CCtrlBase *c);
		/**  Set the default box to use when no keyboard has been previously captured
		  *  The given dialog should be static
		  */
		void setDefaultCaptureKeyboard(CCtrlBase *c){ _DefaultCaptureKeyboard = c; }

		void resetCaptureKeyboard();

		// True if the keyboard is captured
		bool isKeyboardCaptured() const {return _CaptureKeyboard!=NULL;}

		// register a view that wants to be notified at each frame (receive the msg 'clocktick')
		void registerClockMsgTarget(CCtrlBase *vb);
		void unregisterClockMsgTarget(CCtrlBase *vb);
		bool isClockMsgTarget(CCtrlBase *vb) const;
		void sendClockTickEvent();

		void notifyElementCaptured(CCtrlBase *c);

		// Add a group into the windows list of its master goup
		void makeWindow( CInterfaceGroup *group );

		// Remove a group from the windows list of its master group
		void unMakeWindow( CInterfaceGroup *group, bool noWarning = false );

		void setGlobalColor( NLMISC::CRGBA col );
		NLMISC::CRGBA getGlobalColor() const{ return _GlobalColor; }

		void setContentAlpha( uint8 alpha );
		uint8 getContentAlpha() const{ return _ContentAlpha; }

		NLMISC::CRGBA getGlobalColorForContent() const { return _GlobalColorForContent; }
		void setGlobalColorForContent( NLMISC::CRGBA col ){ _GlobalColorForContent = col; }
		void resetColorProps();

		/// Get options by name
		CInterfaceOptions* getOptions( const std::string &optName );
		void addOptions( std::string name, CInterfaceOptions *options );
		void removeOptions( std::string name );
		void removeAllOptions();
		bool serializeOptions( xmlNodePtr parentNode ) const;
		bool serializeTreeData( xmlNodePtr parentNode ) const;
		
		// Enable mouse Events to interface. if false, release Captures.
		void enableMouseHandling( bool handle );
		bool isMouseHandlingEnabled() const{ return _MouseHandlingEnabled; }
		bool isMouseOverWindow() const{ return _MouseOverWindow; }
		void setMouseOverWindow( bool b ){ _MouseOverWindow = b; }
		
		// Get the User DblClick Delay (according to save...), in milisecond
		uint getUserDblClickDelay();
		
		/// \name Global Interface Options
		// @{
		
		// List of system options
		enum TSystemOption{
			OptionCtrlSheetGrayColor=0,
			OptionCtrlTextGrayColor,
			OptionCtrlSheetRedifyColor,
			OptionCtrlTextRedifyColor,
			OptionCtrlSheetGreenifyColor,
			OptionCtrlTextGreenifyColor,
			OptionViewTextOverBackColor,
			OptionFont,
			OptionAddCoefFont,
			OptionMulCoefAnim,
			OptionTimeoutBubbles,
			OptionTimeoutMessages,
			OptionTimeoutContext,
			OptionTimeoutContextHtml,
			NumSystemOptions
		};
		
		void setupOptions();
		/** Get a system option by its enum (faster than getOptions() and getVal())
		 *  NB: array updated after each parseInterface()
		 */
		const CInterfaceOptionValue	&getSystemOption( TSystemOption o ) const{ return _SystemOptions[ o ]; }
		
		// @}

		CInterfaceElement* getOverExtendViewText(){ return _OverExtendViewText; }
		NLMISC::CRGBA& getOverExtendViewTextBackColor(){ return _OverExtendViewTextBackColor; }

		// For single lined ViewText that are clipped: on over of viewText too big, the text is drawn on top. A CRefPtr is kept
		void setOverExtendViewText( CInterfaceElement *vt, NLMISC::CRGBA backGround ){
			_OverExtendViewText = vt;
			_OverExtendViewTextBackColor = backGround;
		}

		float getAlphaRolloverSpeed();
		void resetAlphaRolloverSpeedProps();

		void setContainerAlpha( uint8 alpha );
		uint8 getContainerAlpha() const { return _ContainerAlpha; }
		uint8 getGlobalContentAlpha() const { return _GlobalContentAlpha; }
		uint8 getGlobalContainerAlpha() const { return _GlobalContainerAlpha; }
		uint8 getGlobalRolloverFactorContent() const { return _GlobalRolloverFactorContent; }
		uint8 getGlobalRolloverFactorContainer() const { return _GlobalRolloverFactorContainer; }

		void updateGlobalAlphas();
		void resetGlobalAlphasProps();

		const SInterfaceTimes& getInterfaceTimes() const{ return interfaceTimes; }
		void updateInterfaceTimes( const SInterfaceTimes &times ){ interfaceTimes = times; }

		void setIngame( bool i ){ inGame = i; }
		bool isIngame() const{ return inGame; }

		void setScreenWH( uint32 w, uint32 h ){ screenW = w; screenH = h; }

		void registerNewScreenSizeHandler( INewScreenSizeHandler *handler );
		void removeNewScreenSizeHandler( INewScreenSizeHandler *handler );

		void registerOnWidgetsDrawnHandler( IOnWidgetsDrawnHandler* handler );
		void removeOnWidgetsDrawnHandler( IOnWidgetsDrawnHandler *handler );
		
		void startAnim( const std::string &animId );
		void stopAnim( const std::string &animId );
		void updateAnims();
		void removeFinishedAnims();
		
		// execute a procedure. give a list of parameters. NB: the first param is the name of the proc (skipped)...
		void runProcedure( const std::string &procName, CCtrlBase *pCaller, const std::vector< std::string > &paramList );
		// replace an action in a procedure (if possible)
		void setProcedureAction( const std::string &procName, uint actionIndex, const std::string &ah, const std::string &params );

		const CEventDescriptorKey& getLastKeyEvent() const{ return lastKeyEvent; }

		IParser* getParser() const{ return parser; }

		/// Retrieves the Id of the currently selected widgets
		void getEditorSelection( std::vector< std::string > &selection );

		/// Adds the widget with the specified Id to the selected widgets
		void selectWidget( const std::string &name );

		/// Clears the selection
		void clearEditorSelection();

		void notifySelectionWatchers();
		void registerSelectionWatcher( IEditorSelectionWatcher *watcher );
		void unregisterSelectionWatcher( IEditorSelectionWatcher *watcher );


		void onWidgetAdded( const std::string &id );
		void onWidgetMoved( const std::string &oldid, const std::string &newid );
		void registerWidgetWatcher( IWidgetWatcher *watcher );
		void unregisterWidgetWatcher( IWidgetWatcher *watcher );

		CInterfaceElement* addWidgetToGroup( std::string &group, std::string &widgetClass, std::string &widgetName );

		void setGroupSelection( bool b ){ _GroupSelection = b; }
		bool groupSelection();
		bool unGroupSelection();
		void setMultiSelection( bool b ){ multiSelection = b; }
				
	private:
		CWidgetManager();
		~CWidgetManager();

		IParser *parser;

		static CWidgetManager *instance;
		std::vector< SMasterGroup > _MasterGroups;
		std::vector< SModalWndInfo > _ModalStack;
		static std::string _CtrlLaunchingModalId;
		NLMISC::CRefPtr< CCtrlBase > curContextHelp;
		CViewPointerBase *_Pointer;

		// Options description
		std::map< std::string, NLMISC::CSmartPtr< CInterfaceOptions > > _OptionsMap;

		NLMISC::CRefPtr< CInterfaceGroup > _WindowUnder;

		// Capture
		NLMISC::CRefPtr<CCtrlBase>	_CaptureKeyboard;
		NLMISC::CRefPtr<CCtrlBase>	_OldCaptureKeyboard;
		NLMISC::CRefPtr<CCtrlBase>	_DefaultCaptureKeyboard;
		NLMISC::CRefPtr<CCtrlBase>	_CapturePointerLeft;
		NLMISC::CRefPtr<CCtrlBase>	_CapturePointerRight;

		NLMISC::CRefPtr< CViewBase > _CapturedView;

		NLMISC::CRefPtr< CInterfaceElement > draggedElement; // the element that we're currently dragging

		bool startDragging();
		void stopDragging();

		// What is under pointer
		std::vector< CViewBase* > _ViewsUnderPointer;
		std::vector< CCtrlBase* > _CtrlsUnderPointer;
		std::vector< CInterfaceGroup* > _GroupsUnderPointer;

		// view that should be notified from clock msg
		std::vector<CCtrlBase*> _ClockMsgTargets;

		NLMISC::CRGBA _GlobalColor;
		NLMISC::CRGBA _GlobalColorForContent;
		uint8 _ContentAlpha;

		NLMISC::CCDBNodeLeaf *_RProp;
		NLMISC::CCDBNodeLeaf *_GProp;
		NLMISC::CCDBNodeLeaf *_BProp;
		NLMISC::CCDBNodeLeaf *_AProp;
		NLMISC::CCDBNodeLeaf *_AlphaRolloverSpeedDB;

		NLMISC::CCDBNodeLeaf *_GlobalContentAlphaDB;
		NLMISC::CCDBNodeLeaf *_GlobalContainerAlphaDB;
		NLMISC::CCDBNodeLeaf *_GlobalContentRolloverFactorDB;
		NLMISC::CCDBNodeLeaf *_GlobalContainerRolloverFactorDB;
		
		uint8 _ContainerAlpha;
		uint8 _GlobalContentAlpha;
		uint8 _GlobalContainerAlpha;
		uint8 _GlobalRolloverFactorContent;
		uint8 _GlobalRolloverFactorContainer;

		bool _MouseHandlingEnabled;
		
		// System Options
		CInterfaceOptionValue _SystemOptions[ NumSystemOptions ];
		
		// The next ViewText to draw for Over
		NLMISC::CRefPtr< CInterfaceElement > _OverExtendViewText;
		NLMISC::CRGBA _OverExtendViewTextBackColor;

		SInterfaceTimes interfaceTimes;

		ucstring _ContextHelpText;
		bool _ContextHelpActive;

		bool inGame;
		
		bool _MouseOverWindow;

		CEventDescriptorKey lastKeyEvent;

		uint32 screenH;
		uint32 screenW;
		
		std::vector< CInterfaceAnim* > activeAnims;

		std::vector< INewScreenSizeHandler* > newScreenSizeHandlers;
		std::vector< IOnWidgetsDrawnHandler* > onWidgetsDrawnHandlers;
		std::vector< IEditorSelectionWatcher* > selectionWatchers;
		std::vector< IWidgetWatcher* > widgetWatchers;
		
		std::vector< std::string > editorSelection;
		bool _GroupSelection;
		bool multiSelection;
		uint32 _WidgetCount;
	};

}

#endif


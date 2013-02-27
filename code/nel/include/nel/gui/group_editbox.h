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



#ifndef RZ_CTRL_EDITBOX_H
#define RZ_CTRL_EDITBOX_H

#include "nel/gui/interface_group.h"
#include "nel/gui/group_editbox_base.h"
#include "nel/3d/u_texture.h"

namespace NLGUI
{
	class CEventDescriptor;
	class CViewText;

	// ----------------------------------------------------------------------------
	class CGroupEditBox : public CGroupEditBoxBase
	{
	public:
        DECLARE_UI_CLASS( CGroupEditBox )

		class IComboKeyHandler
		{
		public:
			virtual ~IComboKeyHandler(){}
			virtual bool isComboKeyChat( const NLGUI::CEventDescriptorKey &edk ) const = 0;
		};

		enum TEntryType { Text, Integer, PositiveInteger, Float, PositiveFloat, Alpha, AlphaNum, AlphaNumSpace, Password, Filename, PlayerName }; // the type of entry this edit bot can deal with

		/// Constructor
		CGroupEditBox(const TCtorParam &param);
		/// Dtor
		~CGroupEditBox();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		bool parse(xmlNodePtr cur,CInterfaceGroup * parentGroup);
		virtual uint32 getMemory() { return (uint32)(sizeof(*this)+_Id.size()); }

		virtual void draw();

		virtual bool handleEvent (const NLGUI::CEventDescriptor& eventDesc);

		/// Accessors
		ucstring getInputString() const { return _InputString; }
		const ucstring &getInputStringRef() const { return _InputString; }
		const ucstring &getPrompt() const { return _Prompt; }

		/** Set the prompt
		  * NB : line returns are encoded as '\n', not '\r\n'
		  */
		void		setPrompt(const ucstring &s) { _Prompt = s; }
		void		setInputString(const ucstring &str);
		void		setInputStringRef(const ucstring &str) {_InputString = str; };
		void		setInputStringAsInt(sint32 val);
		sint32		getInputStringAsInt() const;
		void		setInputStringAsInt64(sint64 val);
		sint64		getInputStringAsInt64() const;
		void		setInputStringAsFloat(float val);
		float		getInputStringAsFloat() const;
		void		setInputStringAsStdString(const std::string &str);
		std::string	getInputStringAsStdString() const;
		void		setInputStringAsUtf8(const std::string &str);
		std::string	getInputStringAsUtf8() const;
		void		setColor(NLMISC::CRGBA col);


		/// force the selection of all the text
		void		setSelectionAll();

		virtual void checkCoords();
		virtual void updateCoords();
		virtual void clearViews ();

		virtual void setActive (bool state);

		static CGroupEditBox *getMenuFather() { return _MenuFather; }

		void setCommand(const ucstring &command, bool execute);

		// Stop parent from blinking
		void		stopParentBlink() { if (_Parent) _Parent->disableBlink(); }

		// Get / set cursor position
		sint32		getCursorPos () const {return _CursorPos;}
		void		setCursorPos (sint32 pos) {_CursorPos=pos;}

		// Get / set cursor at previous line end
		bool		isCursorAtPreviousLineEnd () const {return _CursorAtPreviousLineEnd;}
		void		setCursorAtPreviousLineEnd (bool setCursor) {_CursorAtPreviousLineEnd=setCursor;}

		// Get / set current selection position
		static sint32	getSelectCursorPos () {return _SelectCursorPos;}
		static void		setSelectCursorPos (sint32 pos) {_SelectCursorPos=pos;}

		// Get the view text
		const CViewText	*getViewText () const {return _ViewText;}

		// Get the historic information
		sint32	getMaxHistoric() const {return _MaxHistoric;}
		sint32	getCurrentHistoricIndex () const {return _CurrentHistoricIndex;}
		void	setCurrentHistoricIndex (sint32 index) {_CurrentHistoricIndex=index;}
		const ucstring	&getHistoric(uint32 index) const {return _Historic[index];}
		uint32	getNumHistoric() const {return (uint32)_Historic.size ();}

		// Get on change action handler
		const std::string	&getAHOnChange() const {return _AHOnChange;}
		const std::string	&getParamsOnChange() const {return _ParamsOnChange;}

		void	cutSelection();

		/// From CInterfaceElement
		sint32	getMaxUsedW() const;
		sint32	getMinUsedW() const;

		// Copy the selection into buffer
		void		copy();
		// Paste the selection into buffer
		void		paste();
		// Write the string into buffer
		void		writeString(const ucstring &str, bool replace = true, bool atEnd = true);

		// Expand the expression (true if there was a '/' at the start of the line)
		bool		expand();

		// Back space
		void		back();

		// ignore the next char/key event -> useful when a key set the focus on an editbox (keydown is received, the focus, then keychar is received by the editbox again, but is irrelevant)
		void		bypassNextKey() { _BypassNextKey = true; }

		// True if the editBox loose the focus on enter
		bool		getLooseFocusOnEnter() const {return _LooseFocusOnEnter;}
		//
		virtual void    clearAllEditBox();
		// From CInterfaceElement
		virtual bool wantSerialConfig() const;
		// From CInterfaceElement
		virtual void serialConfig(NLMISC::IStream &f);
		// From CInterfaceElement
		virtual void onQuit();
		// From CInterfaceElement
		virtual void onLoadConfig();

		// from CCtrlBase
		virtual	void elementCaptured(CCtrlBase *capturedElement);

		// from CCtrlBase
		virtual void onKeyboardCaptureLost();

		// set the input string as "default". will be reseted at first click (used for user information)
		void	setDefaultInputString(const ucstring &str);

		// For Interger and PositiveInteger, can specify min and max values
		void	setIntegerMinValue(sint32 minValue) {_IntegerMinValue=minValue;}
		void	setIntegerMaxValue(sint32 maxValue) {_IntegerMaxValue=maxValue;}
		void	setPositiveIntegerMinValue(uint32 minValue) {_PositiveIntegerMinValue=minValue;}
		void	setPositiveIntegerMaxValue(uint32 maxValue) {_PositiveIntegerMaxValue=maxValue;}

		void setFocusOnText();

		int luaSetSelectionAll(CLuaState &ls);
		int luaSetupDisplayText(CLuaState &ls);
		int luaSetFocusOnText(CLuaState &ls);
		int luaCancelFocusOnText(CLuaState &ls);
		REFLECT_EXPORT_START(CGroupEditBox, CGroupEditBoxBase)
			REFLECT_LUA_METHOD("setupDisplayText", luaSetupDisplayText);
			REFLECT_LUA_METHOD("setSelectionAll", luaSetSelectionAll);
			REFLECT_LUA_METHOD("setFocusOnText", luaSetFocusOnText);
			REFLECT_LUA_METHOD("cancelFocusOnText", luaCancelFocusOnText);
			REFLECT_STRING("input_string", getInputStringAsStdString, setInputStringAsStdString);
			REFLECT_UCSTRING("uc_input_string", getInputString, setInputString);
		REFLECT_EXPORT_END

		/** Restore the original value of the edit box.
		  * This value is captured when the edit box get focus
		  * (return true if no undo was available)
		  * Will always fails ifthe edito box do not have the focus
		  */
		bool undo();

		/** Cancel last undo operation
		  * Return true if redo operation is available
		  */
		bool redo();

		/// freeze the control (loose focus, and cannot edit)
		void	setFrozen (bool state);
		bool	getFrozen () const { return _Frozen; }

	protected:

		// Cursor infos
		float	_BlinkTime;
		sint32	_CursorPos;
		uint32  _MaxNumChar;
		uint32  _MaxNumReturn;
		uint32  _MaxFloatPrec;		// used in setInputStringAsFloat() only
		sint32	_MaxCharsSize;
		sint32	_FirstVisibleChar;
		sint32	_LastVisibleChar;

		// Text selection
		static sint32	      _SelectCursorPos;
		bool	_SelectingText;
		NLMISC::CRGBA	_TextSelectColor;
		NLMISC::CRGBA	_BackSelectColor;

		// Text info
		ucstring	_Prompt;
		ucstring	_InputString;
		CViewText	*_ViewText;

		// undo / redo
		ucstring	_StartInputString;  // value of the input string when focus was acuired first
		ucstring	_ModifiedInputString;


		// Historic info
		typedef std::deque<ucstring>		THistoric;
		THistoric	_Historic;
		uint32		_MaxHistoric;
		sint32		_CurrentHistoricIndex;
		sint32      _PrevNumLine;

		// Action Handler
		std::string _AHOnChange;
		std::string _ParamsOnChange;
		std::string _ListMenuRight;

		std::string _AHOnFocusLost;
		std::string _AHOnFocusLostParams;

		// entry type
		TEntryType _EntryType;


		bool	_Setupped                  : 1; // setup
		bool	_BypassNextKey             : 1;
		bool	_BlinkState                : 1;
		bool	_CursorAtPreviousLineEnd   : 1; // force the cursor to be displayed at the end of the previous line end (if END has beeen pressed while in a string split accross 2 lines)
		bool	_LooseFocusOnEnter         : 1;
		bool    _ResetFocusOnHide          : 1;
		bool	_BackupFatherContainerPos  : 1; // Backup father container position when characters are typed.
												// If the edit box is at the bottom of the screen and if it expands on y
												// because of multiline, thz parent container will be moved to top
												// The good position can be restored by a press on enter then
		bool	_WantReturn				   : 1; // Want return char, don't call the enter action handler
		bool	_Savable				   : 1; // should content be saved ?
		bool	_DefaultInputString		   : 1; // Is the current input string the default one (should not be edited)
		bool	_Frozen					   : 1;	// is the control frozen? (cannot edit in it)

		bool	_CanRedo                   : 1;
		bool	_CanUndo                   : 1;

		std::vector<char>					_NegativeFilter;

		sint	_CursorTexID;
		sint32	_CursorWidth;

		sint32	_IntegerMinValue;
		sint32	_IntegerMaxValue;
		uint32	_PositiveIntegerMinValue;
		uint32	_PositiveIntegerMaxValue;

		sint32	_ViewTextDeltaX;

	private:
		void setupDisplayText();
		void makeTopWindow();
		void handleEventChar(const NLGUI::CEventDescriptorKey &event);
		void handleEventString(const NLGUI::CEventDescriptorKey &event);
		void setup();
		void triggerOnChangeAH();
		void appendStringFromClipboard(const ucstring &str);

		ucstring	getSelection();

		static CGroupEditBox *_MenuFather;

		static bool isValidAlphaNumSpace(ucchar c)
		{
			if (c > 255) return false;
			char ac = (char) c;
			return (ac >= '0' && ac <= '9') ||
				(ac >= 'a' && ac <= 'z') ||
				(ac >= 'A' && ac <= 'Z') ||
				ac==' ';
		}

		static bool isValidAlphaNum(ucchar c)
		{
			if (c > 255) return false;
			char ac = (char) c;
			return (ac >= '0' && ac <= '9') ||
				   (ac >= 'a' && ac <= 'z') ||
				   (ac >= 'A' && ac <= 'Z');
		}

		static bool isValidAlpha(ucchar c)
		{
			if (c > 255) return false;
			char ac = (char) c;
			return (ac >= 'a' && ac <= 'z') ||
				   (ac >= 'A' && ac <= 'Z');
		}

		static bool isValidPlayerNameChar(ucchar c)
		{
			// valid player name (with possible shard prefix / suffix format
			return isValidAlpha(c) || c=='.' || c=='(' || c==')';
		}

		static bool isValidFilenameChar(ucchar c)
		{
			if (c == '\\' ||
				c == '/' ||
				c == ':' ||
				c == '*' ||
				c == '?' ||
				c == '\"' ||
				c == '<' ||
				c == '>' ||
				c == '|') return false;
			return true;
		}
		//
		bool isFiltered(ucchar c)
		{
			uint length = (uint)_NegativeFilter.size();
			for (uint k = 0; k < length; ++k)
			{
				if ((ucchar) _NegativeFilter[k] == c) return true;
			}
			return false;
		}

		static IComboKeyHandler *comboKeyHandler;

	public:
		static void setComboKeyHandler( IComboKeyHandler *handler ){ comboKeyHandler = handler; }

	};


}

#endif // RZ_CTRL_EDITBOX_H

/* End of ctrl_editbox.h */

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



#ifndef NL_CTRL_BASE_BUTTON_H
#define NL_CTRL_BASE_BUTTON_H

#include "nel/gui/ctrl_base.h"
#include "nel/gui/action_handler.h"

namespace NLGUI
{

	// ***************************************************************************
	/**
	 * Base Class For Buttons.
	 * \author Lionel Berenguier
	 * \author Nevrax France
	 * \date 2003
	 */
	class CCtrlBaseButton : public CCtrlBase
	{

	public:
		enum EType { PushButton = 0, ToggleButton, RadioButton, ButtonTypeCount };

		/// Constructor
		CCtrlBaseButton(const TCtorParam &param);

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		virtual bool parse (xmlNodePtr cur,CInterfaceGroup * parentGroup);
		virtual bool handleEvent (const NLGUI::CEventDescriptor& event);

		/// \name Misc
		// @{
		void	setType (EType t) { _Type = t; }
		EType	getType() { return _Type; }
		std::string getTypeString() const;
		void setTypeFromString( const std::string &type );

		void	setClickWhenPushed(bool click) { _ClickWhenPushed = click; }
		bool	getClickWhenPushed() const { return _ClickWhenPushed; }

		void	setPushed (bool state);
		bool	getPushed () const { return _Pushed; }

		void	setFrozen (bool state);
		bool	getFrozen () const { return _Frozen; }

		// Set half tone mode for the display of frozen buttons. Default is true.
		void	setFrozenHalfTone(bool enabled);
		bool	getFrozenHalfTone() const { return _FrozenHalfTone; }

		// if the radio is a radio button, then all radio button are unselected
		void	unselect();
		// @}


		/// \name Colors
		// @{
		void setColor(NLMISC::CRGBA col) { _ColorNormal = col; }
		void setColorPushed(NLMISC::CRGBA col) { _ColorPushed = col; }
		void setColorOver(NLMISC::CRGBA col) { _ColorOver = col; }

		NLMISC::CRGBA getColor() const { return _ColorNormal; }
		NLMISC::CRGBA getColorPushed() const { return _ColorPushed; }
		NLMISC::CRGBA getColorOver() const { return _ColorOver; }

		// Override because mustupdate 3 states
		void setModulateGlobalColorAll(bool state);
		void setModulateGlobalColorNormal(bool state) {_ModulateGlobalColorNormal= state;}
		void setModulateGlobalColorPushed(bool state) {_ModulateGlobalColorPushed= state;}
		void setModulateGlobalColorOver(bool state) {_ModulateGlobalColorOver= state;}

		virtual sint32 getAlpha() const { return _ColorNormal.A; }
		virtual void setAlpha (sint32 a) { _ColorOver.A = _ColorNormal.A = _ColorPushed.A = (uint8)a; }

		std::string getColorAsString() const
		{	return	NLMISC::toString(_ColorNormal.R) + " " + NLMISC::toString(_ColorNormal.G) + " " +
					NLMISC::toString(_ColorNormal.B) + " " + NLMISC::toString(_ColorNormal.A); }
		std::string getColorOverAsString() const
		{	return	NLMISC::toString(_ColorOver.R) + " " + NLMISC::toString(_ColorOver.G) + " " +
					NLMISC::toString(_ColorOver.B) + " " + NLMISC::toString(_ColorOver.A); }
		std::string getColorPushedAsString() const
		{	return	NLMISC::toString(_ColorPushed.R) + " " + NLMISC::toString(_ColorPushed.G) + " " +
					NLMISC::toString(_ColorPushed.B) + " " + NLMISC::toString(_ColorPushed.A); }

		void setColorAsString(const std::string &col)		{ _ColorNormal = convertColor (col.c_str()); }
		void setColorOverAsString(const std::string &col)	{ _ColorOver = convertColor (col.c_str()); }
		void setColorPushedAsString(const std::string &col)	{ _ColorPushed = convertColor (col.c_str()); }
		// @}

		///\name radio button specific
		//@{
		/** Initialize radio button reference
		 *	Advanced:
		 *	NB: must call initRBRef() for radio button if button is created without parse().
		 *	NB: setParent() must be called before (else assert)
		 */
		void initRBRef();
		//@}
		void initRBRefFromRadioButton(CCtrlBaseButton * pBut);


		/// \name Handlers
		// @{
		// Event part
		void setActionOnLeftClick (const std::string &actionHandlerName) { _AHOnLeftClickString = actionHandlerName; _AHOnLeftClick = CAHManager::getInstance()->getAH(actionHandlerName, _AHLeftClickParams); }
		void setActionOnLeftClickParams(const std::string &params) { _AHOnLeftClickStringParams = params; }
		void setActionOnOver (const std::string &actionHandlerName) { _AHOnOver = CAHManager::getInstance()->getAH(actionHandlerName, _AHOverParams); }
		void setActionOnRightClick (const std::string &actionHandlerName) { _AHOnRightClick = CAHManager::getInstance()->getAH(actionHandlerName, _AHRightClickParams); }
		void setActionOnClockTick (const std::string &ahName) { _AHOnClockTick = CAHManager::getInstance()->getAH(ahName, _AHClockTickParams); }
		void setParamsOnLeftClick (const std::string &paramsHandlerName) { _AHLeftClickParams = paramsHandlerName; }
		void setParamsOnOver (const std::string &paramsHandlerName) { _AHOverParams = paramsHandlerName; }
		void setParamsOnRightClick (const std::string &paramsHandlerName) { _AHRightClickParams = paramsHandlerName; }
		void setParamsOnClockTick (const std::string &ahParamsName) { _AHClockTickParams = ahParamsName; }

		// get Event part
		std::string _getActionOnOver() const{ return CAHManager::getInstance()->getAHName( _AHOnOver ); }
		std::string _getActionOnLeftClick() const { return CAHManager::getInstance()->getAHName( _AHOnLeftClick ); }
		std::string _getActionOnLeftLongClick() const { return CAHManager::getInstance()->getAHName( _AHOnLeftLongClick ); }
		std::string _getActionOnDblLeftClick() const { return CAHManager::getInstance()->getAHName( _AHOnLeftDblClick ); }
		std::string _getActionOnRightClick() const { return CAHManager::getInstance()->getAHName( _AHOnRightClick ); }
		std::string _getActionOnClockTick() const { return CAHManager::getInstance()->getAHName( _AHOnClockTick ); }
		
		IActionHandler *getActionOnLeftClick () const { return _AHOnLeftClick; }
		IActionHandler *getActionOnRightClick () const { return _AHOnRightClick; }
		IActionHandler *getActionOnClockTick () const { return _AHOnClockTick; }
		std::string			_getParamsOnOver() const{ return _AHOverParams.toString(); }
		std::string			_getParamsOnLeftClick () const { return _AHLeftClickParams.toString(); }
		std::string			_getParamsOnRightClick () const { return _AHRightClickParams.toString(); }
		const std::string	&getParamsOnLeftClick () const { return _AHLeftClickParams; }
		const std::string	&getParamsOnRightClick () const { return _AHRightClickParams; }
		const std::string	&getParamsOnClockTick () const { return _AHClockTickParams; }

		// run action on left click
		void runLeftClickAction();
		void runRightClickAction();

		// Context menu accessor/ One for each button
		void setListMenuLeft (const std::string &cm) { _ListMenuLeft = cm; }
		void setListMenuRight (const std::string &cm) { _ListMenuRight = cm; }
		void setListMenuBoth (const std::string &cm) { _ListMenuLeft= _ListMenuRight= cm; }
		std::string getListMenuLeft () { return _ListMenuLeft.toString(); }
		std::string getListMenuRight () { return _ListMenuRight.toString(); }
		// @}



		int luaRunLeftClickAction(CLuaState &ls);
		int luaRunRightClickAction(CLuaState &ls);
		REFLECT_EXPORT_START(CCtrlBaseButton, CCtrlBase)
			REFLECT_BOOL("pushed", getPushed, setPushed);
			REFLECT_STRING("col_normal", getColorAsString, setColorAsString);
			REFLECT_STRING("col_over", getColorOverAsString, setColorOverAsString);
			REFLECT_STRING("col_pushed", getColorPushedAsString, setColorPushedAsString);
			REFLECT_RGBA("col_normal_rgba", getColor, setColor);
			REFLECT_RGBA("col_over_rgba", getColorOver, setColorOver);
			REFLECT_RGBA("col_pushed_rgba", getColorPushed, setColorPushed);
			REFLECT_BOOL("frozen", getFrozen, setFrozen);
			REFLECT_BOOL("frozen_half_tone", getFrozenHalfTone, setFrozenHalfTone);
			REFLECT_STRING("onclick_l", _getActionOnLeftClick, setActionOnLeftClick);
			REFLECT_STRING("params_l", _getParamsOnLeftClick, setParamsOnLeftClick);
			REFLECT_LUA_METHOD("runLeftClickAction", luaRunLeftClickAction);
			REFLECT_STRING("onclick_r", _getActionOnRightClick, setActionOnRightClick);
			REFLECT_STRING("params_r", _getParamsOnRightClick, setParamsOnRightClick);
			REFLECT_LUA_METHOD("runRightClickAction", luaRunRightClickAction);
			REFLECT_STRING("onover", _getActionOnOver, setActionOnOver);
			REFLECT_STRING("params_over", _getParamsOnOver, setParamsOnOver);
		REFLECT_EXPORT_END

	protected:
		EType	_Type;

		// State
		bool _Pushed			: 1;
		bool _Over				: 1;
		bool _OverWhenPushed	: 1;
		bool _Frozen			: 1;
		bool _FrozenHalfTone    : 1;
		bool _ClickWhenPushed	: 1;
		bool _ModulateGlobalColorNormal	: 1;
		bool _ModulateGlobalColorPushed	: 1;
		bool _ModulateGlobalColorOver	: 1;
		bool _LeftLongClickHandled       : 1; // Is it already handled ?
		bool _LeftDblClickHandled		 : 1;


		///\name radio button specific
		//@{
		CCtrlBaseButton *_RBRefBut;	// The reference button. If NULL the control do not own the reference
		// There is only one radio button per group that own the reference (the first one)
		CCtrlBaseButton **_RBRef;	// The pointer onto the reference button
		//@}


		// Colors
		NLMISC::CRGBA	_ColorNormal;
		NLMISC::CRGBA	_ColorPushed;
		NLMISC::CRGBA	_ColorOver;

		///\name Long click specific
		//@{
		sint64 _LeftLongClickDate;	// Time we left click down
		//@}

		// for double click : last date at which last left click occurred
		static sint64 _LastLeftClickDate;
		static NLMISC::CRefPtr<CCtrlBaseButton> _LastLeftClickButton;

		///\name Action Handler
		//@{
		IActionHandler *_AHOnOver;
		CStringShared	_AHOverParams;
		std::string		_AHOnLeftClickString;
		std::string		_AHOnLeftClickStringParams;
		IActionHandler *_AHOnLeftClick;
		CStringShared	_AHLeftClickParams;
		IActionHandler *_AHOnLeftDblClick;
		CStringShared	_AHLeftDblClickParams;
		IActionHandler *_AHOnRightClick;
		CStringShared	_AHRightClickParams;
		IActionHandler *_AHOnClockTick;
		CStringShared	_AHClockTickParams;
		IActionHandler *_AHOnLeftLongClick;
		CStringShared	_AHLeftLongClickParams;
		//@}
		CStringShared	_ListMenuLeft;
		CStringShared	_ListMenuRight;

		// get the colors modulated on request
		NLMISC::CRGBA	getCurrentColorNormal(NLMISC::CRGBA globalColor) const
		{
			NLMISC::CRGBA	rgba  = _ColorNormal;
			if(_ModulateGlobalColorNormal)
				rgba.modulateFromColor(rgba, globalColor);
			return rgba;
		}
		NLMISC::CRGBA	getCurrentColorPushed(NLMISC::CRGBA globalColor) const
		{
			NLMISC::CRGBA	rgba  = _ColorPushed;
			if(_ModulateGlobalColorPushed)
				rgba.modulateFromColor(rgba, globalColor);
			return rgba;
		}
		NLMISC::CRGBA	getCurrentColorOver(NLMISC::CRGBA globalColor) const
		{
			NLMISC::CRGBA	rgba  = _ColorOver;
			if(_ModulateGlobalColorOver)
				rgba.modulateFromColor(rgba, globalColor);
			return rgba;
		}

		// call it at draw
		void	updateOver(bool &lastOver);
		virtual	void elementCaptured(CCtrlBase *capturedElement);
	};

}

#endif // NL_CTRL_BASE_BUTTON_H

/* End of ctrl_base_button.h */

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



#ifndef RZ_CTRL_SCROLL_H
#define RZ_CTRL_SCROLL_H

#include "nel/misc/types_nl.h"
#include "nel/gui/ctrl_scroll_base.h"


namespace NLGUI
{

	/**
	 * Class handling scollbar function
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */
	class CCtrlScroll : public CCtrlScrollBase, public NLMISC::ICDBNode::IPropertyObserver
	{

	public:
		DECLARE_UI_CLASS( CCtrlScroll )
		CCtrlScroll(const TCtorParam &param);
		~CCtrlScroll();

		std::string getProperty( const std::string &name ) const;
		void setProperty( const std::string &name, const std::string &value );
		xmlNodePtr serialize( xmlNodePtr parentNode, const char *type ) const;

		virtual bool parse(xmlNodePtr cur, CInterfaceGroup * parentGroup);

		virtual void updateCoords();
		virtual void draw();
		virtual bool handleEvent (const NLGUI::CEventDescriptor &event);

		void	setTarget (CInterfaceGroup *pIG);
		// Return the delta value the track has moved
		sint32	moveTrackX (sint32 dx);
		sint32	moveTrackY (sint32 dy);

		/** Move the Target Ofs with a Delta, and recompute TrackPos from this Ofs.
		 *	Useful for finer controled group scrolling when the list is very big (with mouseWheel or scroll buttons)
		 */
		void	moveTargetX (sint32 dx);
		void	moveTargetY (sint32 dy);

		void	setAlign (sint32 nAlign) { _Aligned = nAlign; }
		// invert the factor for target
		void	setInverted(bool invert) { _Inverted = invert; }

		void	setTextureBottomOrLeft	(const std::string &txName);
		void	setTextureMiddle		(const std::string &txName);
		void	setTextureTopOrRight	(const std::string &txName);
		std::string getTextureBottomOrLeft() const;
		std::string getTextureMiddle() const;
		std::string getTextureTopOrRight() const;

		void	setTextureBottomOrLeft	(sint32 txid) { _TxIdB = txid; }
		void	setTextureMiddle		(sint32 txid) { _TxIdM = txid; }
		void	setTextureMiddleTile	(uint8 tile) { _TileM = tile; } // 0 - not tiled (1 BL) (2 BR) (3 TL) (4 TR)
		void	setTextureTopOrRight	(sint32 txid) { _TxIdT = txid; }

		// number scroller
		sint32	getValue() const { return _IsDBLink ? _DBLink.getSInt32() : _Value; }
		// NB: the value is clamped (see setMinMax) and stepped (see setStepValue())
		void	setValue(sint32 value);
		void	setMinMax(sint32 nMin, sint32 nMax) { _Min = nMin; _Max = nMax; }
		void	setStepValue(uint32 step) { _StepValue= step; }

		void	setTrackPos(sint32 pos);
		sint32	getTrackPos() const { return _TrackPos; }
		sint32	getTrackSize() const { return _TrackSize; }
		// dummy set for track size (forlua export)
		void	setTrackSize(sint32 /* trackSize */) { throw  NLMISC::Exception("TrackSize is read-only"); }


		void	setFrozen (bool state);
		bool	getFrozen () const { return _Frozen; }

		int luaSetTarget(CLuaState &ls);
		int luaEnsureVisible(CLuaState &ls);

		// name
		void			setName(const std::string & val) {_Name = val;}
		std::string		getName() const {return _Name;}

		// max
		void			setMax(sint32 max) {_Max = max;}
		sint32			getMax() const {return _Max;}

		REFLECT_EXPORT_START(CCtrlScroll, CCtrlScrollBase)
			REFLECT_LUA_METHOD("setTarget", luaSetTarget)
			REFLECT_LUA_METHOD("ensureVisible", luaEnsureVisible);
			REFLECT_SINT32("value", getValue, setValue);
			REFLECT_SINT32("trackPos", getTrackPos, setTrackPos);
			REFLECT_SINT32("trackSize", getTrackSize, setTrackSize);
			REFLECT_STRING("name", getName, setName);
			REFLECT_SINT32("max", getMax, setMax);
		REFLECT_EXPORT_END

		/** Ensure that a child element be visible into the frame through which
		  * its parent group is displayed.
		  * Example : Had we a list of items for which we want some item 'itemPtr' to have its top position
		  * matching the middle of the list, we would do :
		  * this->ensureVisible(itemPtr, Hotspot_Tx, Hotspot_Mx);
		  *
		  * The scrollbar will be moved accordingly.
		  */
		void ensureVisible(CInterfaceElement *childElement, THotSpot childHotSpot, THotSpot parentHotSpot);


	protected:

		CInterfaceProperty	_DBLink;	// If this is a value scroller we can link it with db
		sint32				_Value;		// Or we can use a normal value
		sint32				_InitialValue;

		sint32 _Min, _Max;
		std::string _AHOnScroll;
		std::string _AHOnScrollParams;
		//
		std::string _AHOnScrollEnd;
		std::string _AHOnScrollEndParams;
		//
		//
		std::string _AHOnScrollCancel;
		std::string _AHOnScrollCancelParams;


		sint32 _Aligned; // 0-Top 1-Bottom 2-Left 3-Right

		sint32 _TrackDispPos;
		float  _TrackPos;
		sint32 _TrackSize;
		sint32 _TrackSizeMin;

		sint32 _MouseDownOffsetX;
		sint32 _MouseDownOffsetY;

		sint32 _TxIdB; // Same as Left if Horizontal sb
		sint32 _TxIdM;
		sint32 _TxIdT; // Same as Right if Horizontal sb

		uint8	_TileM;

		sint32 _LastTargetHReal;
		sint32 _LastTargetMaxHReal;
		sint32 _LastTargetOfsY;
		sint32 _LastTargetWReal;
		sint32 _LastTargetMaxWReal;
		sint32 _LastTargetOfsX;

		bool	_Vertical   : 1; // true if vertical track bar
		bool	_IsDBLink   : 1;
		bool	_ObserverOn : 1;
		bool    _Inverted   : 1;
		bool    _MouseDown  : 1;
		bool	_CallingAH  : 1;
		bool	_Cancelable : 1; // true if the slider may be cancelled when pressed on the mouse right button
		bool	_Frozen		: 1;
		bool	_Scale		: 1;

		// For Target Scroller only: the target offset step in pixel.
		sint32	_TargetStepX;
		sint32	_TargetStepY;

		// For Value Scroller only: indicate the step the scroll bar has. 0 or 1 means no step
		uint32	_StepValue;

		// Slider's name
		std::string _Name;

		void	computeTargetOfsFromPos();

		// from IPropertyObserver
		virtual void update(NLMISC::ICDBNode *node);

		// step the value, and clamp it
		void	normalizeValue(sint32 &value);

		void runAH(const std::string &name, const std::string &params);

	};
}

#endif // RZ_CTRL_SCROLL_H

/* End of ctrl_scroll.h */



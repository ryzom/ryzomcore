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



#ifndef NL_INTERFACE_DDX_H
#define NL_INTERFACE_DDX_H

#include "nel/misc/types_nl.h"
#include "nel/gui/interface_element.h"
#include "interface_pointer.h"

// Values for float (scrollbar float) must not exceed 200,00 this is due to storing on
// an integer value with a precision of 10,000
// For the moment realtime parameters are only valid on scrollbar
// ***************************************************************************
class CInterfaceDDX : public CInterfaceElement
{
public:
	// The number of preset in cfg.
	enum	{NumPreset= 4, CustomPreset=NumPreset};

public:
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

	CInterfaceDDX();
	virtual ~CInterfaceDDX();

	// DB -> Parameters
	void init();

	// Parameters -> DB
	void update();

	// Restore realtime values
	void cancel();

	// Update parameters that are realtime
	void updateRealtime(CCtrlBase *pSB, bool updateOnScrollEnd);

	// Update all parameters to obey their preset (no op if no preset or if preset is Custom)
	void updateParamPreset(NLMISC::CCDBNodeLeaf *presetChanged);

	// set apply button can be pushed
	void validateApplyButton();

private:

	class CParam
	{
	public:
		// -----------------------
		enum SParamType { DataBase, ConfigFile };
		enum SParamWidget { ColorButton, BoolButton, ScrollBarInt, ScrollBarFloat };
		enum TRealTimeMode { RTModeFalse=0, RTModeTrue, RTModeEndScroll};

		// -----------------------
		// The control
		CInterfaceElementPtr	Elt;
		SParamType			Type;
		SParamWidget		Widget;
		std::string			Link;
		// The tex view, result of the scroll
		CViewTextPtr		ResultView;
		// The unit to append to the result string
		ucstring			ResultUnit;
		// For ScrollBarFloat widget only
		uint8				ResultDecimal;
		// For ScrollBarFloat widget only
		bool				RoundMode;

		// If Widget is scrollbar int or float
		sint32				Min, Max;

		// When the effect of the param is applied
		TRealTimeMode		RealTimeMode;
		sint32	RTBackupValue;	// When canceling

		// For ConfigFile widget only
		NLMISC::CCDBNodeLeaf		*PresetDB;

		// -----------------------
		CParam()
		{
			ResultDecimal= 0;
			RoundMode= false;
			PresetDB= NULL;
			RealTimeMode= RTModeFalse;
		}
		void DBToWidget();
		void CFGToWidget();
		void WidgetToDB();
		void WidgetToCFG();
		void backupDB();
		void backupCFG();
		void restoreDB();
		void restoreCFG();

		// update ResultView according to widget. Don't modify DB or CFG
		void WidgetToResultView();

		// restore the preset value
		void restoreCFGPreset(uint presetVal);

		// From the current CFG value, compare to each preset value, and init the bitfield
		uint32	getPresetPossibleBF();

	private:
		void updateScrollView(sint32 nVal);
		void updateScrollView(double nVal);
		void tryRound(double nVal);
	};

	// Array of parameters
	std::vector<CParam> _Parameters;

	// For preset change
	class CPresetObs : public NLMISC::ICDBNode::IPropertyObserver
	{
	public:
		virtual void update(NLMISC::ICDBNode* node);
		CInterfaceDDX		*Owner;

		CPresetObs() : Owner(NULL) {}
	};
	CPresetObs					_PresetObs;
	std::set<NLMISC::CCDBNodeLeaf*>		_PresetNodes;

	// reset the preset values according to parameters values
	void	resetPreSet();

	// For apply button ungraying
	class CCtrlBaseButton		*_ApplyButton;
};

// ***************************************************************************
class CDDXManager
{
public:

	static CDDXManager *getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CDDXManager;
		return _Instance;
	}

	// release memory
	static void releaseInstance();

	void add(CInterfaceDDX *pDDX);
	CInterfaceDDX *get(const std::string &ddxName);
	// Get the ddx from its parent name
	CInterfaceDDX *getFromParent(const std::string &ddxParentName);

	// Release the manager
	void release();

private:
	CDDXManager();

private:
	static CDDXManager *_Instance;
	std::map<std::string, CInterfaceDDX*> _DDXes;
};

#endif // NL_INTERFACE_ELEMENT_H

/* End of interface_element.h */

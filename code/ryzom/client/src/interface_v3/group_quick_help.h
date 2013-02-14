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




#ifndef CL_GROUP_QUICK_HELP_H
#define CL_GROUP_QUICK_HELP_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_html.h"


/**
 * Quick help group
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CGroupQuickHelp : public CGroupHTML
{
public:

	// Constructor
	CGroupQuickHelp(const TCtorParam &param);
	~CGroupQuickHelp();

	// Submit an event
	bool submitEvent (const char *event);

private:

	// CInterfaceGroup Interface
	virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);
	virtual void updateCoords();

	// From CGroupHTML
	virtual void beginElement (uint element_number, const BOOL *present, const char **value);
	virtual void endBuild ();
	virtual void browse (const char *url);
	virtual std::string	home();

	// Init parsing value
	void initParameters();

	// Update the paragraph font size according the current step
	void updateParagraph ();

	// Set the text attribute for a group and its sub group
	void setGroupTextSize (CInterfaceGroup *group, bool selected);

	// Evaluate a condition
	bool evalExpression (const std::string &condition);

	// A quick help step
	class CStep
	{
	public:
		std::set<std::string>	EventToComplete;
		std::string				Condition;
		std::string				URL;
	};

	// Activate a step
	void activateCurrentStep ();

	// The quick help steps
	std::vector<CStep>	_Steps;

	// The current step
	uint	_CurrentStep;

	// Parameters
	uint			_NonSelectedSize;
	NLMISC::CRGBA	_NonSelectedColor;
	NLMISC::CRGBA	_NonSelectedLinkColor;
	bool			_NonSelectedGlobalColor;
	bool			_UpdateParagraphNextUpdateCoords;

	bool			_IsQuickHelp;
};

#endif

// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
	virtual void beginElement (NLGUI::CHtmlElement &elm);
	virtual void endBuild ();
	virtual void browse (const char *url);
	virtual std::string	home() const NL_OVERRIDE;

	// Modify uri with '.html' or '_??.html' ending to have current user language,
	// If the uri is not found locally, then try "en" as fallback language
	// ie. 'help_ru.html' does not exists, return 'help_en.html'
	std::string getLanguageUrl(const std::string &href, std::string lang) const;

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

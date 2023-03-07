// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010-2015  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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




#ifndef CL_GROUP_HTML_CS_H
#define CL_GROUP_HTML_CS_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_html.h"


/**
 * Forum HTML group
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2002
 */
class CGroupHTMLCS : public CGroupHTML
{
public:

	// Constructor
	CGroupHTMLCS(const TCtorParam &param);
	~CGroupHTMLCS();

	// From CGroupHTML
	virtual void addHTTPGetParams (std::string &url, bool trustedDomain);
	virtual void addHTTPPostParams (SFormFields &formfields, bool trustedDomain);
	virtual std::string	home() const NL_OVERRIDE;

private:

	struct CParameter
	{
		std::string Name;
		std::string Value;
	};
	void getParameters(std::vector<CParameter> &parameters, bool encodeForUrl);
};

// convert an ascii string to a valid url param string
std::string convertToHTML(const std::string &s);

#endif


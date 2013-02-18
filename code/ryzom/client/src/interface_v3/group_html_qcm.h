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




#ifndef CL_GROUP_HTML_QCM_H
#define CL_GROUP_HTML_QCM_H

#include "nel/misc/types_nl.h"
#include "nel/gui/group_html.h"


/**
 * Mail HTML group
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date April 2004
 */
class CGroupHTMLQCM : public CGroupHTML
{
public:

	// Constructor
	CGroupHTMLQCM(const TCtorParam &param);
	~CGroupHTMLQCM();

	// From CGroupHTML
	virtual void addText (const char * buf, int len);

private:

};

#endif // CL_GROUP_HTML_MAIL_H


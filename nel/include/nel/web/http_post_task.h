// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_HTTP_POST_TASK_H
#define NL_HTTP_POST_TASK_H

#include "nel/misc/types_nl.h"

#include "nel/misc/thread.h"
#include <string>

namespace NLWEB {

class CHttpPostTask : public NLMISC::IRunnable
{
public:
	CHttpPostTask(const std::string &host, const std::string &page, const std::string &params);
	void run(void);

private:
	std::string m_Host;
	std::string m_Page;
	std::string m_Params;
};

}

#endif // NL_HTTP_POST_TASK_H

/* end of file */

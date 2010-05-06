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

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdafx.h"
#include "displayer.h"
#include "georges_edit.h"
#include "main_frm.h"

using namespace std;
using namespace NLMISC;

CGeorgesDisplayer::CGeorgesDisplayer () : IDisplayer ("Georges edit output console")
{
}

void CGeorgesDisplayer::doDisplay( const CLog::TDisplayInfo& args, const char *message)
{
	string temp;
	uint size = strlen (message);
	temp.reserve (2*size);
	bool previousR=false;
	for (uint c=0; c<size; c++)
	{
		if ((message[c] == '\n') && (!previousR))
			temp += "\r\n";
		else
			temp += message[c];
		previousR = (message[c] == '\r');
	}

	((CMainFrame*)(theApp.m_pMainWnd))->outputConsoleString (temp.c_str ());
}

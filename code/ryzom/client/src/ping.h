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

#ifndef CL_PING_H
#define CL_PING_H

#include <nel/misc/types_nl.h>
#include <nel/misc/cdb.h>

///////////
// CLASS //
///////////
/**
 * Class to manage the ping computed with the database.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2003
 */
class CPing : public NLMISC::ICDBNode::IPropertyObserver
{
private:
	uint32	_Ping;
	bool	_RdyToPing;

public:
	// Constructor.
	CPing() {_Ping = 0; _RdyToPing = true;}
	// Destructor.
	~CPing() {;}

	// Add an observer on the database for the ping.
	void init();

	// Release the observer on the database for the ping.
	void release();

	// Method called when the ping message is back.
	virtual void update(NLMISC::ICDBNode* node);

	// return the ping in ms.
	uint32 getValue() {return _Ping;}

	void rdyToPing(bool rdy) {_RdyToPing = rdy;}
	bool rdyToPing() const {return _RdyToPing;}
};

#endif // CL_PING_H

/* end of file */
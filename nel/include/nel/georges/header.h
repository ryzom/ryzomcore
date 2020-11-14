// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_HEADER_H
#define NL_HEADER_H

#include "nel/misc/types_nl.h"

// Forward declarations for libxml2
typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;

typedef struct _xmlDoc xmlDoc;
typedef xmlDoc *xmlDocPtr;

namespace NLGEORGES
{

class CFileHeader
{
public:
	/// Default constructor
	CFileHeader ();

	// Form states
	enum TState
	{
		Modified =0,
		Checked,
		StateCount,
	};

	/// Add a log
	void				addLog(const std::string &log);

	/// Set the comment
	void				setComments (const std::string &comments);

	/// Major version number
	uint32				MajorVersion;

	/// Minor version number
	uint32				MinorVersion;

	/// State of the form
	TState				State;

	/// Comments of the form
	std::string			Comments;

	/// Log of the form
	std::string			Log;

	/// ** IO functions
	void				read (xmlNodePtr root);
	void				write (xmlNodePtr node) const;

	// Get state string
	static const char	*getStateString (TState state);

	// Error handling
	void				warning (bool exception, const std::string &function, const char *format, ... ) const;
};

} // NLGEORGES

#endif // NL_HEADER_H

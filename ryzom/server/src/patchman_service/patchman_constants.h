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

#ifndef PATCHMAN_CONSTANTS_H
#define PATCHMAN_CONSTANTS_H

namespace PATCHMAN
{
	/*
		Note that the contants in this file are 'static', meaning that they
		will be instantiated in every .o file independently, this is the price
		to pay for typed constants in C++ :(
	*/

	/*
		The following conatants are used in module manifests to identify
		modules that fit different criteria.
	*/

//	static const char* ManifestEntryIsServerPatchRepository="isServerPatchRepository";
//	static const char* ManifestEntryIsServerPatchManager="isServerPatchManager";
	static const char* ManifestEntryIsAdministered="isAdministered";
	static const char* ManifestEntryIsAdministrator="isAdministrator";
	static const char* ManifestEntryIsFileRepository="isFileRepository";
	static const char* ManifestEntryIsFileReceiver="isFileReceiver";
	static const char* ManifestEntryIsTerminal="isTerminal";

}

#endif

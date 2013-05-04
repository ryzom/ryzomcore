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


#ifndef CL_3D_NOTES_H
#define CL_3D_NOTES_H

#include "nel/misc/types_nl.h"

#include "interface_v3/group_in_scene_bubble.h"

/**
 * Class to manage 3d notes
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2003
 */
class C3DNotes
{
public:

	C3DNotes()
	{
		NextId = 1;
		NotesAvailable = false;
	}

	void init();
	void update();
	void release();

	void addNote(const NLMISC::CVector &pos, const std::string &note, sint id = 0);
	void removeNote(sint id);

private:

	void saveNotes();

	NLMISC::CConfigFile NotesConfigFile;

	// true if the config file is ok and notes are ready to use
	bool				NotesAvailable;

	struct CNote
	{
		CNote(const NLMISC::CVector &pos, const std::string &note, sint id);
		~CNote();

		sint Id;
		NLMISC::CVector Position;
		std::string Note;
		CGroupInSceneBubble *Bubble;
	};

	std::list<CNote> Notes;

	sint NextId;

	friend struct commands_displayNotesClass;
	friend struct commands_gotoNoteClass;
	friend void cbNotesChanged();
};


#endif // CL_3D_NOTES_H

/* End of 3d_notes.h */

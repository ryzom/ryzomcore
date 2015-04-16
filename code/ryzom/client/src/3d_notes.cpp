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




#include "stdpch.h"

#include "3d_notes.h"
#include "client_cfg.h"
#include "init_main_loop.h"
#include "user_entity.h"

using namespace std;
using namespace NLMISC;

void cbNotesChanged()
{
	nlinfo("cbNotesChanged called");
	Notes.Notes.clear();

	CConfigFile::CVar &var = Notes.NotesConfigFile.getVar("Notes");
	for(sint i = 0; i < var.size(); i+=3)
	{
		sint id = var.asInt(i);
		if(i == 0 && id == 0)
		{
			// first is always 0
			continue;
		}
		if(id == 0)
		{
			nlwarning("Malformated id 0 in note file '%s', discard all notes", ClientCfg.NotesFilename.c_str());
			Notes.Notes.clear();
			return;
		}

		CVector pos;
		vector<string> posstr;
		explode(var.asString(i+1), ",", posstr);
		if(posstr.size()  != 3)
		{
			nlwarning("Malformated position in note number %d in file '%s', discard all notes", id, ClientCfg.NotesFilename.c_str());
			Notes.Notes.clear();
			return;
		}

		NLMISC::fromString(posstr[0], pos.x);
		NLMISC::fromString(posstr[1], pos.y);
		NLMISC::fromString(posstr[2], pos.z);

		string note(var.asString(i+2));
		if(note.empty())
		{
			nlwarning("Malformated because empty note in note number %d in file '%s', skipping it", id, ClientCfg.NotesFilename.c_str());
		}
		else
		{
			Notes.addNote(pos, note, id);
		}
	}
}

C3DNotes::CNote::CNote(const NLMISC::CVector &pos, const std::string &note, sint id) :
	Id(id), Position(pos), Note(note), Bubble(NULL)
{
	nlassert(Id > 0);
}

C3DNotes::CNote::~CNote()
{
	if(Bubble)
	{
		Bubble->setActive(false);
		Bubble->unlink();
		Bubble = NULL;
	}
}

void C3DNotes::init()
{
	if(!ClientCfg.NotesFilename.empty())
	{
		// load the 3d notes
		try
		{
			NotesConfigFile.load(ClientCfg.NotesFilename);
			NotesConfigFile.setCallback(cbNotesChanged);
			NotesAvailable = true;
			cbNotesChanged();
		}
		catch(Exception &e)
		{
			nlwarning("Error while loading '%s': %s", ClientCfg.NotesFilename.c_str(), e.what());
			NotesAvailable = false;
		}
	}
}

void C3DNotes::update()
{
	if (!NotesAvailable) return;

	// check if we need new bubble
	CVector userPos = UserEntity->pos();
	for(list<CNote>::iterator it = Notes.begin(); it != Notes.end(); it++)
	{
		if((*it).Bubble == NULL && ((*it).Position - userPos).norm() < 100)
		{
			string n = toString("NOTE: %d %s", (*it).Id, (*it).Note.c_str());
			(*it).Bubble = InSceneBubbleManager.newBubble (ucstring(n));
			if((*it).Bubble)
			{
				(*it).Bubble->link(NULL, 60*60*10);
				(*it).Bubble->setActive (true);
				nlinfo("NOTE: id %d pos(%f,%f,%f) note '%s'", (*it).Id, (*it).Position.x, (*it).Position.y, (*it).Position.z, (*it).Note.c_str());
			}
		}
		else if((*it).Bubble && ((*it).Position - userPos).norm() >= 100)
		{
			(*it).Bubble->setActive(false);
			(*it).Bubble->unlink();
			(*it).Bubble = NULL;
		}

		if((*it).Bubble)
		{
			(*it).Bubble->Position = (*it).Position;
		}
	}
}

void C3DNotes::release()
{
	Notes.clear();
}

void C3DNotes::saveNotes()
{
	if (!NotesAvailable) return;

	vector<string> vals;
	vals.push_back("0");
	vals.push_back("0,0,0");
	vals.push_back("0");
	for(list<CNote>::iterator it = Notes.begin(); it != Notes.end(); it++)
	{
		vals.push_back(toString("%d", (*it).Id));
		vals.push_back(toString("%f,%f,%f", (*it).Position.x, (*it).Position.y, (*it).Position.z));
		vals.push_back((*it).Note);
	}

	CConfigFile::CVar &var = NotesConfigFile.getVar("Notes");
	var.setAsString(vals);
	var.SaveWrap = 3;
	NotesConfigFile.save();
}

void C3DNotes::addNote(const NLMISC::CVector &pos, const string &note, sint id)
{
	if (!NotesAvailable) return;

	bool needToSave = false;
	if(id == 0)
	{
		id = NextId++;
		needToSave = true;
	}

	Notes.push_back(CNote(pos, note, id));
	if(id >= NextId)
	{
		NextId = id + 1;
	}
	if(needToSave)
	{
		saveNotes();
	}
}

void C3DNotes::removeNote(sint id)
{
	if (!NotesAvailable) return;

	for(list<CNote>::iterator it = Notes.begin(); it != Notes.end(); it++)
	{
		if((*it).Id == id)
		{
			Notes.erase(it);
			saveNotes();
			return;
		}
	}
}

NLMISC_COMMAND(note, "Add a 3d note at your location", "<note>")
{
	if(args.size() == 0)
		return false;

#ifdef NL_OS_WINDOWS
	char un[256];
	DWORD s = 256;
	GetUserName(un,&s);
#else
	char *un="";
#endif

	string note = toString("%s %s", un, IDisplayer::dateToHumanString());
	for (uint i = 0; i < args.size(); i++)
	{
		note += " " + args[i];
	}

	CVector pos = UserEntity->pos();

	Notes.addNote(pos, note);

	return true;
}

NLMISC_COMMAND(removeNote, "Remove a 3d note with its id", "<id>")
{
	if(args.size() == 0)
		return false;

	sint id = atoi(args[0].c_str());
	if(id == 0)
	{
		log.displayNL("0 is an invalid note id");
		return false;
	}

	Notes.removeNote(id);

	return true;
}

NLMISC_COMMAND(displayNotes, "display all notes", "")
{
	if(args.size() != 0)
		return false;

	log.displayNL("Displaying %d notes:", Notes.Notes.size());
	for(list<C3DNotes::CNote>::iterator it = Notes.Notes.begin(); it != Notes.Notes.end(); it++)
	{
		log.displayNL("id %d pos(%f,%f,%f) note '%s'", (*it).Id, (*it).Position.x, (*it).Position.y, (*it).Position.z, (*it).Note.c_str());
	}

	return true;
}

NLMISC_COMMAND(gotoNote, "go to a note", "<id>|<text>")
{
	if(args.size() == 0)
		return false;

	CVector pos(CVector::Null);

	sint id = atoi(args[0].c_str());
	if(id == 0)
	{
		// it's a text
		string tok;
		for(uint i = 0; i < args.size(); i++)
		{
			if(i != 0) tok += " ";
			tok += args[i];
		}
		tok = strlwr(tok);
		list<C3DNotes::CNote>::iterator it;
		for(it = Notes.Notes.begin(); it != Notes.Notes.end(); it++)
		{
			string note = strlwr((*it).Note);
			if(note.find(tok) != string::npos)
			{
				pos = (*it).Position;
				break;
			}
		}
		if(it == Notes.Notes.end())
		{
			log.displayNL("No notes match the text '%s'", tok.c_str());
		}
	}
	else
	{
		// it's an id
		list<C3DNotes::CNote>::iterator it;
		for(it = Notes.Notes.begin(); it != Notes.Notes.end(); it++)
		{
			if((*it).Id == id)
			{
				pos = (*it).Position;
				break;
			}
		}
		if(it == Notes.Notes.end())
		{
			log.displayNL("No notes match the id %d", id);
		}
	}

	if(pos != CVector::Null)
	{
		string cmd;
		if (ClientCfg.Local)
		{
			cmd = toString("pos %f %f %f", pos.x, pos.y, pos.z);
		}
		else
		{
			cmd = toString("a Position %f,%f", pos.x, pos.y);
		}
		log.displayNL("Go to note at position (%f,%f,%f)", pos.x, pos.y, pos.z);
		ICommand::execute(cmd, *InfoLog);
	}

	return true;
}


#if !FINAL_VERSION
NLMISC_COMMAND(debug, "open the debug window. alias to ah show_hide debug_info", "")
{
	if(args.size() != 0) return false;
	string cmd("ah show debug_info");
	ICommand::execute(cmd, log);
	return true;
}
#endif

// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdmisc.h"

#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/thread.h"

#include "nel/misc/window_displayer.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

class CUpdateThread : public IRunnable
{
	CWindowDisplayer *Disp;
	string WindowNameEx;
	sint X, Y, W, H, HS;
	bool Iconified;
	uint32 FS;
	string FN;
	bool WW;
	CLog *Log;

public:
	CUpdateThread (CWindowDisplayer *disp, string windowNameEx, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww, CLog *log) :
	  Disp(disp), WindowNameEx(windowNameEx), X(x), Y(y), W(w), H(h), HS(hs), Iconified(iconified), FS(fs), FN(fn), WW(ww), Log(log)
	{
	}

	void run()
	{
		Disp->open (WindowNameEx, Iconified, X, Y, W, H, HS, FS, FN, WW, Log);
		Disp->display_main ();
	}
};

CWindowDisplayer::~CWindowDisplayer ()
{
	// we have to wait the exit of the thread
	_Continue = false;
	nlassert (_Thread != NULL);
	_Thread->wait();
	delete _Thread;
}

bool CWindowDisplayer::update ()
{
	vector<string> copy;
	{
		CSynchronized<std::vector<std::string> >::CAccessor access (&_CommandsToExecute);
		copy = access.value();
		access.value().clear ();
	}

	// execute all commands in the main thread
	for (uint i = 0; i < copy.size(); i++)
	{
		nlassert (Log != NULL);
		ICommand::execute (copy[i], *Log);
	}

	return _Continue;
}

uint CWindowDisplayer::createLabel (const char *value)
{
	uint pos;
	{
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access (&_Labels);
		access.value().push_back (CLabelEntry(value));
		pos = (uint)access.value().size()-1;
	}
	return pos;
}

void CWindowDisplayer::setLabel (uint label, const string &value)
{
	{
		CSynchronized<std::vector<CLabelEntry> >::CAccessor access (&_Labels);
		nlassert (label < access.value().size());
		if (access.value()[label].Value != value)
		{
			access.value()[label].Value = value;
			access.value()[label].NeedUpdate = true;
		}
	}
}

void CWindowDisplayer::create (string windowNameEx, bool iconified, sint x, sint y, sint w, sint h, sint hs, sint fs, const std::string &fn, bool ww, CLog *log)
{
	nlassert (_Thread == NULL);
	_Thread = IThread::create (new CUpdateThread(this, windowNameEx, iconified, x, y, w, h, hs, fs, fn, ww, log));

	Log = log;

	_Thread->start ();
}

void CWindowDisplayer::doDisplay (const NLMISC::CLog::TDisplayInfo &args, const char *message)
{
	bool needSpace = false;
	//stringstream ss;
	string str;

	uint32 color = 0xFF000000;

	if (args.LogType != CLog::LOG_NO)
	{
		str += logTypeToString(args.LogType);
		if (args.LogType == CLog::LOG_ERROR || args.LogType == CLog::LOG_ASSERT) color = 0x00FF0000;
		else if (args.LogType == CLog::LOG_WARNING) color = 0x00800000;
		else if (args.LogType == CLog::LOG_DEBUG) color = 0x00808080;
		else color = 0;
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		if (needSpace) { str += " "; needSpace = false; }
#ifdef NL_OS_WINDOWS
		str += NLMISC::toString("%4x", args.ThreadId);
#else
		str += NLMISC::toString("%08x", args.ThreadId);
#endif
		needSpace = true;
	}

	if (args.FileName != NULL)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString("%20s", CFile::getFilename(args.FileName).c_str());
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString("%4u", args.Line);
		//ss << setw(4) << args.Line;
		needSpace = true;
	}

	if (args.FuncName != NULL)
	{
		if (needSpace) { str += " "; needSpace = false; }
		str += NLMISC::toString("%20s", args.FuncName);
		needSpace = true;
	}

	if (needSpace) { str += ": "; needSpace = false; }

	uint nbl = 1;

	char *npos, *pos = const_cast<char *>(message);
	while ((npos = strchr (pos, '\n')))
	{
		*npos = '\0';
		str += pos;
		if (needSlashR)
			str += "\r";
		str += "\n";
		*npos = '\n';
		pos = npos+1;
		nbl++;
	}
	str += pos;

	pos = const_cast<char *>(args.CallstackAndLog.c_str());
	while ((npos = strchr (pos, '\n')))
	{
		*npos = '\0';
		str += pos;
		if (needSlashR)
			str += "\r";
		str += "\n";
		*npos = '\n';
		pos = npos+1;
		nbl++;
	}
	str += pos;

	{
		CSynchronized<std::list<std::pair<uint32, std::string> > >::CAccessor access (&_Buffer);
		if (_HistorySize > 0 && access.value().size() >= (uint)_HistorySize)
		{
			access.value().erase (access.value().begin());
		}
		access.value().push_back (make_pair (color, str));
	}
}

} // NLMISC

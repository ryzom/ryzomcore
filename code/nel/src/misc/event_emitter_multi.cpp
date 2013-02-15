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
#include "nel/misc/event_emitter_multi.h"
#include "nel/misc/system_utils.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// a predicate to find an emitter in a list
struct EmitterEqualPred
{
	EmitterEqualPred(IEventEmitter *e) : E(e) {}
	IEventEmitter *E;
	bool operator()(const std::pair<IEventEmitter *, bool> &el)
	{
		return el.first == E;
	}
};

///============================================================
CEventEmitterMulti::~CEventEmitterMulti()
{
	for (TEmitterCont::iterator it = _Emitters.begin(); it != _Emitters.end(); ++it)
	{
		if (it->second) // asked "must Delete' ?
		{
			delete it->first;
		}
	}
}

///============================================================
void	CEventEmitterMulti::addEmitter(IEventEmitter *e, bool mustDelete)
{
	nlassert(e != this); // avoid infinite recursion
	nlassert(!isEmitter(e));
	_Emitters.push_back(std::make_pair(e, mustDelete));
}

///============================================================
void	CEventEmitterMulti::removeEmitter(IEventEmitter *e)
{
	TEmitterCont::iterator it = std::find_if(_Emitters.begin(), _Emitters.end(), EmitterEqualPred(e));
	nlassert(it!= _Emitters.end());
	if (it->second)
	{
		delete it->first;
	}
	_Emitters.erase(it);
}

///============================================================
bool	CEventEmitterMulti::isEmitter(IEventEmitter *e) const
{
	TEmitterCont::const_iterator it = std::find_if(_Emitters.begin(), _Emitters.end(), EmitterEqualPred(e));
	return it != _Emitters.end();
}

///============================================================
void CEventEmitterMulti::submitEvents(CEventServer &server, bool allWindows)
{
	for (TEmitterCont::iterator it = _Emitters.begin(); it != _Emitters.end(); ++it)
	{
		it->first->submitEvents(server, allWindows);
	}
}

///============================================================
void CEventEmitterMulti::emulateMouseRawMode(bool enable)
{
}

///============================================================
IEventEmitter *CEventEmitterMulti::getEmitter(uint index)
{
	nlassert(index < _Emitters.size());
	return _Emitters[index].first;
}

///============================================================
const IEventEmitter *CEventEmitterMulti::getEmitter(uint index) const
{
	nlassert(index < _Emitters.size());
	return _Emitters[index].first;
}

bool CEventEmitterMulti::copyTextToClipboard(const ucstring &text)
{
	// Naush: wrapped to old API to avoid duplicate code
	return CSystemUtils::copyTextToClipboard(text);
}

bool CEventEmitterMulti::pasteTextFromClipboard(ucstring &text)
{
	// Naush: wrapped to old API to avoid duplicate code
	return CSystemUtils::pasteTextFromClipboard(text);
}


} // NLMISC

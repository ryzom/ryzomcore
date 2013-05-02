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

#ifndef NL_EVENT_EMITTER_MULTI_H
#define NL_EVENT_EMITTER_MULTI_H

#include "types_nl.h"
#include "event_emitter.h"
#include <vector>


namespace NLMISC {


/** The composite pattern applied to events emmitters.
 * This is useful when you don't have the opportunity to register more than
 * one event emitter to an event server.
 */
class CEventEmitterMulti : public IEventEmitter
{
public:
	/// dtor
	virtual ~CEventEmitterMulti();
	/// add an emitter
	void	addEmitter(IEventEmitter *e, bool mustDelete);
	/// remove an emitter (and delete it if necessary)
	void	removeEmitter(IEventEmitter *e);
	/// test whether e is in the emitter list
	bool	isEmitter(IEventEmitter *e) const;
	// Get the number of registered emitters
	uint	getNumEmitters() const { return (uint)_Emitters.size(); }
	// Get an emitter
	IEventEmitter *getEmitter(uint index);
	const IEventEmitter *getEmitter(uint index) const;
	/// From IEventEmitter. This call submitEvents on all the emitters
	virtual void submitEvents(CEventServer &server, bool allWindows);
	virtual void emulateMouseRawMode(bool enable);

	virtual bool copyTextToClipboard(const ucstring &text);
	virtual bool pasteTextFromClipboard(ucstring &text);

private:
	typedef std::vector<std::pair<IEventEmitter *, bool> > TEmitterCont;
	TEmitterCont	_Emitters;
};


} // NLMISC


#endif // NL_EVENT_EMITTER_MULTI_H

/* End of event_emitter_multi.h */

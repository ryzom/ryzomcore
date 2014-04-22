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

#ifndef SPEAKER_LISTENER_H
#define SPEAKER_LISTENER_H


#include "types_nl.h"

namespace NLMISC
{
	class ISpeaker;

	class IListener
	{
	public:
		virtual void speakerIsDead	(ISpeaker *speaker) =0;
	};

	class ISpeaker
	{
	public:
		virtual void registerListener	(IListener *listener) =0;
		virtual void unregisterListener	(IListener *listener) =0;
	};

	template <class Listener>
	class CSpeaker : public ISpeaker
	{
	public:
		typedef std::set<IListener*>	TListeners;

	private:
		TListeners		_Listeners;

	public:

		~CSpeaker()
		{
			while (!_Listeners.empty())
			{
				IListener *listener = *_Listeners.begin();
				listener->speakerIsDead(this);
				_Listeners.erase(_Listeners.begin());
			}
		}

		void registerListener	(IListener *listener)
		{
			_Listeners.insert(listener);
		}
		void unregisterListener	(IListener *listener)
		{
			_Listeners.erase(listener);
		}

		const TListeners &getListeners()
		{
			return _Listeners;
		}

	};


	/** A macro to facilitate method invocation on listeners */
#define NLMISC_BROADCAST_TO_LISTENER(listenerClass, methodAndParam)	\
	CSpeaker<listenerClass>::TListeners::const_iterator first(CSpeaker<listenerClass>::getListeners().begin()), last(CSpeaker<listenerClass>::getListeners().end()); \
	for (; first != last; ++first)				\
	{											\
		listenerClass *listener = static_cast<listenerClass*>(*first);		\
												\
		listener->methodAndParam;				\
	}											\



	template <class Speaker>
	class CListener : public IListener
	{
		ISpeaker	*_Speaker;

		void speakerIsDead(ISpeaker *speaker)
		{
			nlassert(speaker == _Speaker);
			_Speaker = NULL;
		}

	public:

		CListener()
			: _Speaker(NULL)
		{
		}

		~CListener()
		{
			if (_Speaker != NULL)
				_Speaker->unregisterListener(this);
		}

		void registerListener(ISpeaker *speaker)
		{
			nlassert(_Speaker == NULL);
			_Speaker = speaker;
			_Speaker->registerListener(this);
		}

		void unregisterListener(ISpeaker * /* speaker */)
		{
			nlassert(_Speaker != NULL);
			_Speaker->unregisterListener(this);
			_Speaker = NULL;
		}

		ISpeaker *getSpeaker()
		{
			return _Speaker;
		}

	};


} // NLMISC

#endif // SPEAKER_LISTENER_H

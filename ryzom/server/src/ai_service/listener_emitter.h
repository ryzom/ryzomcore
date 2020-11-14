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



#ifndef	LISTENER_EMITTER
#define	LISTENER_EMITTER

/************************************************************************/
//	Two classes to use here : CListener and CEmitter
//	its an abstract event service.
//	CLink and CListenerEmitterLink mist not be used.
//	they have not been tested yet.
//	implementation must be cleaned after first use validation.
/************************************************************************/


//	only used in listener and emitters classes
template	<class	T>
class	CLink
{
public:
	
	CLink	()	:	_previous(NULL),	_next(NULL)
	{
	}
	virtual ~CLink	()
	{
	}

	void	attachAfter	(T*	other,	const	int&	which)
	{
		other.list[which].detach(which);
		if (_next)
			_next->list[which].previous	=	other;
		other.list[which].next		=	_next;
		_next			=	other;
		other.list[which].previous	=	this;
	}

	void	detach	(const	int&	which)
	{
		if (_previous)	
			_previous->list[which]._next	=	_next;
		if (_next)
			_next->list[which].previous		=	_previous;
		_previous	=	_next	=	NULL;
	}

//protected:
//private:
	T*	_previous;
	T*	_next;
};

template <class T> class CListenerEmitterLink;
template <class T> class CListener;
template <class T> class CEmitter;


//	listener system classes. never used. to keep.

template <class T>
class	CListenerEmitterLink
{
public:
	CListenerEmitterLink () : _listener(NULL), _emitter(NULL)
	{		
	}
	
	CListenerEmitterLink (CListener<T>* listener, CEmitter<T>* emitter) : _listener(listener), _emitter(emitter)
	{		
	}
	enum
	{
		LISTENER=0,
		EMITTER,
		MAXLINK
	};
	
	
	virtual ~CListenerEmitterLink	()
	{
		link[LISTENER].detach	(LISTENER);
		link[EMITTER].detach	(EMITTER);
	}
	
//protected:
//private:
	CLink<CListenerEmitterLink<T> >	link[MAXLINK];	//_nextListener;

	CListener<T>*	_listener;
	CEmitter<T>*	_emitter;
};


//	T is the message type.
template	<class	T>
class	CListener
{
public:
	CListener	()
	{
	}
	
	virtual ~CListener	()
	{
		CListenerEmitterLink<T>*	current=_emitterList._next;
		while (current!=NULL)
		{
			CListenerEmitterLink<T>*	nextcurrent=current->link[CListenerEmitterLink::EMITTER]._next;
			delete	current;
			current=nextcurrent;
		}
		
	}
	
	virtual	void	append(const	T&	message)	=	0;
	
	//protected:
	//private:
	CLink<CListenerEmitterLink<T> >	_emitterList;
};

template	<class	T>
class	CEmitter
{
public:
	CEmitter	()
	{
	}
	virtual ~CEmitter	()
	{
		CListenerEmitterLink<T>*	current=_listenerList._next;
		while (current!=NULL)
		{
			CListenerEmitterLink<T>*	nextcurrent=current->link[CListenerEmitterLink::LISTENER]._next;
			delete	current;
			current=nextcurrent;
		}
	}
	
	CListenerEmitterLink<T>*	find	(CListener*	listener)
	{
		CListenerEmitterLink<T>*	current=_listenerList._next;
		while (current!=NULL	&&	current->_listener!=listener)
		{
			current=current->list[CListenerEmitterLink::LISTENER]._next;
		};
		return	current;
	}

	void	addListener		(CListener*	listener)
	{
		if	(!find(listener))
		{
			CListenerEmitterLink<T>*	link=new	CListenerEmitterLink<T>(listener,this);
			_listenerList.attachAfter			(link,CListenerEmitterLink::LISTENER);
			listener->_emitterList.attachAfter	(link,CListenerEmitterLink::EMITTER);
		}

	}

	void	removeListener	(CListener*	listener)
	{
		CListenerEmitterLink<T>*	link=find(listener);
		if	(link)
			delete	link;	//	automatic detach ..
	}
	
	void	fireEvent	(const	T&	event)
	{
		CListenerEmitterLink<T>*	current=_listenerList._next;
		
		while (current!=NULL)
		{
			current->_listener->append(event);
			current=current->link[CListenerEmitterLink::LISTENER]._next;
		}

	}
	
//protected:
//private:
	CLink<CListenerEmitterLink<T> >	_listenerList;
};

#endif

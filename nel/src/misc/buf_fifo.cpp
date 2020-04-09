// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/misc/time_nl.h"
#include "nel/misc/command.h"
#include "nel/misc/buf_fifo.h"

using namespace std;

#define DEBUG_FIFO 0

// if 0, don't stat the time of different function
#define STAT_FIFO 1

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

#ifdef BUFFIFO_TRACK_ALL_BUFFERS
CBufFIFO::TAllBuffers		CBufFIFO::_AllBuffers;
#endif


CBufFIFO::CBufFIFO() : _Buffer(NULL), _BufferSize(0), _Empty(true), _Head(NULL), _Tail(NULL), _Rewinder(NULL)
{
#ifdef BUFFIFO_TRACK_ALL_BUFFERS
	_AllBuffers.insert(this);
#endif

	// reset statistic
	_BiggestBlock = 0;
	_SmallestBlock = 999999999;
	_BiggestBuffer = 0;
	_SmallestBuffer = 999999999;
	_Pushed = 0;
	_Fronted = 0;
	_Resized = 0;
	_PushedTime = 0;
	_FrontedTime = 0;
	_ResizedTime = 0;
}

CBufFIFO::~CBufFIFO()
{
#ifdef BUFFIFO_TRACK_ALL_BUFFERS
	_AllBuffers.erase(this);
#endif

	if (_Buffer != NULL)
	{
		delete []_Buffer;
#if DEBUG_FIFO
		nldebug("%p delete", this);
#endif
	}
}

void	 CBufFIFO::push (const uint8 *buffer, uint32 s)
{
	// if the buffer is more than 1 meg, there s surely a problem, no?
//	nlassert( buffer.size() < 1000000 ); // size check in debug mode

#if STAT_FIFO
	TTicks before = CTime::getPerformanceTime();
#endif

#if DEBUG_FIFO
	nldebug("%p push(%d)", this, s);
#endif

	nlassert(s > 0 && s < pow(2.0, static_cast<double>(sizeof(TFifoSize)*8)));

	// stat code
	if (s > _BiggestBlock) _BiggestBlock = s;
	if (s < _SmallestBlock) _SmallestBlock = s;
	_Pushed++;

	while (!canFit (s + sizeof (TFifoSize)))
	{
		resize(_BufferSize * 2);
	}

	*(TFifoSize *)_Head = s;
	_Head += sizeof(TFifoSize);

	CFastMem::memcpy(_Head, buffer, s);

	_Head += s;

	_Empty = false;

#if STAT_FIFO
	// stat code
	TTicks after = CTime::getPerformanceTime();
	_PushedTime += after - before;
#endif

#if DEBUG_FIFO
	display ();
#endif
}

void CBufFIFO::push(const std::vector<uint8> &buffer1, const std::vector<uint8> &buffer2)
{
#if STAT_FIFO
	TTicks before = CTime::getPerformanceTime();
#endif

	TFifoSize s = (TFifoSize)(buffer1.size() + buffer2.size());

#if DEBUG_FIFO
	nldebug("%p push2(%d)", this, s);
#endif

	nlassert((buffer1.size() + buffer2.size ()) > 0 && (buffer1.size() + buffer2.size ()) < pow(2.0, static_cast<double>(sizeof(TFifoSize)*8)));

	// avoid too big fifo
	if (this->size() > 10000000)
	{
		throw Exception ("CBufFIFO::push(): stack full (more than 10mb)");
	}


	// stat code
	if (s > _BiggestBlock) _BiggestBlock = s;
	if (s < _SmallestBlock) _SmallestBlock = s;

	_Pushed++;

	// resize while the buffer is enough big to accept the block
	while (!canFit (s + sizeof (TFifoSize)))
	{
		resize(_BufferSize * 2);
	}

	// store the size of the block
	*(TFifoSize *)_Head = s;
	_Head += sizeof(TFifoSize);

	// store the block itself
	CFastMem::memcpy(_Head, &(buffer1[0]), buffer1.size());
	CFastMem::memcpy(_Head + buffer1.size(), &(buffer2[0]), buffer2.size());
	_Head += s;

	_Empty = false;

#if STAT_FIFO
	// stat code
	TTicks after = CTime::getPerformanceTime();
	_PushedTime += after - before;
#endif

#if DEBUG_FIFO
	display ();
#endif
}

void CBufFIFO::pop ()
{
	if (empty ())
	{
		nlwarning("BF: Try to pop an empty fifo!");
		return;
	}

	if (_Rewinder != NULL && _Tail == _Rewinder)
	{
#if DEBUG_FIFO
		nldebug("%p pop rewind!", this);
#endif

		// need to rewind
		_Tail = _Buffer;
		_Rewinder = NULL;
	}

	TFifoSize s = *(TFifoSize *)_Tail;

#if DEBUG_FIFO
	nldebug("%p pop(%d)", this, s);
#endif

#ifdef NL_DEBUG
	// clear the message to be sure user doesn't use it anymore
	memset (_Tail, '-', s + sizeof (TFifoSize));
#endif

	_Tail += s + sizeof (TFifoSize);

	if (_Tail == _Head) _Empty = true;

#if DEBUG_FIFO
	display ();
#endif
}

uint8 CBufFIFO::frontLast ()
{
	uint8	*tail = _Tail;

	if (empty ())
	{
		nlwarning("BF: Try to get the front of an empty fifo!");
		return 0;
	}

	if (_Rewinder != NULL && tail == _Rewinder)
	{
#if DEBUG_FIFO
		nldebug("%p front rewind!", this);
#endif

		// need to rewind
		tail = _Buffer;
	}

	TFifoSize s = *(TFifoSize *)tail;

#if DEBUG_FIFO
	nldebug("%p frontLast() returns %d ", this, s, *(tail+sizeof(TFifoSize)+size-1));
#endif

	return *(tail+sizeof(TFifoSize)+s-1);
}


void CBufFIFO::front (vector<uint8> &buffer)
{
	uint8 *tmpbuffer;
	uint32 s;

	buffer.clear ();

	front (tmpbuffer, s);

	buffer.resize (s);

	CFastMem::memcpy (&(buffer[0]), tmpbuffer, s);

/*	TTicks before = CTime::getPerformanceTime ();

	uint8	*tail = _Tail;

	buffer.clear ();

	if (empty ())
	{
		nlwarning("Try to get the front of an empty fifo!");
		return;
	}

	_Fronted++;

	if (_Rewinder != NULL && tail == _Rewinder)
	{
#if DEBUG_FIFO
		nldebug("%p front rewind!", this);
#endif

		// need to rewind
		tail = _Buffer;
	}

	TFifoSize size = *(TFifoSize *)tail;

#if DEBUG_FIFO
	nldebug("%p front(%d)", this, size);
#endif

	tail += sizeof (TFifoSize);

	buffer.resize (size);

	CFastMem::memcpy (&(buffer[0]), tail, size);

	// stat code
	TTicks after = CTime::getPerformanceTime ();
	_FrontedTime += after - before;

#if DEBUG_FIFO
	display ();
#endif
*/}


void CBufFIFO::front (NLMISC::CMemStream &buffer)
{
	uint8 *tmpbuffer;
	uint32 s;

	buffer.clear ();

	front (tmpbuffer, s);

	buffer.fill (tmpbuffer, s);

	/*
	TTicks before = CTime::getPerformanceTime ();

	uint8	*tail = _Tail;

	buffer.clear ();

	if (empty ())
	{
		nlwarning("Try to get the front of an empty fifo!");
		return;
	}

	_Fronted++;

	if (_Rewinder != NULL && tail == _Rewinder)
	{
#if DEBUG_FIFO
		nldebug("%p front rewind!", this);
#endif

		// need to rewind
		tail = _Buffer;
	}

	TFifoSize size = *(TFifoSize *)tail;

#if DEBUG_FIFO
	nldebug("%p front(%d)", this, size);
#endif

	tail += sizeof (TFifoSize);

	//buffer.resize (size);
	//CFastMem::memcpy (&(buffer[0]), tail, size);

	buffer.fill (tail, size);

	// stat code
	TTicks after = CTime::getPerformanceTime ();
	_FrontedTime += after - before;

#if DEBUG_FIFO
	display ();
#endif*/
}

void CBufFIFO::front (uint8 *&buffer, uint32 &s)
{
#if STAT_FIFO
	TTicks before = CTime::getPerformanceTime ();
#endif

	uint8	*tail = _Tail;

	if (empty ())
	{
		nlwarning("BF: Try to get the front of an empty fifo!");
		return;
	}

	_Fronted++;

	if (_Rewinder != NULL && tail == _Rewinder)
	{
#if DEBUG_FIFO
		nldebug("%p front rewind!", this);
#endif

		// need to rewind
		tail = _Buffer;
	}

	s = *(TFifoSize *)tail;

#if DEBUG_FIFO
	nldebug("%p front(%d)", this, s);
#endif

	tail += sizeof (TFifoSize);

#if STAT_FIFO
	// stat code
	TTicks after = CTime::getPerformanceTime ();
	_FrontedTime += after - before;
#endif

#if DEBUG_FIFO
	display ();
#endif

	buffer = tail;
}



void CBufFIFO::clear ()
{
	_Tail = _Head = _Buffer;
	_Rewinder = NULL;
	_Empty = true;
}

uint32 CBufFIFO::size ()
{
	if (empty ())
	{
		return 0;
	}
	else if (_Head == _Tail)
	{
		// buffer is full
		if (_Rewinder == NULL)
			return _BufferSize;
		else
			return (uint32)(_Rewinder - _Buffer);
	}
	else if (_Head > _Tail)
	{
		return (uint32)(_Head - _Tail);
	}
	else if (_Head < _Tail)
	{
		nlassert (_Rewinder != NULL);
		return (uint32)((_Rewinder - _Tail) + (_Head - _Buffer));
	}
	nlstop;
	return 0;
}

void CBufFIFO::resize (uint32 s)
{
#if STAT_FIFO
	TTicks before = CTime::getPerformanceTime();
#endif

	if (s == 0) s = 100;

#if DEBUG_FIFO
	nldebug("%p resize(%d)", this, s);
#endif

	if (s > _BiggestBuffer) _BiggestBuffer = s;
	if (s < _SmallestBuffer) _SmallestBuffer = s;

	_Resized++;

	uint32 UsedSize = CBufFIFO::size();

	// create a new array and copy the old in the new one
	if (s < _BufferSize && UsedSize > s)
	{
		// problem, we don't have enough room for putting data => don't do it
		nlwarning("BF: Can't resize the FIFO because there's not enough room in the new wanted buffer (%d bytes needed at least)", UsedSize);
		return;
	}

	uint8 *NewBuffer = new uint8[s];
	if (NewBuffer == NULL)
	{
		nlerror("Not enough memory to resize the FIFO to %u bytes", s);
	}
#ifdef NL_DEBUG
	// clear the message to be sure user doesn't use it anymore
	memset (NewBuffer, '-', s);
#endif

#if DEBUG_FIFO
	nldebug("%p new %d bytes", this, s);
#endif

	// copy the old buffer to the new one
	// if _Tail == _Head => empty fifo, don't copy anything
	if (!empty())
	{
		if (_Tail < _Head)
		{
			CFastMem::memcpy (NewBuffer, _Tail, UsedSize);
		}
		else if (_Tail >= _Head)
		{
			nlassert (_Rewinder != NULL);

			uint size1 = (uint)(_Rewinder - _Tail);
			CFastMem::memcpy (NewBuffer, _Tail, size1);
			uint size2 = (uint)(_Head - _Buffer);
			CFastMem::memcpy (NewBuffer + size1, _Buffer, size2);

			nlassert (size1+size2==UsedSize);
		}
	}

	// resync the circular pointer
	// Warning: don't invert these 2 lines position or it ll not work
	_Tail = NewBuffer;
	_Head = NewBuffer + UsedSize;
	_Rewinder = NULL;

	// delete old buffer if needed
	if (_Buffer != NULL)
	{
		delete []_Buffer;
#if DEBUG_FIFO
		nldebug ("delete", this);
#endif
	}

	// affect new buffer
	_Buffer = NewBuffer;
	_BufferSize = s;

#if STAT_FIFO
	TTicks after = CTime::getPerformanceTime();
	_ResizedTime += after - before;
#endif

#if DEBUG_FIFO
	display ();
#endif
}

void CBufFIFO::displayStats (CLog *log)
{
	log->displayNL ("%p CurrentQueueSize: %d, TotalQueueSize: %d", this, size(), _BufferSize);
	log->displayNL ("%p InQueue: %d", this, _Pushed - _Fronted);

	log->displayNL ("%p BiggestBlock: %d, SmallestBlock: %d", this, _BiggestBlock, _SmallestBlock);
	log->displayNL ("%p BiggestBuffer: %d, SmallestBuffer: %d", this, _BiggestBuffer, _SmallestBuffer);
	log->displayNL ("%p Pushed: %d, PushedTime: total %" NL_I64 "d ticks, mean %f ticks", this, _Pushed, _PushedTime, (_Pushed>0?(double)(sint64)_PushedTime / (double)_Pushed:0.0));
	log->displayNL ("%p Fronted: %d, FrontedTime: total %" NL_I64 "d ticks, mean %f ticks", this, _Fronted, _FrontedTime, (_Fronted>0?(double)(sint64)_FrontedTime / (double)_Fronted:0.0));
	log->displayNL ("%p Resized: %d, ResizedTime: total %" NL_I64 "d ticks, mean %f ticks", this, _Resized, _ResizedTime, (_Resized>0?(double)(sint64)_ResizedTime / (double)_Resized:0.0));
}

void CBufFIFO::display ()
{
	int s = 64;
	int gran = s/30;

	char str[1024];

	smprintf(str, 1024, "%p %p (%5d %5d) %p %p %p ", this, _Buffer, _BufferSize, CBufFIFO::size(), _Rewinder, _Tail, _Head);

	int i;
	for (i = 0; i < (sint32) _BufferSize; i+= gran)
	{
		uint8 *pos = _Buffer + i;
		if (_Tail >= pos && _Tail < pos + gran)
		{
			if (_Head >= pos && _Head < pos + gran)
			{
				if (_Rewinder != NULL && _Rewinder >= pos && _Rewinder < pos + gran)
				{
					strncat (str, "*", 1);
				}
				else
				{
					strncat (str, "@", 1);
				}
			}
			else
			{
				strncat (str, "T", 1);
			}
		}
		else if (_Head >= pos && _Head < pos + gran)
		{
			strncat (str, "H", 1);
		}
		else if (_Rewinder != NULL && _Rewinder >= pos && _Rewinder < pos + gran)
		{
			strncat (str, "R", 1);
		}
		else
		{
			if (strlen(str) < 1023)
			{
				uint32 p = (uint32)strlen(str);
				if (isprint(*pos))
					str[p] = *pos;
				else
					str[p] = '$';

				str[p+1] = '\0';
			}
		}
	}

	for (; i < s; i+= gran)
	{
		strncat (str, " ", 1);
	}
#ifdef NL_DEBUG
	strncat (str, "\n", 1);
#else
	strncat (str, "\r", 1);
#endif
	DebugLog->display (str);
}

bool CBufFIFO::canFit (uint32 s)
{
	if (_Tail == _Head)
	{
		if (empty())
		{
			// is the buffer large enough?
			if (_BufferSize >= s)
			{
				// reset the pointer
#if DEBUG_FIFO
				nldebug("%p reset tail and head", this);
#endif
				_Head = _Tail = _Buffer;
				return true;
			}
			else
			{
				// buffer not big enough
#if DEBUG_FIFO
				nldebug("%p buffer full buffersize<size", this);
#endif
				return false;
			}
		}
		else
		{
			// buffer full
#if DEBUG_FIFO
			nldebug("%p buffer full h=t", this);
#endif
			return false;
		}
	}
	else if (_Tail < _Head)
	{
		if (_Buffer + _BufferSize - _Head >= (sint32) s)
		{
			// can fit after _Head
#if DEBUG_FIFO
			nldebug("%p fit after", this);
#endif
			return true;
		}
		else if (_Tail - _Buffer >= (sint32) s)
		{
			// can fit at the beginning
#if DEBUG_FIFO
			nldebug("%p fit at beginning", this);
#endif
			_Rewinder = _Head;
#if DEBUG_FIFO
			nldebug("%p set the rewinder", this);
#endif
			_Head = _Buffer;
			return true;
		}
		else
		{
			// can't fit
#if DEBUG_FIFO
			nldebug("%p no room t<h", this);
#endif
			return false;
		}
	}
	else // the last case is : if (_Tail > _Head)
	{
		if (_Tail - _Head >= (sint32) s)
		{
#if DEBUG_FIFO
			nldebug("%p fit t>h", this);
#endif
			return true;
		}
		else
		{
#if DEBUG_FIFO
			nldebug("%p no room t>h", this);
#endif
			return false;
		}
	}
}

#ifdef BUFFIFO_TRACK_ALL_BUFFERS
NLMISC_CATEGORISED_COMMAND(misc, dumpAllBuffers, "Dump all the fifo buffer", "no args")
{
	log.displayNL("Dumping %u FIFO buffers :", CBufFIFO::_AllBuffers.size());

	CBufFIFO::TAllBuffers::iterator first(CBufFIFO::_AllBuffers.begin()), last(CBufFIFO::_AllBuffers.end());
	for (; first != last; ++first)
	{
		CBufFIFO *buf = *first;

		log.displayNL("Dumping buffer %p:", buf);

		buf->displayStats(&log);
	}

	return true;
}
#endif


} // NLMISC

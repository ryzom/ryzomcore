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

#ifndef NL_BUF_FIFO_H
#define NL_BUF_FIFO_H

#include "types_nl.h"

#include <vector>

#include "time_nl.h"
#include "mem_stream.h"
#include "command.h"


namespace NLMISC {

#undef BUFFIFO_TRACK_ALL_BUFFERS


/**
 * This class is a dynamic size FIFO that contains variable size uint8 buffer.
 * It's used in the layer1 network for storing temporary messages.
 * You can resize the internal FIFO buffer if you know the average size
 * of data you'll put in it. It have the same behavior as STL so if the
 * buffer is full the size will be automatically increase by 2.
 *
 * TODO: Add a method getMsgNb() that will return the number of messages in queue.
 * For acceptable performance, it would need to store the current number instead
 * of browsing the blocks.
 *
 * \code
 	CBufFIFO fifo;
	fifo.resize(10000);
	vector<uint8> vec;
	vec.resize(rand()%256);
	memset (&(vec[0]), '-', vec.size());
	// push the vector
	fifo.push(vec);
	// display the fifo
	fifo.display();
	vector<uint8> vec2;
	// get the vector
	fifo.pop(vec2);
 * \endcode
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CBufFIFO
{
public:

	CBufFIFO ();
	~CBufFIFO ();

	/// Push 'buffer' in the head of the FIFO
	void	 push (const std::vector<uint8> &buffer) { push (&buffer[0], (uint32)buffer.size()); }

	void	 push (const NLMISC::CMemStream &buffer) { push (buffer.buffer(), buffer.length()); }

	void	 push (const uint8 *buffer, uint32 size);

	/// Concate and push 'buffer1' and buffer2 in the head of the FIFO. The goal is to avoid a copy
	void	 push (const std::vector<uint8> &buffer1, const std::vector<uint8> &buffer2);

	/// Get the buffer in the tail of the FIFO and put it in 'buffer'
	void	 front (std::vector<uint8> &buffer);

	void	 front (NLMISC::CMemStream &buffer);

	void	 front (uint8 *&buffer, uint32 &size);

	/// This function returns the last byte of the front message
	/// It is used by the network to know a value quickly without doing front()
	uint8	 frontLast ();


	/// Pop the buffer in the tail of the FIFO
	void	 pop ();

	/// Set the size of the FIFO buffer in byte
	void	 resize (uint32 size);

	/// Return true if the FIFO is empty
	bool	 empty () { return _Empty; }


	/// Erase the FIFO
	void	 clear ();

	/// Returns the size of the FIFO
	uint32	 size ();

	/// display the FIFO to stdout (used to debug the FIFO)
	void	 display ();

	/// display the FIFO statistics (speed, nbcall, etc...) to stdout
	void	 displayStats (CLog *log = InfoLog);

private:

	typedef uint32 TFifoSize;

	// pointer to the big buffer
	uint8	*_Buffer;
	// size of the big buffer
	uint32	 _BufferSize;

	// true if the bufffer is empty
	bool	 _Empty;

	// head of the FIFO
	uint8	*_Head;
	// tail of the FIFO
	uint8	*_Tail;
	// pointer to the rewinder of the FIFO
	uint8	*_Rewinder;

	// return true if we can put size bytes on the buffer
	// return false if we have to resize
	bool	 canFit (uint32 size);


	// statistics of the FIFO
	uint32 _BiggestBlock;
	uint32 _SmallestBlock;
	uint32 _BiggestBuffer;
	uint32 _SmallestBuffer;
	uint32 _Pushed ;
	uint32 _Fronted;
	uint32 _Resized;
	TTicks _PushedTime;
	TTicks _FrontedTime;
	TTicks _ResizedTime;

#ifdef BUFFIFO_TRACK_ALL_BUFFERS
	typedef std::set<CBufFIFO*> TAllBuffers;
	// All the buffer for debug output
	static TAllBuffers		_AllBuffers; // WARNING: not mutexed, can produce some crashes!
#endif

	NLMISC_CATEGORISED_COMMAND_FRIEND(misc, dumpAllBuffers);
};


} // NLMISC


#endif // NL_BUF_FIFO_H

/* End of buf_fifo.h */

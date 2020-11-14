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

#ifndef RY_INTER_WINDOW_MSG_QUEUE_H
#define RY_INTER_WINDOW_MSG_QUEUE_H

#include "types_nl.h"

#ifdef NL_OS_WINDOWS

#include "nel/misc/thread.h"
#include "nel/misc/mutex.h"
#include "nel/misc/shared_memory.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/dummy_window.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINDOWS
#	define _WIN32_WINDOWS 0x0500
#endif
#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
#	define WINVER 0x0500
#endif
#ifndef NOMINMAX
#	define NOMINMAX
#endif
#include <windows.h>

namespace NLMISC
{

// **************************************************************************************************
/** IPC Utility class to enable easy window-window communication
  *
  * This enable 2-ways non blocking communication between 2 windows on the same machine (windows
  * may belong to different processes)
  *
  * Only works on microsoft windows os for now
  *
  * The implementation relies on the WM_COPYDATA message to enable communication, but create a separate thread
  * to enable non blocking exchange. (The WM_COPYDATA message require the use of SendMessage, which is blocking until the
  * target window receive the message and respond)
  *
  * Identifier should be agreed on for the 2 windows that are to communicate (windows handles are transparently
  * exchanged through shared memory based on this name)
  *
  * When initializing the queue for a specific window, this window message proc will be subclassed
  * to enable interception of the WM_COPYDATA for that window.
  *
  * 'release' should be called before the hooked window is destroyed, to avoid dangling reference to it
  * in this object. As one can expect 'release' is also automatically called when this object is destroyed
  *
  * Additionnaly, an internal, invisible window will be create if no one is given (second form of 'init'). This avoid having to worry
  * about order of init / release of subclassing.
  *
  * FIXME : In fact, creating an invisible, internal window (that is required to have a message queue), should be the default,
  * this would hide the internal communication mean, and we simply would have something called like 'CInterProcessMsgQueue'.
  * Alternatives have been considered, but this one seemed simpler at first because of the facility offered by WM_COPYDATA
  * Other possible implementations include shared memory (would require additionnal synchronisation & possible splitting of messages then), sockets (would make the firewall complain then ...) etc.
  * .
  */

class CInterWindowMsgQueue
{
public:
	CInterWindowMsgQueue();
	~CInterWindowMsgQueue();
	/** Create a 2-way inter process message queue.
	  * Each ownerWindow / localId / foreignId triplet should be unique, repeated call with the same value will return false
	  * as long as the message queue created from them is alive
	  * IMPORTANT: 'ownerWindow' will be subclassed to handle the messages. If multiple subclassing are done on that window,
	  * they must be undone in reverse order, or an assertion will be raised. The simplest thing is use the second form of init,
	  * which will create an internal invisible window, thusavoiding this concern.
	  */
	bool init(HINSTANCE hInstance, uint32 localId, uint32 foreignId);
	/** Create a 2-way message queue between 2 windows.
	  * Each ownerWindow / localId / foreignId triplet should be unique, repeated call with the same value will return false
	  * as long as the message queue created from them is alive
	  * IMPORTANT: 'ownerWindow' will be subclassed to handle the messages. If multiple subclassing are done on that window,
	  * they must be undone in reverse order, or an assertion will be raised. The simplest thing is use the second form of init,
	  * which will create an internal invisible window, thusavoiding this concern.
	  *
	  * return true on success
	  */
	bool init(HWND ownerWindow, uint32 localId, uint32 foreignId);
	/** Release the msg queue
	  * This will unhook the window, restoring its previous message procedure
	  * Note than if the window was hooked by someone else, an assert will be raised
	  * Hooks on windows msg proc should thus be 'unhooked' in reverse order
	  */
	void release();
	/** Send a new message
	  * The msg is guaranteed to be received by the other window as long as it remains alive
	  */
	void sendMessage(CMemStream &msg);
	// See if a message has arrived from foreign window, return true if so
	bool pumpMessage(CMemStream &dest);
	// Test if other window is present
	bool connected() const;
	//
	uint getSendQueueSize() const;
	uint getReceiveQueueSize() const;


// **************************************************************************************************
private:
	struct CMsg
	{
		std::vector<uint8> Msg;
		void serial(NLMISC::IStream &f)
		{
			f.serialVersion(0);
			f.serialCont(Msg);
		}
	};

	typedef std::list<CMsg> TMsgList;
	CSynchronized<TMsgList> _OutMessageQueue; // outgoing messages : filled by main thread, pumped by
													  // this 'CInterWindowMsgQueue' internal thread to hide latency of SendMessage
													  // to the main thread

	TMsgList						_InMessageQueue;  // Incoming messages : not synchronised here, because the wnd
												      // message proc that receive foreign window messages (through WM_COPYDATA)
													  // belong to the same thread than the reader (that calls 'pumpMessage')

	CDummyWindow					_DummyWindow;


	// internal send thread
	class CSendTask : public IRunnable
	{
	public:
		CSendTask(CInterWindowMsgQueue *parent);
		// from IRunnable
		virtual void run();
		// parent should call this to ask the thread to terminate
		void stop();
	private:
		CInterWindowMsgQueue *_Parent;
		bool				  _StopAsked;
	};

	friend class CSendTask;

	/** One protagonist in the communication. May be the 'local' of 'foreign' window
	  * The responsbility of this class is to allow to set / retrieve the window handle
	  * of local / foreign window.
	  * Window handle is needed by the send thread when it need to call SendMessage
	  */
	class CProtagonist
	{
	public:
		CProtagonist();
		~CProtagonist();
		void release();
		// init this 'protagonist'
		bool init(uint32 id);
		// set handle for foreign window
		void setWnd(HWND wnd);
		// get local or foreign window handle
		HWND getWnd();
		uint32 getId() const { return _Id; }
	private:
		uint32					_Id;
		HWND					_Wnd;
		HANDLE					_SharedMemMutex;			// system-wide mutex for access to window handle in shared memory
		void					*_SharedWndHandle; // no need for mutex here as the value will be written atomically (a single memory word)
	private:
		// shared memory mutex
		void acquireSMMutex();
		void releaseSMMutex();
	};
	//
	CProtagonist		_LocalWindow;
	CProtagonist		_ForeignWindow;
	CSendTask			*_SendTask;
	IThread				*_SendThread;
	static const uint   _CurrentVersion; // for messages serialisation




	// Unique identifier for a 2-way message queue (that is, a CInterWindowMsgQueue object after it has been
	// initialized)
	// This is required since the window message proc 'ListenerProc' is static so we must be
	// retrieve the associated 'CInterWindowMsgQueue' object from the (local id, foreign id) pair
	class CMsgQueueIdent
	{
	public:
		HWND Wnd;
		uint32 LocalId;
		uint32 ForeignId;
	public:
		CMsgQueueIdent(HWND wnd, uint32 localId, uint32 foreignId)
					 : Wnd(wnd), LocalId(localId), ForeignId(foreignId) {}
		bool operator < (const CMsgQueueIdent &other) const
		{
			if (Wnd != other.Wnd) return Wnd < other.Wnd;
			if (LocalId != other.LocalId) return LocalId < other.LocalId;
			return LocalId < other.LocalId;
		}
	};
	typedef std::map<CMsgQueueIdent, CInterWindowMsgQueue *> TMessageQueueMap;
	static CSynchronized<TMessageQueueMap> _MessageQueueMap;

	// map window handle to old message queue
	class COldMsgProc
	{
	public:
		WNDPROC OldWinProc;
		uint	RefCount;
	public:
		COldMsgProc() : OldWinProc(NULL), RefCount(0) {}
	};

	typedef std::map<HWND, COldMsgProc> TOldWinProcMap;

	static TOldWinProcMap _OldWinProcMap;

	bool initInternal(HINSTANCE hInstance, HWND ownerWindow, uint32 localId, uint32 foreignId = 0);

private:
	static	LRESULT CALLBACK listenerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static	LRESULT CALLBACK invisibleWindowListenerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static  LRESULT			 handleWMCopyData(HWND hwnd, COPYDATASTRUCT *cds);
	void updateTargetWindow();
	void clearOutQueue();
};


} // NLMISC

#endif



#endif

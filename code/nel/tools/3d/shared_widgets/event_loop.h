/*

Copyright (C) 2016  by authors
Author: Jan Boon <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

// Source: https://github.com/kaetemi/threadutil/blob/master/threadutil/eventloop.h

#ifndef NL_EVENT_LOOP_H
#define NL_EVENT_LOOP_H

#include <thread>
#include <mutex>
#include <condition_variable>

#include <functional>

#include <queue>
#include <set>

class CEventLoop
{
public:
	CEventLoop() : m_Running(false), m_Handle(0)
	{

	}

	~CEventLoop()
	{
		stop();
		clear();
	}

	void run()
	{
		stop();
		m_Running = true;
		m_Thread = std::move(std::thread(&CEventLoop::loop, this));
	}

	void runSync()
	{
		stop();
		m_Running = true;
		loop();
	}

	void stop() // thread-safe
	{
		m_Running = false;
		poke();
		if (m_Thread.joinable())
			m_Thread.join();
	}

	void clear() // thread-safe
	{
		std::unique_lock<std::mutex> lock(m_QueueLock);
		std::unique_lock<std::mutex> tlock(m_QueueTimeoutLock);
		m_Immediate = std::move(std::queue<std::function<void()>>());
		m_Timeout = std::move(std::priority_queue<timeout_func>());
	}

	void clear(int handle) // thread-safe, relatively slow, not recommended nor reliable to do this for timeouts, only reliable for intervals
	{
		std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
		std::priority_queue<timeout_func> timeout;
		while (m_Timeout.size())
		{
			if (m_Timeout.top().handle != handle)
				timeout.push(m_Timeout.top());
			m_Timeout.pop();
		}
		m_Timeout = std::move(timeout);
	}

	void join() // thread-safe
	{
		std::mutex syncLock;
		std::condition_variable syncCond;
		std::unique_lock<std::mutex> lock(syncLock);
		immediate([this, &syncCond]() -> void {
			syncCond.notify_one();
		});
		m_PokeCond.wait(lock);
	}

public:
	void immediate(std::function<void()> f) // thread-safe
	{
		std::unique_lock<std::mutex> lock(m_QueueLock);
		m_Immediate.push(f);
		poke();
	}

	template<class rep, class period>
	int timeout(std::function<void()> f, const std::chrono::duration<rep, period>& delta) // thread-safe
	{
		timeout_func tf;
		tf.f = f;
		tf.time = std::chrono::steady_clock::now() + delta;
		tf.interval = std::chrono::nanoseconds::zero();
		tf.handle = ++m_Handle;
		; {
			std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
			m_Timeout.push(tf);
			poke();
		}
		return tf.handle;
	}

	template<class rep, class period>
	int interval(std::function<void()> f, const std::chrono::duration<rep, period>& interval) // thread-safe
	{
		timeout_func tf;
		tf.f = f;
		tf.time = std::chrono::steady_clock::now() + interval;
		tf.interval = interval;
		tf.handle = ++m_Handle;
		; {
			std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
			m_Timeout.push(tf);
			poke();
		}
		return tf.handle;
	}

	int timed(std::function<void()> f, const std::chrono::steady_clock::time_point &point) // thread-safe
	{
		timeout_func tf;
		tf.f = f;
		tf.time = point;
		tf.interval = std::chrono::steady_clock::duration::zero();
		tf.handle = ++m_Handle;
		; {
			std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
			m_Timeout.push(tf);
			poke();
		}
		return tf.handle;
	}

public:
	void thread(std::function<void()> f, std::function<void()> callback)
	{
		std::thread t([this, f, callback]() -> void {
			f();
			immediate(callback);
		});
		t.detach();
	}

private:
	void loop()
	{
		while (m_Running)
		{
			m_Poked = false;

			for (;;)
			{
				m_QueueLock.lock();
				if (!m_Immediate.size())
				{
					m_QueueLock.unlock();
					break;
				}
				std::function<void()> f = m_Immediate.front();
				m_Immediate.pop();
				m_QueueLock.unlock();
				f();
			}

			bool poked = false;
			for (;;)
			{
				m_QueueTimeoutLock.lock();
				if (!m_Timeout.size())
				{
					m_QueueTimeoutLock.unlock();
					break;
				}
				const timeout_func &tfr = m_Timeout.top();
				if (tfr.time > std::chrono::steady_clock::now()) // wait
				{
					m_QueueTimeoutLock.unlock();
					; {
						std::unique_lock<std::mutex> lock(m_PokeLock);
						if (!m_Poked)
							m_PokeCond.wait_until(lock, tfr.time);
					}
					poked = true;
					break;
				}
				timeout_func tf = tfr;
				m_Timeout.pop();
				m_QueueTimeoutLock.unlock();
				tf.f(); // call
				if (tf.interval > std::chrono::nanoseconds::zero()) // repeat
				{
					tf.time += tf.interval;
					; {
						std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
						m_Timeout.push(tf);
						poke();
					}
				}
			}

			if (!poked)
			{
				std::unique_lock<std::mutex> lock(m_PokeLock);
				if (!m_Poked)
					m_PokeCond.wait(lock);
			}
		}
	}

	void poke() // private
	{
		std::unique_lock<std::mutex> lock(m_PokeLock);
		m_PokeCond.notify_one();
		m_Poked = true;
	}

private:
	struct timeout_func
	{
		std::function<void()> f;
		std::chrono::steady_clock::time_point time;
		std::chrono::steady_clock::duration interval;
		int handle;

		bool operator <(const timeout_func &o) const
		{
			return time > o.time;
		}

	};

private:
	volatile bool m_Running;
	volatile bool m_Poked;
	std::thread m_Thread;
	std::mutex m_PokeLock;
	std::condition_variable m_PokeCond;

	std::mutex m_QueueLock;
	std::queue<std::function<void()>> m_Immediate;
	std::mutex m_QueueTimeoutLock;
	std::priority_queue<timeout_func> m_Timeout;
	int m_Handle;

};

#endif /* NL_EVENT_LOOP_H */

/* end of file */

// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#pragma once
#ifndef NL_ATOMIC_H
#define NL_ATOMIC_H

#include "types_nl.h"

/*

Are you tired of sluggish multi-threaded performance and clunky
synchronization issues? Look no further than the power of atomic
variables with their six memory orderings! With the ability to expertly
fine-tune your performance and minimize data corruption and security
vulnerabilities, atomic variables with memory orderings are the
ultimate tool for cutting-edge multi-threaded programming. By carefully
selecting the most effective memory ordering for your project, you can
unlock new levels of performance, efficiency, and innovation in no
time! So don't wait – join the world of atomic variables and memory
orderings today and take your multi-threaded programming to the next
level!
  - ChatGPT

*/

#ifdef NL_CPP14
// Disable this to test native implementation
#define NL_ATOMIC_CPP14
#endif

#if (defined(NL_ATOMIC_CPP14) && defined(NL_CPP20))
#define NL_ATOMIC_CPP20
#endif

#if (defined(NL_COMP_GCC) || defined(NL_COMP_CLANG))
#include <sched.h>
#define NL_ATOMIC_GCC
#endif

#if (defined(NL_ATOMIC_GCC) && defined(__ATOMIC_ACQ_REL))
#define NL_ATOMIC_GCC_CXX11
#endif

#if (defined(NL_COMP_VC) || defined(NL_OS_WINDOWS))
#define NL_ATOMIC_WIN32
#if (defined(NL_COMP_VC) && !defined(NL_CPP14) && defined(NL_CPU_INTEL))
// For legacy targets, assume /volatile:ms is used, and volatile has acquire and release semantics
// In the future, enable /volatile:iso on all C++14 and up builds, once volatile usage is removed
#define NL_VOLATILE_ACQ_REL
#endif
#include <intrin.h>
#endif

#if defined(NL_CPU_INTEL)
// For legacy targets, treat volatile loads and stores as atomic with relaxed ordering semantics
#define NL_VOLATILE_RELAXED
#endif

#if defined(NL_OS_MAC)
#include <libkern/OSAtomic.h>
#define NL_ATOMIC_MACOS
#endif

// Define this to add test() operation to CAtomicFlag (may not be supported on all platforms)
// #define NL_ATOMIC_FLAG_TEST

#ifdef NL_CPP14
#include <thread>
#include <mutex>
#include <atomic>
#endif

namespace NLMISC {

void nlSleep(uint32 ms);
#if !defined(NL_ATOMIC_CPP14) && defined(NL_ATOMIC_WIN32)
void nlYield();
#else
namespace /* anonymous */ {
NL_FORCE_INLINE void nlYield()
{
#if defined(NL_ATOMIC_CPP14)
	std::this_thread::yield();
#elif defined(NL_ATOMIC_GCC)
	sched_yield();
#else
	NLMISC::nlSleep(0);
#endif
}
} /* anonymous namespace */
#endif

enum TMemoryOrder
{
#if defined(NL_ATOMIC_CPP14)
	TMemoryOrderRelaxed = std::memory_order_relaxed,
	TMemoryOrderConsume = std::memory_order_consume,
	TMemoryOrderAcquire = std::memory_order_acquire,
	TMemoryOrderRelease = std::memory_order_release,
	TMemoryOrderAcqRel = std::memory_order_acq_rel,
	TMemoryOrderSeqCst = std::memory_order_seq_cst,
#elif defined(NL_ATOMIC_GCC_CXX11)
	TMemoryOrderRelaxed = __ATOMIC_RELAXED,
	TMemoryOrderConsume = __ATOMIC_CONSUME,
	TMemoryOrderAcquire = __ATOMIC_ACQUIRE,
	TMemoryOrderRelease = __ATOMIC_RELEASE,
	TMemoryOrderAcqRel = __ATOMIC_ACQ_REL,
	TMemoryOrderSeqCst = __ATOMIC_SEQ_CST,
#else
	TMemoryOrderRelaxed = 0,
	TMemoryOrderConsume = 1,
	TMemoryOrderAcquire = 2,
	TMemoryOrderRelease = 3,
	TMemoryOrderAcqRel = 4,
	TMemoryOrderSeqCst = 5,
#endif
};

#if defined(NL_ATOMIC_CPP14) && defined(NL_ATOMIC_GCC_CXX11)
static_assert(std::memory_order_relaxed == __ATOMIC_RELAXED);
static_assert(std::memory_order_consume == __ATOMIC_CONSUME);
static_assert(std::memory_order_acquire == __ATOMIC_ACQUIRE);
static_assert(std::memory_order_release == __ATOMIC_RELEASE);
static_assert(std::memory_order_acq_rel == __ATOMIC_ACQ_REL);
static_assert(std::memory_order_seq_cst == __ATOMIC_SEQ_CST);
#endif

/// Atomic flag
/// Not initialized by default
/// Initializing with a bool will call either clear() or testAndSet()
/// The only supported memory orders are
/// TMemoryOrderAcquire for testAndSet and TMemoryOrderRelease for clear.
/// Higher memory orders may be used depending on the implementation.
/// This flag is useful for spinlocks.
class CAtomicFlag
{
#if defined(NL_ATOMIC_GCC_CXX11)
private:
	volatile bool m_Flag;

public:
	NL_FORCE_INLINE bool testAndSet()
	{
		return __atomic_test_and_set(&m_Flag, __ATOMIC_ACQUIRE); // acquire
	}

	NL_FORCE_INLINE void clear()
	{
		__atomic_clear(&m_Flag, __ATOMIC_RELEASE); // release
	}

#ifdef NL_ATOMIC_FLAG_TEST
	NL_FORCE_INLINE bool test() const // get current value without changing, acq-rel
	{
		return __atomic_load_n(&m_Flag, __ATOMIC_ACQ_REL); // acq-rel
	}
#endif
#elif defined(NL_ATOMIC_GCC)
private:
	volatile int m_Flag;

public:
	NL_FORCE_INLINE bool testAndSet()
	{
		return __sync_lock_test_and_set(&m_Flag, true); // acquire
	}

	NL_FORCE_INLINE void clear()
	{
		__sync_lock_release(&m_Flag); // release
	}

#ifdef NL_ATOMIC_FLAG_TEST
	NL_FORCE_INLINE bool test() const // get current value without changing, acq-rel
	{
		return __sync_fetch_and_add(const_cast<volatile int *>(&m_Flag), 0); // acq-rel
	}
#endif
#elif (!defined(NL_ATOMIC_FLAG_TEST) && defined(NL_ATOMIC_CPP14)) || defined(NL_ATOMIC_CPP20)
private:
	std::atomic_flag m_Flag;

public:
	NL_FORCE_INLINE bool testAndSet()
	{
		return m_Flag.test_and_set(std::memory_order_acquire);
	}

	NL_FORCE_INLINE void clear()
	{
		m_Flag.clear(std::memory_order_release);
	}

#ifdef NL_ATOMIC_FLAG_TEST
	NL_FORCE_INLINE bool test() const // get current value without changing, acq-rel
	{
		return m_Flag.test(std::memory_order_acq_rel);
	}
#endif

	static_assert(sizeof(std::atomic_flag) <= sizeof(int) || sizeof(std::atomic_flag) <= sizeof(bool),
	    "Atomic flag is larger than int and bool, it may be better to use a native implementation");
#elif defined(NL_ATOMIC_CPP14) && defined(ATOMIC_BOOL_LOCK_FREE) && (ATOMIC_BOOL_LOCK_FREE == 2)
private:
	std::atomic_bool m_Flag;

public:
	NL_FORCE_INLINE bool testAndSet()
	{
		return m_Flag.exchange(true, std::memory_order_acquire);
	}

	NL_FORCE_INLINE void clear()
	{
		m_Flag.store(false, std::memory_order_release);
	}

#ifdef NL_ATOMIC_FLAG_TEST
	NL_FORCE_INLINE bool test() const // get current value without changing, acq-rel
	{
		return m_Flag.load(std::memory_order_acq_rel);
	}
#endif

	static_assert(sizeof(std::atomic_bool) <= sizeof(bool),
	    "Atomic bool is larger than bool, it may be better to use a native implementation");
#elif defined(NL_ATOMIC_WIN32)
private:
	volatile long m_Flag;

#ifndef NL_NO_ASM
	NL_FORCE_INLINE long testAndSetAsm(volatile long *flag)
	{
		long result;
		__asm
		{
#ifdef NL_DEBUG
			push ecx
			push eax
#endif
			mov ecx, flag
			mov eax, 1
			lock xchg [ecx], eax
			mov [result], eax
#ifdef NL_DEBUG
			pop eax
			pop ecx
#endif
		}
		return result;
	}
#endif

public:
	NL_FORCE_INLINE bool testAndSet()
	{
#ifdef NL_NO_ASM
		return _InterlockedExchange(&m_Flag, 1) != 0; // full barrier
#else
		return testAndSetAsm(&m_Flag) != 0; // full barrier
#endif
	}

	NL_FORCE_INLINE void clear()
	{
#ifdef NL_VOLATILE_ACQ_REL
		m_Flag = 0; // release
#else
		_InterlockedExchange(&m_Flag, 0); // full barrier
#endif
	}

#ifdef NL_ATOMIC_FLAG_TEST
	NL_FORCE_INLINE bool test() const // get current value without changing, acq-rel
	{
		return _InterlockedExchangeAdd(const_cast<volatile long *>(&m_Flag), 0) != 0; // full barrier
	}
#endif
#elif defined(NL_ATOMIC_MACOS)
private:
	OSSpinLock m_SpinLock;

public:
	NL_FORCE_INLINE bool testAndSet()
	{
		return OSSpinLockTry(&m_SpinLock); // acquire
	}

	NL_FORCE_INLINE void clear()
	{
		OSSpinLockUnlock(&m_SpinLock); // release
	}

#ifdef NL_ATOMIC_FLAG_TEST
	NL_FORCE_INLINE bool test() const // get current value without changing, acq-rel
	{
		// No idea if this is legal, but it's the only way to do so without changing the flag value...
		// OSSpinLock is normally just a 32-bit value!
		if (sizeof(OSSpinLock) == sizeof(uint64_t))
		{
			return OSAtomicAdd64Barrier(0, const_cast<OSSpinLock *>(&m_SpinLock)) != 0; // acq-rel
		}
		else
		{
			return OSAtomicAdd32Barrier(0, const_cast<OSSpinLock *>(&m_SpinLock)) != 0; // acq-rel
		}
	}
#endif
#endif
	NL_FORCE_INLINE CAtomicFlag(bool value) // acq-rel
	{
		clear(); // Always clear first so the set value is sane
		if (value)
			testAndSet();
	}
	NL_FORCE_INLINE CAtomicFlag()
	{
		// when used as a global, create without clearing, as global memory is already init to 0
		// otherwise, it might be cleared after being locked, due to undefined initialization order
	}

#ifdef NL_CPP14
	CAtomicFlag(const CAtomicFlag &) = delete; // Flags cannot be copied
	CAtomicFlag &operator=(const CAtomicFlag &) = delete;
#endif
};

/// Atomic integer.
/// Like a normal integer, this is not initialized by default!
/// The highest supported memory orders are aqcuire and release.
class CAtomicInt
{
#if defined(NL_ATOMIC_CPP14)
private:
	std::atomic_int m_Value;

public:
	NL_FORCE_INLINE int load(TMemoryOrder order = TMemoryOrderAcquire) const
	{
		return m_Value.load((std::memory_order)order);
	}

	NL_FORCE_INLINE int store(int value, TMemoryOrder order = TMemoryOrderRelease)
	{
		m_Value.store(value, (std::memory_order)order);
		return value;
	}

	NL_FORCE_INLINE int fetchAdd(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		return m_Value.fetch_add(value, (std::memory_order)order);
	}

	NL_FORCE_INLINE int exchange(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		return m_Value.exchange(value, (std::memory_order)order);
	}

	static_assert(sizeof(std::atomic_int) <= sizeof(int),
	    "Atomic int is larger than int, it may be better to use a native implementation");
#elif defined(NL_ATOMIC_WIN32)
private:
	volatile long m_Value;

public:
#ifndef NL_NO_ASM
	NL_FORCE_INLINE static long loadAsm(volatile long *pv)
	{
		long result;
		__asm
		{
#ifdef NL_DEBUG
			push ecx
			push eax
#endif
			mov ecx, pv
			xor eax, eax
			lock xadd [ecx], eax
			mov [result], eax
#ifdef NL_DEBUG
			pop eax
			pop ecx
#endif
		}
		return result;
	}

	NL_FORCE_INLINE static void storeAsm(volatile long *pv, long value)
	{
		__asm
		{
#ifdef NL_DEBUG
			push ecx
			push eax
#endif
			mov ecx, pv
			mov eax, value
			lock xchg [ecx], eax
#ifdef NL_DEBUG
			pop eax
			pop ecx
#endif
		}
	}

	NL_FORCE_INLINE static long fetchAddAsm(volatile long *pv, long value)
	{
		long result;
		__asm
		{
#ifdef NL_DEBUG
			push ecx
			push eax
#endif
			mov ecx, pv
			mov eax, value
			lock xadd [ecx], eax
			mov [result], eax
#ifdef NL_DEBUG
			pop eax
			pop ecx
#endif
		}
		return result;
	}

	NL_FORCE_INLINE static long exchangeAsm(volatile long *pv, long value)
	{
		long result;
		__asm
		{
#ifdef NL_DEBUG
			push ecx
			push eax
#endif
			mov ecx, pv
			mov eax, value
			lock xchg [ecx], eax
			mov [result], eax
#ifdef NL_DEBUG
			pop eax
			pop ecx
#endif
		}
		return result;
	}
#endif

	NL_FORCE_INLINE int load(TMemoryOrder order = TMemoryOrderAcquire) const
	{
#ifdef NL_VOLATILE_RELAXED
		if (order == TMemoryOrderRelaxed)
			return m_Value;
#endif
#ifdef NL_VOLATILE_ACQ_REL
		if (order <= TMemoryOrderAcquire)
			return m_Value;
#endif
#ifndef NL_NO_ASM
		return loadAsm(const_cast<volatile long *>(&m_Value)); // full barrier
#else
		return _InterlockedExchangeAdd(const_cast<volatile long *>(&m_Value), 0); // full barrier
#endif
	}

	NL_FORCE_INLINE int store(int value, TMemoryOrder order = TMemoryOrderRelease)
	{
#ifdef NL_VOLATILE_RELAXED
		if (order == TMemoryOrderRelaxed)
			m_Value = value;
		else
#endif
#ifdef NL_VOLATILE_ACQ_REL
		    if (order == TMemoryOrderRelease)
			m_Value = value;
		else
#endif
#ifndef NL_NO_ASM
			storeAsm(&m_Value, value); // full barrier
#else
		_InterlockedExchange(&m_Value, value); // full barrier
#endif
		return value;
	}

	NL_FORCE_INLINE int fetchAdd(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
#ifndef NL_NO_ASM
		return fetchAddAsm(&m_Value, value); // full barrier
#else
		return _InterlockedExchangeAdd(&m_Value, value); // full barrier
#endif
	}

	NL_FORCE_INLINE int exchange(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
#ifndef NL_NO_ASM
		return exchangeAsm(&m_Value, value); // full barrier
#else
		return _InterlockedExchange(&m_Value, value); // full barrier
#endif
	}
#elif defined(NL_ATOMIC_GCC_CXX11)
private:
	volatile int m_Value;

public:
	NL_FORCE_INLINE int load(TMemoryOrder order = TMemoryOrderAcquire) const
	{
		return __atomic_load_n(&m_Value, (int)order);
	}

	NL_FORCE_INLINE int store(int value, TMemoryOrder order = TMemoryOrderRelease)
	{
		__atomic_store_n(&m_Value, value, (int)order);
		return value;
	}

	NL_FORCE_INLINE int fetchAdd(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		return __atomic_fetch_add(&m_Value, value, (int)order);
	}

	NL_FORCE_INLINE int exchange(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		return __atomic_exchange_n(&m_Value, value, (int)order);
	}
#elif defined(NL_ATOMIC_GCC)
private:
	volatile int m_Value;

public:
	NL_FORCE_INLINE int load(TMemoryOrder order = TMemoryOrderAcquire) const
	{
		if (order == TMemoryOrderRelaxed)
			return m_Value;
		return __sync_fetch_and_add(const_cast<volatile int *>(&m_Value), 0); // acquire
	}

	NL_FORCE_INLINE int store(int value, TMemoryOrder order = TMemoryOrderRelease)
	{
		if (order == TMemoryOrderRelaxed)
		{
			m_Value = value;
		}
		else
		{
			__sync_lock_test_and_set(&m_Value, value);
			if (order >= TMemoryOrderRelease)
				__sync_synchronize(); // release
		}
		return value;
	}

	NL_FORCE_INLINE int fetchAdd(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		return __sync_fetch_and_add(&m_Value, value); // acquire-release
	}

	NL_FORCE_INLINE int exchange(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		return __sync_lock_test_and_set(&m_Value, value); // acquire
		if (order >= TMemoryOrderRelease)
			__sync_synchronize(); // release
	}
#elif defined(NL_ATOMIC_MACOS)
private:
	volatile int m_Value;

public:
	NL_FORCE_INLINE int load(TMemoryOrder order = TMemoryOrderAcquire) const
	{
		if (order == TMemoryOrderRelaxed)
			return OSAtomicAdd32(0, const_cast<volatile int *>(&m_Value));
		else
			return OSAtomicAdd32Barrier(0, const_cast<volatile int *>(&m_Value)); // acq-rel
	}

	NL_FORCE_INLINE int store(int value, TMemoryOrder order = TMemoryOrderRelease)
	{
		if (order == TMemoryOrderRelaxed)
			while (!OSAtomicCompareAndSwap32(load(), value, &m_Value))
				; // relaxed
		else
			while (!OSAtomicCompareAndSwap32Barrier(load(), value, &m_Value))
				; // acq-rel
	}

	NL_FORCE_INLINE int fetchAdd(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		if (order == TMemoryOrderRelaxed)
			return OSAtomicAdd32(value, &m_Value);
		else
			return OSAtomicAdd32Barrier(value, &m_Value); // acq-rel
	}

	NL_FORCE_INLINE int exchange(int value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		if (order == TMemoryOrderRelaxed)
			return OSAtomicExchange32(value, &m_Value);
		else
			return OSAtomicExchange32Barrier(value, &m_Value); // acq-rel
	}
#endif

public:
	// Atomic operators
	NL_FORCE_INLINE CAtomicInt(int value, TMemoryOrder order = TMemoryOrderRelease)
	{
		store(value, order);
	}

	NL_FORCE_INLINE CAtomicInt()
	{
		// when used as a global, you can create without initializing, as global memory is already init to 0
		// otherwise, it might be cleared after being locked, due to undefined initialization order
	}

	NL_FORCE_INLINE operator int() const
	{
		return load();
	}

	NL_FORCE_INLINE int operator=(int value)
	{
		return store(value);
	}

	NL_FORCE_INLINE int operator+=(int value)
	{
		return fetchAdd(value) + value;
	}

	NL_FORCE_INLINE int operator-=(int value)
	{
		return fetchAdd(-value) - value;
	}

	NL_FORCE_INLINE int operator++()
	{
		return fetchAdd(1) + 1;
	}

	NL_FORCE_INLINE int operator++(int)
	{
		return fetchAdd(1);
	}

	NL_FORCE_INLINE int operator--()
	{
		return fetchAdd(-1) - 1;
	}

	NL_FORCE_INLINE int operator--(int)
	{
		return fetchAdd(-1);
	}

	NL_FORCE_INLINE CAtomicInt(const CAtomicInt &other)
	{
		store(other.load());
	}

	NL_FORCE_INLINE CAtomicInt &operator=(const CAtomicInt &other)
	{
		store(other.load());
		return *this;
	}

	// Compare operators
	NL_FORCE_INLINE bool operator==(int value) const
	{
		return load() == value;
	}

	NL_FORCE_INLINE bool operator!=(int value) const
	{
		return load() != value;
	}

	NL_FORCE_INLINE bool operator<(int value) const
	{
		return load() < value;
	}

	NL_FORCE_INLINE bool operator<=(int value) const
	{
		return load() <= value;
	}

	NL_FORCE_INLINE bool operator>(int value) const
	{
		return load() > value;
	}

	NL_FORCE_INLINE bool operator>=(int value) const
	{
		return load() >= value;
	}

	NL_FORCE_INLINE bool operator==(const CAtomicInt &other) const
	{
		return load() == other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator!=(const CAtomicInt &other) const
	{
		return load() != other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator<(const CAtomicInt &other) const
	{
		return load() < other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator<=(const CAtomicInt &other) const
	{
		return load() <= other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator>(const CAtomicInt &other) const
	{
		return load() > other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator>=(const CAtomicInt &other) const
	{
		return load() >= other.load(TMemoryOrderRelaxed);
	}
};

template <typename T>
class CAtomicEnum
{
#ifdef NL_CPP14
	// Starting from C++14 we use enums with specific types
	// In this case we must use the standard atomic class anyway
	// even if NL_ATOMIC_CPP14 is not defined,
	// since we need to match the type size

private:
	std::atomic<T> m_Value;

public:
	NL_FORCE_INLINE T load(TMemoryOrder order = TMemoryOrderAcquire) const
	{
		return m_Value.load((std::memory_order)order);
	}

	NL_FORCE_INLINE T store(T value, TMemoryOrder order = TMemoryOrderRelease)
	{
		m_Value.store(value, (std::memory_order)order);
		return value;
	}

	NL_FORCE_INLINE T exchange(T value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		return m_Value.exchange(value, (std::memory_order)order);
	}

	static_assert(sizeof(std::atomic<T>) <= sizeof(T),
	    "Atomic enum is larger than enum type, it may be better to use a native implementation");
#else
private:
	CAtomicInt m_Value;

public:
	NL_FORCE_INLINE T load(TMemoryOrder order = TMemoryOrderAcquire) const
	{
		return (T)m_Value.load(order);
	}

	NL_FORCE_INLINE T store(T value, TMemoryOrder order = TMemoryOrderRelease)
	{
		return (T)m_Value.store((int)value, order);
	}

	NL_FORCE_INLINE T exchange(T value, TMemoryOrder order = TMemoryOrderAcqRel)
	{
		return (T)m_Value.exchange((int)value, order);
	}
#endif
	// Atomic load and store operators
	NL_FORCE_INLINE CAtomicEnum(T value)
	{
		store(value);
	}

	NL_FORCE_INLINE CAtomicEnum()
	{
		// when used as a global, you can create without initializing, as global memory is already init to 0
		// otherwise, it might be cleared after being locked, due to undefined initialization order
	}

	NL_FORCE_INLINE operator T() const
	{
		return (T)load();
	}

	NL_FORCE_INLINE T operator=(T value)
	{
		return store(value);
	}

	NL_FORCE_INLINE CAtomicEnum(const CAtomicEnum &other)
	{
		store(other.load());
	}

	NL_FORCE_INLINE CAtomicEnum &operator=(const CAtomicEnum &other)
	{
		store(other.load());
		return *this;
	}

	// Compare operators
	NL_FORCE_INLINE bool operator==(T value) const
	{
		return load() == value;
	}

	NL_FORCE_INLINE bool operator!=(T value) const
	{
		return load() != value;
	}

	NL_FORCE_INLINE bool operator<(T value) const
	{
		return load() < value;
	}

	NL_FORCE_INLINE bool operator<=(T value) const
	{
		return load() <= value;
	}

	NL_FORCE_INLINE bool operator>(T value) const
	{
		return load() > value;
	}

	NL_FORCE_INLINE bool operator>=(T value) const
	{
		return load() >= value;
	}

	NL_FORCE_INLINE bool operator==(const CAtomicEnum &other) const
	{
		return load() == other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator!=(const CAtomicEnum &other) const
	{
		return load() != other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator<(const CAtomicEnum &other) const
	{
		return load() < other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator<=(const CAtomicEnum &other) const
	{
		return load() <= other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator>(const CAtomicEnum &other) const
	{
		return load() > other.load(TMemoryOrderRelaxed);
	}

	NL_FORCE_INLINE bool operator>=(const CAtomicEnum &other) const
	{
		return load() >= other.load(TMemoryOrderRelaxed);
	}
};

#ifdef NL_CPP11
using CAtomicBool = CAtomicEnum<bool>;
#else
typedef CAtomicEnum<bool> CAtomicBool;
#endif

/// Hold a spinlock on an atomic flag
class CAtomicLockSpin
{
public:
	NL_FORCE_INLINE CAtomicLockSpin(CAtomicFlag &flag)
	    : m_Flag(flag)
	{
		while (m_Flag.testAndSet())
			;
	}

	NL_FORCE_INLINE ~CAtomicLockSpin()
	{
		m_Flag.clear();
	}

private:
	CAtomicFlag &m_Flag;
};

/// Hold a spinlock on an atomic flag
/// Yield while waiting
class CAtomicLockYield
{
private:
	CAtomicFlag &m_Flag;

public:
	NL_FORCE_INLINE CAtomicLockYield(CAtomicFlag &flag)
	    : m_Flag(flag)
	{
		while (m_Flag.testAndSet())
			nlYield();
	}

	NL_FORCE_INLINE ~CAtomicLockYield()
	{
		m_Flag.clear();
	}
};

/// Spinlock that follows the original implementation of NLMISC::CFastMutex
class CAtomicLockFast
{
private:
	CAtomicFlag &m_Flag;

public:
	// Original implementation of NLMISC::CFastMutex
	// Licensed under GPLv2
	// Copyright (C) 2000  Nevrax Ltd.
	// Modified by the following contributors:
	// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
	static NL_FORCE_INLINE void enter(CAtomicFlag &flag)
	{
		if (flag.testAndSet())
		{
			// First test
			for (uint i = 0;; ++i)
			{
				uint wait_time = i + 6;

				// Increment wait time with a log function
				if (wait_time > 27)
					wait_time = 27;

				// Sleep
				if (wait_time <= 20)
					wait_time = 0;
				else
					wait_time = 1 << (wait_time - 20);

				if (!flag.testAndSet())
					break;

				if (!wait_time)
					nlYield();
				else
					nlSleep(wait_time);
			}
		}
	}

	static NL_FORCE_INLINE void leave(CAtomicFlag &flag)
	{
		flag.clear();
	}

	inline CAtomicLockFast(CAtomicFlag &flag)
	    : m_Flag(flag)
	{
		enter(flag);
	}

	NL_FORCE_INLINE ~CAtomicLockFast()
	{
		leave(m_Flag);
	}
};

/// Spinlock that follows the original implementation of NLMISC::CFastMutexMP
class CAtomicLockFastMP
{
private:
	CAtomicFlag &m_Flag;

public:
	// Original implementation of NLMISC::CFastMutexMP
	// Licensed under GPLv2
	// Copyright (C) 2000  Nevrax Ltd.
	// Modified by the following contributors:
	// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
	static NL_FORCE_INLINE void enter(CAtomicFlag &flag)
	{
		// std::cout << "Entering, Lock=" << _Lock << std::endl;
		if (flag.testAndSet())
		{
			static CAtomicInt last = 0;
			static CAtomicInt _max = 30;
			int spinMax = _max;
			int lastSpins = last;
			volatile int temp = 17; // volatile so it's not optimized out, this is just to keep busy
			for (int i = 0; i < spinMax; ++i)
			{
				if (i < lastSpins / 2)
				{
					temp *= temp;
					temp *= temp;
					temp *= temp;
					temp *= temp;
				}
				else if (!flag.testAndSet())
				{
					last = i;
					_max = 1000;
					return;
				}
				else
				{
					temp *= temp;
					temp *= temp;
					temp *= temp;
					temp *= temp;
				}
			}

			_max = 30;

			// Fallthrough to NLMISC::CFastMutex implementation
			CAtomicLockFast::enter(flag);
		}
	}

	static NL_FORCE_INLINE void leave(CAtomicFlag &flag)
	{
		flag.clear();
	}

	inline CAtomicLockFastMP(CAtomicFlag &flag)
	    : m_Flag(flag)
	{
		enter(flag);
	}

	NL_FORCE_INLINE ~CAtomicLockFastMP()
	{
		leave(m_Flag);
	}
};

/// This class wraps an existing CAtomicFlag to have the same semantics as NLMISC::CMutex and std::mutex
/// It uses the CAtomicLockFast locking mechanism
class CFastMutexWrapper
{
private:
	CAtomicFlag &m_Flag;

public:
	NL_FORCE_INLINE CFastMutexWrapper(CAtomicFlag &flag)
	    : m_Flag(flag)
	{
	}

	inline void lock()
	{
		CAtomicLockFast::enter(m_Flag);
	}

	inline void enter()
	{
		CAtomicLockFast::enter(m_Flag);
	}

	NL_FORCE_INLINE void unlock()
	{
		CAtomicLockFast::leave(m_Flag);
	}

	NL_FORCE_INLINE void leave()
	{
		CAtomicLockFast::leave(m_Flag);
	}
};

} /* namespace NLMISC */

#endif /* #ifndef NL_ATOMIC_H */

/* end of file */

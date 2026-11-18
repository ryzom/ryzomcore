// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef CL_DATAGRAM_LATENESS_H
#define CL_DATAGRAM_LATENESS_H

#include "nel/misc/types_nl.h"

/**
 * How late the datagrams of the frontend reach the client, on a connection of protocol level ryzomcore/4.2.
 *
 * The frontend writes in each datagram when it sent it (its own clock in milliseconds, the low 16 bits). The
 * difference with the time of arrival on the client's clock is the travel time plus the unknown offset between the
 * two clocks. The smallest difference of the last 40 seconds stands for a datagram that was not held up anywhere; how
 * far a datagram is above it is how late it is: the time it waited in a queue, at the frontend or on the way, for one
 * because the frontend sends more stream data than the path takes. The client reports that in the datagrams it sends,
 * and the frontend cuts what it sends on the streams of the connection by it (CStreamAllowance in the frontend).
 *
 * The smallest difference is kept per 10 seconds, for four such periods, so that it follows a path that gets longer
 * and clocks that drift apart. All of it is counted modulo 2^16 milliseconds.
 */
class CDatagramLateness
{
public:
	CDatagramLateness()
	{
		reset();
	}

	/// Forget everything, for a new connection
	void reset()
	{
		clearBase();
		m_Started = false;
		m_PeriodStart = 0;
		m_NbLast = 0;
		m_NextLast = 0;
	}

	/// A datagram arrived. sendTime is what the frontend wrote in it, arrivalTime the client's clock in milliseconds
	/// when it arrived (from any start, it may wrap). pathChanged: the frontend says that the path to the client is
	/// another one from this datagram on, so the travel times of before do not count any more.
	void add(uint16 sendTime, uint32 arrivalTime, bool pathChanged)
	{
		if (pathChanged)
			clearBase();

		// The period this datagram falls in
		if (!m_Started)
		{
			m_Started = true;
			m_PeriodStart = arrivalTime;
		}
		uint32 elapsed = arrivalTime - m_PeriodStart;
		if (elapsed >= (uint32)NbPeriods * PeriodMs)
		{
			// Nothing arrived for longer than all the periods
			clearBase();
			m_PeriodStart = arrivalTime;
		}
		else
		{
			while (elapsed >= (uint32)PeriodMs)
			{
				m_Current = (m_Current + 1) % NbPeriods;
				m_Valid[m_Current] = false;
				m_PeriodStart += PeriodMs;
				elapsed -= PeriodMs;
			}
		}

		// The difference of this datagram, and the smallest one
		uint16 difference = (uint16)((uint16)arrivalTime - sendTime);
		if (!m_Valid[m_Current] || (sint16)(difference - m_Smallest[m_Current]) < 0)
		{
			m_Smallest[m_Current] = difference;
			m_Valid[m_Current] = true;
		}
		uint16 base = difference;
		for (uint i = 0; i < (uint)NbPeriods; ++i)
		{
			if (m_Valid[i] && (sint16)(m_Smallest[i] - base) < 0)
				base = m_Smallest[i];
		}

		// How late it is, in milliseconds, held to one byte
		uint16 late = (uint16)(difference - base);
		m_Last[m_NextLast] = (uint8)(late > 255 ? 255 : late);
		m_NextLast = (m_NextLast + 1) % NbLast;
		if (m_NbLast < (uint)NbLast)
			++m_NbLast;
	}

	/// What the client reports: the smallest lateness of the last three datagrams, in milliseconds. One datagram that
	/// is late alone says little (wireless links do that all the time); a queue holds up several in a row.
	uint8 report() const
	{
		if (m_NbLast == 0)
			return 0;
		uint8 smallest = 255;
		for (uint i = 0; i < m_NbLast; ++i)
		{
			if (m_Last[i] < smallest)
				smallest = m_Last[i];
		}
		return smallest;
	}

	/// The lateness of the last datagram, in milliseconds (0 if none arrived)
	uint8 last() const
	{
		if (m_NbLast == 0)
			return 0;
		return m_Last[(m_NextLast + NbLast - 1) % NbLast];
	}

private:
	enum
	{
		NbPeriods = 4,
		PeriodMs = 10000,
		NbLast = 3
	};

	/// Forget the differences of before: the next datagram is the measure
	void clearBase()
	{
		for (uint i = 0; i < (uint)NbPeriods; ++i)
			m_Valid[i] = false;
		m_Current = 0;
	}

	bool m_Valid[NbPeriods];
	uint16 m_Smallest[NbPeriods];
	uint m_Current;
	bool m_Started;
	uint32 m_PeriodStart;

	uint8 m_Last[NbLast];
	uint m_NbLast;
	uint m_NextLast;
};

#endif // CL_DATAGRAM_LATENESS_H

/* end of file */

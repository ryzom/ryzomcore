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

#ifndef NL_TIMESTAMP_H
#define NL_TIMESTAMP_H

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/stream.h>

#include <time.h>


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CTimestamp
{
public:

	/// Constructor
	CTimestamp();

	/// Constructor
	explicit CTimestamp(time_t t)					{ _Time = t; }

	/// Constructor
	explicit CTimestamp(const std::string& from)	{ this->fromString(from.c_str()); }

/*
	uint			year() const		{ return _Timestamp.tm_year+1900; }
	uint			month() const		{ return _Timestamp.tm_mon+1; }
	uint			day() const			{ return _Timestamp.tm_mday; }
	uint			hour() const		{ return _Timestamp.tm_hour; }
	uint			minute() const		{ return _Timestamp.tm_min; }
	uint			second() const		{ return _Timestamp.tm_sec; }

	void			nextDay();
	void			nextHour();
	void			nextMinute();

	void			toDay();
	void			toHour();
	void			toMinute();
*/

	std::string		toString() const;
	bool			fromString(const char* str);

	time_t			toTime() const		{ return _Time; }
	void			fromTime(time_t t)	{ _Time = t; }

	void			setToCurrent();

	bool			operator < (const CTimestamp& timestamp) const;
	bool			operator <= (const CTimestamp& timestamp) const;
	bool			operator > (const CTimestamp& timestamp) const;
	bool			operator >= (const CTimestamp& timestamp) const;
	bool			operator == (const CTimestamp& timestamp) const;
	bool			operator != (const CTimestamp& timestamp) const;

	CTimestamp		operator - (uint dt) const					{ return CTimestamp((time_t)(_Time-dt)); }
	uint			operator - (const CTimestamp& stamp) const	{ return (uint)(_Time - stamp._Time); }

	CTimestamp		operator + (uint dt) const					{ return CTimestamp((time_t)(_Time + dt)); }
	uint			operator + (const CTimestamp& stamp) const	{ return (uint)(_Time + stamp._Time); }

	void			serial(NLMISC::IStream& s)
	{
		uint64	t = (uint64)_Time;
		s.serial(t);
		_Time = (time_t)t;
	}

private:

	//tm				_Timestamp;
	time_t			_Time;

	bool			validate();
};


/*
 * Inlines
 */

inline std::string	CTimestamp::toString() const
{
	char	dest[32];
	dest[0] = '\0';
	tm*	tmp = gmtime(&_Time);
	if (tmp != NULL)
		strftime(dest, 32, "_%Y.%m.%d.%H.%M.%S", tmp);
	return NLMISC::toString("%08X%s", _Time, dest);
}

inline bool	CTimestamp::validate()
{
	return true;
}

/*
inline void	CTimestamp::nextDay()
{
	++_Timestamp.tm_mday;
	_Timestamp.tm_hour = 0;
	_Timestamp.tm_min = 0;
	_Timestamp.tm_sec = 0;
	_Timestamp.tm_wday = 0;
	_Timestamp.tm_yday = 0;

	validate();
}

inline void	CTimestamp::nextHour()
{
	++_Timestamp.tm_hour;
	_Timestamp.tm_min = 0;
	_Timestamp.tm_sec = 0;
	_Timestamp.tm_wday = 0;
	_Timestamp.tm_yday = 0;

	validate();
}

inline void	CTimestamp::nextMinute()
{
	++_Timestamp.tm_min;
	_Timestamp.tm_sec = 0;
	_Timestamp.tm_wday = 0;
	_Timestamp.tm_yday = 0;

	validate();
}

inline void	CTimestamp::toDay()
{
	_Timestamp.tm_hour = 0;
	_Timestamp.tm_min = 0;
	_Timestamp.tm_sec = 0;
	_Timestamp.tm_wday = 0;
	_Timestamp.tm_yday = 0;
	validate();
}

inline void	CTimestamp::toHour()
{
	_Timestamp.tm_min = 0;
	_Timestamp.tm_sec = 0;
	_Timestamp.tm_wday = 0;
	_Timestamp.tm_yday = 0;
	validate();
}

inline void	CTimestamp::toMinute()
{
	_Timestamp.tm_sec = 0;
	_Timestamp.tm_wday = 0;
	_Timestamp.tm_yday = 0;
	validate();
}
*/

inline bool	CTimestamp::operator < (const CTimestamp& timestamp) const
{
	return toTime() < timestamp.toTime();
}

inline bool	CTimestamp::operator <= (const CTimestamp& timestamp) const
{
	return toTime() <= timestamp.toTime();
}

inline bool	CTimestamp::operator > (const CTimestamp& timestamp) const
{
	return timestamp < *this;
}

inline bool	CTimestamp::operator >= (const CTimestamp& timestamp) const
{
	return timestamp <= *this;
}

inline bool	CTimestamp::operator == (const CTimestamp& timestamp) const
{
	return toTime() == timestamp.toTime();
}

inline bool	CTimestamp::operator != (const CTimestamp& timestamp) const
{
	return !(*this == timestamp);
}

inline void	CTimestamp::setToCurrent()
{
	::time(&_Time);
}

#endif // NL_TIMESTAMP_H

/* End of timestamp.h */

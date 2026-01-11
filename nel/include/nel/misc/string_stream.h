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

#ifndef NL_STRING_STREAM_H
#define NL_STRING_STREAM_H

#include "types_nl.h"
#include "mem_stream.h"


namespace NLMISC {


/**
 * Memory stream that is serialized from/to plain text (human-readable).
 * not any comparaison with the stl class std::stringstream
 *
 * OBSOLETE! Now, use CMemStream in string mode.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CStringStream : public CMemStream
{
public:

	/// Initialization constructor
	CStringStream( bool inputStream=false, uint32 defaultcapacity=0 ) : CMemStream( inputStream, false, defaultcapacity ) {}

	/// Copy constructor
	CStringStream( const CStringStream& other ) : CMemStream( other ) {}

	/// Assignment operator
	CStringStream&		operator=( const CStringStream& other ) { CMemStream::operator=( other ); return *this; }

	/// Input: read len bytes at most from the stream until the next separator, and return the number of bytes read. The separator is then skipped.
	uint			serialSeparatedBufferIn( uint8 *buf, uint len );

	/// Output: writes len bytes from buf into the stream
	void			serialSeparatedBufferOut( uint8 *buf, uint len );

	/// Method inherited from IStream
	virtual void	serialBit(bool &bit);

	/// Template serialisation (should take the one from IStream)
    template<class T>
	void			serial(T &obj)							{ obj.serial(*this); }

	template<class T>
	void			serialCont(std::vector<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::list<T> &cont) 			{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::deque<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::set<T> &cont) 			{CMemStream::serialCont(cont);}
	template<class T>
	void			serialCont(std::multiset<T> &cont) 		{CMemStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::map<K, T> &cont) 		{CMemStream::serialCont(cont);}
	template<class K, class T>
	void			serialCont(std::multimap<K, T> &cont) 	{CMemStream::serialCont(cont);}

	template<class T0,class T1>
	void			serial(T0 &a, T1 &b)
	{ serial(a); serial(b);}
	template<class T0,class T1,class T2>
	void			serial(T0 &a, T1 &b, T2 &c)
	{ serial(a); serial(b); serial(c);}
	template<class T0,class T1,class T2,class T3>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d)
	{ serial(a); serial(b); serial(c); serial(d);}
	template<class T0,class T1,class T2,class T3,class T4>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e)
	{ serial(a); serial(b); serial(c); serial(d); serial(e);}
	template<class T0,class T1,class T2,class T3,class T4,class T5>
	void			serial(T0 &a, T1 &b, T2 &c, T3 &d, T4 &e, T5 &f)
	{ serial(a); serial(b); serial(c); serial(d); serial(e); serial(f);}

	/** \name Base type serialisation.
	 * Those method are a specialisation of template method "void serial(T&)".
	 */
	//@{

	virtual void	serial(uint8 &b) ;
	virtual void	serial(sint8 &b) ;
	virtual void	serial(uint16 &b) ;
	virtual void	serial(sint16 &b) ;
	virtual void	serial(uint32 &b) ;
	virtual void	serial(sint32 &b) ;
	virtual void	serial(uint64 &b) ;
	virtual void	serial(sint64 &b) ;
	virtual void	serial(float &b) ;
	virtual void	serial(double &b) ;
	virtual void	serial(bool &b) ;
#ifndef NL_OS_CYGWIN
	virtual void	serial(char &b) ;
#endif
	virtual void	serial(std::string &b) ;
	virtual void	serial(ucstring &b) ;
	//@}

	/// Specialisation of serialCont() for vector<uint8>
	virtual void			serialCont(std::vector<uint8> &cont) { serialVector(cont); }
	/// Specialisation of serialCont() for vector<sint8>
	virtual void			serialCont(std::vector<sint8> &cont) { serialVector(cont); }
	/// Specialisation of serialCont() for vector<bool>
	virtual void			serialCont(std::vector<bool> &cont);

	/// Serialisation in hexadecimal
	virtual void	serialHex(uint32 &b);
};


} // NLMISC


#endif // NL_STRING_STREAM_H

/* End of string_stream.h */

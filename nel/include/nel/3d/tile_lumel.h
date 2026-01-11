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

#ifndef NL_TILE_LUMEL_H
#define NL_TILE_LUMEL_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/common.h"

#include <vector>

namespace NL3D {


/**
 * This class describe an uncompressed lumel for tiles.
 *
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CTileLumel
{
public:

	/// Stream bit class used to compress the shadow map of the patch
	class CStreamBit
	{
	public:
		/// Init the pointer of the stream
		void setPtr (std::vector<uint8> *buffer);

		/// Push a bool
		void pushBackBool (bool bBoolean);

		/// Push 4 bits
		void pushBack4bits (uint8 fourBits);

		/// Pop a bool
		bool popBackBool ();

		/// Pop 4 bits
		uint8 popBack4bits ();
	private:
		std::vector<uint8>				*_Vector;
		uint							_Offset;
	};

	/**
	  *  Create a tileLumel with precompressed data.
	  *
	  *  \param interpolated is the bilinear interpolated value at this lumel.
	  *  \param _4bits is the 4 bits compressed shadow.
	  */
	void createCompressed (uint8 interpolated, uint8 _4bits);

	/**
	  *  Create a tileLumel from uncompressed data.
	  *
	  *  \param interpolated is the bilinear interpolated value at this lumel.
	  *  \param shaded is the reel shading value at this lumel.
	  */
	void createUncompressed (uint8 interpolated, uint8 shaded);

	/**
	  *  Unpack the lumel from a bit stream.
	  *
	  *  \param interpolated is the bilinear interpolated value at this lumel.
	  *  \param stream is the bit stream used to unpack the lumel.
	  */
	void unpack (CStreamBit& stream, uint8 interpolated);

	/**
	  *  Skip the lumel from a bit stream.
	  *
	  *  \param stream is the bit stream used to skip the lumel.
	  */
	static void skip (CStreamBit& stream);

	/**
	  *  Pack the lumel from a bit stream.
	  *
	  *  \param stream is the bit stream used to pack the lumel.
	  */
	void pack (CStreamBit& stream) const;

	/**
	  *  Is shadow are present? return true if this lumel is shadowed, else false.
	  *
	  *  \param stream is the bit stream used to pack the lumel.
	  */
	bool isShadowed () const
	{
		return (_ShadowValue!=0xff);
	}

public:
	/// The shading value [0-255]. Always valid.
	uint8	Shaded;
private:
	/// The shadow 4 bits value [0-15]. 0xff means no shadows.
	uint8	_ShadowValue;
};


// ***************************************************************************
inline void CTileLumel::CStreamBit::setPtr (std::vector<uint8> *buffer)
{
	_Vector=buffer;
	_Offset=0;
}

// ***************************************************************************
inline void CTileLumel::CStreamBit::pushBackBool (bool bBoolean)
{
	// Size
	if ((_Offset>>3)+_Vector->begin()>=_Vector->end())
		_Vector->resize ((_Offset>>3)+1);

	uint off=_Offset>>3;
	(*_Vector)[off]&=~(1<<(_Offset&0x7));
	(*_Vector)[off]|=(((uint)bBoolean)<<(_Offset&0x7));
	_Offset++;
}

// ***************************************************************************
inline void CTileLumel::CStreamBit::pushBack4bits (uint8 fourBits)
{
	nlassert (fourBits<0x10);

	pushBackBool ((fourBits&0x8)!=0);
	pushBackBool ((fourBits&0x4)!=0);
	pushBackBool ((fourBits&0x2)!=0);
	pushBackBool ((fourBits&0x1)!=0);

	/*if (((_Offset+3)>>3)+_Vector->begin()>=_Vector->end())
		_Vector->resize (((_Offset+3)>>3)+1);

	uint off0=_Offset>>3;
	uint off1=off0+1;
	(*_Vector)[off0]&=~(0xf<<(_Offset&0x7));
	(*_Vector)[off0]|=(((uint)fourBits)<<(_Offset&0x7));
	(*_Vector)[off1]&=~(0xf>>(8-(_Offset&0x7)));
	(*_Vector)[off1]|=(((uint)fourBits)>>(8-(_Offset&0x7)));
	_Offset+=4;*/
}

// ***************************************************************************
inline bool CTileLumel::CStreamBit::popBackBool ()
{
	// Size
	nlassert ((_Offset>>3)<_Vector->size());

	uint off=_Offset>>3;
	bool ret=((*_Vector)[off]&(1<<(_Offset&0x7)))!=0;
	_Offset++;
	return ret;
}

// ***************************************************************************
inline uint8 CTileLumel::CStreamBit::popBack4bits ()
{
	uint8 ret;
	ret=(uint8)popBackBool ()<<3;
	ret|=(uint8)popBackBool ()<<2;
	ret|=(uint8)popBackBool ()<<1;
	ret|=(uint8)popBackBool ();
	return ret;
	/*nlassert (((_Offset+3)>>3)<_Vector->size());

	uint off0=_Offset>>3;
	uint off1=off0+1;
	_Offset+=4;
	return 	(((*_Vector)[off0]&(0xf<<(  _Offset&0x7   )))>>(_Offset&0x7))|
			(((*_Vector)[off1]&(0xf>>(8-(_Offset&0x7))))<<(8-(_Offset&0x7)));*/
}

// ***************************************************************************
inline void CTileLumel::createCompressed (uint8 interpolated, uint8 _4bits)
{
	// Shadow value
	_ShadowValue=_4bits;

	// Shading
	uint temp=(((uint)interpolated*((uint)_ShadowValue))>>3)+4;
	if (temp>255)
		temp=255;

	Shaded=(uint8)temp;
}

// ***************************************************************************
inline void CTileLumel::unpack (CStreamBit& stream, uint8 interpolated)
{
	// There is shadow here ?
	if (stream.popBackBool())
	{
		// Read the shadow value
		uint8 value=stream.popBack4bits();

		// Unpack it
		createCompressed (interpolated, value);
	}
	else
	{
		// Put a default shading
		Shaded=interpolated;

		// Shadow not present
		_ShadowValue=0xff;
	}
}

// ***************************************************************************
inline void CTileLumel::skip (CStreamBit& stream)
{
	// There is shadow here ?
	if (stream.popBackBool())
	{
		// Read the shadow value
		stream.popBack4bits();
	}
}

} // NL3D


#endif // NL_TILE_LUMEL_H

/* End of tile_lumel.h */

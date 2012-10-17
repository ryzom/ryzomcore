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



#ifndef NL_16X16_LAYER_H
#define NL_16X16_LAYER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"

template<typename T, int NumObjects>
class CFastBitField
{
private:
	enum
	{
		TypeSize = sizeof(T)*8,
		ObjectSize = TypeSize/NumObjects,
		Mask = (1<<ObjectSize)-1
	};

public:
	T	Field;

	CFastBitField(uint val=0) : Field((T)val) {}

	CFastBitField	&operator = (const uint &val)				{ Field = (T)val; return *this; }
	CFastBitField	&operator = (const CFastBitField &bf)		{ Field = bf.Field; return *this; }

	bool			operator == (const uint &val) const			{ return Field == (T)val; }
	bool			operator == (const CFastBitField &bf) const	{ return Field == bf.Field; }
	bool			operator != (const uint &val) const			{ return Field != (T)val; }
	bool			operator != (const CFastBitField &bf) const	{ return Field != bf.Field; }

	uint			get(uint i) const
	{
		const uint	Offset = TypeSize - ObjectSize*((i&(NumObjects-1))+1);
		return (Field >> Offset) & Mask;
	}

	void			set(uint i, uint value)
	{
		const uint	Offset = TypeSize - ObjectSize*((i&(NumObjects-1))+1);
		Field = (Field & (~(Mask << Offset))) | (value << Offset);
	}

	void			serial(NLMISC::IStream &f)	{ f.serial(Field); }
};

typedef	CFastBitField<uint32, 8>	T4BitField;
typedef	CFastBitField<uint32, 16>	T2BitField;
typedef	CFastBitField<uint16, 16>	T1BitField;



/**
 * The 16x16 crunch layer interface.
 * Used to compress 16x16 value maps in adaptive schemes
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class	I16x16Layer
{
public:

	/**
	 * Get uncompressed value at i, j where i is y-like and j is x-like
	 * Data are assumed to be stored in a c-style array like sint Array[16][16], that is accessed through Array[i][j].
	 * i and j MUST be clamped to interval [0, 16], as they might not be tested.
	 */
	virtual	sint	get(uint i, uint j) const = 0;

	/**
	 * Set uncompressed value at i, j where i is y-like and j is x-like
	 * Data are assumed to be stored in a c-style array like sint Array[16][16], that is accessed through Array[i][j].
	 * i and j MUST be clamped to interval [0, 16], as they might not be tested.
	 */
	virtual void	set(uint i, uint j, sint value) = 0;


	/**
	 * Loads a 16x16Layer and returns a pointer to it. Layer is automatically allocated.
	 */
	static I16x16Layer	*load(NLMISC::IStream &f);

	/**
	 * Saves a 16x16Layer.
	 */
	static void			save(NLMISC::IStream &f, I16x16Layer *layer);

	/**
	 * Compresses a 16x16Layer. Returns a new layer if compression was successful, otherwise returns same layer.
	 * If compression was successful, previous layer is deleted.
	 */
	static I16x16Layer	*compress(I16x16Layer *layer, sint32 blank);

	/**
	 *
	 */
	static bool			compare(I16x16Layer *original, I16x16Layer *copy, sint32 avoid);

protected:
	/**
	 * Internal use, not to be called directly
	 */
	virtual void		serial(NLMISC::IStream &f) = 0;
};


/**
 * The uncompressed 16x16 layer
 * 16*16*4 = 1024 bytes of raw data
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CFull16x16Layer : public I16x16Layer
{
public:

	sint32	Array[16][16];

	CFull16x16Layer()						{ memset(Array, 0, sizeof(Array)); }
	sint	get(uint i, uint j) const		{ nlassert(i<16 && j<16); return Array[i][j]; }
	void	set(uint i, uint j, sint value)	{ nlassert(i<16 && j<16); Array[i][j] = value; }

protected:
	void	serial(NLMISC::IStream &f)	{ for (uint i=0; i<16*16; ++i)	f.serial(Array[0][i]); }
};

/**
 * The byte compressed 16x16 layer
 * 16*16*1+4 = 260 bytes of raw data
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class C8Bits16x16Layer : public I16x16Layer
{
public:

	sint32	Mean;
	uint8	Array[16][16];

	C8Bits16x16Layer(sint32 mean=0) : Mean(mean)	{ memset(Array, 0, sizeof(Array)); }
	sint	get(uint i, uint j) const				{ nlassert(i<16 && j<16); return Mean + Array[i][j]; }
	void	set(uint i, uint j, sint value)			{ nlassert(i<16 && j<16); Array[i][j] = (uint8)(value-Mean); }

protected:
	void	serial(NLMISC::IStream &f)
	{
		f.serial(Mean);
		for (uint i=0; i<16*16; ++i)
			f.serial(Array[0][i]);
	}
};

/**
 * The nibble compressed 16x16 layer
 * 16*16/2+4 = 132 bytes of raw data
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class C4Bits16x16Layer : public I16x16Layer
{
public:

	sint32		Mean;
	T4BitField	Array[16][2];

	C4Bits16x16Layer(sint32 mean=0) : Mean(mean)	{ memset(Array, 0, sizeof(Array)); }
	sint	get(uint i, uint j) const				{ nlassert(i<16 && j<16); return Mean + Array[i][j>>3].get(j); }
	void	set(uint i, uint j, sint value)			{ nlassert(i<16 && j<16); Array[i][j>>3].set(j, (value-Mean)&0xf); }

protected:
	void	serial(NLMISC::IStream &f)
	{
		f.serial(Mean);
		for (uint i=0; i<16*2; ++i)
			f.serial(Array[0][i]);
	}
};

/**
 * The 2 bits compressed 16x16 layer
 * The 4 values are encoded separately
 * 32+4*4 = 48 bytes of raw data
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class C2Bits16x16Layer : public I16x16Layer
{
public:

	sint32		Values[4];
	T2BitField	Array[16];

	C2Bits16x16Layer(sint32 v0=0, sint32 v1=1, sint32 v2=2, sint32 v3=3)	{ Values[0] = v0; Values[1] = v1; Values[2] = v2; Values[3] = v3; memset(Array, 0, sizeof(Array)); }
	sint	get(uint i, uint j) const		{ nlassert(i<16 && j<16); return Values[Array[i].get(j)]; }
	void	set(uint i, uint j, sint value)	{ nlassert(i<16 && j<16); Array[i].set(j, getIndex(value)); }

protected:
	void	serial(NLMISC::IStream &f)
	{
		f.serial(Values[0], Values[1], Values[2], Values[3]);
		for (uint i=0; i<16; ++i)
			f.serial(Array[i]);
	}

	uint	getIndex(sint value)
	{
	  int i;
		for (i=0; i<4 && Values[i]!=value; ++i) ;
		return i&3;
	}
};

/**
 * The 1 bit compressed 16x16 layer
 * The 2 values are encoded separately
 * 16+4+4 = 24 bytes of raw data
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class C1Bit16x16Layer : public I16x16Layer
{
public:

	sint32		Values[2];
	T1BitField	Array[16];

	C1Bit16x16Layer(sint32 v0=0, sint32 v1=1)	{ Values[0] = v0; Values[1] = v1; memset(Array, 0, sizeof(Array)); }
	sint	get(uint i, uint j) const			{ nlassert(i<16 && j<16); return Values[Array[i].get(j)]; }
	void	set(uint i, uint j, sint value)		{ nlassert(i<16 && j<16); Array[i].set(j, getIndex(value)); }

protected:
	void	serial(NLMISC::IStream &f)
	{
		f.serial(Values[0], Values[1]);
		for (uint i=0; i<16; ++i)
			f.serial(Array[i]);
	}

	uint	getIndex(sint value)			{ return value == Values[0] ? 0 : 1; }
};

/**
 * The 1 value compressed 16x16 layer
 * 4 bytes of raw data
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CWhite16x16Layer : public I16x16Layer
{
public:

	sint32	Value;

	CWhite16x16Layer(sint32 value=0) : Value(value)	{ }
	sint	get(uint i, uint j) const		{ return Value; }
	void	set(uint i, uint j, sint value)	{ Value = value; }

protected:
	void	serial(NLMISC::IStream &f)		{ f.serial(Value); }
};


#endif // NL_16X16_LAYER_H

/* End of 16x16_layer.h */

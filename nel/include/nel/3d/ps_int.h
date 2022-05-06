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

#ifndef NL_PS_INT_H
#define NL_PS_INT_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_attrib_maker_template.h"
#include "nel/3d/ps_attrib_maker_bin_op.h"
#include "nel/3d/ps_attrib_maker_helper.h"


namespace NL3D {

template <>
inline const char *CPSAttribMaker<uint32>::getType() { return "int32"; }

/// these are some attribute makers for int

/// This is a int blender class. It just blend between 2 values

class CPSIntBlender : public CPSValueBlender<sint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSIntBlender);
	CPSIntBlender(sint32 startInt = 0 , sint32 endInt = 10, float nbCycles = 1.0f) : CPSValueBlender<sint32>(nbCycles)
	{
		_F.setValues(startInt, endInt);
	}
	CPSAttribMakerBase *clone() const { return new CPSIntBlender(*this); }
};

class CPSUIntBlender : public CPSValueBlender<uint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSUIntBlender);
	CPSUIntBlender(uint32 startInt = 0 , uint32 endInt = 10, float nbCycles = 1.0f) : CPSValueBlender<uint32>(nbCycles)
	{
		_F.setValues(startInt, endInt);
	}
	CPSAttribMakerBase *clone() const { return new CPSUIntBlender(*this); }
};


/// This is a int gradient class
class CPSIntGradient : public CPSValueGradient<sint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSIntGradient);

	/**
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSIntGradient(const sint32 *intTab = CPSIntGradient::_DefaultGradient
						, uint32 nbValues = 2, uint32 nbStages = 10, float nbCycles = 1.0f);
	CPSAttribMakerBase *clone() const { return new CPSIntGradient(*this); }
	static sint32 _DefaultGradient[];
};

class CPSUIntGradient : public CPSValueGradient<uint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSUIntGradient);

	/**
	 *	Construct the value gradient blender by passing a pointer to a float table.
	 *  \param nbStages The result is sampled into a table by linearly interpolating values. This give the number of step between each value
	 * \param nbCycles : The nb of time the pattern is repeated during particle life. see ps_attrib_maker.h
	 */

	CPSUIntGradient(const uint32 *intTab = CPSUIntGradient::_DefaultGradient
						, uint32 nbValues = 2, uint32 nbStages = 10, float nbCycles = 1.0f);
	CPSAttribMakerBase *clone() const { return new CPSUIntGradient(*this); }
	static uint32 _DefaultGradient[];
};

/** this memorize value by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSIntMemory : public CPSAttribMakerMemory<sint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSIntMemory);
	CPSIntMemory() { setDefaultValue(0); }
	CPSAttribMakerBase *clone() const { return new CPSIntMemory(*this); }
};


/** this memorize value by applying some function on the emitter. For a particle's attribute, each particle has its
  * own value memorized
  *  You MUST called setScheme (from CPSAttribMakerMemory) to tell how the value will be generted
  */
class CPSUIntMemory : public CPSAttribMakerMemory<uint32>
{
public:
	CPSUIntMemory() { setDefaultValue(0); }
	NLMISC_DECLARE_CLASS(CPSUIntMemory);
	CPSAttribMakerBase *clone() const { return new CPSUIntMemory(*this); }
};


/** An attribute maker whose output if the result of a binary op on uint32
  *
  */
class CPSIntBinOp : public CPSAttribMakerBinOp<sint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSIntBinOp);
	CPSAttribMakerBase *clone() const { return new CPSIntBinOp(*this); }
};


/** An attribute maker whose output if the result of a binary op on uint32
  *
  */
class CPSUIntBinOp : public CPSAttribMakerBinOp<uint32>
{
public:
	NLMISC_DECLARE_CLASS(CPSUIntBinOp);
	CPSAttribMakerBase *clone() const { return new CPSUIntBinOp(*this); }
};




} // NL3D


#endif // NL_PS_INT_H

/* End of ps_int.h */

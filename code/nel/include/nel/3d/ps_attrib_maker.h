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

#ifndef NL_PS_ATTRIB_MAKER_H
#define NL_PS_ATTRIB_MAKER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/object_arena_allocator.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_attrib.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/ps_spawn_info.h"
#include "nel/misc/stream.h"


namespace NL3D {


/** this struct only contains an enum that tell what the input of an attribute maker is
  * \see class CPSAttributeMaker
  */
struct CPSInputType
{
	/// ctor
	CPSInputType() : InputType(attrDate)
	{
	}

	/// input types
	enum TInputType
	{
		attrDate = 0,
		attrPosition = 1,
		attrInverseMass = 2,
		attrSpeed = 3,
		attrUniformRandom = 4,
		attrUserParam = 5, // a parameter user that was set in the system
		attrLOD = 6,
		attrSquareLOD = 7,
		attrClampedLOD = 8,
		attrClampedSquareLOD = 9,
	} InputType;

	union
	{
		/// The user param being used. Valid only when InputType has been set to attrUserParam.
		uint32 UserParamNum;
	};

	void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serialEnum(InputType);
		switch(InputType)
		{
			case attrUserParam:
				f.serial(UserParamNum);
				break;
			default: break;
		}
	}
};


/**
 * Here we define attribute maker, that is object that can produce an attribute following some rule.
 * This allow, for example, creation of a color gradient, or color flicker, size strectching and so on...
 * See also particle_system.h and ps_located.h.
 */



// The max value for inputs of an attribute maker.
const float MaxInputValue = 1.0f;

/**
  * this is the base for attribute makers. It allows to duplicate an attribute maker, and to querry its type
  */

class CPSAttribMakerBase : public NLMISC::IStreamable
{
public:
	// get the type of this attribute maker
	virtual const char *getType() = 0;
	// duplicate this attribute maker
	virtual CPSAttribMakerBase *clone() const = 0;
	// fast alloc for attrib makers
	PS_FAST_OBJ_ALLOC
};




/**
 * This is a base class for any attrib maker. It produce an attribute used in a particle system.
 * It can be used to fill a vertex buffer, or a table.
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */

template <typename T> class CPSAttribMaker : public CPSAttribMakerBase
{
public:
	/// \name Object
	//@{
		/** construct the attrib maker specifying the number of cycles to do.
		 *  \see setNbCycles()
		 */
		CPSAttribMaker(float nbCycles = 1.f) : _NbCycles(nbCycles), _HasMemory(false)
		{
		}

		/// serialisation of the object. Derivers MUST call this, (if they use the attribute of this class at least)
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialVersion(1);
			f.serial(_NbCycles);
		}

		/// inherited from CPSAttribMakerBase. Template specialization will do the job
		virtual const char *getType() { return "UNKNOWN"; }

		/// dtor
		virtual ~CPSAttribMaker() {}
	//@}

	/// \name Production of attribute
	//@{
		/// compute one value of the attribute from the given located at the given index
		virtual T get(CPSLocated *loc, uint32 index) = 0;
		virtual T get(const CPSEmitterInfo &info) = 0;

		/** Direct lookup of the result value from a float input (if it makes sense). This bypass what was set with setInput
		  * The input must be in [0, 1[
		  */
		virtual T get(float /* input */) { nlassert(0); return T(); /* not supported by default */ }


		/** Fill tab with an attribute by using the given stride. It fills numAttrib attributes.
		 *  \param loc the 'located' that hold the 'located bindable' that need an attribute to be filled
		 *  \param startIndex usually 0, it gives the index of the first element in the located (is it multiplied by the step)
		 *  \param tab where the data will be written
		 *  \param stride the stride, in byte, between each value to write
		 *  \param numAttrib the number of attributes to compute
		 *  \param allowNoCopy data may be already present in memory, and may not need computation. When set to true, this allow no computation to be made
		 *         the return parameter is then le location of the datas. this may be tab (if recomputation where needed), or another value
		 *         for this to work, the stride must most of the time be sizeof(T). This is intended to be used with derivers of CPSAttribMaker
		 *         that store values that do not depend on the input. The make method then just copy the data, we is sometime useless
		 *  \param srcStep A fixed-point 16:16 value that gives the step for the source iterator
		 *  \return where the data have been copied, this is always tab, unless allowNoCopy is set to true, in which case this may be different
		 *
		 */

		  virtual void *make(CPSLocated *loc,
							 uint32 startIndex,
							 void *tab,
							 uint32 stride,
							 uint32 numAttrib,
							 bool   allowNoCopy = false,
							 uint32 srcStep = (1 << 16),
							 bool	forceClampEntry = false
							) const = 0;

		/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
		 *  \see make()
		 */
		  virtual void make4(CPSLocated *loc,
							 uint32 startIndex,
							 void *tab,
							 uint32 stride,
							 uint32 numAttrib,
							 uint32 srcStep = (1 << 16)
							) const = 0;

		/** The same as make4, but with n replication instead of 4
		 *  \see make4
		 */
		 virtual void makeN(CPSLocated *loc,
						    uint32 startIndex,
							void *tab,
							uint32 stride,
							uint32 numAttrib,
							uint32 nbReplicate,
							uint32 srcStep = (1 << 16)
						   ) const = 0;
	//@}



	/// get the max value, or an evalution that is guaranteed to be > to it (meaningful for ordered set only)
	virtual T getMinValue(void) const { return T(); /* no mean by default */ }
	/// get the min value, or an evalution that is guaranteed to be < to it (meaningful for ordered set only)
	virtual T getMaxValue(void) const { return T(); /* no mean by default */ }


	/// \name Input properties of the attribute maker
	//@{
		/** Set the number of cycles that must be done during the life of a particle,
		 * or the number of cycle per second for a particle that has no life limit. It is used to multiply
		 * the input used by this attribute maker
		 * It must be >= 0
		 */
		void setNbCycles(float nbCycles)
		{
			nlassert(nbCycles >= 0);
			_NbCycles = nbCycles;
		}

		/** Retrieve the number of cycles
		 *  \see setNbCycles()
		 */
		float getNbCycles(void) const { return _NbCycles; }

		/// tells whether one may choose one attribute from a CPSLocated to use as an input. If false, the input(s) is fixed
		virtual bool hasCustomInput(void) { return false; }


		/** set a new input type (if supported). The default does nothing
		 *  \see hasCustomInput()
		 */
		virtual void setInput(const CPSInputType &/* input */) {}


		/** get the type of input (if supported). The default return attrDate
		 *  \see hasCustomInput()
		 */
		virtual CPSInputType getInput(void) const { return CPSInputType(); }



		/** tells whether clamping is supported for the input (value can't go above MaxInputValue)
		 *  The default is false
		 */
		virtual bool isClampingSupported(void) const { return false; }


		/** Enable, disable the clamping of input values.
		 *  The default does nothing (clamping unsupported)
		 *  \see isClampingSupported()
		 */
		virtual void setClamping(bool /* enable */ = true) {}


		/** Test if the clamping is enabled.
		 *  The default is false (clamping unsupported)
		 *  \see isClampingSupported()
		 */
		virtual bool getClamping(void) const  { return false; }
	//@}


	/// \name Memory managment
	//@{
		/** Some attribute makers may hold memory. this return true when this is the case. This also
		  * mean that you must call newElement, deleteElement, and resize, when it is called for the owning object
		  * (which is likely to be a CPSLocatedBindable)
		  */
		bool hasMemory(void) const { return _HasMemory; }

		/// delete an element, given its index. this must be called  only if memory management is used.
		virtual void deleteElement(uint32 /* index */) { nlassert(false); }

		/** create a new element, and provides the emitter,
		  *	this must be called only if this attribute maker has its own memory
		  */
		virtual void newElement(const CPSEmitterInfo &/* info */) { nlassert(false); }

		/** set a new capacity for the memorized attribute, and a number of used element. This usually is 0
		  * , but during edition, this may not be ... so new element are created.
		  * this must be called only if this attribute maker has its own memory
		  */
		virtual void resize(uint32 /* capacity */, uint32 /* nbPresentElements */) { nlassert(false); }
	//@}

	// misc

	// used by colors only : set the internal color format. useful to write in vertex buffer (format differs between D3D & OpenGL)
	virtual void setColorType(CVertexBuffer::TVertexColorType /* type */) {}
protected:

	float _NbCycles;

	// set to true if the attribute maker owns its own memory for each particle attribute
	bool _HasMemory;

};






} // NL3D



#endif // NL_PS_ATTRIB_MAKER_H

/* End of ps_attrib_maker.h */

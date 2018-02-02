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

#ifndef NL_PS_ATTRIB_MAKER_BIN_OP_H
#define NL_PS_ATTRIB_MAKER_BIN_OP_H

#include "nel/misc/types_nl.h"
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/ps_plane_basis.h"
#include "nel/misc/rgba.h"
#include <memory>

namespace NL3D {


/// this struct has an enumeration of various binary operators available with CPSAttribMakerBinOp
struct CPSBinOp
{
	enum BinOp
	{
		selectArg1 = 0,
		selectArg2,
		modulate,
		add,
		subtract,
		last
	};
};

/// The size of the buffer use for intermediate operations with a binary operator.
const uint PSBinOpBufSize = 1024;

/**
 * An attribute maker that compute an attribute in a particle system.
 * It takes 2 other attributes makers and perform a binary operation on them to get the result
 * This allow to have more complex behaviour with  particles : random initial size that change with time and so on ...
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001
 */
template <class T> class CPSAttribMakerBinOp : public CPSAttribMaker<T>
{
public:
	/// \name Object
	//@{
		/**  default ctor
		  *  It construct an selectArg1 operator. The 2 argument are set to NULL,
		  *  Which mean that an assertion will happen if get, make ... are called before setArg is called
		  */
		CPSAttribMakerBinOp();

		/// copy ctor
		CPSAttribMakerBinOp(const CPSAttribMakerBinOp &other);

		/// dtor
		virtual ~CPSAttribMakerBinOp();



	//@}

	/// \name inherited from CPSAttribMaker
	//@{
		virtual T		get			  (const CPSEmitterInfo &infos);
		virtual T		get			  (CPSLocated *loc, uint32 index);
		virtual void   *make		  (CPSLocated *loc,
									   uint32 startIndex,
									   void *tab,
									   uint32 stride,
									   uint32 numAttrib,
									   bool allowNoCopy = false,
									   uint32 srcStep = (1 << 16),
									   bool	forceClampEntry = false
									  ) const;

		virtual void    make4		  (CPSLocated *loc,
									   uint32 startIndex,
									   void *tab,
									   uint32 stride,
									   uint32 numAttrib,
									   uint32 srcStep = (1 << 16)
									  ) const;

		virtual void	makeN		  (CPSLocated *loc,
									   uint32 startIndex,
									   void *tab,
									   uint32 stride,
									   uint32 numAttrib,
									   uint32 nbReplicate,
									   uint32 srcStep = (1 << 16)
									  ) const;

		virtual void    serial		  (NLMISC::IStream &f);
		virtual void    deleteElement (uint32 index);
		virtual void    newElement	  (const CPSEmitterInfo &info);
		virtual void	resize		  (uint32 capacity, uint32 nbPresentElements);
	//@}

	/// \name Input argument of the operator
	//@{
		/** set an argument for the operator
		  * \param argNb must be 0 or 1 for the first and second argument
		  * \param arg The argument. Must have been allocated by new, and is then owned by this object
		  */
		void setArg(uint argNb, CPSAttribMaker<T> *arg)
		{
			nlassert(argNb < 2);
			delete _Arg[argNb];
			_Arg[argNb] = arg;
			if (arg->hasMemory())
			{
				arg->resize(_MaxSize, _Size);
			}
		}

		/** get an argument
		  * \see setArg
		  */
		CPSAttribMaker<T> *getArg(uint argNb)
		{
			nlassert(argNb < 2);
			return _Arg[argNb];
		}

		/** get an argument, const version
		  * \see setArg
		  */
		const CPSAttribMaker<T> *getArg(uint argNb) const
		{
			nlassert(argNb < 2);
			return _Arg[argNb];
		}
	//@}

	/// \name Operator that is performed
	//@{
		/** Set the operator to use
		  * An assertion is thrown when no available
		  */
		void setOp(CPSBinOp::BinOp op)
		{
			nlassert(supportOp(op));
			_Op = op;
		}

		/// return true if an operation is supported. The default support all ops
		bool supportOp(CPSBinOp::BinOp /* op */) { return true; }

		/// get the current operator
		CPSBinOp::BinOp getOp(void) const { return _Op; }
	//@}

	// from CPSAttribMaker
	virtual T getMinValue(void) const { return T() ; /* no mean by default */ }
	virtual T getMaxValue(void) const { return T() ; /* no mean by default */ }

protected:
	void   *makePrivate	(T *buf1,
						 T *buf2,
						 CPSLocated *loc,
						 uint32 startIndex,
						 void *tab,
						 uint32 stride,
						 uint32 numAttrib,
						 bool allowNoCopy = false,
						 uint32 srcStep = (1 << 16),
						 bool	forceClampEntry = false
						) const;

	void    make4Private	(T *buf1,
							 T *buf2,
							 CPSLocated *loc,
							 uint32 startIndex,
							 void *tab,
							 uint32 stride,
							 uint32 numAttrib,
							 uint32 srcStep = (1 << 16)
							) const;

	void	makeNPrivate  (T *buf1,
						   T *buf2,
						   CPSLocated *loc,
						   uint32 startIndex,
						   void *tab,
						   uint32 stride,
						   uint32 numAttrib,
						   uint32 nbReplicate,
						   uint32 srcStep = (1 << 16)
						  ) const;

	CPSBinOp::BinOp   _Op; // the operator being used
	CPSAttribMaker<T> *_Arg[2]; // the arguments for the binary operator
	void clean(void);
	uint32 _Size, _MaxSize;
};


} // NL3D

#include "ps_attrib_maker_bin_op_inline.h"


#endif // NL_PS_ATTRIB_MAKER_BIN_OP_H

/* End of ps_attrib_maker_bin_op.h */

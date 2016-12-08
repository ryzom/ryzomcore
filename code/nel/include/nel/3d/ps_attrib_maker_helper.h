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

#ifndef NL_PS_ATTRIB_MAKER_HELPER_H
#define NL_PS_ATTRIB_MAKER_HELPER_H

#include "nel/3d/ps_attrib_maker.h"

#include "nel/misc/fast_floor.h" // inline assembly for fast float<->int conversions
#include "nel/3d/ps_attrib_maker_iterators.h" // some iterators we use

#include <memory>

namespace NL3D
{






/** This template generate an attrib maker by defining the methods of the CPSCAttribMaker class. You can derive your own class
 * but it is a shortcut to do the job
 *  \param T : the type to produce
 *  \param F : a class that override the () operator, the input is chosen by the user (age, speed ...)
 *             , and the output is the same type as T.
 *             Inline is preferable, as it will be called a lot
 *             It can stores info that indicate how to build it
 */

template <typename T, class F> class CPSAttribMakerT : public CPSAttribMaker<T>
{
	public:
		/// the functor object
		F _F;

		/// compute one value of the attribute for the given index
		virtual T get(CPSLocated *loc, uint32 index);
		virtual T get(const CPSEmitterInfo &info);

		virtual T get(float input)
		{
			NLMISC::OptFastFloorBegin();
			nlassert(input >= 0.f && input <= 1.f);
			T tmp=  _F(input);
			NLMISC::OptFastFloorEnd();
			return tmp;
		}

		/** Fill tab with an attribute by using the given stride. It fills numAttrib attributes, and use it to get the
		 * The particle life as an input
		 */
		/*  virtual void *make(CPSLocated *loc,
							 uint32 startIndex,
							 void *tab, uint32 stride,
							 uint32 numAttrib,
							 bool allowNoCopy = false,
							 uint32 srcStep = (1 << 16)
							 				 ) const;*/

		/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
		 *  \see make()
		 */
		 /* virtual void make4(CPSLocated *loc,
							 uint32 startIndex,
							 void *tab,
							 uint32 stride,
							 uint32 numAttrib,
							 uint32 srcStep = (1 << 16)
							) const;*/

		/** The same as make4, but with n replication instead of 4
		 *  \see make4
		 */
		/* virtual void makeN(CPSLocated *loc,
							uint32 startIndex,
							void *tab,
							uint32 stride,
							uint32 numAttrib,
							uint32 nbReplicate,
							uint32 srcStep = (1 << 16)
						) const;*/


		/// serialization of the object
		virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			sint ver = f.serialVersion(2);
			CPSAttribMaker<T>::serial(f);
		    f.serial(_F);
			switch (ver)
			{
				case 1:
				{
					CPSInputType it;
					f.serialEnum(it.InputType);
					_InputType = it;
				}
				break;
				case 2:
					f.serial(_InputType);
				break;
			}
			f.serial(_Clamp);
		}

		/** construct the attrib maker specifying the number of cycles to do.
		 *  \see setNbCycles()
		 */
		CPSAttribMakerT(float nbCycles) :  CPSAttribMaker<T>(nbCycles), _Clamp(false)
		{}

		/// dtor
		  virtual ~CPSAttribMakerT() {}


		/** tells whether one may choose one attribute from a CPSLocated to use as an input. If false, the input(s) is fixed
		 *  For this class, it is supported
		 */
		virtual bool hasCustomInput(void) { return true; }


		/** set a new input type
		 */
		virtual void setInput(const CPSInputType &input) { _InputType = input; }


		/** get the type of input (if supported). The default return attrDate
		 *  \see hasCustomInput()
		 */
		virtual CPSInputType getInput(void) const { return _InputType; }


		/** tells whether clamping is supported for the input (value can't go above MaxInputValue)
		 */
		bool isClampingSupported(void) const { return true; }


		/** Enable, disable the clamping of input values.
		 *  \see isClampingSupported()
		 */
		virtual void setClamping(bool enable = true) { _Clamp = enable; };


		/** Test if the clamping is enabled.
		 *  \see isClampingSupported()
		 */
		virtual bool getClamping(void) const  { return _Clamp; };


		/// the type of the attribute to be produced
		  typedef T value_type;

		/// the type of the functor object
		  typedef F functor_type;

	private:

		// type of the input
		CPSInputType _InputType;

		// clamping on/ off
		bool _Clamp;



		 /** generate an attribute by using the given iterator. this allow to choose the input of tha attribute maker
  		  *  \param canOverlapOne must be true if the entry iterator can give values above 1
		  *  the attribute maker with no speed penalty
		  */

		 template <typename It> void makeByIterator(It it,
													 void *tab,
													 uint32 stride,
													 uint32 numAttrib,
													 bool canOverlapOne,
													 bool forceClampEntry
													 ) const
		 {
			uint8 *pt = (uint8 *) tab;


			if (this->_NbCycles > 1 || canOverlapOne || forceClampEntry)
			{
				// the value could cycle, so we need to clamp it to 0.0f 1.0f

				if (!_Clamp && !forceClampEntry)
				{
					if (this->_NbCycles == 1)
					{
						while (numAttrib --)
						{
							*(T *)pt = _F(NLMISC::OptFastFractionnalPart(it.get()));
							pt += stride;
							it.advance();
						}
					}
					else
					{
						while (numAttrib --)
						{
							const float time =  this->_NbCycles * (it.get());
							*(T *)pt = _F(NLMISC::OptFastFractionnalPart(time));
							pt += stride;
							it.advance();
						}
					}
				}
				else
				{
					// clamping is on
					float value;
					if (this->_NbCycles == 1)
					{
						while (numAttrib --)
						{
							value = it.get();
							if (value > MaxInputValue)
							{
								value = MaxInputValue;
							}
							*(T *)pt = _F(value);
							pt += stride;
							it.advance();
						}
					}
					else
					{
						while (numAttrib --)
						{
							float value =  this->_NbCycles * (it.get());
							if (value > MaxInputValue)
							{
								value = MaxInputValue;
							}
							*(T *)pt = _F(value);
							pt += stride;
							it.advance();
						}
					}
				}
			}
			else
			{
				// the fastest case : it match the particle's life perfectly

				if (this->_NbCycles == 1)
				{
					while (numAttrib --)
					{
						*(T *)pt = _F(it.get());
						pt += stride;
						it.advance();
					}
				}
				else
				{
					// the particle won't cover the whole pattern during his life
					while (numAttrib --)
					{
						*(T *)pt = _F(this->_NbCycles  * (it.get()));
						pt += stride;
						it.advance();
					}
				}
			}
		}

		/** The same as make, but it replicate each attribute 4 times, thus filling 4*numAttrib. Useful for facelookat and the like
		 *  \param canOverlapOne must be true if the entry iterator can give values above 1
		 *  \see makeByIterator()
		 */
		 template <typename It> void make4ByIterator(It it,
													 void *tab,
													 uint32 stride,
												     uint32 numAttrib,
													 bool canOverlapOne) const
		 {


			uint8 *pt = (uint8 *) tab;


			// first precompute the various strides (stride * 2, 3 and 4)
			// const uint32 stride2 = stride << 1, stride3 = stride + stride2, stride4 = stride2 << 1;

			const uint32 stride2 = stride << 1;

			if (this->_NbCycles > 1 || canOverlapOne)
			{

				if (!_Clamp)
				{
					if (this->_NbCycles == 1)
					{
						while (numAttrib --)
						{
							// fill 4 attrib with the same value at once
							//*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(OptFastFractionnalPart(it.get()));
							*(T *) pt = _F(NLMISC::OptFastFractionnalPart(it.get()));
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride;
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride;
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride2;
							it.advance();
						}
					}
					else
					{
						while (numAttrib --)
						{
							const float time =  this->_NbCycles * (it.get());
							// fill 4 attrib with the same value at once
							//*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(OptFastFractionnalPart(time));
							*(T *) pt =	_F(NLMISC::OptFastFractionnalPart(time));
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride;
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride;
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride2;

							it.advance();
						}
					}
				}
				else
				{
					float value;

					if (this->_NbCycles == 1)
					{
						while (numAttrib --)
						{
							value = it.get();
							if (value > MaxInputValue)
							{
								value = MaxInputValue;
							}
							// fill 4 attrib with the same value at once
							//*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(value);
							*(T *) pt =	_F(value);
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride;
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride;
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride2;

							it.advance();
						}
					}
					else
					{
						while (numAttrib --)
						{
							value =   this->_NbCycles * (it.get());
							if (value > MaxInputValue)
							{
								value = MaxInputValue;
							}
							// fill 4 attrib with the same value at once
							//*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(value);
							*(T *) pt =	_F(value);
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride;
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride;
							*(T *) (pt + stride) = *(T *) pt;
							pt += stride2;
							//pt += stride4; // advance of 4
							it.advance();
						}
					}
				}
			}
			else
			{
				// the fastest case : it match the particle's life perfectly

				if (this->_NbCycles == 1)
				{
					while (numAttrib --)
					{
						// fill 4 attrib with the same value at once
						//*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(it.get());
						*(T *) pt =	_F(it.get());
						*(T *) (pt + stride) = *(T *) pt;
						pt += stride;
						*(T *) (pt + stride) = *(T *) pt;
						pt += stride;
						*(T *) (pt + stride) = *(T *) pt;
						pt += stride2;

						//pt += stride4; // advance of 4
						it.advance();
					}
				}
				else
				{
					// the particle won't cover the whole pattern during his life

					while (numAttrib --)
					{
						// fill 4 attrib with the same value at once
						*(T *) pt =	_F(this->_NbCycles * it.get());
						*(T *) (pt + stride) = *(T *) pt;
						pt += stride;
						*(T *) (pt + stride) = *(T *) pt;
						pt += stride;
						*(T *) (pt + stride) = *(T *) pt;
						pt += stride2;
						//*(T *)pt = *(T *)(pt + stride) = *(T *)(pt + stride2)  = *(T *)(pt + stride3) = _F(_NbCycles * it.get());
						//pt += stride4; // advance of 4
						it.advance();
					}
				}
			}
		 }


		/** The same as make4, but with n replication instead of 4
		 *  \param canOverlapOne must be true if the entry iterator can give values above 1
		 *  \see make4ByIterator
		 */
		 template <typename It> void makeNByIterator(It it, void *tab, uint32 stride, uint32 numAttrib
												  , uint32 nbReplicate, bool canOverlapOne) const
		 {

				nlassert(nbReplicate > 1);

				uint8 *pt = (uint8 *) tab;

				// loop counter
				uint k;


				if (this->_NbCycles > 1 || canOverlapOne)
				{

					if (!_Clamp)
					{
						if (this->_NbCycles == 1)
						{
							while (numAttrib --)
							{
								// fill 4 attrib with the same value at once
								*(T *)pt = _F(NLMISC::OptFastFractionnalPart(it.get()));
								k = nbReplicate - 1;
								do
								{
									*(T *) (pt + stride) = *(T *) pt;
									pt += stride;
								}
								while (--k);
								pt += stride;
								it.advance();
							}
						}
						else
						{
							while (numAttrib --)
							{
								const float time =  this->_NbCycles * (it.get());
								// fill 4 attrib with the same value at once
								*(T *)pt = _F(NLMISC::OptFastFractionnalPart(time));
								k = nbReplicate - 1;
								do
								{
									*(T *) (pt + stride) = *(T *) pt;
									pt += stride;
								}
								while (--k);
								pt += stride;
								it.advance();
							}
						}
					}
					else
					{
						float value;
						// clamping is on
						if (this->_NbCycles == 1)
						{
							while (numAttrib --)
							{
								// fill 4 attrib with the same value at once
								value = it.get();
								if (value > MaxInputValue)
								{
									value = MaxInputValue;
								}
								*(T *)pt = _F(value);
								k = nbReplicate - 1;
								do
								{
									*(T *) (pt + stride) = *(T *) pt;
									pt += stride;
								}
								while (--k);
								pt += stride;
								it.advance();
							}
						}
						else
						{
							while (numAttrib --)
							{
								value =  this->_NbCycles * (it.get());
								if (value > MaxInputValue)
								{
									value = MaxInputValue;
								}
								// fill 4 attrib with the same value at once
								*(T *)pt = _F(value);
								k = nbReplicate - 1;
								do
								{
									*(T *) (pt + stride) = *(T *) pt;
									pt += stride;
								}
								while (--k);
								pt += stride;
								it.advance();
							}
						}
					}
				}
				else
				{
					// the fastest case : it match the particle's life perfectly

					if (this->_NbCycles == 1)
					{
						while (numAttrib --)
						{
							// fill 4 attrib with the same value at once
							*(T *)pt = _F(it.get());
							k = nbReplicate - 1;
							do
							{
								*(T *) (pt + stride) = *(T *) pt;
								pt += stride;
							}
							while (--k);
							pt += stride;
							it.advance();
						}
					}
					else
					{
						// the particle won't cover the whole pattern during his life

						while (numAttrib --)
						{
							// fill 4 attrib with the same value at once
							*(T *)pt =  _F(this->_NbCycles * it.get());
							k = nbReplicate - 1;
							do
							{
								*(T *) (pt + stride) = *(T *) pt;
								pt += stride;
							}
							while (--k);
							pt += stride;
							it.advance();
						}
					}
				}
			}



			/* template <typename T, class F> */
			void /*CPSAttribMakerT<T, F>::*/ *make(CPSLocated *loc,
											  uint32 startIndex,
											  void *tab,
											  uint32 stride,
											  uint32 numAttrib,
											  bool /* allowNoCopy */ /* = false */,
											  uint32 srcStep /*= (1 << 16)*/,
											  bool	forceClampEntry /*= false*/
											 ) const
			{

				NLMISC::OptFastFloorBegin();
				nlassert(loc);

				if (srcStep == (1 << 16))
				{
					switch (_InputType.InputType)
					{
						case CPSInputType::attrDate:
						{
							CPSBaseIterator<TIteratorFloatStep1>
								it(TIteratorFloatStep1(loc->getTime().begin(), startIndex));
							makeByIterator(it, tab, stride, numAttrib, loc->getLastForever(), forceClampEntry);
						}
						break;
						case CPSInputType::attrInverseMass:
						{
							CPSBaseIterator<TIteratorFloatStep1>
								it(TIteratorFloatStep1(loc->getInvMass().begin() , startIndex));
							makeByIterator(it, tab, stride, numAttrib, true, forceClampEntry);
						}
						break;
						case CPSInputType::attrSpeed:
						{
							CVectNormIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getSpeed().begin(), startIndex));
							makeByIterator(it, tab, stride, numAttrib, true, forceClampEntry);
						}
						break;

						case CPSInputType::attrPosition:
						{
							CVectNormIterator<TIteratorVectStep1>
								it( TIteratorVectStep1(loc->getPos().begin(), startIndex) );
							makeByIterator(it, tab, stride, numAttrib, true, forceClampEntry);
						}
						break;
						case CPSInputType::attrUniformRandom:
						{
							CRandomIterator it;
							makeByIterator(it, tab, stride, numAttrib, true, forceClampEntry);
						}
						break;
						case CPSInputType::attrUserParam:
						{
							CDecalIterator it;
							it.Value = loc->getUserParam(_InputType.UserParamNum);
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
						case CPSInputType::attrLOD:
						{

							CFDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
						case CPSInputType::attrSquareLOD:
						{

							CFSquareDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
						case CPSInputType::attrClampedLOD:
						{

							CFClampDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
						case CPSInputType::attrClampedSquareLOD:
						{

							CFClampSquareDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
					}

					NLMISC::OptFastFloorEnd();
					// we must alway copy the data there ...
					return tab;
				}
				else // fixed point steps
				{
					switch (_InputType.InputType)
					{
						case CPSInputType::attrDate:
						{
							CPSBaseIterator<TIteratorFloatStep1616>
								it(TIteratorFloatStep1616(loc->getTime().begin(), startIndex, srcStep));
							makeByIterator(it, tab, stride, numAttrib, loc->getLastForever(), forceClampEntry);
						}
						break;
						case CPSInputType::attrInverseMass:
						{
							CPSBaseIterator<TIteratorFloatStep1616>
								it( TIteratorFloatStep1616(loc->getInvMass().begin(), startIndex, srcStep));
							makeByIterator(it, tab, stride, numAttrib, true, forceClampEntry);
						}
						break;
						case CPSInputType::attrSpeed:
						{
							CVectNormIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getSpeed().begin(), startIndex, srcStep) );
							makeByIterator(it, tab, stride, numAttrib, true, forceClampEntry);
						}
						break;

						case CPSInputType::attrPosition:
						{
							CVectNormIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							makeByIterator(it, tab, stride, numAttrib, true, forceClampEntry);
						}
						break;
						case CPSInputType::attrUniformRandom:
						{
							CRandomIterator it;
							makeByIterator(it, tab, stride, numAttrib, true, forceClampEntry);
						}
						break;
						case CPSInputType::attrUserParam:
						{
							CDecalIterator it;
							it.Value = loc->getUserParam(_InputType.UserParamNum);
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
						case CPSInputType::attrLOD:
						{

							CFDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
						case CPSInputType::attrSquareLOD:
						{

							CFSquareDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
						case CPSInputType::attrClampedLOD:
						{

							CFClampDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
						case CPSInputType::attrClampedSquareLOD:
						{

							CFClampSquareDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeByIterator(it, tab, stride, numAttrib, false, forceClampEntry);
						}
						break;
					}

					NLMISC::OptFastFloorEnd();
					// we must alway copy the data there ...
					return tab;
				}
			}



			/* template <typename T, class F> */
			void /*CPSAttribMakerT<T, F>::*/make4(CPSLocated *loc,
											   uint32 startIndex,
											   void *tab,
											   uint32 stride,
											   uint32 numAttrib,
											   uint32 srcStep /*= (1 << 16)*/
											  ) const
			{
				NLMISC::OptFastFloorBegin();
				nlassert(loc);

				if (srcStep == (1 << 16))
				{
					switch (_InputType.InputType)
					{
						case CPSInputType::attrDate:
						{
							CPSBaseIterator<TIteratorFloatStep1>
								it(TIteratorFloatStep1( loc->getTime().begin(), startIndex) );
							make4ByIterator(it, tab, stride, numAttrib, loc->getLastForever());
						}
						break;
						case CPSInputType::attrInverseMass:
						{
							CPSBaseIterator<TIteratorFloatStep1>
								it( TIteratorFloatStep1(loc->getInvMass().begin(), startIndex) );
							make4ByIterator(it, tab, stride, numAttrib, true);
						}
						break;
						case CPSInputType::attrSpeed:
						{
							CVectNormIterator<TIteratorVectStep1>
								it( TIteratorVectStep1(loc->getSpeed().begin(), startIndex) );
							make4ByIterator(it, tab, stride, numAttrib, true);
						}
						break;

						case CPSInputType::attrPosition:
						{
							CVectNormIterator<TIteratorVectStep1>
								it( TIteratorVectStep1(loc->getPos().begin() , startIndex) );
							make4ByIterator(it, tab, stride, numAttrib, true);
						}
						break;
						case CPSInputType::attrUniformRandom:
						{
							CRandomIterator it;
							make4ByIterator(it, tab, stride, numAttrib, true);
						}
						break;
						case CPSInputType::attrUserParam:
						{
							CDecalIterator it;
							it.Value = loc->getUserParam(_InputType.UserParamNum);
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
						case CPSInputType::attrLOD:
						{

							CFDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
						case CPSInputType::attrSquareLOD:
						{

							CFSquareDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
						case CPSInputType::attrClampedLOD:
						{

							CFClampDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
						case CPSInputType::attrClampedSquareLOD:
						{

							CFClampSquareDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
					}

					NLMISC::OptFastFloorEnd();
				}
				else // fixed point steps
				{
					switch (_InputType.InputType)
					{
						case CPSInputType::attrDate:
						{
							CPSBaseIterator<TIteratorFloatStep1616>
								it(TIteratorFloatStep1616(loc->getTime().begin(), startIndex, srcStep));
							make4ByIterator(it, tab, stride, numAttrib, loc->getLastForever());
						}
						break;
						case CPSInputType::attrInverseMass:
						{
							CPSBaseIterator<TIteratorFloatStep1616>
								it( TIteratorFloatStep1616(loc->getInvMass().begin() , startIndex, srcStep));
							make4ByIterator(it, tab, stride, numAttrib, true);
						}
						break;
						case CPSInputType::attrSpeed:
						{
							CVectNormIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getSpeed().begin(), startIndex, srcStep) );
							make4ByIterator(it, tab, stride, numAttrib, true);
						}
						break;

						case CPSInputType::attrPosition:
						{
							CVectNormIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin() , startIndex, srcStep) );
							make4ByIterator(it, tab, stride, numAttrib, true);
						}
						break;
						case CPSInputType::attrUniformRandom:
						{
							CRandomIterator it;
							make4ByIterator(it, tab, stride, numAttrib, true);
						}
						break;
						case CPSInputType::attrUserParam:
						{
							CDecalIterator it;
							it.Value = loc->getUserParam(_InputType.UserParamNum);
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
						case CPSInputType::attrLOD:
						{

							CFDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
						case CPSInputType::attrSquareLOD:
						{

							CFSquareDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
						case CPSInputType::attrClampedLOD:
						{

							CFClampDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
						case CPSInputType::attrClampedSquareLOD:
						{

							CFClampSquareDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							make4ByIterator(it, tab, stride, numAttrib, false);
						}
						break;
					}

					NLMISC::OptFastFloorEnd();

				}
			}


			/* template <typename T, class F> */
			virtual void /*CPSAttribMakerT<T, F>::*/makeN(CPSLocated *loc,
										uint32 startIndex,
										void *tab,
										uint32 stride,
										uint32 numAttrib,
										uint32 nbReplicate,
										uint32 srcStep /* = (1 << 16)*/
									) const
			{
				NLMISC::OptFastFloorBegin();
				nlassert(loc);
				if (srcStep == (1 << 16))
				{
					switch (_InputType.InputType)
					{
						case CPSInputType::attrDate:
						{
							CPSBaseIterator<TIteratorFloatStep1>
								it(TIteratorFloatStep1(loc->getTime().begin(), startIndex) );
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, loc->getLastForever());
						}
						break;
						case CPSInputType::attrInverseMass:
						{
							CPSBaseIterator<TIteratorFloatStep1>
								it( TIteratorFloatStep1(loc->getInvMass().begin() , startIndex) );
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true);
						}
						break;
						case CPSInputType::attrSpeed:
						{
							CVectNormIterator<TIteratorVectStep1>
								it( TIteratorVectStep1(loc->getSpeed().begin(), startIndex) );
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true);
						}
						break;

						case CPSInputType::attrPosition:
						{
							CVectNormIterator<TIteratorVectStep1>
								it( TIteratorVectStep1( loc->getPos().begin() , startIndex ) );
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true);
						}
						break;
						case CPSInputType::attrUniformRandom:
						{
							CRandomIterator it;
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true);
						}
						break;
						case CPSInputType::attrUserParam:
						{
							CDecalIterator it;
							it.Value = loc->getUserParam(_InputType.UserParamNum);
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
						case CPSInputType::attrLOD:
						{

							CFDot3AddIterator<TIteratorVectStep1>
								it( TIteratorVectStep1(loc->getPos().begin(), startIndex) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
						case CPSInputType::attrSquareLOD:
						{

							CFSquareDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
						case CPSInputType::attrClampedLOD:
						{

							CFClampDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
						case CPSInputType::attrClampedSquareLOD:
						{

							CFClampSquareDot3AddIterator<TIteratorVectStep1>
								it(TIteratorVectStep1(loc->getPos().begin(), startIndex));
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
					}

					NLMISC::OptFastFloorEnd();
				}
				else // fixed point steps
				{
					switch (_InputType.InputType)
					{
						case CPSInputType::attrDate:
						{
							CPSBaseIterator<TIteratorFloatStep1616>
								it(TIteratorFloatStep1616(loc->getTime().begin(), startIndex, srcStep));
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, loc->getLastForever());
						}
						break;
						case CPSInputType::attrInverseMass:
						{
							CPSBaseIterator<TIteratorFloatStep1616>
								it( TIteratorFloatStep1616(loc->getInvMass().begin() , startIndex, srcStep) );
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true);
						}
						break;
						case CPSInputType::attrSpeed:
						{
							CVectNormIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getSpeed().begin(), startIndex, srcStep) );
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true);
						}
						break;

						case CPSInputType::attrPosition:
						{
							CVectNormIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true);
						}
						break;
						case CPSInputType::attrUniformRandom:
						{
							CRandomIterator it;
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, true);
						}
						break;
						case CPSInputType::attrUserParam:
						{
							CDecalIterator it;
							it.Value = loc->getUserParam(_InputType.UserParamNum);
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
						case CPSInputType::attrLOD:
						{

							CFDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
						case CPSInputType::attrSquareLOD:
						{

							CFSquareDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616(loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
						case CPSInputType::attrClampedLOD:
						{

							CFClampDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616( loc->getPos().begin(), startIndex, srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
						case CPSInputType::attrClampedSquareLOD:
						{

							CFClampSquareDot3AddIterator<TIteratorVectStep1616>
								it( TIteratorVectStep1616( loc->getPos().begin(), startIndex,  srcStep) );
							loc->getLODVect(it.V, it.Offset, loc->getMatrixMode());
							makeNByIterator(it, tab, stride, numAttrib, nbReplicate, false);
						}
						break;
					}

					NLMISC::OptFastFloorEnd();
				}
			}
			// Compute a single value from the input assuming that NLMISC::OptFastFloorBegin() has been called
			virtual T getInternal(float input)
			{
				input *= this->_NbCycles;
				if (_Clamp)
				{
					if (input >= MaxInputValue) return _F(1.f);
					return _F(input);
				}
				else
				{
					return (input == MaxInputValue) ? _F(MaxInputValue) :  _F(NLMISC::OptFastFractionnalPart(input));
				}
			}
};



///////////////////////////////////////////////
// implementation of CPSAttribMakerT methods //
///////////////////////////////////////////////
template <typename T, class F>
T  CPSAttribMakerT<T, F>::get(CPSLocated *loc, uint32 index)
{
	NLMISC::OptFastFloorBegin();
	T result;
	nlassert(loc);
	switch (_InputType.InputType)
	{
		case CPSInputType::attrDate:
			result = getInternal(loc->getTime()[index]);
		break;
		case CPSInputType::attrInverseMass:
			result=  getInternal(loc->getInvMass()[index]);
		break;
		case CPSInputType::attrSpeed:
			result = getInternal(loc->getSpeed()[index].norm());
		break;
		case CPSInputType::attrPosition:
			result = getInternal(loc->getPos()[index].norm());
		break;
		case CPSInputType::attrUniformRandom:
		{
			result =  _F(float(rand() * (1 / double(RAND_MAX))));
		}
		break;
		case CPSInputType::attrUserParam:
		{
			result = getInternal(loc->getUserParam(_InputType.UserParamNum));
		}
		break;
		case CPSInputType::attrLOD:
		{
			static NLMISC::CVector lodVect;
			float lodOffset;
			loc->getLODVect(lodVect, lodOffset, loc->getMatrixMode());
			float r = fabsf(loc->getPos()[index] * lodVect + lodOffset);
			r = this->_NbCycles * r > MaxInputValue ? MaxInputValue : r;
			if (_Clamp)
			{
				result = _F(r > MaxInputValue ? MaxInputValue : r);
			}
			else result = (r == MaxInputValue) ? _F(MaxInputValue) : _F(NLMISC::OptFastFractionnalPart(r));
		}
		break;
		case CPSInputType::attrSquareLOD:
		{
			static NLMISC::CVector lodVect;
			float lodOffset;
			loc->getLODVect(lodVect, lodOffset, loc->getMatrixMode());
			float r = loc->getPos()[index] * lodVect + lodOffset;
			r = this->_NbCycles * (r > MaxInputValue ? MaxInputValue : r * r);

			if (_Clamp)
			{
				result = _F(r > MaxInputValue ? MaxInputValue : r);
			}
			else result = (r == MaxInputValue) ? _F(MaxInputValue) : _F(NLMISC::OptFastFractionnalPart(r));
		}
		break;
		case CPSInputType::attrClampedLOD:
		{
			static NLMISC::CVector lodVect;
			float lodOffset;
			loc->getLODVect(lodVect, lodOffset, loc->getMatrixMode());

			float r = loc->getPos()[index] * lodVect + lodOffset;
			if (r < 0)
			{
				result = _F(MaxInputValue);
				break;
			}
			r = this->_NbCycles * (r > MaxInputValue ? MaxInputValue : r);
			if (_Clamp)
			{
				result = _F(r > MaxInputValue ? MaxInputValue : r);
			}
			else result = (r == MaxInputValue) ? _F(MaxInputValue) : _F(NLMISC::OptFastFractionnalPart(r));
		}
		break;
		case CPSInputType::attrClampedSquareLOD:
		{
			static NLMISC::CVector lodVect;
			float lodOffset;
			loc->getLODVect(lodVect, lodOffset, loc->getMatrixMode());

			float r = loc->getPos()[index] * lodVect + lodOffset;
			if (r < 0)
			{
				result = _F(MaxInputValue);
				break;
			}
			r = this->_NbCycles * (r > MaxInputValue ? MaxInputValue : r * r);
			if (_Clamp)
			{
				result = _F(r > MaxInputValue ? MaxInputValue : r);
			}
			else result = (r == MaxInputValue) ? _F(MaxInputValue) : _F(NLMISC::OptFastFractionnalPart(r));
		}
		break;
		default:
			result = T();
		break;
	}

	NLMISC::OptFastFloorEnd();
	return result;
}

template <typename T, class F>
T  CPSAttribMakerT<T, F>::get(const CPSEmitterInfo &infos)
{
	NLMISC::OptFastFloorBegin();
	T result;
	switch (_InputType.InputType)
	{
		case CPSInputType::attrDate:
		{
			result = getInternal(infos.Life);
		}
		break;
		case CPSInputType::attrInverseMass:
		{
			result = getInternal(infos.InvMass);
		}
		break;
		case CPSInputType::attrSpeed:
		{
			result = getInternal(infos.Speed.norm());
		}
		break;
		case CPSInputType::attrPosition:
		{
			result = getInternal(infos.Pos.norm());
		}
		break;
		case CPSInputType::attrUniformRandom:
		{
			result =  _F(float(rand() * (1 / double(RAND_MAX))));
		}
		break;
		case CPSInputType::attrUserParam:
		{
			result = getInternal(infos.Loc->getUserParam(_InputType.UserParamNum));
		}
		break;
		case CPSInputType::attrLOD:
		{
			static NLMISC::CVector lodVect;
			float lodOffset;
			infos.Loc->getLODVect(lodVect, lodOffset, infos.Loc->getMatrixMode());
			float r = fabsf(infos.Pos * lodVect + lodOffset);
			r = this->_NbCycles * r > MaxInputValue ? MaxInputValue : r;
			if (_Clamp)
			{
				result = _F(r > MaxInputValue ? MaxInputValue : r);
			}
			else result = (r == MaxInputValue) ? _F(MaxInputValue) : _F(NLMISC::OptFastFractionnalPart(r));
		}
		break;
		case CPSInputType::attrSquareLOD:
		{
			static NLMISC::CVector lodVect;
			float lodOffset;
			infos.Loc->getLODVect(lodVect, lodOffset, infos.Loc->getMatrixMode());
			float r = infos.Pos * lodVect + lodOffset;
			r = this->_NbCycles * (r > MaxInputValue ? MaxInputValue : r * r);

			if (_Clamp)
			{
				result = _F(r > MaxInputValue ? MaxInputValue : r);
			}
			else result = (r == MaxInputValue) ? _F(MaxInputValue) : _F(NLMISC::OptFastFractionnalPart(r));
		}
		break;
		case CPSInputType::attrClampedLOD:
		{
			static NLMISC::CVector lodVect;
			float lodOffset;
			infos.Loc->getLODVect(lodVect, lodOffset, infos.Loc->getMatrixMode());
			float r = infos.Pos * lodVect + lodOffset;
			if (r < 0)
			{
				result = _F(MaxInputValue);
				break;
			}
			r = this->_NbCycles * (r > MaxInputValue ? MaxInputValue : r);
			if (_Clamp)
			{
				result = _F(r > MaxInputValue ? MaxInputValue : r);
			}
			else result = (r == MaxInputValue) ? _F(MaxInputValue) : _F(NLMISC::OptFastFractionnalPart(r));
		}
		break;
		case CPSInputType::attrClampedSquareLOD:
		{
			static NLMISC::CVector lodVect;
			float lodOffset;
			infos.Loc->getLODVect(lodVect, lodOffset, infos.Loc->getMatrixMode());

			float r = infos.Pos * lodVect + lodOffset;
			if (r < 0)
			{
				result = _F(MaxInputValue);
				break;
			}
			r = this->_NbCycles * (r > MaxInputValue ? MaxInputValue : r * r);
			if (_Clamp)
			{
				result = _F(r > MaxInputValue ? MaxInputValue : r);
			}
			else result = (r == MaxInputValue) ? _F(MaxInputValue) : _F(NLMISC::OptFastFractionnalPart(r));
		}
		break;
		default:
			result = T();
		break;
	}
	NLMISC::OptFastFloorEnd();
	return result;
}

/** this functor
  *
  */


/**  This class is an attribute maker that has memory, all what is does is to duplicate its mem when 'make' is called
  *  It own an attribute maker that tells how to produce the attribute from its emiter date, speed and so on ...
  */
template <typename T> class CPSAttribMakerMemoryBase : public CPSAttribMaker<T>
{
public:

	/// ctor (note : we don't use the nbCycle field ...)
	CPSAttribMakerMemoryBase() : CPSAttribMaker<T>(1.f), _Scheme(NULL)
	{
		this->_HasMemory = true;
	}

	/** set a default value for initialisation, otherwise it will be garbage.
	  * This is needed when new element are generated, but not from an emitter
	  * for example, when you set this scheme to a LocatedBindable that does have a least one instance in it
	  *
	  *  example :
	  *      CPSDot *d = new CPSDot;
	  *      CPSAttribMakerMemory<RGBA> *genAttribMaker = new CPSAttribMakerMemory<RGBA>;
	  *      genAttribMaker->setScheme(CPSColorBlender(CRGBA::White, CRGBA::Black)
	  *      Now, if an emitter emit these particle, it'll start to emit white ones, and then black ones
      *      d->setColorScheme(  genAttribMaker);
	  *      now, suppose that there were several dot instanciated before the setScheme is performed :
	  *          d->newElement();
	  *      no color has been memorized for this element, so when setScheme is performed, it has to generate one
	  *      There are no emitter that provides it, so its taken from the default value
	  *      Note : this should only be useful in an editor, that allow the user to change the scheme with a running system ...
	  *
	  */

    virtual void setDefaultValue(T defaultValue) { _DefaultValue = defaultValue;}

	/// get the default value :
	virtual T getDefaultValue(void) const { return _DefaultValue; }



	/** set the scheme used to store attribute. this MUST be called, otherwise an assertion will be thrown later
	  * It must have been allocated by new, and it will be deleted by this object
	  */
	void setScheme(CPSAttribMaker<T> *scheme)
	{
		nlassert(scheme);
		if (_Scheme) delete _Scheme;
		_Scheme = scheme;
		if (_Scheme->hasMemory())
		{
			_Scheme->resize(_T.getMaxSize(), _T.getSize());
		}
	}

	/// get the scheme used
	CPSAttribMaker<T> *getScheme(void) { return _Scheme; }
	/// get the scheme used (const version)
	const CPSAttribMaker<T> *getScheme(void) const { return _Scheme; }


	// copy ctor
	CPSAttribMakerMemoryBase(const CPSAttribMakerMemoryBase &src) : CPSAttribMaker<T>(src) // parent copy ctor
	{
		nlassert(src._Scheme);
		std::auto_ptr<CPSAttribMaker<T> > s(NLMISC::safe_cast<CPSAttribMaker<T> *>(src._Scheme->clone()));
		this->_T = src._T;
		this->_DefaultValue = src._DefaultValue;
		this->_Scheme = s.release();
	}
	/// dtor
	~CPSAttribMakerMemoryBase()
	{
		if (_Scheme)
		{
			delete _Scheme;
		}
	}

	/// inherited from CPSAttribMaker
	virtual T get(CPSLocated * /* loc */, uint32 index)
	{
		if (index < _T.getSize()) return _T[index];
		else return _DefaultValue;
	}
	virtual T get(const CPSEmitterInfo &/* infos */) { 	return _DefaultValue; }

	/// inherited from CPSAttribMaker
	virtual void *make(CPSLocated * /* loc */,
					   uint32 startIndex,
					   void *output,
					   uint32 stride,
					   uint32 numAttrib,
					   bool allowNoCopy = false,
					   uint32 srcStep = (1 << 16),
					   bool /* forceClampEntry */ = false
					  ) const
	{
		if (!numAttrib) return output;
		void *tab = output;
		if (!allowNoCopy || srcStep != (1 << 16) || sizeof(T) != stride)
		{
			// we just copy what we have memorized
			if (srcStep == (1 << 16))
			{
				typename CPSAttrib<T>::const_iterator it = _T.begin() + startIndex, endIt = _T.begin() + startIndex + numAttrib;
				do
				{
					*(T *) tab = *it;
					++it;
					tab = (uint8 *) tab + stride;
				}
				while (it != endIt);
			}
			else // no constant step
			{
				uint32 fpIndex = startIndex * srcStep;
				typename CPSAttrib<T>::const_iterator startIt = _T.begin();
				while (numAttrib --)
				{
					*(T *) tab = *(startIt + (fpIndex >> 16));
					tab = (uint8 *) tab + stride;
					fpIndex += srcStep;
				}
			}
			return output;
		}
		else
		{
			// the caller will read data directly in the vector ...
			return (void *) &(*(_T.begin() + startIndex));
		}
	}

	/// inherited from CPSAttribMaker
	virtual void make4(CPSLocated * /* loc */,
					   uint32 startIndex,
					   void *tab,
					   uint32 stride,
					   uint32 numAttrib,
					   uint32 srcStep = (1 << 16)
					   ) const
	{
		// we just copy what we have memorized
		if (srcStep == (1 << 16))
		{
			typename CPSAttrib<T>::const_iterator it = _T.begin() + startIndex, endIt = _T.begin() + startIndex + numAttrib;
			while (it != endIt)
			{
				*(T *) tab = *it;
				tab = (uint8 *) tab + stride;
				*(T *) tab = *it;
				tab = (uint8 *) tab + stride;
				*(T *) tab = *it;
				tab = (uint8 *) tab + stride;
				*(T *) tab = *it;
				tab = (uint8 *) tab + stride;
				++it;
			}
		}
		else
		{
			uint32 fpIndex = startIndex * srcStep;
			typename CPSAttrib<T>::const_iterator startIt = _T.begin();
			while (numAttrib --)
			{
				*(T *) tab = *(startIt + (fpIndex >> 16));
				*(T *) ((uint8 *) tab + stride) = *(T *) tab;
				tab = (uint8 *) tab + stride;
				*(T *) ((uint8 *) tab + stride) = *(T *) tab;
				tab = (uint8 *) tab + stride;
				*(T *) ((uint8 *) tab + stride) = *(T *) tab;

				tab = (uint8 *) tab + stride + stride;
				fpIndex += srcStep;
			}
		}
	}

	/// inherited from CPSAttribMaker
	virtual void makeN(CPSLocated * /* loc */,
					   uint32 startIndex,
					   void *tab,
					   uint32 stride,
					   uint32 numAttrib,
					   uint32 nbReplicate,
					   uint32 srcStep = (1 << 16)
					  ) const
	{
		// we just copy what we have memorized
		uint k;
		typename CPSAttrib<T>::const_iterator it = _T.begin() + startIndex, endIt = _T.begin() + startIndex + numAttrib;
		if (srcStep == (1 << 16))
		{
			while (it != endIt)
			{

				for (k = 0; k < nbReplicate; ++k)
				{
					*(T *) tab = *it;
					tab = (uint8 *) tab + stride;
				}
				++it;
			}
		}
		else
		{
			uint32 fpIndex = startIndex * srcStep;
			typename CPSAttrib<T>::const_iterator startIt = _T.begin();

			while (numAttrib --)
			{
				*(T *) tab = *(startIt + (fpIndex >> 16));
				for (k = 1; k < nbReplicate; ++k)
				{
					*(T *) ((uint8 *) tab + stride) = *(T *) tab;
					tab = (uint8 *) tab + stride;
				}
				tab = (uint8 *) tab + stride;
				fpIndex += srcStep;
			}

		}
	}

	/// serialisation of the object. Derivers MUST call this, (if they use the attribute of this class at least)
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream)
	{

		f.serialVersion(1);
		CPSAttribMaker<T>::serial(f);
		if (f.isReading())
		{
			if (_Scheme) delete _Scheme;
		}
		f.serialPolyPtr(_Scheme);
		f.serial(_T);
		f.serial(_DefaultValue);
	}

	/// inherited from CPSAttribMaker
	virtual void deleteElement(uint32 index)
	{
		nlassert(_Scheme); // you should have called setScheme !
		_T.remove(index);
		if (_Scheme->hasMemory())
		{
			_Scheme->deleteElement(index);
		}
	}
	/// inherited from CPSAttribMaker
	virtual void newElement(const CPSEmitterInfo &info)
	{
		nlassert(_Scheme); // you should have called setScheme !

		// we should create the contained scheme before this one if it has memory...
		if (_Scheme->hasMemory())
		{
			_Scheme->newElement(info);
		}

		if (info.Loc)
		{
			_T.insert(_Scheme->get(info));
		}
		else
		{
			/** well a value may be returned without having to know the emitter (random, user param ...)
			  * but this case is really useless anyway ...
			  */

			_T.insert(_DefaultValue);
		}
	}
	virtual void resize(uint32 capacity, uint32 nbPresentElements)
	{
		nlassert(capacity < (1 << 16));
		_T.resize(capacity);
		if (nbPresentElements > _T.getSize())
		{
			while (_T.getSize() != nbPresentElements)
			{
				_T.insert(_DefaultValue);
			}
		}
		else if (nbPresentElements < _T.getSize())
		{
			while (_T.getSize() != nbPresentElements)
			{
				_T.remove(_T.getSize() - 1);
			}
		}


		if (_Scheme && _Scheme->hasMemory())
		{
			_Scheme->resize(capacity,   nbPresentElements);
		}

	}
	virtual bool hasCustomInput() { return false; }

protected:
	// the attribute we memorize
	CPSAttrib<T> _T;

	// the default value for generation (when no emitter can be used)
	T _DefaultValue;

	/** this attribute maker tells us how to produce arguments from an emitter. as an example, we may want to have a gradient
	  * of color : the emitter emit green then blue particles, following a gradient. the color is produced by _Scheme and
	  * _T stores it
	  */
	CPSAttribMaker<T> *_Scheme;
};


/** Standard version for attrib maker memory : don't redefine the getMinValue & getMaxValue methods -> meaningful for ordered sets only
  */
template <typename T> class CPSAttribMakerMemory : public CPSAttribMakerMemoryBase<T>
{
public:
	// default ctor
	CPSAttribMakerMemory() : CPSAttribMakerMemoryBase<T>() {}
	CPSAttribMakerMemory(const CPSAttribMakerMemory &other) : CPSAttribMakerMemoryBase<T>(other) {}
};

/** specializations for integral types : they have method getMin & getMax
  * We update the min & max value each time a new element is inserted so it is just a minoration or a majoration of the real value.
  * But as told in CPSAttribMaker, we just need an approximation
  */
/** specialization for uint32
  */
template <>
class CPSAttribMakerMemory<uint32> : public  CPSAttribMakerMemoryBase<uint32>
{
public:
	// default ctor
	CPSAttribMakerMemory() : CPSAttribMakerMemoryBase<uint32>() {}
	// copy ctor
	CPSAttribMakerMemory(const CPSAttribMakerMemory<uint32> &other) : CPSAttribMakerMemoryBase<uint32>(other)
	{
		_MinValue = other._MinValue;
		_MaxValue = other._MaxValue;
	}
	// serial. Should update min / max when reading
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	virtual uint32 getMinValue(void) const { return _MinValue; }
	virtual uint32 getMaxValue(void) const { return _MaxValue; }
	virtual void newElement(const CPSEmitterInfo &info);
private:
	uint32 _MinValue;
	uint32 _MaxValue;
};
/** specialization for sint32
  */
template <>
class CPSAttribMakerMemory<sint32> : public  CPSAttribMakerMemoryBase<sint32>
{
public:
	// default ctor
	CPSAttribMakerMemory() : CPSAttribMakerMemoryBase<sint32>() {}
	// copy ctor
	CPSAttribMakerMemory(const CPSAttribMakerMemory<sint32> &other) : CPSAttribMakerMemoryBase<sint32>(other)
	{
		_MinValue = other._MinValue;
		_MaxValue = other._MaxValue;
	}
	// serial. Should update min / max when reading
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	virtual sint32 getMinValue(void) const { return _MinValue; }
	virtual sint32 getMaxValue(void) const { return _MaxValue; }
	virtual void newElement(const CPSEmitterInfo &info);
private:
	sint32 _MinValue;
	sint32 _MaxValue;
};
/** specialization for float
  */
template <>
class CPSAttribMakerMemory<float> : public  CPSAttribMakerMemoryBase<float>
{
public:
	// default ctor
	CPSAttribMakerMemory() : CPSAttribMakerMemoryBase<float>() {}
	// copy ctor
	CPSAttribMakerMemory(const CPSAttribMakerMemory<float> &other) : CPSAttribMakerMemoryBase<float>(other)
	{
		_MinValue = other._MinValue;
		_MaxValue = other._MaxValue;
	}
	// serial. Should update min / max when reading
	virtual void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	virtual float getMinValue(void) const { return _MinValue; }
	virtual float getMaxValue(void) const { return _MaxValue; }
	virtual void newElement(const CPSEmitterInfo &info);
private:
	float _MinValue;
	float _MaxValue;
};





} // NL3D


#endif // NL_PS_ATTRIB_MAKER_HELPER_H

/* End of ps_attrib_maker_helper.h */

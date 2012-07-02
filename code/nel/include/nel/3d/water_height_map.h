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


#ifndef WATER_HEIGHT_MAP_H
#define WATER_HEIGHT_MAP_H


#include "nel/misc/types_nl.h"
#include "nel/misc/vector_2f.h"
#include "nel/3d/u_water.h"
#include "nel/3d/animation_time.h"


#include <vector>
#include <string>


#ifdef _X
#	undef _X
#endif


namespace NL3D
{


class CWaterPoolManager;


/**
  * This class is a portion of water, it encodes its height, and simulates its propagation.
  * It contains datas about the eight around the user position
  *
  *
  *			size x 2
  *     |--------|
  *	   s| (x,y)  |
  *	   i|   |---||
  *	   z|   |   ||
  *	   e|   |---||
  *	   x|        |
  *    2|--------|
  */

const uint NumWaterMap = 3; // number of water maps

class CWaterHeightMap : public UWaterHeightMap
{
public:

	/** Animate this water pool.
	  * Usually called by CWaterModel before the display, if this height map date is not the same as the current date.
	  */
	void animate(float deltaT);

	/** Set this quad dimension. It is given as a power of 2
	  * This also reset the eightField values
	  */
	void					setSize(uint32 size);

	/// return the size of the height map
	uint32					getSize(void) const { return _Size; }

	/// return the power of 2 used for this quad size

	/** Set the userPos (relative to the height map coordinates). This is needed because a height map can't be used with large surface (such as a sea).
	  * As a consequence, the height map is only valid below the user (e.g from user.x - 0.5 * size to user.x + 0.5 *size).
	  * When setPos is called, and if a move has occured, new area of the height field are set to 0
	  * The pos set will be taken in account when buffers have been swapped (e.g when the propagation time as ellapsed)
	  */
	void					setUserPos(sint x, sint y);


	/** Retrieve the use pos
	  * NB: this may be different from the params of a former call to setUserPos (should have been taken in account)
	  */
	void					getUserPos(sint &x, sint &y) const;



	/// create a perturbation in the height map.
	void					perturbate(sint x, sint y, sint radius, float intensity);

	/** Inherited from UWaterHeightMap. This version takes a location in world space
	  */
	virtual void	perturbate(const NLMISC::CVector2f &pos, float strenght, float radius) ;


	/// create a point perturbation in the height map.
	void					perturbatePoint(sint x, sint y, float intensity);

	/** Inherited from UWaterHeightMap. This version takes a location in world space
	  */
	virtual void	perturbatePoint(const NLMISC::CVector2f &pos, float strenght);

	/** Inherited from UWaterHeightMap. Get the height of water at the given location.
	  */
	virtual float	getHeight(const NLMISC::CVector2f &pos);


	/// get a pointer on the current buffer.
	float					*getPointer(void) { return &(_Map[_CurrMap][0]); }

	/// get a pointer on the previous buffer.
	float					*getPrevPointer(void) { return &(_Map[(_CurrMap + (NumWaterMap - 1)) % NumWaterMap][0]); }

	/// get the ratio between the previous and the current buffer
	float					getBufferRatio() const { return _PropagationTime != 0 ? _PropagateEllapsedTime / _PropagationTime : 0.f; }

	/// get a pointer on the gradient buffer
	//NLMISC::CVector2f		*getGradPointer(void) { return &_Grad[0]; }


	/// enable automatic waves generation
	void					enableWaves(bool enabled = true) { _WavesEnabled = enabled; }

	/// test wheter automatic waves generation is enabled
	bool					areWavesEnabled() const { return _WavesEnabled; }

	/** Tells this height map the params to automatically generate waves. They are generated as perturbation on the border
	  * of the field
	  * \param   intensity The intensity of the waves. 0 disable waves
	  * \param period  the time ellapsed between each waves
	  * \param radius  the radius od the impulsion of the waves to be created
	  * \param border  true if waves should only be generated on the border of height map, (actually, where waves can't be seen because of distance, this avoid to see the impulsion)
	  */
	void					setWaves(float intensity, float period, uint radius, bool border);

	/// get the intensity of waves
	float					getWaveIntensity() const { return _WaveIntensity; }

	/// get the period of waves
	float					getWavePeriod() const { return _WavePeriod; }

	/// radius of impulsion for the waves
	uint32					getWaveImpulsionRadius() const { return _WaveImpulsionRadius; }

	/// Test whether waves are enabled on the border
	bool					getBorderWaves() const { return _BorderWaves; }

	/// damping
	void					setDamping(float damping) { nlassert(damping >= 0 && damping < 1); _Damping = damping; }
	float					getDamping() const { return _Damping; }

	/// filter weight
	void					setFilterWeight(float filterWeight) { _FilterWeight = filterWeight; }
	float					getFilterWeight() const { return _FilterWeight; }

	/// water unit size
	void					setUnitSize(float unitSize) { _UnitSize = unitSize; }
	float					getUnitSize() const { return _UnitSize; }

	/// the last update date
	sint64					Date;

	/// serial the pools data's
	void					serial(NLMISC::IStream &f)  throw(NLMISC::EStream);

	/// Set this pool name.
	void					setName(const std::string &name) { _Name = name; }

	/// Get this pool name.
	const std::string		&getName() const { return _Name; }

	// ctor (use the water pool manager instead)
	CWaterHeightMap();
	// dtor
	virtual ~CWaterHeightMap() {}

	/// Set the propagation time. This is the time needed to go from one unit to one other in the height map
	void					setPropagationTime(float time);

	/// Get the propagation time
	TAnimationTime			getPropagationTime() const { return _PropagationTime; }


private:
	void animateWaves(float deltaT);

	void updateUserPos();

	void animatePart(float startTime, float endTime);

	/** Perform water propagation on this quad.
	  * You should call swapBuffers after this, or after calling filter.
	  * \param damping The attenuation factor used for propagation.
	  */
	void					propagate(uint startLine, uint endLine);


	/// apply a filter on the height field
	void					filter(uint startLine, uint endLine);

	/// swap the height maps. It must be called once propagation and filtering have been performed
	void					swapBuffers(float deltaT);


	friend class CWaterPoolManager;

	std::string                _Name;
	bool					   _WavesEnabled;
	float					   _Damping;
	float					   _FilterWeight;
	float					   _UnitSize;
	float					   _WaveIntensity;
	float					   _WavePeriod;
	uint32					   _WaveImpulsionRadius;
	bool					   _BorderWaves;
	float					   _EmitEllapsedTime;
	float					   _PropagateEllapsedTime;
	TAnimationTime             _PropagationTime; // the time needed to perform a propagation, this allow split the propagation computation over time.


	uint					   _X, _Y;
	uint					   _NewX, _NewY;
	typedef std::vector<float>				TFloatVect;
	typedef std::vector<NLMISC::CVector2f > TFloat2Vect;

	TFloatVect				   _Map[NumWaterMap]; // the 2 maps used for propagation
	//TFloat2Vect				   _Grad;   // used to store the gradient
	uint8					   _CurrMap;
	uint32					   _Size;

	/// clear an area of the water height map (has clipping, but no wrapping)
	void						clearArea(uint8 currMap, sint x, sint y, sint width, sint height);
	// same than clearArea, but perform on both maps
	void						clearZone(sint x, sint y, sint width, sint height);
	/// displace the height map, when a boundary has been reached
	void						makeCpy(uint buffer, uint dX, uint dY, uint sX, uint sY, uint width, uint height);

	/// clear the borders
	void						clearBorder(uint currMap);


};

} // NL3D

#endif

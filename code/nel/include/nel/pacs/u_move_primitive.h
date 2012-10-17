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

#ifndef NL_U_MOVE_PRIMITIVE_H
#define NL_U_MOVE_PRIMITIVE_H

#include "nel/misc/types_nl.h"
#include "u_global_position.h"

namespace NLMISC
{
	class CVectorD;
}

namespace NLPACS
{

class UMoveContainer;
class UGlobalPosition;

/**
 * Description of movables primitives.
 *
 * This primitive can be a 2d oriented box or a 2d oriented cylinder.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UMovePrimitive
{
public:

	/// Type of the collision mask.
	typedef uint32 TCollisionMask;
	typedef uint64 TUserData;
	//typedef uintptr_t TUserData;

	/// Primitive mode
	enum TType
	{
		/**
		  * This is a static 2d oriented bounding box. It can be oriented only on the Z axis.
		  * It has a height. Collision can be performed only on its sides but not on its top and bottom
		  * planes. It doesn't mode. Default value.
		  */
		_2DOrientedBox=0x0,

		/**
		  * This is a movable 2d oriented cylinder. It can be oriented only on the Z axis.
		  * It has a height. Collision can be performed only on its sides but not on its top and bottom
		  * planes. It can move only with 3d translations.
          */
		_2DOrientedCylinder=0x1,
	};


	/// Reaction mode
	enum TReaction
	{
		/**
		  * No reaction. For static objects or not influenced objects. Default value.
		  */
		DoNothing=0x0,

		/**
		  * This object slides over surfaces.
		  */
		Slide=0x10,

		/**
		  * This object reflects over surfaces.
		  */
		Reflexion=0x20,

		/**
		  * This object stops over surfaces.
		  */
		Stop=0x40,
	};

	/// Reaction mode
	enum TTrigger
	{
		/**
		  * Not a trigger.
		  */
		NotATrigger=0x0,

		/**
		  * This is a one time trigger. This trigger is actived only when an object enter in its volume.
		  */
		EnterTrigger=0x100,

		/**
		  * This is a one time trigger. This trigger is actived only when an object exit from its volume.
		  */
		ExitTrigger=0x200,

		/**
		  * This is an overlap trigger. This trigger is actived each time the object overlap the trigger.
		  */
		OverlapTrigger=0x400,
	};

	/**
	  * User data.
	  */
	TUserData		UserData;

	virtual ~UMovePrimitive() {}

	/// \name Setup the primitive static parts.

	/**
	  * Set the primitive type.
	  *
	  * \param type is the new primitive type.
	  */
	virtual void	setPrimitiveType (TType type) =0;

	/**
	  * Set the reaction type.
	  *
	  * \param type is the new reaction type.
	  */
	virtual void	setReactionType (TReaction type) =0;

	/**
	  * Set the trigger type. Default type is NotATrigger.
	  *
	  * \param type is the new trigger type.
	  */
	virtual void	setTriggerType (TTrigger type) =0;

	/**
	  * Set the collision mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new collision mask.
	  */
	virtual void	setCollisionMask (TCollisionMask mask) =0;

	/**
	  * Set the occlusion mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new occlusion mask.
	  */
	virtual void	setOcclusionMask (TCollisionMask mask) =0;

	/**
	  * Set the obstacle flag.
	  *
	  * \param obstacle is true if this primitive is an obstacle, else false.
	  */
	virtual void	setObstacle (bool obstacle) =0;

	/**
	  * Set the attenuation of collision for this object. Default value is 1. Should be between 0~1.
	  * 0, all the energy is attenuated by the collision. 1, all the energy stay in the object.
	  * Used only with the flag Reflexion.
	  *
	  * \param attenuation is the new attenuation for the primitive.
	  */
	virtual void	setAbsorbtion (float attenuation) =0;

	/** Tells that the primitive should not be snapped to ground.
	  * The default is false
	  */
	virtual	void	setDontSnapToGround(bool dont) = 0;

	/// Test if snapping to ground is off
	virtual bool	getDontSnapToGround() const = 0;

	/**
	  * Set the box size. Only for boxes.
	  *
	  * \param width is the new width size of the box. It the size of the sides aligned on OX.
	  * \param depth is the new depth size of the box. It the size of the sides aligned on OY.
	  */
	virtual void	setSize (float width, float depth) =0;

	/**
	  * Set the height. For boxes or cylinder.
	  *
	  * \param height is the new height size of the box. It the size of the sides aligned on OZ.
	  */
	virtual void	setHeight (float height) =0;

	/**
	  * Set the cylinder size. Only for cylinder.
	  *
	  * \param radius is the new radius size of the cylinder.
	  */
	virtual void	setRadius (float radius) =0;

	/// \name Access the primitive static parts.

	/**
	  * Set the primitive type.
	  *
	  * \param type is the new primitive type.
	  */
	virtual TType		getPrimitiveType () const =0;

	/**
	  * Set the reaction type.
	  *
	  * \param type is the new reaction type.
	  */
	virtual TReaction	getReactionType () const =0;

	/**
	  * Set the trigger type. Default type is NotATrigger.
	  *
	  * \param type is the new trigger type.
	  */
	virtual TTrigger	getTriggerType () const =0;

	/**
	  * Set the collision mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new collision mask.
	  */
	virtual TCollisionMask	getCollisionMask () const =0;

	/**
	  * Set the occlusion mask for this primitive. Default mask is 0xffffffff.
	  *
	  * \param mask is the new occlusion mask.
	  */
	virtual TCollisionMask	getOcclusionMask () const =0;

	/**
	  * Set the obstacle flag.
	  *
	  * \param obstacle is true if this primitive is an obstacle, else false.
	  */
	virtual bool			getObstacle () const =0;

	/**
	  * Set the attenuation of collision for this object. Default value is 1. Should be between 0~1.
	  * 0, all the energy is attenuated by the collision. 1, all the energy stay in the object.
	  * Used only with the flag Reflexion.
	  *
	  * \param attenuation is the new attenuation for the primitive.
	  */
	virtual float			getAbsorbtion () const =0;

	/**
	  * Set the box size. Only for boxes.
	  *
	  * \param width is the new width size of the box. It the size of the sides aligned on OX.
	  * \param depth is the new depth size of the box. It the size of the sides aligned on OY.
	  */
	virtual void			getSize (float& width, float& depth) const =0;

	/**
	  * Set the height. For boxes or cylinder.
	  *
	  * \param height is the new height size of the box. It the size of the sides aligned on OZ.
	  */
	virtual float			getHeight () const =0;

	/**
	  * Set the cylinder size. Only for cylinder.
	  *
	  * \param radius is the new radius size of the cylinder.
	  */
	virtual float			getRadius () const =0;

	/**
	  * Return true if the primitive is collisionable
	  */
	virtual bool			isCollisionable() const =0;

	/// \name Setup the primitive dynamic parts.

	/**
	  * Set the new orientation of the move primitive. Only for the box primitives.
	  *
	  * If you modify a noncollisionable primitive with this method, you must evaluate in the world
	  * image where you have modify it before modify any other dynamic properties in another world image.
	  *
	  * \param rot is the new OZ rotation in radian.
	  * \param worldImage is the world image in which the primitive must be oriented.
	  */
	virtual void	setOrientation (double rot, uint8 worldImage) =0;

	/// \name Access the primitive dynamic parts.

	/**
	  * Set the new orientation of the move primitive. Only for the box primitives.
	  *
	  * If you modify a noncollisionable primitive with this method, you must evaluate in the world
	  * image where you have modified it before modify any other dynamic properties in another world image.
	  *
	  * \param rot is the new OZ rotation in radian.
	  * \param worldImage is the world image in which the primitive must be oriented.
	  */
	virtual double			getOrientation (uint8 worldImage) const =0;

	/**
	  * Set the global position of the move primitive. This method is fast because
	  * you must pass the global position of the primitive.
	  *
	  * If you modify a noncollisionable primitive with this method, you must evaluate in the world
	  * image where you have modified it before modify any other dynamic properties in another world image.
	  *
	  * \param pos is the new global position of the primitive.
	  */
	virtual void			getGlobalPosition (UGlobalPosition& pos, uint8 worldImage) const =0;

	/**
	  * Get the position of the move primitive at the end of the movement.
	  * This method is slow. Just for initilisation and teleportation.
	  *
	  * If you modify a noncollisionable primitive with this method, you must evaluate in the world
	  * image where you have modified it before modify any other dynamic properties in another world image.
	  *
	  * \return the new position of the primitive.
	  */
	virtual NLMISC::CVectorD	getFinalPosition (uint8 worldImage)  const=0;

	/**
	  * Get the speed vector for this primitive.
	  *
	  * \Return the new speed vector.
	  */
	virtual const NLMISC::CVectorD&	getSpeed (uint8 worldImage) const =0;

	/// \name Move the primitive.

	/**
	  * Insert the primitive in a world image of the move container.
	  *
	  * This primitive must a collisionable primitive.
	  *
	  * \param worldImage is the number of the world image where you want to insert the primitive.
	  */
	virtual void	insertInWorldImage (uint8 worldImage) =0;

	/**
	  * Remove the primitive from a world image of the move container.
	  *
	  * This primitive must a collisionable primitive.
	  *
	  * \param worldImage is the number of the world image from where you want to remove the primitive.
	  */
	virtual void	removeFromWorldImage (uint8 worldImage) =0;

	/**
	  * Set the global position of the move primitive. Setting the global position
	  * can take a long time if you use a UGlobalRetriever. Set the position with
	  * this method only the first time or for teleporting.
	  *
	  * If you modify a noncollisionable primitive with this method, you must evaluate in the world
	  * image where you have modified it before modify any other dynamic properties in another world image.
	  *
	  * \param pos is the new global position of the primitive.
	  */
	virtual void	setGlobalPosition (const NLMISC::CVectorD& pos, uint8 worldImage, UGlobalPosition::TType type = UGlobalPosition::Unspecified) =0;

	/**
	  * Set the global position of the move primitive. This method is fast because
	  * you must pass the global position of the primitive.
	  *
	  * If you modify a noncollisionable primitive with this method, you must evaluate in the world
	  * image where you have modified it before modify any other dynamic properties in another world image.
	  *
	  * \param pos is the new global position of the primitive.
	  */
	virtual void	setGlobalPosition (const UGlobalPosition& pos, uint8 worldImage) =0;

	/**
	  * Move the primitive.
	  * This method is fast. Use it to move primitives.
	  *
	  * If you modify a noncollisionable primitive with this method, you must evaluate in the world
	  * image where you have modified it before modify any other dynamic properties in another world image.
	  *
	  * \param speed is the speed of the primitive.
	  */
	virtual void	move (const NLMISC::CVectorD& speed, uint8 worldImage) =0;

	/**
	  * Return the first world image
	  */
	virtual uint8				getFirstWorldImageV () const =0;

	/**
	  * Return the num of world image
	  */
	virtual uint8				getNumWorldImageV () const =0;
};


} // NLPACS


#endif // NL_U_MOVE_PRIMITIVE_H

/* End of u_move_primitive.h */

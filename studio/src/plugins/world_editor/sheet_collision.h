// Ryzom Core Studio - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef SHEET_COLLISION_H
#define SHEET_COLLISION_H

// NeL includes
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_class.h>

// Qt includes
#include <QHash>
#include <QString>

namespace NLGEORGES
{
class UFormLoader;
}

namespace WorldEditor
{

/**
@struct CollisionShape
@brief The footprint a sheet occupies in the XY plane.
@details PACS knows two shapes for a movable entity, and the sheet decides which one:
	a collision radius makes it a cylinder, otherwise length and width make an oriented
	box. Sizes are metres, the same unit the primitive coordinates use.
*/
struct CollisionShape
{
	enum Kind
	{
		None,
		Circle,
		Box
	};

	CollisionShape() : Kind_(None), Radius(0.0f), Length(0.0f), Width(0.0f) {}

	bool isValid() const { return Kind_ != None; }

	Kind  Kind_;
	float Radius;   ///< Circle: radius.
	float Length;   ///< Box: extent along the direction the entity faces.
	float Width;    ///< Box: extent across it.
};

/**
@class SheetCollision
@brief Reads the collision footprint of the sheet a primitive points at.
@details Which property holds the sheet name is not hard coded: the primitive class in
	world_editor_classes.xml marks those parameters with a FILE_EXTENSION, and the value
	stored in the primitive carries no extension, so the two are combined here.

	Two sheet families declare a footprint, and both are read the way the game reads them:

	* .creature - Collision.CollisionRadius, Collision.Length, Collision.Width
	  (see ryzom/client/src/entity_cl.cpp: a radius greater than zero wins and gives a
	  cylinder, otherwise a length greater than zero gives a box)
	* .plant    - 3D.Collision Radius, always a circle

	Results are cached per sheet name, including the misses, so a city full of NPCs
	sharing a handful of sheets costs a handful of Georges loads.
*/
class SheetCollision
{
public:
	/// Return the shared instance.
	static SheetCollision &instance();

	/// Footprint of the sheet this primitive names, invalid if there is none.
	CollisionShape shapeOf(const NLLIGO::IPrimitive *primitive,
						   const NLLIGO::CPrimitiveClass *primitiveClass) const;

private:
	SheetCollision();
	~SheetCollision();

	/// Load one sheet and read its collision fields, cached.
	CollisionShape shapeOfSheet(const QString &sheetName) const;

	NLGEORGES::UFormLoader *m_formLoader;
	mutable QHash<QString, CollisionShape> m_cache;
};

} /* namespace WorldEditor */

#endif // SHEET_COLLISION_H

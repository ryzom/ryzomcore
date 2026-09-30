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

// Project includes
#include "sheet_collision.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/georges/u_form.h>
#include <nel/georges/u_form_elm.h>
#include <nel/georges/u_form_loader.h>

namespace WorldEditor
{

SheetCollision &SheetCollision::instance()
{
	static SheetCollision collision;
	return collision;
}

SheetCollision::SheetCollision()
	: m_formLoader(NLGEORGES::UFormLoader::createLoader())
{
}

SheetCollision::~SheetCollision()
{
	if (m_formLoader != NULL)
		NLGEORGES::UFormLoader::releaseLoader(m_formLoader);
}

CollisionShape SheetCollision::shapeOfSheet(const QString &sheetName) const
{
	const QHash<QString, CollisionShape>::const_iterator cached = m_cache.constFind(sheetName);
	if (cached != m_cache.constEnd())
		return cached.value();

	CollisionShape shape;

	// A sheet that cannot be loaded is cached as "no shape" as well, so a missing file is
	// looked up - and warned about - only once.
	NLGEORGES::UForm *form = m_formLoader->loadForm(sheetName.toUtf8().constData());
	if (form == NULL)
	{
		nlwarning("World Editor: sheet '%s' not found, no collision shape drawn.",
				  sheetName.toUtf8().constData());
		m_cache.insert(sheetName, shape);
		return shape;
	}

	const NLGEORGES::UFormElm &root = form->getRootNode();
	float radius = 0.0f, length = 0.0f, width = 0.0f;

	if (sheetName.endsWith(QLatin1String(".plant"), Qt::CaseInsensitive))
	{
		// Plants are static and always round.
		root.getValueByName(radius, "3D.Collision Radius");
	}
	else
	{
		root.getValueByName(radius, "Collision.CollisionRadius");
		root.getValueByName(length, "Collision.Length");
		root.getValueByName(width, "Collision.Width");
	}

	// Same order of preference as the client, so what is drawn is what PACS uses.
	if (radius > 0.0f)
	{
		shape.Kind_ = CollisionShape::Circle;
		shape.Radius = radius;
	}
	else if (length > 0.0f)
	{
		shape.Kind_ = CollisionShape::Box;
		shape.Length = length;
		shape.Width = (width > 0.0f) ? width : length;
	}

	m_cache.insert(sheetName, shape);
	return shape;
}

CollisionShape SheetCollision::shapeOf(const NLLIGO::IPrimitive *primitive,
									   const NLLIGO::CPrimitiveClass *primitiveClass) const
{
	if ((primitive == NULL) || (primitiveClass == NULL) || (m_formLoader == NULL))
		return CollisionShape();

	// Which property carries a sheet name is declared by the class, not by us: the
	// parameter is marked with a FILE_EXTENSION in world_editor_classes.xml. Only the two
	// sheet families that describe a physical extent are of interest here.
	for (uint i = 0; i < primitiveClass->Parameters.size(); ++i)
	{
		const NLLIGO::CPrimitiveClass::CParameter &parameter = primitiveClass->Parameters[i];
		const std::string &extension = parameter.FileExtension;
		if ((extension != "creature") && (extension != "plant"))
			continue;

		std::string sheet;
		if (!primitive->getPropertyByName(parameter.Name.c_str(), sheet) || sheet.empty())
			continue;

		// The primitive stores the bare name, the file has the extension.
		const QString sheetName =
				QString::fromUtf8(sheet.c_str()) + QLatin1Char('.') +
				QString::fromUtf8(extension.c_str());

		const CollisionShape shape = shapeOfSheet(sheetName);
		if (shape.isValid())
			return shape;
	}

	return CollisionShape();
}

} /* namespace WorldEditor */

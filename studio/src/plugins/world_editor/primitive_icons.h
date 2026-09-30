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

#ifndef PRIMITIVE_ICONS_H
#define PRIMITIVE_ICONS_H

// NeL includes
#include <nel/ligo/primitive.h>

// Qt includes
#include <QHash>
#include <QIcon>
#include <QString>

namespace WorldEditor
{

/**
@class PrimitiveIcons
@brief Supplies the tree icons for primitives, the way the MFC world editor does.
@details The original editor distinguishes primitives in two steps (see
	ryzom/tools/leveldesign/world_editor/world_editor/tools_logic.cpp, updateItem()):

	1. The "class" property is looked up in an image list that was filled from every
	   *.ico file below a directory named "ui". A class may supply up to three icons,
	   named <class>_closed, <class>_opened and <class>_hidden, or a single <class>.
	2. Classes without an icon fall back to the primitive kind - root, folder, property,
	   point, line or zone - which the original ships as embedded resources.

	The fallback icons are compiled into the plugin, so structure is always visible. The
	per-class icons stay data: they live next to the level design data and are looked up
	at run time, which keeps them editable and lets a modified data set supply its own.
*/
class PrimitiveIcons
{
public:
	/// Return the shared instance. The first call resolves the icon directory.
	static PrimitiveIcons &instance();

	/// Return the icon for a primitive. Never returns a null icon.
	QIcon icon(const NLLIGO::IPrimitive *primitive, bool hidden = false) const;

	/// Icon of a primitive class as declared in world_editor_classes.xml.
	/// Returns a null icon when the class ships none - callers decide on a fallback.
	QIcon iconForClass(const QString &className, bool hidden = false) const;

	/// Directory the per-class icons were taken from, empty if none was found.
	QString iconDirectory() const;

private:
	PrimitiveIcons();

	/// Resolve the directory holding the per-class icons, see the .cpp for the order.
	static QString resolveIconDirectory();

	/// Icon derived from the primitive kind. Always yields an icon.
	QIcon kindIcon(const NLLIGO::IPrimitive *primitive, bool hidden) const;

	/// Load <baseName>.ico from the icon directory, cached. Null if absent.
	QIcon loadClassIcon(const QString &baseName) const;

	/// Load a compiled-in fallback icon, cached.
	QIcon loadBuiltinIcon(const QString &baseName) const;

	QString m_iconDirectory;

	/// Lower case base name -> real file name, built once for the icon directory.
	/// Needed because the class names are lower case while the files are not always.
	QHash<QString, QString> m_iconFiles;

	mutable QHash<QString, QIcon> m_cache;
};

} /* namespace WorldEditor */

#endif // PRIMITIVE_ICONS_H

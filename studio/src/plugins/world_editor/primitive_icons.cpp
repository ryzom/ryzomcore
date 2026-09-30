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
#include "primitive_icons.h"
#include "world_editor_constants.h"

#include "../core/icore.h"

// NeL includes
#include <nel/misc/path.h>
#include <nel/misc/debug.h>

// Qt includes
#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace WorldEditor
{

namespace
{
/// A file that exists in every icon set, used to recognise the directory.
const char *const PROBE_ICON = "npc_group.ico";
}

PrimitiveIcons &PrimitiveIcons::instance()
{
	static PrimitiveIcons icons;
	return icons;
}

PrimitiveIcons::PrimitiveIcons()
	: m_iconDirectory(resolveIconDirectory())
{
	if (m_iconDirectory.isEmpty())
	{
		nlwarning("World Editor: no primitive icon directory found, falling back to the "
		          "built-in icons only. Set [%s] %s to the directory holding the *.ico "
		          "files of the original editor.",
		          Constants::WORLD_EDITOR_SECTION, Constants::ICON_PATH);
		return;
	}

	// Build the lower case index once. The class names in world_editor_classes.xml are
	// lower case, the icon files are not consistently so (this only ever mattered once
	// the editor left Windows, where the file system did the folding).
	const QDir dir(m_iconDirectory);
	Q_FOREACH (const QFileInfo &info, dir.entryInfoList(QStringList("*.ico"), QDir::Files))
		m_iconFiles.insert(info.completeBaseName().toLower(), info.absoluteFilePath());

	nlinfo("World Editor: %d primitive icons found in '%s'.",
	       m_iconFiles.size(), m_iconDirectory.toUtf8().constData());
}

QString PrimitiveIcons::resolveIconDirectory()
{
	// 1. An explicitly configured directory always wins.
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::WORLD_EDITOR_SECTION);
	const QString configured = settings->value(Constants::ICON_PATH).toString();
	settings->endGroup();

	if (!configured.isEmpty())
	{
		if (QDir(configured).exists())
			return configured;

		nlwarning("World Editor: [%s] %s points to '%s', which does not exist.",
		          Constants::WORLD_EDITOR_SECTION, Constants::ICON_PATH,
		          configured.toUtf8().constData());
	}

	// 2. Otherwise let NeL's search paths find one of the icons and take its directory.
	//    This is the equivalent of the original CPath::getPathContent("ui", ...) call,
	//    which relied on the editor's working directory instead.
	const std::string found = NLMISC::CPath::lookup(PROBE_ICON, false, false, true);
	if (!found.empty())
		return QFileInfo(QString::fromUtf8(found.c_str())).absolutePath();

	// 3. Finally the two directory names the original used, relative to the program.
	const char *const legacyDirs[] = { "ui", "old_ico" };
	for (size_t i = 0; i < sizeof(legacyDirs) / sizeof(legacyDirs[0]); ++i)
	{
		const QDir dir(QString::fromLatin1(legacyDirs[i]));
		if (dir.exists(QString::fromLatin1(PROBE_ICON)))
			return dir.absolutePath();
	}

	return QString();
}

QString PrimitiveIcons::iconDirectory() const
{
	return m_iconDirectory;
}

QIcon PrimitiveIcons::loadClassIcon(const QString &baseName) const
{
	const QHash<QString, QIcon>::const_iterator cached = m_cache.constFind(baseName);
	if (cached != m_cache.constEnd())
		return cached.value();

	// A null icon is cached as well, so a missing file is looked up only once.
	const QString fileName = m_iconFiles.value(baseName.toLower());
	const QIcon icon = fileName.isEmpty() ? QIcon() : QIcon(fileName);
	m_cache.insert(baseName, icon);
	return icon;
}

QIcon PrimitiveIcons::loadBuiltinIcon(const QString &baseName) const
{
	const QString key = QString("@builtin/") + baseName;
	const QHash<QString, QIcon>::const_iterator cached = m_cache.constFind(key);
	if (cached != m_cache.constEnd())
		return cached.value();

	const QIcon icon(QString(Constants::ICON_TREE_PREFIX) + baseName + ".png");
	m_cache.insert(key, icon);
	return icon;
}

QIcon PrimitiveIcons::iconForClass(const QString &className, bool hidden) const
{
	if (className.isEmpty() || m_iconFiles.isEmpty())
		return QIcon();

	// The original picks a closed/opened pair and shows the closed one on a collapsed
	// item. A QIcon carries no expanded state, so the closed variant is what we want;
	// the single-icon form and the opened variant only serve as fallbacks.
	QStringList candidates;
	if (hidden)
		candidates << className + "_hidden";
	candidates << className + "_closed" << className << className + "_opened";

	Q_FOREACH (const QString &candidate, candidates)
	{
		const QIcon icon = loadClassIcon(candidate);
		if (!icon.isNull())
			return icon;
	}

	return QIcon();
}

QIcon PrimitiveIcons::kindIcon(const NLLIGO::IPrimitive *primitive, bool hidden) const
{
	const QString suffix = hidden ? "_hidden" : "_closed";

	// The root of a primitive file has no parent.
	if (primitive->getParent() == NULL)
		return loadBuiltinIcon("root" + suffix);

	if (dynamic_cast<const NLLIGO::CPrimPoint *>(primitive) != NULL)
		return loadBuiltinIcon("point" + suffix);

	// A path is drawn as a line in the original, and its icon is named accordingly.
	if (dynamic_cast<const NLLIGO::CPrimPath *>(primitive) != NULL)
		return loadBuiltinIcon("line" + suffix);

	if (dynamic_cast<const NLLIGO::CPrimZone *>(primitive) != NULL)
		return loadBuiltinIcon("zone" + suffix);

	// Everything else is a plain node: a folder when it groups something, a single
	// property when it does not.
	if (primitive->getNumChildren() != 0)
		return loadBuiltinIcon("folder" + suffix);

	return loadBuiltinIcon("property" + suffix);
}

QIcon PrimitiveIcons::icon(const NLLIGO::IPrimitive *primitive, bool hidden) const
{
	nlassert(primitive != NULL);

	std::string className;
	if (primitive->getPropertyByName("class", className))
	{
		const QIcon icon = iconForClass(QString::fromUtf8(className.c_str()), hidden);
		if (!icon.isNull())
			return icon;
	}

	return kindIcon(primitive, hidden);
}

} /* namespace WorldEditor */

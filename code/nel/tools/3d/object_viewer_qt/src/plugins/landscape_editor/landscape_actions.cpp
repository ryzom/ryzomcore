// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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
#include "landscape_actions.h"
#include "builder_zone.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes

namespace LandscapeEditor
{

ActionLigoTile::ActionLigoTile(const LigoData &data, ZoneBuilder *zoneBuilder, QGraphicsScene *scene, QUndoCommand *parent)
	: QUndoCommand(parent),
	  m_item(0),
	  m_zoneBuilder(zoneBuilder),
	  m_scene(scene)
{
	m_ligoData = data;
}

ActionLigoTile::~ActionLigoTile()
{
}

void ActionLigoTile::undo()
{
	m_scene->removeItem(m_item);
	delete m_item;
	m_item = 0;
}

void ActionLigoTile::redo()
{
	QPixmap *pixmap = m_zoneBuilder->pixmapDatabase()->pixmap(QString(m_ligoData.ZoneName.c_str()));
	m_item = new QGraphicsPixmapItem(*pixmap, 0, m_scene);
	m_item->setPos(m_ligoData.PosX, m_ligoData.PosY);
	m_item->setScale(m_ligoData.Scale);
	setText(QObject::tr("Add tile(%1, %2)").arg(m_ligoData.PosX).arg(m_ligoData.PosY));
}

} /* namespace LandscapeEditor */

// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef WORLD_EDITOR_SCENE_H
#define WORLD_EDITOR_SCENE_H

// Project includes
#include "world_editor_global.h"

#include "../landscape_editor/landscape_scene_base.h"

// NeL includes

// Qt includes

namespace WorldEditor
{

/*
@class WorldEditorScene
@brief
@details
*/
class WORLD_EDITOR_EXPORT WorldEditorScene : public LandscapeEditor::LandscapeSceneBase
{
	Q_OBJECT

public:
	WorldEditorScene(int sizeCell = 160, QObject *parent = 0);
	virtual ~WorldEditorScene();

public Q_SLOTS:

protected:
//	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
//	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
//	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

private:
};

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_SCENE_H

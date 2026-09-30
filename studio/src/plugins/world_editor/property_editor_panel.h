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

#ifndef PROPERTY_EDITOR_PANEL_H
#define PROPERTY_EDITOR_PANEL_H

// Project includes
#include "primitive_item.h"

// Qt includes
#include <QList>
#include <QWidget>

class QLabel;
class QSplitter;
class QUndoStack;

namespace WorldEditor
{
class PrimitivePropertyForm;
class PrimitivesTreeModel;
class WorldEditorScene;

/**
@class PropertyEditorPanel
@brief Shows one property editor per selected primitive, side by side.
@details Merging several primitives into a single list and writing "<different values>"
	wherever they disagree tells you that they differ but not what either of them holds,
	which is no use when the point is to compare two NPCs. So each selected primitive
	keeps its own editor, complete with its combo boxes, check boxes and the multi line
	editor for script blocks, and they sit next to each other in a splitter.

	The number of editors is capped: selecting a whole group would otherwise build a
	hundred of them, and a hundred columns compare nothing.
*/
class PropertyEditorPanel : public QWidget
{
	Q_OBJECT

public:
	explicit PropertyEditorPanel(QWidget *parent = 0);

	/// Largest number of editors shown at once.
	static const int MAX_EDITORS = 6;

	/// Makes property changes undoable, see PrimitivePropertyForm::setContext().
	void setContext(QUndoStack *undoStack, PrimitivesTreeModel *model, WorldEditorScene *scene);

public Q_SLOTS:
	void clearProperties();
	void updateSelection(Node *node);
	void updateSelection(const NodeList &nodes);

private Q_SLOTS:
	/// A property changed from outside the form showing it - undo, redo, the map.
	void refreshNode(Node *node);

private:
	/// Grow or shrink the row of editors to count, reusing the ones already there.
	void setEditorCount(int count);

	QSplitter *m_splitter;
	QLabel *m_note;
	QList<PrimitivePropertyForm *> m_editors;
	QUndoStack *m_undoStack;
	PrimitivesTreeModel *m_model;
	WorldEditorScene *m_scene;
};

} /* namespace WorldEditor */

#endif // PROPERTY_EDITOR_PANEL_H

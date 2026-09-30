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

#ifndef NAVIGATION_TOOLS_H
#define NAVIGATION_TOOLS_H

// Qt includes
#include <QtCore/QPersistentModelIndex>
#include <QtCore/QPointF>
#include <QtWidgets/QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;

namespace WorldEditor
{
class PrimitivesTreeModel;

namespace Navigation
{

/// World coordinates (as in the .primitive files and in game) and scene coordinates
/// differ in the sign of y and by one cell, see LoadRootPrimitiveCommand.
QPointF worldToScene(const QPointF &world);
QPointF sceneToWorld(const QPointF &scene);

/// Read "x y", "x, y", "x;y" or "x/y" - whatever gets pasted from a chat line, a
/// script or the status bar. Returns false unless exactly two numbers are found.
bool parseWorldPosition(const QString &text, QPointF &world);

/// Text for a world position, the form parseWorldPosition() reads back.
QString formatWorldPosition(const QPointF &world);

} // namespace Navigation

/**
@class PositionDialog
@brief Two coordinate fields that also take a pasted "x y" pair.
@details Used for "Go to" (with the zoom option) and for reading or setting the
position of a primitive or a group.
*/
class PositionDialog: public QDialog
{
	Q_OBJECT

public:
	enum Mode
	{
		GotoMode,
		MoveMode
	};

	PositionDialog(Mode mode, const QPointF &world, const QString &subject, QWidget *parent = 0);

	QPointF position() const;
	bool zoomIn() const;

private Q_SLOTS:
	void copyPosition();
	void splitPastedText(const QString &text);

private:
	QDoubleSpinBox *m_x;
	QDoubleSpinBox *m_y;
	QLineEdit *m_paste;
	QCheckBox *m_zoom;
};

/**
@class FindPrimitiveDialog
@brief Search the loaded primitives by name, class and property values.
@details The MFC editor had "Find" for a single property and value. This looks in the
name, the class and every string property at once unless a property is named, and lists
all hits instead of stepping through them.
*/
class FindPrimitiveDialog: public QDialog
{
	Q_OBJECT

public:
	FindPrimitiveDialog(PrimitivesTreeModel *model, QWidget *parent = 0);

	/// Put the cursor in the search field, text selected, ready for typing.
	void activate();

Q_SIGNALS:
	/// A hit was double clicked or picked with Enter.
	void primitiveActivated(const QModelIndex &index);

private Q_SLOTS:
	void search();
	void activateItem(QTreeWidgetItem *item);

private:
	void searchBelow(const QModelIndex &parent, const QString &property, const QString &text, int &hits);

	PrimitivesTreeModel *m_model;
	QLineEdit *m_text;
	QComboBox *m_property;
	QTreeWidget *m_results;
	QLabel *m_summary;
};

} /* namespace WorldEditor */

#endif // NAVIGATION_TOOLS_H

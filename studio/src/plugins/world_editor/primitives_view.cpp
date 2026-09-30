// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
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
#include "primitives_view.h"
#include "world_editor_misc.h"
#include "primitive_icons.h"
#include "primitives_model.h"
#include "world_editor_actions.h"
#include "world_editor_scene_item.h"
#include "world_editor_scene.h"
#include "world_editor_constants.h"

#include "../core/core_constants.h"
#include "../landscape_editor/landscape_editor_constants.h"
#include "../landscape_editor/builder_zone_base.h"
#include "../landscape_editor/zone_region_editor.h"
#include "../core/icore.h"

// NeL includes
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>
#include <nel/ligo/primitive_class.h>
#include <nel/ligo/primitive_utils.h>
#include <nel/misc/debug.h>

// Qt includes
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QStyledItemDelegate>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

namespace WorldEditor
{

namespace
{

/// Draws the eye at the right end of each primitive row, as in the layer list of a
/// paint program: struck through and coloured while the primitive is hidden, a faint
/// open eye under the mouse otherwise. A click on it toggles, without any menu.
class PrimitiveRowDelegate : public QStyledItemDelegate
{
public:
	PrimitiveRowDelegate(PrimitivesView *view)
		: QStyledItemDelegate(view),
		  m_view(view)
	{
	}

	static bool hasEye(const QModelIndex &index)
	{
		Node *node = static_cast<Node *>(index.internalPointer());
		return (node != 0) && ((node->type() == Node::PrimitiveNodeType) ||
							   (node->type() == Node::RootPrimitiveNodeType));
	}

	static QRect eyeRect(const QRect &row)
	{
		const int size = 14;
		return QRect(row.right() - size - 4, row.center().y() - size / 2, size, size);
	}

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
					   const QModelIndex &index) const
	{
		if (!hasEye(index))
		{
			QStyledItemDelegate::paint(painter, option, index);
			return;
		}

		// Keep the text clear of the eye.
		QStyleOptionViewItem textOption(option);
		textOption.rect.setRight(option.rect.right() - 22);
		QStyledItemDelegate::paint(painter, textOption, index);

		Node *node = static_cast<Node *>(index.internalPointer());
		const bool hidden = PrimitivesTreeModel::isHiddenNode(node);
		const bool hovered = (option.state & QStyle::State_MouseOver) != 0;
		if (!hidden && !hovered)
			return;

		QColor color = hidden ? QColor(210, 70, 45) : option.palette.color(QPalette::Text);
		if (!hidden)
			color.setAlphaF(0.45);
		painter->drawPixmap(eyeRect(option.rect), PrimitiveIcons::eyeGlyph(14, hidden, color));
	}

	virtual bool editorEvent(QEvent *event, QAbstractItemModel *model,
							 const QStyleOptionViewItem &option, const QModelIndex &index)
	{
		if (hasEye(index) &&
			((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::MouseButtonRelease) ||
			 (event->type() == QEvent::MouseButtonDblClick)))
		{
			QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
			if ((mouseEvent->button() == Qt::LeftButton) &&
				eyeRect(option.rect).adjusted(-3, -3, 3, 3).contains(mouseEvent->pos()))
			{
				// Act on the press and swallow the rest, so the click neither changes
				// the selection nor opens anything.
				if (event->type() == QEvent::MouseButtonPress)
					m_view->toggleHidden(index, (mouseEvent->modifiers() & Qt::ShiftModifier) != 0);
				return true;
			}
		}
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}

private:
	PrimitivesView *m_view;
};

} // anonymous namespace

PrimitivesView::PrimitivesView(QWidget *parent)
	: QTreeView(parent),
	  m_undoStack(0),
	  m_worldEditorScene(0),
	  m_zoneBuilder(0),
	  m_primitivesTreeModel(0)
{
	setContextMenuPolicy(Qt::DefaultContextMenu);

	setItemDelegate(new PrimitiveRowDelegate(this));
	// Hover state for the faint eye on visible rows.
	setMouseTracking(true);
	viewport()->setAttribute(Qt::WA_Hover, true);

	m_unloadAction = new QAction("Unload", this);

	m_saveAction = new QAction("Save", this);
	m_saveAction->setIcon(QIcon(Core::Constants::ICON_SAVE));

	m_saveAsAction = new QAction("Save As...", this);
	m_saveAsAction->setIcon(QIcon(Core::Constants::ICON_SAVE_AS));

	m_loadLandAction = new QAction("Load landscape file", this);
	m_loadLandAction->setIcon(QIcon(LandscapeEditor::Constants::ICON_ZONE_ITEM));

	m_loadPrimitiveAction = new QAction("Load primitive file", this);
	m_loadPrimitiveAction->setIcon(QIcon(Constants::ICON_ROOT_PRIMITIVE));

	m_newPrimitiveAction = new QAction("New primitive", this);

	m_deleteAction = new QAction("Delete", this);

	m_selectChildrenAction = new QAction("Select children", this);

	m_showInTreeAction = new QAction(tr("Show in Tree"), this);
	connect(m_showInTreeAction, SIGNAL(triggered()), this, SLOT(showInTree()));

	m_zoomToAction = new QAction(tr("Zoom in"), this);
	m_zoomToAction->setToolTip(tr("Move the map view onto this primitive"));
	connect(m_zoomToAction, SIGNAL(triggered()), this, SLOT(zoomToPrimitive()));

	m_helpAction = new QAction("Help", this);
	m_helpAction->setEnabled(false);

	// Hide a layer on the map to get at what lies below it - a continent, region or
	// stable zone covering the smaller zones inside. Editor state only, as in the MFC
	// editor; the tree keeps the row, greyed out with the hidden icon.
	m_showAction = new QAction(tr("Show on Map"), this);
	m_showAction->setShortcut(QKeySequence(tr("Ctrl+Shift+H")));
	m_showAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	addAction(m_showAction);

	m_positionAction = new QAction(tr("Position..."), this);
	m_positionAction->setToolTip(tr("Read, copy or set the position of the primitive or group"));
	connect(m_positionAction, SIGNAL(triggered()), this, SLOT(requestPosition()));

	// The whole branch at once: a continent, a region with everything in it, a file.
	m_hideWithChildrenAction = new QAction(tr("Hide with Children"), this);
	m_hideWithChildrenAction->setShortcut(QKeySequence(tr("Ctrl+Alt+H")));
	m_hideWithChildrenAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	m_hideWithChildrenAction->setToolTip(tr("Hide this and everything below it (Shift+click on the eye)"));
	connect(m_hideWithChildrenAction, SIGNAL(triggered()), this, SLOT(hidePrimitiveWithChildren()));
	addAction(m_hideWithChildrenAction);

	m_showWithChildrenAction = new QAction(tr("Show with Children"), this);
	m_showWithChildrenAction->setShortcut(QKeySequence(tr("Ctrl+Alt+Shift+H")));
	m_showWithChildrenAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(m_showWithChildrenAction, SIGNAL(triggered()), this, SLOT(showPrimitiveWithChildren()));
	addAction(m_showWithChildrenAction);

	m_showAllAction = new QAction(tr("Show All Hidden"), this);
	connect(m_showAllAction, SIGNAL(triggered()), this, SLOT(showAllPrimitives()));

	m_hideAction = new QAction(tr("Hide on Map"), this);
	m_hideAction->setShortcut(QKeySequence(tr("Ctrl+H")));
	m_hideAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	addAction(m_hideAction);

	connect(m_loadLandAction, SIGNAL(triggered()), this, SLOT(loadLandscape()));
	connect(m_loadPrimitiveAction, SIGNAL(triggered()), this, SLOT(loadRootPrimitive()));
	connect(m_newPrimitiveAction, SIGNAL(triggered()), this, SLOT(createRootPrimitive()));
	connect(m_selectChildrenAction, SIGNAL(triggered()), this, SLOT(selectChildren()));
	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deletePrimitives()));
	connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));
	connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
	connect(m_unloadAction, SIGNAL(triggered()), this, SLOT(unload()));
	connect(m_showAction, SIGNAL(triggered()), this, SLOT(showPrimitive()));
	connect(m_hideAction, SIGNAL(triggered()), this, SLOT(hidePrimitive()));

#ifdef Q_OS_DARWIN
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
#endif
}

PrimitivesView::~PrimitivesView()
{
}

void PrimitivesView::setUndoStack(QUndoStack *undoStack)
{
	m_undoStack = undoStack;
}

void PrimitivesView::setZoneBuilder(LandscapeEditor::ZoneBuilderBase *zoneBuilder)
{
	m_zoneBuilder = zoneBuilder;
}

void PrimitivesView::setWorldScene(WorldEditorScene *worldEditorScene)
{
	m_worldEditorScene = worldEditorScene;
}

void PrimitivesView::setModel(PrimitivesTreeModel *model)
{
	QTreeView::setModel(model);
	m_primitivesTreeModel = model;
}

void PrimitivesView::loadRootPrimitive()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo primitive file"),
							Utils::lastDirectory(Constants::LAST_PRIMITIVE_DIR),
							tr("All NeL Ligo primitive files (*.primitive)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		if (fileNames.count() > 1)
			m_undoStack->beginMacro(tr("Load primitive files"));

		Q_FOREACH(QString fileName, fileNames)
		{
			Utils::setLastDirectory(Constants::LAST_PRIMITIVE_DIR, fileName);
			m_undoStack->push(new LoadRootPrimitiveCommand(fileName, m_worldEditorScene, m_primitivesTreeModel, this));
		}

		if (fileNames.count() > 1)
			m_undoStack->endMacro();
	}
	setCursor(Qt::ArrowCursor);
}

void PrimitivesView::loadLandscape()
{
	nlassert(m_undoStack);
	nlassert(m_zoneBuilder);
	nlassert(m_primitivesTreeModel);

	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo land file"),
							Utils::lastDirectory(Constants::LAST_LAND_DIR),
							tr("All NeL Ligo land files (*.land)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		// The zone bank has to be the one of the continent being opened. At start-up the
		// builder holds the configured default (LandscapeDataDirectory), and a .land
		// opened on its own never changed it - nexus.land drawn against the fyros bank
		// finds none of its zones and shows nothing but placeholders. By convention the
		// bank sits next to the .land (zoneligos/ and zonebitmaps/); a .land without one
		// keeps the bank already loaded.
		const QDir landDir = QFileInfo(fileNames.first()).absoluteDir();
		if (landDir.exists("zoneligos") && (landDir.absolutePath() != QDir(m_zoneBuilder->dataPath()).absolutePath()))
		{
			if (m_zoneBuilder->init(landDir.absolutePath(), true))
				nlinfo("World editor: zone bank switched to '%s'", landDir.absolutePath().toUtf8().constData());
		}

		if (fileNames.count() > 1)
			m_undoStack->beginMacro(tr("Load land files"));

		// Frame what was loaded now, not every region of the session: with lands of
		// several continents open, their union is a view in which nothing can be seen.
		QRectF loadedBounds;
		QStringList emptyFiles;
		QStringList failedFiles;
		Q_FOREACH(QString fileName, fileNames)
		{
			Utils::setLastDirectory(Constants::LAST_LAND_DIR, fileName);

			const QList<int> idsBefore = m_zoneBuilder->zoneRegionIds();
			m_undoStack->push(new LoadLandscapeCommand(fileName, m_primitivesTreeModel, m_zoneBuilder));

			bool loaded = false;
			Q_FOREACH (int id, m_zoneBuilder->zoneRegionIds())
			{
				if (idsBefore.contains(id) || (m_zoneBuilder->zoneRegion(id) == 0))
					continue;
				loaded = true;
				const QRectF rect = Utils::zoneRegionSceneRect(m_zoneBuilder->zoneRegion(id)->ligoZoneRegion());
				if (rect.isNull())
					emptyFiles << QFileInfo(fileName).fileName();
				else
					loadedBounds = loadedBounds.isNull() ? rect : loadedBounds.united(rect);
			}
			if (!loaded)
				failedFiles << QFileInfo(fileName).fileName();
		}

		if (fileNames.count() > 1)
			m_undoStack->endMacro();

		if (!loadedBounds.isNull())
			Q_EMIT zoomToRectRequested(loadedBounds);

		// An empty .land loads without complaint and simply shows nothing, which reads
		// like a broken view. Say so.
		QStringList notes;
		if (!emptyFiles.isEmpty())
			notes << tr("%1: empty, contains no zones").arg(emptyFiles.join(", "));
		if (!failedFiles.isEmpty())
			notes << tr("%1: could not be loaded").arg(failedFiles.join(", "));
		if (!notes.isEmpty())
		{
			Core::ICore::instance()->mainWindow()->statusBar()->showMessage(notes.join("  -  "), 10000);
			nlwarning("World editor: %s", notes.join("; ").toUtf8().constData());
		}

		Q_EMIT landscapeLoaded();
	}
	setCursor(Qt::ArrowCursor);
}

void PrimitivesView::createRootPrimitive()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	m_undoStack->push(new CreateRootPrimitiveCommand("NewPrimitive", m_primitivesTreeModel));
}

void PrimitivesView::selectChildren()
{
	QModelIndexList indexList = selectionModel()->selectedRows();
	QModelIndex parentIndex = indexList.first();

	selectionModel()->clearSelection();

	QItemSelection itemSelection;
	selectChildren(parentIndex, itemSelection);
	selectionModel()->select(itemSelection, QItemSelectionModel::Select);
}

void PrimitivesView::save()
{
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();
	QModelIndex index = indexList.first();

	RootPrimitiveNode *node = static_cast<RootPrimitiveNode *>(index.internalPointer());

	if (node->data(Constants::PRIMITIVE_FILE_IS_CREATED).toBool())
	{
		if (!NLLIGO::saveXmlPrimitiveFile(*node->primitives(), node->fileName().toUtf8().constData()))
			QMessageBox::warning(this, "World Editor Qt", tr("Error writing output file: %1").arg(node->fileName()));
		else
			node->setData(Constants::PRIMITIVE_IS_MODIFIED, false);
	}
	else
		saveAs();
}

void PrimitivesView::saveAs()
{
	nlassert(m_primitivesTreeModel);

	QString fileName = QFileDialog::getSaveFileName(this,
					   tr("Save NeL Ligo primitive file"),
					   Utils::lastDirectory(Constants::LAST_PRIMITIVE_DIR),
					   tr("NeL Ligo primitive file (*.primitive)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		Utils::setLastDirectory(Constants::LAST_PRIMITIVE_DIR, fileName);
		QModelIndexList indexList = selectionModel()->selectedRows();
		QModelIndex index = indexList.first();

		RootPrimitiveNode *node = static_cast<RootPrimitiveNode *>(index.internalPointer());

		if (!NLLIGO::saveXmlPrimitiveFile(*node->primitives(), fileName.toUtf8().constData()))
			QMessageBox::warning(this, "World Editor Qt", tr("Error writing output file: %1").arg(fileName));
		else
		{
			node->setFileName(fileName);
			node->setData(Constants::PRIMITIVE_FILE_IS_CREATED, true);
			node->setData(Constants::PRIMITIVE_IS_MODIFIED, false);
		}
	}
	setCursor(Qt::ArrowCursor);
}

void PrimitivesView::deletePrimitives()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();

	QModelIndex index = indexList.first();

	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());

	if (node->primitiveClass()->Deletable)
		m_undoStack->push(new DeletePrimitiveCommand(index, m_primitivesTreeModel, m_worldEditorScene, this));

}

void PrimitivesView::unload()
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();
	QModelIndex index = indexList.first();
	Node *node = static_cast<Node *>(index.internalPointer());
	switch (node->type())
	{
	case Node::WorldEditNodeType:
	{
		break;
	}
	case Node::LandscapeNodeType:
	{
		m_undoStack->push(new UnloadLandscapeCommand(index, m_primitivesTreeModel, m_zoneBuilder));
		break;
	}
	case Node::RootPrimitiveNodeType:
	{
		m_undoStack->push(new UnloadRootPrimitiveCommand(index, m_worldEditorScene, m_primitivesTreeModel, this));
		break;
	}
	}
}

QModelIndexList PrimitivesView::hideTargets() const
{
	// The selected rows; a right click on the map may land on a primitive outside the
	// selection, then just that one.
	QModelIndexList targets = selectionModel()->selectedRows();
	if (m_contextMenuIndex.isValid())
	{
		const QModelIndex clicked = m_contextMenuIndex.operator const QModelIndex &();
		if (!targets.contains(clicked))
			targets = QModelIndexList() << clicked;
	}
	return targets;
}

void PrimitivesView::setHidden(const QModelIndex &index, bool hidden, QItemSelection &deselect,
							   bool withChildren)
{
	Node *node = static_cast<Node *>(index.internalPointer());
	if ((node == 0) || ((node->type() != Node::PrimitiveNodeType) && (node->type() != Node::RootPrimitiveNodeType)))
		return;

	m_primitivesTreeModel->setPrimitiveHidden(index, hidden);
	if (hidden)
		deselect.select(index, index);

	QGraphicsItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
	if (item != 0)
	{
		m_worldEditorScene->updateItemVisibility(item);
		// Plain Hide stops here: the children are the smaller zones one hides a region
		// to get at. "with Children" takes the whole branch - a continent, a file.
		if (!withChildren)
			return;
	}

	// A folder or group has no shape of its own; hiding it means hiding what is in it.
	const int count = model()->rowCount(index);
	for (int i = 0; i < count; ++i)
		setHidden(model()->index(i, 0, index), hidden, deselect, withChildren);
}

void PrimitivesView::setHiddenOnTargets(bool hidden, bool withChildren)
{
	if (m_worldEditorScene == 0)
		return;

	QItemSelection deselect;
	Q_FOREACH (const QModelIndex &index, hideTargets())
		setHidden(index, hidden, deselect, withChildren);

	// A hidden primitive stays selected otherwise, and a drag on the map would move it
	// without it being seen.
	if (!deselect.isEmpty())
		selectionModel()->select(deselect, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
	m_contextMenuIndex = QPersistentModelIndex();
}

void PrimitivesView::requestPosition()
{
	Q_EMIT positionRequested(hideTargets());
}

void PrimitivesView::focusIndex(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
	scrollTo(index, QAbstractItemView::PositionAtCenter);

	const QRectF bounds = sceneRectOfSubtree(index);
	if (!bounds.isNull())
		Q_EMIT zoomToRectRequested(bounds);
}

void PrimitivesView::toggleHidden(const QModelIndex &index, bool withChildren)
{
	if (m_worldEditorScene == 0)
		return;

	Node *node = static_cast<Node *>(index.internalPointer());
	if (node == 0)
		return;

	QItemSelection deselect;
	setHidden(index, !PrimitivesTreeModel::isHiddenNode(node), deselect, withChildren);
	if (!deselect.isEmpty())
		selectionModel()->select(deselect, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}

void PrimitivesView::showPrimitive()
{
	setHiddenOnTargets(false, false);
}

void PrimitivesView::hidePrimitive()
{
	setHiddenOnTargets(true, false);
}

void PrimitivesView::showPrimitiveWithChildren()
{
	setHiddenOnTargets(false, true);
}

void PrimitivesView::hidePrimitiveWithChildren()
{
	setHiddenOnTargets(true, true);
}

void PrimitivesView::showAllPrimitives(const QModelIndex &parent)
{
	const int count = model()->rowCount(parent);
	for (int i = 0; i < count; ++i)
	{
		const QModelIndex index = model()->index(i, 0, parent);
		Node *node = static_cast<Node *>(index.internalPointer());
		if ((node != 0) && PrimitivesTreeModel::isHiddenNode(node))
		{
			m_primitivesTreeModel->setPrimitiveHidden(index, false);
			QGraphicsItem *item = qvariant_cast<AbstractWorldItem *>(node->data(Constants::GRAPHICS_DATA_QT4_2D));
			if (item != 0)
				m_worldEditorScene->updateItemVisibility(item);
		}
		showAllPrimitives(index);
	}
}

void PrimitivesView::showAllPrimitives()
{
	if (m_worldEditorScene != 0)
		showAllPrimitives(QModelIndex());
}

void PrimitivesView::addHideActions(QMenu *menu, const QModelIndex &index)
{
	Node *node = static_cast<Node *>(index.internalPointer());
	const bool hidden = (node != 0) && PrimitivesTreeModel::isHiddenNode(node);
	menu->addAction(hidden ? m_showAction : m_hideAction);
	if (model()->rowCount(index) > 0)
	{
		menu->addAction(m_hideWithChildrenAction);
		menu->addAction(m_showWithChildrenAction);
	}
	menu->addAction(m_showAllAction);
}

void PrimitivesView::addNewPrimitiveByClass(int value)
{
	nlassert(m_undoStack);
	nlassert(m_primitivesTreeModel);

	QModelIndexList indexList = selectionModel()->selectedRows();

	PrimitiveNode *node = static_cast<PrimitiveNode *>(indexList.first().internalPointer());

	// Get class name
	QString className = node->primitiveClass()->DynamicChildren[value].ClassName.c_str();

	m_undoStack->push(new AddPrimitiveByClassCommand(className, m_primitivesTreeModel->pathFromIndex(indexList.first()),
					  m_worldEditorScene, m_primitivesTreeModel, this));
}

void PrimitivesView::generatePrimitives(int value)
{
}

void PrimitivesView::openItem(int value)
{
}

void PrimitivesView::revealIndex(const QModelIndex &index)
{
	if (!index.isValid())
		return;

	// scrollTo() does not open collapsed parents, so walk up and expand them first.
	QModelIndexList parents;
	for (QModelIndex parent = index.parent(); parent.isValid(); parent = parent.parent())
		parents.prepend(parent);

	Q_FOREACH (const QModelIndex &parent, parents)
		expand(parent);

	// NoUpdate: move the cursor without changing what is selected - the caller may just
	// have built a selection of several items.
	selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
	scrollTo(index, QAbstractItemView::PositionAtCenter);
}

void PrimitivesView::showInTree()
{
	revealIndex(m_contextMenuIndex);
}

QRectF PrimitivesView::sceneRectOfSubtree(const QModelIndex &index) const
{
	QRectF bounds;

	Node *node = static_cast<Node *>(index.internalPointer());
	if (node != 0)
	{
		AbstractWorldItem *item = qvariant_cast<AbstractWorldItem *>(
				node->data(Constants::GRAPHICS_DATA_QT4_2D));
		if (item != 0)
		{
			// worldRect() is the real extent in item coordinates, centred on the item's
			// own position - not boundingRect(), which is padded for painting.
			const QRectF itemRect = item->worldRect().translated(item->scenePos());
			bounds = bounds.isNull() ? itemRect : bounds.united(itemRect);
		}
	}

	// A group is a node primitive: it has no shape of its own, everything it stands for
	// hangs below it. Framing the whole subtree is what "zoom in on a group" means.
	const int count = model()->rowCount(index);
	for (int i = 0; i < count; ++i)
	{
		const QRectF childRect = sceneRectOfSubtree(model()->index(i, 0, index));
		if (childRect.isNull())
			continue;

		bounds = bounds.isNull() ? childRect : bounds.united(childRect);
	}

	return bounds;
}

QRectF PrimitivesView::landscapeSceneRect(const QModelIndex &index) const
{
	Node *node = static_cast<Node *>(index.internalPointer());
	if ((node == 0) || (node->type() != Node::LandscapeNodeType) || (m_zoneBuilder == 0))
		return QRectF();

	LandscapeEditor::ZoneRegionObject *region =
			m_zoneBuilder->zoneRegion(static_cast<LandscapeNode *>(node)->id());
	if (region == 0)
		return QRectF();
	return Utils::zoneRegionSceneRect(region->ligoZoneRegion());
}

void PrimitivesView::zoomToPrimitive()
{
	if (!m_contextMenuIndex.isValid())
		return;

	const QModelIndex index = m_contextMenuIndex.operator const QModelIndex &();
	Node *node = static_cast<Node *>(index.internalPointer());
	const QRectF bounds = ((node != 0) && (node->type() == Node::LandscapeNodeType))
						  ? landscapeSceneRect(index) : sceneRectOfSubtree(index);
	if (bounds.isNull())
		return;

	Q_EMIT zoomToRectRequested(bounds);
}

void PrimitivesView::showContextMenu(const QModelIndex &index, const QPoint &globalPos,
									 bool fromScene)
{
	if (!index.isValid())
		return;

	m_contextMenuIndex = index;

	QMenu menu(this);
	if (fromScene)
	{
		menu.addAction(m_showInTreeAction);
		menu.addSeparator();
	}

	Node *node = static_cast<Node *>(index.internalPointer());
	switch (node->type())
	{
	case Node::WorldEditNodeType:
		fillMenu_WorldEdit(&menu);
		break;
	case Node::RootPrimitiveNodeType:
		fillMenu_RootPrimitive(&menu, index);
		break;
	case Node::LandscapeNodeType:
		fillMenu_Landscape(&menu, index);
		break;
	case Node::PrimitiveNodeType:
		fillMenu_Primitive(&menu, index);
		break;
	};

	if (!menu.isEmpty())
		menu.exec(globalPos);

	// The menu's actions have run by now. Forget the row, or a later Ctrl+H in the tree
	// would act on it instead of on the selection.
	m_contextMenuIndex = QPersistentModelIndex();
}

void PrimitivesView::contextMenuEvent(QContextMenuEvent *event)
{
	QWidget::contextMenuEvent(event);
	QModelIndexList indexList = selectionModel()->selectedRows();

	// Several rows selected: only what works on all of them at once.
	if (indexList.size() == 1)
		showContextMenu(indexList.first(), event->globalPos());
	else if (indexList.size() > 1)
	{
		m_contextMenuIndex = QPersistentModelIndex();
		QMenu menu(this);
		menu.addAction(m_positionAction);
		menu.addSeparator();
		menu.addAction(m_hideAction);
		menu.addAction(m_showAction);
		menu.addAction(m_hideWithChildrenAction);
		menu.addAction(m_showWithChildrenAction);
		menu.addAction(m_showAllAction);
		menu.exec(event->globalPos());
	}

	event->accept();
}

void PrimitivesView::selectChildren(const QModelIndex &parent, QItemSelection &itemSelection)
{
	const int rowCount = model()->rowCount(parent);

	QItemSelection mergeItemSelection(parent.child(0, 0), parent.child(rowCount - 1, 0));
	itemSelection.merge(mergeItemSelection, QItemSelectionModel::Select);

	for (int i = 0; i < rowCount; ++i)
	{
		QModelIndex childIndex = parent.child(i, 0);
		if (model()->rowCount(childIndex) != 0)
			selectChildren(childIndex, itemSelection);
	}
}

void PrimitivesView::fillMenu_WorldEdit(QMenu *menu)
{
	QAction *closeAction = menu->addAction(tr("Close World"));
	connect(closeAction, SIGNAL(triggered()), this, SIGNAL(closeWorldRequested()));
	menu->addAction(m_showAllAction);
	//menu->addAction(m_unloadAction);
	//menu->addAction(m_saveAction);
	//menu->addAction(m_saveAsAction);
	menu->addSeparator();
	menu->addAction(m_loadLandAction);
	menu->addAction(m_loadPrimitiveAction);
	menu->addAction(m_newPrimitiveAction);
	menu->addSeparator();
	menu->addAction(m_helpAction);
}

void PrimitivesView::fillMenu_Landscape(QMenu *menu, const QModelIndex &index)
{
	// Jump to this land on the map. An empty one gets a note instead, so the missing
	// entry does not look like a fault.
	if (!landscapeSceneRect(index).isNull())
		menu->addAction(m_zoomToAction);
	else
	{
		QAction *emptyNote = menu->addAction(tr("Empty land - contains no zones"));
		emptyNote->setEnabled(false);
	}
	menu->addSeparator();
	menu->addAction(m_unloadAction);
}

void PrimitivesView::fillMenu_RootPrimitive(QMenu *menu, const QModelIndex &index)
{
	menu->addAction(m_saveAction);
	menu->addAction(m_saveAsAction);
	menu->addAction(m_unloadAction);
	fillMenu_Primitive(menu, index);
	menu->removeAction(m_deleteAction);
}

void PrimitivesView::fillMenu_Primitive(QMenu *menu, const QModelIndex &index)
{
	// Offered whenever anything below this row is on the map, so it works on a group
	// just as well as on a single NPC.
	if (!sceneRectOfSubtree(index).isNull())
	{
		menu->addAction(m_zoomToAction);
		menu->addAction(m_positionAction);
		menu->addSeparator();
	}

	menu->addAction(m_deleteAction);
	menu->addAction(m_selectChildrenAction);
	menu->addAction(m_helpAction);
	menu->addSeparator();
	addHideActions(menu, index);

	QSignalMapper *addSignalMapper = new QSignalMapper(menu);
	QSignalMapper *generateSignalMapper = new QSignalMapper(menu);
	//QSignalMapper *openSignalMapper = new QSignalMapper(menu);
	connect(addSignalMapper, SIGNAL(mapped(int)), this, SLOT(addNewPrimitiveByClass(int)));
	connect(generateSignalMapper, SIGNAL(mapped(int)), this, SLOT(generatePrimitives(int)));
	//connect(openSignalMapper, SIGNAL(mapped(int)), this, SLOT(openItem(int)));

	PrimitiveNode *node = static_cast<PrimitiveNode *>(index.internalPointer());
	const NLLIGO::CPrimitiveClass *primClass = node->primitiveClass();

	// What class is it ?
	if (primClass && primClass->DynamicChildren.size())
	{
		menu->addSeparator();

		// For each child, add a create method
		for (size_t i = 0; i < primClass->DynamicChildren.size(); i++)
		{
			// Get class name
			QString className = primClass->DynamicChildren[i].ClassName.c_str();

			// Get icon
			QIcon icon = PrimitiveIcons::instance().iconForClass(className);

			// Create and add action in popur menu
			QAction *action = menu->addAction(icon, tr("Add %1").arg(className));
			addSignalMapper->setMapping(action, i);
			connect(action, SIGNAL(triggered()), addSignalMapper, SLOT(map()));
		}
	}

	// What class is it ?
	if (primClass && primClass->GeneratedChildren.size())
	{
		menu->addSeparator();

		// For each child, add a create method
		for (size_t i = 0; i < primClass->GeneratedChildren.size(); i++)
		{
			// Get class name
			QString childName = primClass->GeneratedChildren[i].ClassName.c_str();

			// Create and add action in popur menu
			QAction *action = menu->addAction(tr("Generate %1").arg(childName));
			generateSignalMapper->setMapping(action, i);
			connect(action, SIGNAL(triggered()), generateSignalMapper, SLOT(map()));
		}
	}
}

} /* namespace WorldEditor */
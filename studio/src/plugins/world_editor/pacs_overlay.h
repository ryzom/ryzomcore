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

#ifndef PACS_OVERLAY_H
#define PACS_OVERLAY_H

// Qt includes
#include <QtCore/QVector>
#include <QtCore/QLineF>
#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtGui/QPainterPath>
#include <QtWidgets/QGraphicsItem>

namespace WorldEditor
{

/*
@class PacsOverlay
@brief Draws the PACS borders of a continent over the map, like ID_VIEW_PACS in the MFC editor.
@details The MFC editor kept the retrievers loaded and asked them for the borders of the
visible area on every redraw, which made panning slow. Here every border is fetched once
while loading, the retrievers are released again, and the lines are kept in a grid of
buckets so a redraw only touches what is on screen. Zoomed out, where a single pixel covers
several metres, pre-rendered images are drawn instead of hundreds of thousands of lines.
*/
class PacsOverlay: public QGraphicsItem
{
public:
	/// Edge kinds as NLPACS::CGlobalRetriever::getBorders() reports them. The two kinds
	/// of the exterior mesh of interiors (4 and 5) are folded into Other.
	enum EdgeType
	{
		Block = 0,
		Surmountable,
		Link,
		Waterline,
		Other,
		EdgeTypeCount
	};

	explicit PacsOverlay(int cellSize, QGraphicsItem *parent = 0);
	virtual ~PacsOverlay();

	/// Load every <name>.rbank / <name>.gr pair found in the directory, replacing what
	/// was loaded before. Returns the number of retrievers that could be loaded.
	int load(const QString &pacsDir);

	/// Drop all loaded borders.
	void clear();

	bool isEmpty() const;

	/// The directory the borders were loaded from, empty if nothing is loaded.
	QString directory() const;

	/// Number of border segments, per type and in total.
	int edgeCount(EdgeType type) const;
	int edgeCount() const;

	/// Kept apart from the world items, see pacs_overlay.cpp.
	enum { Type = QGraphicsItem::UserType + 20 };
	virtual int type() const;

	/// Empty: the borders are a picture, not something to click on. The default shape
	/// would be the whole bounding rectangle and catch every pick on the map.
	virtual QPainterPath shape() const;

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	/// Colour each edge type is drawn in, the same as in the MFC editor.
	static QColor edgeColor(EdgeType type);

private:
	struct Bucket
	{
		QVector<QLineF> lines[EdgeTypeCount];
	};

	/// A pre-rendered picture of every border, for when the view is zoomed out.
	struct Level
	{
		qreal metresPerPixel;
		QImage image;
	};

	bool loadRetriever(const QString &rbankFile, const QString &grFile);
	void addLine(const QLineF &line, int type);
	void buildBuckets();
	void buildLevels();
	QPointF toScene(double x, double y) const;

	int m_cellSize;
	QString m_dir;

	/// Every loaded line, in scene coordinates. Only kept until the buckets are built.
	QVector<QLineF> m_pendingLines[EdgeTypeCount];
	int m_edgeCount[EdgeTypeCount];

	QRectF m_bounds;
	QVector<Bucket> m_buckets;
	int m_bucketColumns, m_bucketRows;

	QVector<Level> m_levels;
};

} /* namespace WorldEditor */

#endif // PACS_OVERLAY_H

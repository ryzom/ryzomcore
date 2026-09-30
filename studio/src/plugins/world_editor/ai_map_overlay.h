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

#ifndef AI_MAP_OVERLAY_H
#define AI_MAP_OVERLAY_H

// Qt includes
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QImage>
#include <QtGui/QPainterPath>
#include <QtWidgets/QGraphicsItem>

namespace WorldEditor
{

/*
@class AiMapOverlay
@brief Shows where the AI service can move mobs and NPCs: the collision map of the MFC
editor ("View Collisions"), read from the world map the AI service itself loads.
@details The AI does not path on PACS directly. ai_data_service (pacs_scan.cpp) floods the
PACS surfaces from start points and writes <continent>_0.cwmap2, which the AI service loads
(ai_service/world_container.cpp). Ground outside that map has no AI support, even where a
player can walk. The old collisionmap/*.png are a picture of the same map, made once
(pacsBuildBitmap) - here the map itself is read with the server's own code
(ai_share/world_map), so copying newer .cwmap2 files is all it takes to see a newer state.

Drawn at one pixel per metre in 256 m tiles, only where the map has something; zoomed
out, pre-scaled images instead.
*/
class AiMapOverlay: public QGraphicsItem
{
public:
	/// What a metre of the map says, as colours of the overlay.
	enum CellKind
	{
		NoAi = 0,
		AiGround,		///< one layer the AI can use
		AiLayered,		///< two or three layers on top of each other (bridges, caves)
		AiBorder,		///< the AI cannot go on in at least one direction
		CellKindCount
	};

	explicit AiMapOverlay(int cellSize, QGraphicsItem *parent = 0);
	virtual ~AiMapOverlay();

	/// Load a .cwmap2 file, replacing what was loaded. Returns false if it cannot be read.
	bool load(const QString &fileName);
	void clear();

	bool isEmpty() const;
	QString fileName() const;

	/// Square metres the AI can use, and how many of them lie on a border.
	qint64 accessibleArea() const;
	qint64 borderArea() const;

	enum { Type = QGraphicsItem::UserType + 21 };
	virtual int type() const;
	virtual QPainterPath shape() const;
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

	/// Colour of each kind, alpha included.
	static QColor kindColor(CellKind kind);

private:
	struct Level
	{
		qreal metresPerPixel;
		QImage image;
	};

	void buildLevels();

	int m_cellSize;
	QString m_fileName;

	/// Scene rectangle of the whole map; one pixel of the tiles is one metre of it.
	QRectF m_bounds;
	int m_width, m_height;

	/// 256 x 256 metre tiles, keyed by column and row; empty tiles are not stored.
	static const int TILE = 256;
	QHash<quint32, QImage> m_tiles;
	int m_tileColumns, m_tileRows;

	QVector<Level> m_levels;
	qint64 m_accessible, m_border;
};

} /* namespace WorldEditor */

#endif // AI_MAP_OVERLAY_H

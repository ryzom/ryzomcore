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

// The AI service's own world map code (ryzom/server/src/ai_share), built into the plugin.
// First, before any Qt header: it has a variable named "slots", which Qt turns into a
// macro unless QT_NO_KEYWORDS is defined.
#include "world_map.h"

// Project includes
#include "ai_map_overlay.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/file.h>

// Qt includes
#include <QtCore/QElapsedTimer>
#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include <cmath>

using namespace RYAI_MAP_CRUNCH;

namespace WorldEditor
{

// Above the zone bitmaps (z 2), below the PACS lines (z 50), so both can be shown at once.
static const qreal AI_MAP_LAYER = 40;

// Zoomed in closer than this, the 1 m tiles are drawn; further out, the levels below.
static const qreal FINEST_LEVEL_METRES_PER_PIXEL = 2.0;
static const qreal LEVEL_FACTOR = 3;
static const int LEVEL_COUNT = 3;

AiMapOverlay::AiMapOverlay(int cellSize, QGraphicsItem *parent)
	: QGraphicsItem(parent),
	  m_cellSize(cellSize),
	  m_width(0),
	  m_height(0),
	  m_tileColumns(0),
	  m_tileRows(0),
	  m_accessible(0),
	  m_border(0)
{
	setZValue(AI_MAP_LAYER);
	setAcceptedMouseButtons(Qt::NoButton);
	setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
}

AiMapOverlay::~AiMapOverlay()
{
}

QColor AiMapOverlay::kindColor(CellKind kind)
{
	switch (kind)
	{
	case AiGround:
		return QColor(30, 110, 255, 95);
	case AiLayered:
		return QColor(170, 60, 255, 130);
	case AiBorder:
		return QColor(255, 40, 40, 210);
	default:
		return QColor(0, 0, 0, 0);
	}
}

bool AiMapOverlay::load(const QString &fileName)
{
	clear();

	QElapsedTimer timer;
	timer.start();

	CWorldMap worldMap;
	try
	{
		NLMISC::CIFile file;
		if (!file.open(fileName.toUtf8().constData()))
		{
			nlwarning("AI map: cannot open '%s'", fileName.toUtf8().constData());
			return false;
		}
		file.serial(worldMap);
	}
	catch (const NLMISC::Exception &e)
	{
		nlwarning("AI map: cannot read '%s': %s", fileName.toUtf8().constData(), e.what());
		return false;
	}

	CMapPosition min, max;
	worldMap.getBounds(min, max);
	m_width = max.x() - min.x();
	m_height = max.y() - min.y();
	if ((m_width <= 0) || (m_height <= 0))
	{
		nlwarning("AI map: '%s' is empty", fileName.toUtf8().constData());
		return false;
	}

	// The map is a grid of 256 x 256 super cells, 65536 m a side, and a position is found
	// through its coordinates masked to that range - a negative y wraps around, and
	// getBounds() reports the wrapped value (fyros: 38400..41728 for -27136..-23808).
	// toVectorD() hands that back unchanged, fine for the AI service, which only compares
	// map positions. Ryzom's world lies at x >= 0 and y <= 0, so y is unwrapped here.
	// A map cell stands for the metre around its world point (CMapPosition rounds with
	// +0.5). Rows run north, the scene runs down, so the image is filled bottom up.
	const qreal originX = min.x();
	const qreal originY = qreal(min.y()) - 65536.0;
	const qreal left = originX - 0.5;
	const qreal topWorld = originY + m_height - 0.5;
	prepareGeometryChange();
	m_bounds = QRectF(left, m_cellSize - topWorld, m_width, m_height);

	m_tileColumns = (m_width + TILE - 1) / TILE;
	m_tileRows = (m_height + TILE - 1) / TILE;

	const QRgb colors[CellKindCount] =
	{
		kindColor(NoAi).rgba(), kindColor(AiGround).rgba(), kindColor(AiLayered).rgba(), kindColor(AiBorder).rgba()
	};
	// QImage wants premultiplied values for Format_ARGB32_Premultiplied.
	QRgb premultiplied[CellKindCount];
	for (int i = 0; i < CellKindCount; ++i)
		premultiplied[i] = qPremultiply(colors[i]);

	CMapPosition rowStart(min.x(), min.y());
	for (int y = 0; y < m_height; ++y)
	{
		const int imageRow = m_height - 1 - y;
		const int tileRow = imageRow / TILE;
		const int inTileRow = imageRow % TILE;

		CMapPosition pos(rowStart);
		for (int x = 0; x < m_width; ++x)
		{
			int kind = NoAi;
			const CRootCell *cell = worldMap.getRootCellCst(pos);
			if (cell != 0)
			{
				// The same test as pacsBuildBitmap: count the layers, and whether any of them
				// has a direction the AI cannot continue in.
				int layers = 0;
				bool border = false;
				for (uint slot = 0; slot < 3; ++slot)
				{
					const CWorldPosition wp = worldMap.getSafeWorldPosition(pos, CSlot(slot));
					if (!wp.isValid())
						continue;
					++layers;
					const CCellLinkage links = cell->getCellLink(wp);
					if (!links.isESlotValid() || !links.isWSlotValid() ||
						!links.isSSlotValid() || !links.isNSlotValid())
						border = true;
				}
				if (layers > 0)
				{
					kind = border ? AiBorder : ((layers > 1) ? AiLayered : AiGround);
					++m_accessible;
					if (border)
						++m_border;
				}
			}

			if (kind != NoAi)
			{
				const quint32 key = (quint32(tileRow) << 16) | quint32(x / TILE);
				QHash<quint32, QImage>::iterator tile = m_tiles.find(key);
				if (tile == m_tiles.end())
				{
					QImage image(TILE, TILE, QImage::Format_ARGB32_Premultiplied);
					image.fill(Qt::transparent);
					tile = m_tiles.insert(key, image);
				}
				reinterpret_cast<QRgb *>(tile->scanLine(inTileRow))[x % TILE] = premultiplied[kind];
			}
			pos = pos.getStepE();
		}
		rowStart = rowStart.getStepN();
	}

	buildLevels();
	m_fileName = fileName;

	nlinfo("AI map: %s - %d x %d m, %lld m2 with AI support, %d tiles, %lld ms",
		   fileName.toUtf8().constData(), m_width, m_height, (long long)m_accessible,
		   m_tiles.size(), (long long)timer.elapsed());
	update();
	return true;
}

void AiMapOverlay::buildLevels()
{
	m_levels.clear();
	qreal metresPerPixel = FINEST_LEVEL_METRES_PER_PIXEL;
	for (int level = 0; level < LEVEL_COUNT; ++level, metresPerPixel *= LEVEL_FACTOR)
	{
		Level entry;
		entry.metresPerPixel = metresPerPixel;
		entry.image = QImage(qMax(1, int(std::ceil(m_width / metresPerPixel))),
							 qMax(1, int(std::ceil(m_height / metresPerPixel))),
							 QImage::Format_ARGB32_Premultiplied);
		entry.image.fill(Qt::transparent);

		QPainter painter(&entry.image);
		painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
		painter.scale(1.0 / metresPerPixel, 1.0 / metresPerPixel);
		for (QHash<quint32, QImage>::const_iterator it = m_tiles.constBegin(); it != m_tiles.constEnd(); ++it)
		{
			const int column = it.key() & 0xffff;
			const int row = it.key() >> 16;
			painter.drawImage(QPointF(column * TILE, row * TILE), it.value());
		}
		painter.end();
		m_levels.append(entry);
	}
}

void AiMapOverlay::clear()
{
	prepareGeometryChange();
	m_fileName.clear();
	m_bounds = QRectF();
	m_width = m_height = 0;
	m_tiles.clear();
	m_tileColumns = m_tileRows = 0;
	m_levels.clear();
	m_accessible = m_border = 0;
	update();
}

bool AiMapOverlay::isEmpty() const
{
	return m_tiles.isEmpty();
}

QString AiMapOverlay::fileName() const
{
	return m_fileName;
}

qint64 AiMapOverlay::accessibleArea() const
{
	return m_accessible;
}

qint64 AiMapOverlay::borderArea() const
{
	return m_border;
}

int AiMapOverlay::type() const
{
	return Type;
}

QPainterPath AiMapOverlay::shape() const
{
	// A picture, not something to click on.
	return QPainterPath();
}

QRectF AiMapOverlay::boundingRect() const
{
	return m_bounds;
}

void AiMapOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(widget);
	if (m_tiles.isEmpty())
		return;

	const QRectF exposed = option->exposedRect.intersected(m_bounds);
	if (exposed.isEmpty())
		return;

	const qreal pixelsPerMetre = QStyleOptionGraphicsItem::levelOfDetailFromTransform(painter->worldTransform());
	const qreal metresPerPixel = pixelsPerMetre > 0 ? 1.0 / pixelsPerMetre : 1e9;

	painter->save();
	if (!m_levels.isEmpty() && (metresPerPixel >= m_levels.first().metresPerPixel))
	{
		int level = 0;
		while ((level + 1 < m_levels.size()) && (m_levels[level + 1].metresPerPixel <= metresPerPixel))
			++level;
		const Level &entry = m_levels[level];
		const QRectF source((exposed.left() - m_bounds.left()) / entry.metresPerPixel,
							(exposed.top() - m_bounds.top()) / entry.metresPerPixel,
							exposed.width() / entry.metresPerPixel,
							exposed.height() / entry.metresPerPixel);
		painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
		painter->drawImage(exposed, entry.image, source);
	}
	else
	{
		// Close in, every metre is a visible square: no smoothing, so borders stay sharp.
		painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
		const int firstColumn = qMax(0, int((exposed.left() - m_bounds.left()) / TILE));
		const int lastColumn = qMin(m_tileColumns - 1, int((exposed.right() - m_bounds.left()) / TILE));
		const int firstRow = qMax(0, int((exposed.top() - m_bounds.top()) / TILE));
		const int lastRow = qMin(m_tileRows - 1, int((exposed.bottom() - m_bounds.top()) / TILE));
		for (int row = firstRow; row <= lastRow; ++row)
		{
			for (int column = firstColumn; column <= lastColumn; ++column)
			{
				QHash<quint32, QImage>::const_iterator tile = m_tiles.constFind((quint32(row) << 16) | quint32(column));
				if (tile == m_tiles.constEnd())
					continue;
				painter->drawImage(QRectF(m_bounds.left() + column * TILE, m_bounds.top() + row * TILE, TILE, TILE),
								   tile.value());
			}
		}
	}
	painter->restore();
}

} /* namespace WorldEditor */

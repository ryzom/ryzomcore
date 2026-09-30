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
#include "pacs_overlay.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/aabbox.h>
#include <nel/misc/line.h>
#include <nel/pacs/retriever_bank.h>
#include <nel/pacs/global_retriever.h>

// Qt includes
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QElapsedTimer>
#include <QtGui/QPainter>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include <cmath>

namespace WorldEditor
{

// Above the zone bitmaps (z 2) and below every primitive (z 100 and up), as in the MFC
// editor, which draws the borders before the primitives.
static const qreal PACS_LAYER = 50;

// Side of the buckets the lines are sorted into, in metres.
static const qreal BUCKET_SIZE = 160;

// Finest pre-rendered image and the factor between two levels. At fyros' 4.5 x 3.2 km
// the finest level is about 3000 x 2100 pixels. Zoomed in closer than that, the lines
// are drawn as vectors.
static const qreal FINEST_METRES_PER_PIXEL = 1.5;
static const qreal LEVEL_FACTOR = 3;
static const int LEVEL_COUNT = 3;
static const int MAX_IMAGE_SIDE = 8192;

// Side of the tiles a level is cut into, in pixels. Qt's OpenGL paint engine keeps images
// in a texture cache of 64 MB and drops one that is bigger on the spot, so a level of a
// large continent in one piece came out black at the finest level.
static const int TILE_SIDE = 1024;

// Painting order: the blocking borders last, so they stay on top where kinds overlap.
static const PacsOverlay::EdgeType PAINT_ORDER[PacsOverlay::EdgeTypeCount] =
{
	PacsOverlay::Other,
	PacsOverlay::Surmountable,
	PacsOverlay::Link,
	PacsOverlay::Waterline,
	PacsOverlay::Block
};

PacsOverlay::PacsOverlay(int cellSize, QGraphicsItem *parent)
	: QGraphicsItem(parent),
	  m_cellSize(cellSize),
	  m_bucketColumns(0),
	  m_bucketRows(0)
{
	for (int i = 0; i < EdgeTypeCount; ++i)
		m_edgeCount[i] = 0;

	setZValue(PACS_LAYER);
	setAcceptedMouseButtons(Qt::NoButton);
	setAcceptHoverEvents(false);
	setFlag(QGraphicsItem::ItemUsesExtendedStyleOption, true);
}

PacsOverlay::~PacsOverlay()
{
}

QColor PacsOverlay::edgeColor(EdgeType type)
{
	switch (type)
	{
	case Block:
		return QColor(255, 0, 0);
	case Surmountable:
		return QColor(0, 255, 0);
	case Link:
		return QColor(255, 255, 0);
	case Waterline:
		return QColor(0, 110, 255);
	default:
		return QColor(255, 255, 255);
	}
}

int PacsOverlay::load(const QString &pacsDir)
{
	clear();

	QElapsedTimer timer;
	timer.start();

	const QDir dir(pacsDir);
	const QStringList banks = dir.entryList(QStringList() << "*.rbank", QDir::Files, QDir::Name);

	int loaded = 0;
	Q_FOREACH (const QString &bank, banks)
	{
		const QFileInfo bankInfo(dir.filePath(bank));
		const QString grFile = dir.filePath(bankInfo.completeBaseName() + ".gr");
		if (!QFileInfo(grFile).exists())
			continue;

		if (loadRetriever(bankInfo.filePath(), grFile))
			++loaded;
	}

	if (loaded == 0)
	{
		nlwarning("PACS: nothing loaded from '%s'", pacsDir.toUtf8().constData());
		clear();
		return 0;
	}

	m_dir = pacsDir;

	prepareGeometryChange();
	buildBuckets();
	buildLevels();

	nlinfo("PACS: %d retriever(s), %d border segments from '%s' in %lld ms",
		   loaded, edgeCount(), pacsDir.toUtf8().constData(), (long long)timer.elapsed());

	update();
	return loaded;
}

bool PacsOverlay::loadRetriever(const QString &rbankFile, const QString &grFile)
{
	// Everything here goes by full path. NLPACS::URetrieverBank::createRetrieverBank()
	// and createGlobalRetriever() look files up by name through CPath, where the search
	// paths win over the given directory. With the continents on the search path that
	// picks whichever copy was indexed first - continents/nexus has a pacs_old/ next to
	// pacs/ with a different nexus.rbank. The .lr files of a bank go the same way.
	const std::string rbankPath = rbankFile.toUtf8().constData();
	const std::string grPath = grFile.toUtf8().constData();
	const QString prefix = QFileInfo(rbankFile).completeBaseName();
	const QString dir = QFileInfo(rbankFile).absolutePath();

	NLPACS::CRetrieverBank *bank = 0;
	NLPACS::CGlobalRetriever *retriever = 0;
	try
	{
		// Newer banks keep the local retrievers in <name>_<n>.lr files next to them,
		// older ones inside the bank. The header says which.
		bool lrInBank = true;
		{
			NLMISC::CIFile header;
			if (!header.open(rbankPath))
			{
				nlwarning("PACS: cannot open '%s'", rbankPath.c_str());
				return false;
			}
			const uint version = header.serialVersion(1);
			if (version > 0)
				header.serial(lrInBank);
		}

		// Not "all loaded" when the retrievers live in their own files: the bank then only
		// sizes its table while reading, and each .lr is fed to it below.
		bank = new NLPACS::CRetrieverBank(lrInBank);
		{
			NLMISC::CIFile file;
			if (!file.open(rbankPath))
				throw NLMISC::Exception("cannot open " + rbankPath);
			file.serial(*bank);
		}

		if (!lrInBank)
		{
			int missing = 0;
			for (uint i = 0; i < bank->getRetrievers().size(); ++i)
			{
				const std::string lrPath =
						QDir(dir).filePath(QString("%1_%2.lr").arg(prefix).arg(i)).toUtf8().constData();
				NLMISC::CIFile file;
				if (!file.open(lrPath))
				{
					++missing;
					continue;
				}
				bank->loadRetriever(i, file);
			}
			if (missing > 0)
				nlwarning("PACS: %d of %u local retrievers missing next to '%s'",
						  missing, (uint)bank->getRetrievers().size(), rbankPath.c_str());
		}

		retriever = new NLPACS::CGlobalRetriever();
		// The bank has to be set before serializing, see createGlobalRetriever().
		retriever->setRetrieverBank(bank);
		{
			NLMISC::CIFile file;
			if (!file.open(grPath))
				throw NLMISC::Exception("cannot open " + grPath);
			file.serial(*retriever);
		}
		retriever->initAll(false);

		// Fetch every border at once. The box is made generous in height, the borders
		// are selected in 2D anyway.
		NLMISC::CAABBox box = retriever->getBBox();
		box.setHalfSize(box.getHalfSize() + NLMISC::CVector(1.0f, 1.0f, 10000.0f));

		std::vector<std::pair<NLMISC::CLine, uint8> > edges;
		retriever->getBorders(box, edges);

		for (size_t i = 0; i < edges.size(); ++i)
		{
			const NLMISC::CLine &line = edges[i].first;
			addLine(QLineF(toScene(line.V0.x, line.V0.y), toScene(line.V1.x, line.V1.y)),
					edges[i].second);
		}
	}
	catch (const NLMISC::Exception &e)
	{
		nlwarning("PACS: cannot load '%s': %s", rbankPath.c_str(), e.what());
		delete retriever;
		delete bank;
		return false;
	}

	// The lines are all that is needed, the retrievers themselves can go.
	delete retriever;
	delete bank;
	return true;
}

void PacsOverlay::addLine(const QLineF &line, int type)
{
	const EdgeType edgeType = (type >= Block && type < Other) ? EdgeType(type) : Other;
	m_pendingLines[edgeType].append(line);
	++m_edgeCount[edgeType];
}

void PacsOverlay::clear()
{
	prepareGeometryChange();
	m_dir.clear();
	for (int i = 0; i < EdgeTypeCount; ++i)
	{
		m_pendingLines[i].clear();
		m_edgeCount[i] = 0;
	}
	m_bounds = QRectF();
	m_buckets.clear();
	m_bucketColumns = m_bucketRows = 0;
	m_levels.clear();
	update();
}

bool PacsOverlay::isEmpty() const
{
	return edgeCount() == 0;
}

QString PacsOverlay::directory() const
{
	return m_dir;
}

int PacsOverlay::edgeCount(EdgeType type) const
{
	return m_edgeCount[type];
}

int PacsOverlay::edgeCount() const
{
	int count = 0;
	for (int i = 0; i < EdgeTypeCount; ++i)
		count += m_edgeCount[i];
	return count;
}

QPointF PacsOverlay::toScene(double x, double y) const
{
	// The same mapping the primitives use, see LoadRootPrimitiveCommand.
	return QPointF(x, m_cellSize - y);
}

void PacsOverlay::buildBuckets()
{
	m_bounds = QRectF();
	for (int t = 0; t < EdgeTypeCount; ++t)
	{
		Q_FOREACH (const QLineF &line, m_pendingLines[t])
		{
			const QRectF r = QRectF(line.p1(), line.p2()).normalized();
			m_bounds = m_bounds.isNull() ? r : m_bounds.united(r);
		}
	}

	// A line lands in the bucket of its first point and can reach past it into the
	// neighbours. Borders are chains of short segments, so being generous by one bucket
	// all round while painting covers them; the rare longer segment is caught too, as
	// long as it is shorter than a bucket.
	m_bucketColumns = qMax(1, int(std::ceil(m_bounds.width() / BUCKET_SIZE)));
	m_bucketRows = qMax(1, int(std::ceil(m_bounds.height() / BUCKET_SIZE)));
	m_buckets.clear();
	m_buckets.resize(m_bucketColumns * m_bucketRows);

	for (int t = 0; t < EdgeTypeCount; ++t)
	{
		Q_FOREACH (const QLineF &line, m_pendingLines[t])
		{
			const int column = qBound(0, int((line.x1() - m_bounds.left()) / BUCKET_SIZE), m_bucketColumns - 1);
			const int row = qBound(0, int((line.y1() - m_bounds.top()) / BUCKET_SIZE), m_bucketRows - 1);
			m_buckets[row * m_bucketColumns + column].lines[t].append(line);
		}
	}

	for (int t = 0; t < EdgeTypeCount; ++t)
		m_pendingLines[t] = QVector<QLineF>();
}

void PacsOverlay::buildLevels()
{
	m_levels.clear();
	if (m_bounds.isEmpty())
		return;

	const qreal side = qMax(m_bounds.width(), m_bounds.height());
	qreal metresPerPixel = qMax(FINEST_METRES_PER_PIXEL, side / MAX_IMAGE_SIDE);

	for (int level = 0; level < LEVEL_COUNT; ++level, metresPerPixel *= LEVEL_FACTOR)
	{
		const int width = qMax(1, int(std::ceil(m_bounds.width() / metresPerPixel)));
		const int height = qMax(1, int(std::ceil(m_bounds.height() / metresPerPixel)));

		Level entry;
		entry.metresPerPixel = metresPerPixel;
		entry.columns = (width + TILE_SIDE - 1) / TILE_SIDE;
		entry.rows = (height + TILE_SIDE - 1) / TILE_SIDE;
		const qreal tileMetres = TILE_SIDE * metresPerPixel;

		for (int row = 0; row < entry.rows; ++row)
		{
			for (int column = 0; column < entry.columns; ++column)
			{
				QImage tile(qMin(TILE_SIDE, width - column * TILE_SIDE),
							qMin(TILE_SIDE, height - row * TILE_SIDE),
							QImage::Format_ARGB32_Premultiplied);
				tile.fill(Qt::transparent);

				// Only the buckets under the tile, one bucket of margin all round, as in
				// paint().
				const QPointF origin = m_bounds.topLeft() + QPointF(column * tileMetres, row * tileMetres);
				const int firstColumn = qMax(0, int(column * tileMetres / BUCKET_SIZE) - 1);
				const int lastColumn = qMin(m_bucketColumns - 1, int((column + 1) * tileMetres / BUCKET_SIZE) + 1);
				const int firstRow = qMax(0, int(row * tileMetres / BUCKET_SIZE) - 1);
				const int lastRow = qMin(m_bucketRows - 1, int((row + 1) * tileMetres / BUCKET_SIZE) + 1);

				// Each level is drawn from the lines, not scaled down from the one before,
				// so the borders stay one clean pixel wide at every level.
				QPainter painter(&tile);
				painter.scale(1.0 / metresPerPixel, 1.0 / metresPerPixel);
				painter.translate(-origin);
				for (int i = 0; i < EdgeTypeCount; ++i)
				{
					const EdgeType type = PAINT_ORDER[i];
					QPen pen(edgeColor(type), 0);
					pen.setCosmetic(true);
					painter.setPen(pen);
					for (int r = firstRow; r <= lastRow; ++r)
					{
						for (int c = firstColumn; c <= lastColumn; ++c)
						{
							const QVector<QLineF> &lines = m_buckets[r * m_bucketColumns + c].lines[type];
							if (!lines.isEmpty())
								painter.drawLines(lines);
						}
					}
				}
				painter.end();

				entry.tiles.append(tile);
			}
		}

		m_levels.append(entry);
	}
}

int PacsOverlay::type() const
{
	// Kept apart from AbstractWorldItem (UserType + 1) and the point items (UserType + 2).
	// The default type() is UserType itself, which qgraphicsitem_cast would happily take
	// for something else.
	return Type;
}

QPainterPath PacsOverlay::shape() const
{
	return QPainterPath();
}

QRectF PacsOverlay::boundingRect() const
{
	return m_bounds;
}

void PacsOverlay::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(widget);

	if (m_buckets.isEmpty())
		return;

	const QRectF exposed = option->exposedRect.intersected(m_bounds);
	if (exposed.isEmpty())
		return;

	// Pixels per metre on screen.
	const qreal pixelsPerMetre = QStyleOptionGraphicsItem::levelOfDetailFromTransform(painter->worldTransform());
	const qreal metresPerPixel = pixelsPerMetre > 0 ? 1.0 / pixelsPerMetre : 1e9;

	if (!m_levels.isEmpty() && metresPerPixel >= m_levels.first().metresPerPixel)
	{
		// Zoomed out: take the coarsest image that is still at least as fine as the
		// screen, so it is only ever scaled down, by less than LEVEL_FACTOR.
		int level = 0;
		while ((level + 1 < m_levels.size()) && (m_levels[level + 1].metresPerPixel <= metresPerPixel))
			++level;

		const Level &entry = m_levels[level];
		const qreal tileMetres = TILE_SIDE * entry.metresPerPixel;

		painter->save();
		painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
		painter->setClipRect(exposed, Qt::IntersectClip);
		for (int row = 0; row < entry.rows; ++row)
		{
			for (int column = 0; column < entry.columns; ++column)
			{
				// Whole tiles, clipped to the exposed area: a fractional source rectangle
				// per tile would leave seams between them.
				const QImage &tile = entry.tiles[row * entry.columns + column];
				const QRectF target(m_bounds.left() + column * tileMetres,
									m_bounds.top() + row * tileMetres,
									tile.width() * entry.metresPerPixel,
									tile.height() * entry.metresPerPixel);
				if (target.intersects(exposed))
					painter->drawImage(target, tile);
			}
		}
		painter->restore();
		return;
	}

	// Zoomed in: draw the lines of the buckets in view, one bucket of margin all round.
	const int firstColumn = qMax(0, int((exposed.left() - m_bounds.left()) / BUCKET_SIZE) - 1);
	const int lastColumn = qMin(m_bucketColumns - 1, int((exposed.right() - m_bounds.left()) / BUCKET_SIZE) + 1);
	const int firstRow = qMax(0, int((exposed.top() - m_bounds.top()) / BUCKET_SIZE) - 1);
	const int lastRow = qMin(m_bucketRows - 1, int((exposed.bottom() - m_bounds.top()) / BUCKET_SIZE) + 1);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, false);
	// Close in, a hairline gets lost next to the primitives.
	const qreal penWidth = (metresPerPixel < 0.1) ? 2 : 1;
	for (int i = 0; i < EdgeTypeCount; ++i)
	{
		const EdgeType type = PAINT_ORDER[i];
		QPen pen(edgeColor(type), penWidth);
		pen.setCosmetic(true);
		painter->setPen(pen);
		for (int row = firstRow; row <= lastRow; ++row)
		{
			for (int column = firstColumn; column <= lastColumn; ++column)
			{
				const QVector<QLineF> &lines = m_buckets[row * m_bucketColumns + column].lines[type];
				if (!lines.isEmpty())
					painter->drawLines(lines);
			}
		}
	}
	painter->restore();
}

} /* namespace WorldEditor */

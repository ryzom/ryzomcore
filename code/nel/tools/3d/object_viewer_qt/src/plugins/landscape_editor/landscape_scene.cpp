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

// Project includes
#include "landscape_scene.h"
#include "pixmap_database.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QGraphicsSimpleTextItem>
#include <QApplication>

namespace LandscapeEditor
{

static const int ZONE_NAME = 0;
static const int LAYER_ZONES = 2;
static const int LAYER_EMPTY_ZONES = 3;
static const int LAYER_BLACKOUT = 4;
const char *const LAYER_BLACKOUT_NAME = "blackout";

const int MAX_SCENE_WIDTH = 256;
const int MAX_SCENE_HEIGHT = 256;

LandscapeScene::LandscapeScene(int sizeCell, QObject *parent)
	: QGraphicsScene(parent),
	  m_cellSize(sizeCell),
	  m_transitionMode(false),
	  m_mouseButton(Qt::NoButton),
	  m_zoneBuilder(0)
{
	setSceneRect(QRectF(0, m_cellSize, MAX_SCENE_WIDTH * m_cellSize, MAX_SCENE_HEIGHT * m_cellSize));
}

LandscapeScene::~LandscapeScene()
{
}

int LandscapeScene::cellSize() const
{
	return m_cellSize;
}

void LandscapeScene::setZoneBuilder(ZoneBuilder *zoneBuilder)
{
	m_zoneBuilder = zoneBuilder;
}

QGraphicsItem *LandscapeScene::createItemZone(const LigoData &data, const ZonePosition &zonePos)
{
	if ((data.zoneName == STRING_OUT_OF_BOUND) || (checkUnderZone(zonePos.x, zonePos.y)))
		return 0;

	if (data.zoneName == STRING_UNUSED)
		return createItemEmptyZone(zonePos);

	if ((m_zoneBuilder == 0) || (data.zoneName.empty()))
		return 0;

	// Get image from pixmap database
	QPixmap *pixmap = m_zoneBuilder->pixmapDatabase()->pixmap(QString(data.zoneName.c_str()));
	if (pixmap == 0)
		return 0;

	// Rotate the image counterclockwise
	QMatrix matrix;
	matrix.rotate(-data.rot * 90.0);

	QGraphicsPixmapItem *item;

	if (data.flip == 0)
	{
		item = addPixmap(pixmap->transformed(matrix, Qt::SmoothTransformation));
	}
	else
	{
		// mirror image
		QImage mirrorImage = pixmap->toImage();
		QPixmap mirrorPixmap = QPixmap::fromImage(mirrorImage.mirrored(true, false));
		item = addPixmap(mirrorPixmap.transformed(matrix, Qt::SmoothTransformation));
	}
	// Enable bilinear filtering
	item->setTransformationMode(Qt::SmoothTransformation);

	sint32 sizeX = 1, sizeY = 1;
	sizeX = float(pixmap->width()) / m_zoneBuilder->pixmapDatabase()->textureSize();
	sizeY = float(pixmap->width()) / m_zoneBuilder->pixmapDatabase()->textureSize();

	sint32 deltaX = 0, deltaY = 0;

	// Calculate offset for graphics item (for items with size that are larger than 1)
	if ((sizeX > 1) || (sizeY > 1))
	{
		if (data.flip == 0)
		{
			switch (data.rot)
			{
			case 0:
				deltaX = -data.posX;
				deltaY = -data.posY + sizeY - 1;
				break;
			case 1:
				deltaX = -(sizeY - 1 - data.posY);
				deltaY = -data.posX + sizeX - 1;
				break;
			case 2:
				deltaX = -(sizeX - 1 - data.posX);
				deltaY = data.posY;
				break;
			case 3:
				deltaX = -data.posY;
				deltaY = data.posX;
				break;
			}
		}
		else
		{
			switch (data.rot)
			{
			case 0:
				deltaX = -(sizeX - 1 - data.posX);
				deltaY = -data.posY + sizeY - 1;
				break;
			case 1:
				deltaX = -(sizeY - 1 - data.posY);
				deltaY = +data.posX;
				break;
			case 2:
				deltaX = -data.posX;
				deltaY = data.posY;
				break;
			case 3:
				deltaX = -data.posY;
				deltaY = -data.posX + sizeX - 1;
				break;
			}
		}
	}

	// Set position graphics item with offset for large piece
	item->setPos((zonePos.x + deltaX) * m_cellSize, (abs(int(zonePos.y + deltaY))) * m_cellSize);

	// The size graphics item should be equal or proportional m_cellSize
	item->setScale(float(m_cellSize) / m_zoneBuilder->pixmapDatabase()->textureSize());

	item->setData(ZONE_NAME, QString(data.zoneName.c_str()));

	// for not full item zone
	item->setZValue(LAYER_ZONES);

	item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

	return item;
}

QGraphicsItem *LandscapeScene::createItemEmptyZone(const ZonePosition &zonePos)
{
	if (m_zoneBuilder == 0)
		return 0;

	if (checkUnderZone(zonePos.x, zonePos.y))
		return 0;

	// Get image from pixmap database
	QPixmap *pixmap = m_zoneBuilder->pixmapDatabase()->pixmap(QString(STRING_UNUSED));

	if (pixmap == 0)
		return 0;

	QGraphicsPixmapItem *item = addPixmap(*pixmap);

	// Enable bilinear filtering
	item->setTransformationMode(Qt::SmoothTransformation);

	// Set position graphics item
	item->setPos(zonePos.x * m_cellSize, abs(int(zonePos.y)) * m_cellSize);

	// The size graphics item should be equal or proportional m_cellSize
	item->setScale(float(m_cellSize) / m_zoneBuilder->pixmapDatabase()->textureSize());

	// for not full item zone
	item->setZValue(LAYER_EMPTY_ZONES);

	item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

	return item;
}

QGraphicsRectItem *LandscapeScene::createLayerBlackout(const NLLIGO::CZoneRegion &zoneRegion)
{
	QGraphicsRectItem *rectItem = addRect(zoneRegion.getMinX() * m_cellSize,
										  abs(zoneRegion.getMaxY()) * m_cellSize,
										  (abs(zoneRegion.getMaxX() - zoneRegion.getMinX()) + 1) * m_cellSize,
										  (abs(zoneRegion.getMaxY() - zoneRegion.getMinY()) + 1) * m_cellSize,
										  Qt::NoPen, QBrush(QColor(0, 0, 0, 50)));

	rectItem->setZValue(LAYER_BLACKOUT);
	rectItem->setData(ZONE_NAME, QString(LAYER_BLACKOUT_NAME));
	return rectItem;
}

void LandscapeScene::deleteItemZone(const ZonePosition &zonePos)
{
	QGraphicsItem *item = itemAt(zonePos.x * m_cellSize, abs(zonePos.y) * m_cellSize);
	if ((item != 0) && (item->data(ZONE_NAME).toString() != QString(LAYER_BLACKOUT_NAME)))
	{
		removeItem(item);
		delete item;
	}
}

void LandscapeScene::addZoneRegion(const NLLIGO::CZoneRegion &zoneRegion)
{
	for (sint32 i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		for (sint32 j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
		{

			std::string zoneName = zoneRegion.getName(i, j);
			if (zoneName == STRING_UNUSED)
			{
				ZonePosition zonePos(i, j, -1);
				QGraphicsItem *item = createItemEmptyZone(zonePos);
			}
			else if (!zoneName.empty())
			{
				LigoData data;
				ZonePosition zonePos(i, j, -1);
				data.zoneName = zoneName;
				data.rot = zoneRegion.getRot(i, j);
				data.flip = zoneRegion.getFlip(i, j);
				data.posX = zoneRegion.getPosX(i, j);
				data.posY = zoneRegion.getPosY(i, j);
				QGraphicsItem *item = createItemZone(data, zonePos);
			}
		}
	}
}

void LandscapeScene::delZoneRegion(const NLLIGO::CZoneRegion &zoneRegion)
{
	for (sint32 i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		for (sint32 j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
		{
			deleteItemZone(ZonePosition(i, -j, -1));
		}
	}
}

void LandscapeScene::snapshot(const QString &fileName, int width, int height, const QRectF &landRect)
{
	if (m_zoneBuilder == 0)
		return;

	// Create image
	QImage image(landRect.width(), landRect.height(), QImage::Format_RGB888);
	QPainter painter(&image);
	painter.setRenderHint(QPainter::Antialiasing, true);

	// Add white background
	painter.setBrush(QBrush(Qt::white));
	painter.setPen(Qt::NoPen);
	painter.drawRect(0, 0, landRect.width(), landRect.height());

	// Paint landscape
	render(&painter, QRectF(0, 0, landRect.width(), landRect.height()), landRect);

	QImage scaledImage = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	scaledImage.save(fileName);
}

QString LandscapeScene::zoneNameFromMousePos() const
{
	if ((m_posY > 0) || (m_posY < -MAX_SCENE_HEIGHT) ||
			(m_posX < 0) || (m_posX > MAX_SCENE_WIDTH))
		return "NOT A VALID ZONE";

	return QString("%1_%2%3  %4 %5  ").arg(-m_posY).arg(QChar('A' + (m_posX/26))).
		   arg(QChar('A' + (m_posX%26))).arg(m_mouseX, 0,'f',2).arg(-m_mouseY, 0,'f',2);
}

void LandscapeScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	qreal x = mouseEvent->scenePos().x();
	qreal y = mouseEvent->scenePos().y();
	if ((x < 0) || (y < 0))
		return;

	m_posX = sint32(floor(x / m_cellSize));
	m_posY = sint32(-floor(y / m_cellSize));

	if (m_zoneBuilder == 0)
		return;
	if (m_transitionMode)
	{
		if (mouseEvent->button() == Qt::LeftButton)

			// Need add offset(= cellSize) on y axes
			m_zoneBuilder->addTransition(sint(x), sint(-y + m_cellSize));
	}
	else
	{
		if (mouseEvent->button() == Qt::LeftButton)
			m_zoneBuilder->addZone(m_posX, m_posY);
		else if (mouseEvent->button() == Qt::RightButton)
			m_zoneBuilder->delZone(m_posX, m_posY);
	}
	m_mouseButton = mouseEvent->button();

	QGraphicsScene::mousePressEvent(mouseEvent);
}

void LandscapeScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	qreal x = mouseEvent->scenePos().x();
	qreal y = mouseEvent->scenePos().y();

	sint32 posX = sint32(floor(x / m_cellSize));
	sint32 posY = sint32(-floor(y / m_cellSize));

	if ((m_posX != posX || m_posY != posY) &&
			(m_mouseButton == Qt::LeftButton ||
			 m_mouseButton == Qt::RightButton))
	{
		if (m_transitionMode)
		{
		}
		else
		{
			if (m_mouseButton == Qt::LeftButton)
				m_zoneBuilder->addZone(posX, posY);
			else if (m_mouseButton == Qt::RightButton)
				m_zoneBuilder->delZone(posX, posY);
		}
		m_posX = posX;
		m_posY = posY;
		QApplication::processEvents();
	}

	m_posX = posX;
	m_posY = posY;

	m_mouseX = mouseEvent->scenePos().x();
	m_mouseY = mouseEvent->scenePos().y() - m_cellSize;
	QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void LandscapeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	m_mouseButton = Qt::NoButton;
}

bool LandscapeScene::checkUnderZone(const int posX, const int posY)
{
	QGraphicsItem *item = itemAt((posX * m_cellSize), abs(posY) * m_cellSize);
	if (item != 0)
	{
		//if (item->data(ZONE_NAME) == QString(LAYER_BLACKOUT_NAME))
		//	return false;
		//else
		return true;
	}
	return false;
}

bool LandscapeScene::transitionMode() const
{
	return m_transitionMode;
}

void LandscapeScene::setTransitionMode(bool enabled)
{
	m_transitionMode = enabled;
	update();
}

void LandscapeScene::drawForeground(QPainter *painter, const QRectF &rect)
{
	QGraphicsScene::drawForeground(painter, rect);
	if ((m_zoneBuilder->currentIdZoneRegion() != -1) && (m_transitionMode))
		drawTransition(painter, rect);
}

void LandscapeScene::drawTransition(QPainter *painter, const QRectF &rect)
{
	int left = int(floor(rect.left() / m_cellSize));
	int right = int(floor(rect.right() / m_cellSize));
	int top = int(floor(rect.top() / m_cellSize));
	int bottom = int(floor(rect.bottom() / m_cellSize));

	QVector<QLine> redLines;
	QVector<QLine> whiteLines;

	for (int i = left; i < right + 1; ++i)
	{
		for (int j = top; j < bottom + 1; ++j)
		{
			// Get LIGO data
			NLLIGO::CZoneRegion &zoneRegion = m_zoneBuilder->currentZoneRegion()->ligoZoneRegion();
			uint8 ceUp = zoneRegion.getCutEdge (i, -j, 0);
			uint8 ceLeft = zoneRegion.getCutEdge (i, -j, 2);
			if ((ceUp > 0) && (ceUp < 3))
			{
				// Calculate position vertical lines
				int x1, x2, y1, y2;

				y1 = j * m_cellSize + m_cellSize / 12.0f;
				y2 = y1 - (m_cellSize / 6.0f);

				x1 = i * m_cellSize + 3.0f * m_cellSize / 12.0f;
				x2 = i * m_cellSize + 5.0f * m_cellSize / 12.0f;
				if (ceUp == 1)
				{
					whiteLines.push_back(QLine(x1, y1, x1, y2));
					whiteLines.push_back(QLine(x2, y1, x2, y2));
				}
				else
				{
					redLines.push_back(QLine(x1, y1, x1, y2));
					redLines.push_back(QLine(x2, y1, x2, y2));
				}

				x1 = i * m_cellSize + 7.0f * m_cellSize / 12.0f;
				x2 = i * m_cellSize + 9.0f * m_cellSize / 12.0f;
				if (ceUp == 1)
				{
					redLines.push_back(QLine(x1, y1, x1, y2));
					redLines.push_back(QLine(x2, y1, x2, y2));
				}
				else
				{
					whiteLines.push_back(QLine(x1, y1, x1, y2));
					whiteLines.push_back(QLine(x2, y1, x2, y2));
				}
			}
			if ((ceLeft > 0) && (ceLeft < 3))
			{
				// Calculate position horizontal lines
				int x1, x2, y1, y2;

				x1 = i * m_cellSize - m_cellSize / 12.0f;
				x2 = x1 + (m_cellSize / 6.0f);

				y1 = j * m_cellSize + 3.0f * m_cellSize / 12.0f;
				y2 = j * m_cellSize + 5.0f * m_cellSize / 12.0f;
				if (ceLeft == 1)
				{
					redLines.push_back(QLine(x1, y1, x2, y1));
					redLines.push_back(QLine(x1, y2, x2, y2));
				}
				else
				{
					whiteLines.push_back(QLine(x1, y1, x2, y1));
					whiteLines.push_back(QLine(x1, y2, x2, y2));
				}

				y1 = j * m_cellSize + 7.0f * m_cellSize / 12.0f;
				y2 = j * m_cellSize + 9.0f * m_cellSize / 12.0f;
				if (ceLeft == 1)
				{
					whiteLines.push_back(QLine(x1, y1, x2, y1));
					whiteLines.push_back(QLine(x1, y2, x2, y2));
				}
				else
				{
					redLines.push_back(QLine(x1, y1, x2, y1));
					redLines.push_back(QLine(x1, y2, x2, y2));
				}
			}
		}
	}

	// Draw lines
	painter->setPen(QPen(Qt::red, 0, Qt::SolidLine));
	painter->drawLines(redLines);
	painter->setPen(QPen(Qt::white, 0, Qt::SolidLine));
	painter->drawLines(whiteLines);
}

} /* namespace LandscapeEditor */

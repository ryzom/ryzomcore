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
#include "landscape_scene.h"
#include "pixmap_database.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QGraphicsSimpleTextItem>

namespace LandscapeEditor
{

static const int ZONE_NAME = 0;
static const int LAYER_ZONES = 2;
static const int LAYER_EMPTY_ZONES = 3;
static const int LAYER_BLACKOUT = 4;

LandscapeScene::LandscapeScene(QObject *parent)
	: QGraphicsScene(parent),
	  m_mouseX(0.0),
	  m_mouseY(0.0),
	  m_zoneBuilder(0)
{
	m_cellSize = 160;
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

	NLLIGO::CZoneBankElement *zoneBankItem = m_zoneBuilder->getZoneBank().getElementByZoneName(data.zoneName);

	sint32 deltaX = 0, deltaY = 0;

	// Calculate offset for graphics item (for items with size that are larger than 1)
	if ((zoneBankItem->getSizeX() > 1) || (zoneBankItem->getSizeY() > 1))
	{
		sint32 sizeX = zoneBankItem->getSizeX(), sizeY = zoneBankItem->getSizeY();
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

	//item->setData(ZONE_NAME, QString(data.zoneName.c_str()));

	// for not full item zone
	item->setZValue(LAYER_ZONES);

	return item;
}

QGraphicsItem *LandscapeScene::createItemEmptyZone(const ZonePosition &zonePos)
{
	if (m_zoneBuilder == 0)
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
	return rectItem;
}

void LandscapeScene::deleteItemZone(const ZonePosition &zonePos)
{
	QGraphicsItem *item = itemAt(zonePos.x * m_cellSize, abs(zonePos.y) * m_cellSize);
	if (item != 0)
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

void LandscapeScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (m_zoneBuilder == 0)
		return;

	qreal x = mouseEvent->scenePos().rx();
	qreal y = mouseEvent->scenePos().ry();
	if ((x < 0) || (y < 0))
		return;

	sint32 posX = sint32(floor(x / m_cellSize));
	sint32 posY = sint32(-floor(y / m_cellSize));

	if (mouseEvent->button() == Qt::LeftButton)
		m_zoneBuilder->addZone(posX, posY);
	else if (mouseEvent->button() == Qt::RightButton)
		m_zoneBuilder->delZone(posX, posY);

	QGraphicsScene::mousePressEvent(mouseEvent);
}

void LandscapeScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	m_mouseX = mouseEvent->scenePos().x();
	m_mouseY = mouseEvent->scenePos().y();
	QGraphicsScene::mouseMoveEvent(mouseEvent);
}

bool LandscapeScene::checkUnderZone(const int posX, const int posY)
{
	QGraphicsItem *item = itemAt((posX * m_cellSize), abs(posY) * m_cellSize);
	if (item != 0)
	{
		return true;
	}
	return false;
}

void LandscapeScene::drawForeground(QPainter *painter, const QRectF &rect)
{
	QGraphicsScene::drawForeground(painter, rect);
	/*
		// Render debug text (slow!)
		painter->setPen(QPen(Qt::white, 0.5, Qt::SolidLine));

		int left = int(floor(rect.left() / m_cellSize));
		int right = int(floor(rect.right() / m_cellSize));
		int top = int(floor(rect.top() / m_cellSize));
		int bottom = int(floor(rect.bottom() / m_cellSize));

		for (int i = left; i < right; ++i)
		{
			for (int j = top; j < bottom; ++j)
			{
				LigoData data;
				m_zoneBuilder->currentZoneRegion()->ligoData(data, i, -j);
				painter->drawText(i * m_cellSize + 10, j * m_cellSize + 10, QString("%1 %2 %3 %4").arg(i).arg(j).arg(data.posX).arg(data.posY));
			}
		}
	*/
}

} /* namespace LandscapeEditor */

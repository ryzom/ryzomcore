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
#include "landscape_scene_base.h"
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

// TODO: delete
const char * const LAYER_BLACKOUT_NAME = "blackout";

const int MAX_SCENE_WIDTH = 256;
const int MAX_SCENE_HEIGHT = 256;

LandscapeSceneBase::LandscapeSceneBase(int sizeCell, QObject *parent)
	: QGraphicsScene(parent),
	  m_cellSize(sizeCell),
	  m_zoneBuilder(0)
{
	setSceneRect(QRectF(0, m_cellSize, MAX_SCENE_WIDTH * m_cellSize, MAX_SCENE_HEIGHT * m_cellSize));
}

LandscapeSceneBase::~LandscapeSceneBase()
{
}

int LandscapeSceneBase::cellSize() const
{
	return m_cellSize;
}

void LandscapeSceneBase::setZoneBuilder(ZoneBuilder *zoneBuilder)
{
	m_zoneBuilder = zoneBuilder;
}

QGraphicsItem *LandscapeSceneBase::createItemZone(const LigoData &data, const ZonePosition &zonePos)
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

	// Rotate the image counter clockwise
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

	item->setData(ZONE_NAME, QString(data.zoneName.c_str()));

	// for not full item zone
	item->setZValue(LAYER_ZONES);

	return item;
}

QGraphicsItem *LandscapeSceneBase::createItemEmptyZone(const ZonePosition &zonePos)
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

	return item;
}

void LandscapeSceneBase::deleteItemZone(const ZonePosition &zonePos)
{
	QGraphicsItem *item = itemAt(zonePos.x * m_cellSize, abs(zonePos.y) * m_cellSize);

	// TODO: delete LAYER_BLACKOUT_NAME
	if ((item != 0) && (item->data(ZONE_NAME).toString() != QString(LAYER_BLACKOUT_NAME)))
	{
		removeItem(item);
		delete item;
	}
}

void LandscapeSceneBase::addZoneRegion(const NLLIGO::CZoneRegion &zoneRegion)
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

void LandscapeSceneBase::delZoneRegion(const NLLIGO::CZoneRegion &zoneRegion)
{
	for (sint32 i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		for (sint32 j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
		{
			deleteItemZone(ZonePosition(i, -j, -1));
		}
	}
}

void LandscapeSceneBase::snapshot(const QString &fileName, int width, int height, const QRectF &landRect)
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

QString LandscapeSceneBase::zoneNameFromMousePos() const
{
	if ((m_posY > 0) || (m_posY < -MAX_SCENE_HEIGHT) ||
			(m_posX < 0) || (m_posX > MAX_SCENE_WIDTH))
		return "NOT A VALID ZONE";

	return QString("%1_%2%3  %4 %5  ").arg(-m_posY).arg(QChar('A' + (m_posX/26))).
		   arg(QChar('A' + (m_posX%26))).arg(m_mouseX, 0,'f',2).arg(-m_mouseY, 0,'f',2);
}

void LandscapeSceneBase::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	QGraphicsScene::mousePressEvent(mouseEvent);

	qreal x = mouseEvent->scenePos().x();
	qreal y = mouseEvent->scenePos().y();
	m_posX = sint32(floor(x / m_cellSize));
	m_posY = sint32(-floor(y / m_cellSize));

	m_mouseButton = mouseEvent->button();
}

void LandscapeSceneBase::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	m_mouseX = mouseEvent->scenePos().x();
	m_mouseY = mouseEvent->scenePos().y() - m_cellSize;

	m_posX = sint32(floor(m_mouseX / m_cellSize));
	m_posY = sint32(-floor(m_mouseY / m_cellSize));

	QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void LandscapeSceneBase::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	QGraphicsScene::mouseReleaseEvent(mouseEvent);
	m_mouseButton = Qt::NoButton;
}

bool LandscapeSceneBase::checkUnderZone(const int posX, const int posY)
{
	// TODO: it will not work correctly in world editor
	QGraphicsItem *item = itemAt((posX * m_cellSize), abs(posY) * m_cellSize);
	if (item != 0)
		return true;
	return false;
}

} /* namespace LandscapeEditor */

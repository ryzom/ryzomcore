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

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QGraphicsSimpleTextItem>

namespace LandscapeEditor
{

LandscapeScene::LandscapeScene(QObject *parent)
	: QGraphicsScene(parent),
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

QGraphicsItem *LandscapeScene::createZoneItem(const LigoData &data, const ZonePosition &zonePos)
{
	if (data.zoneName == STRING_UNUSED)
		return createEmptyZoneItem(zonePos);

	if ((m_zoneBuilder == 0) || (data.zoneName.empty()) ||
			(data.posX != 0) || (data.posY != 0))
		return 0;

	checkUnderZone(data, zonePos);

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
		item = new QGraphicsPixmapItem(pixmap->transformed(matrix, Qt::SmoothTransformation), 0, this);
	}
	else
	{
		// mirror image
		QImage mirrorImage = pixmap->toImage();
		QPixmap mirrorPixmap = QPixmap::fromImage(mirrorImage.mirrored(true, false));
		item = new QGraphicsPixmapItem(mirrorPixmap.transformed(matrix, Qt::SmoothTransformation), 0, this);
	}
	// Enable bilinear filtering
	item->setTransformationMode(Qt::SmoothTransformation);

	// Set position graphics item with offset for large piece
	NLLIGO::CZoneBankElement *zoneBankItem = m_zoneBuilder->getZoneBank().getElementByZoneName(data.zoneName);
	item->setPos(zonePos.x * m_cellSize, (abs(zonePos.y) - zoneBankItem->getSizeY() + 1) * m_cellSize);

	// The size graphics item should be equal or proportional m_cellSize
	item->setScale(float(m_cellSize) / m_zoneBuilder->pixmapDatabase()->textureSize());

	return item;
}

QGraphicsItem *LandscapeScene::createEmptyZoneItem(const ZonePosition &zonePos)
{
	if (m_zoneBuilder == 0)
		return 0;

	deleteZoneItem(zonePos);

	// Get image from pixmap database
	QPixmap *pixmap = m_zoneBuilder->pixmapDatabase()->pixmap(QString(STRING_UNUSED));
	if (pixmap == 0)
		return 0;

	QGraphicsPixmapItem *item = new QGraphicsPixmapItem(*pixmap, 0, this);

	// Enable bilinear filtering
	item->setTransformationMode(Qt::SmoothTransformation);

	// Set position graphics item
	item->setPos(zonePos.x * m_cellSize, abs(int(zonePos.y)) * m_cellSize);

	// The size graphics item should be equal or proportional m_cellSize
	item->setScale(float(m_cellSize) / m_zoneBuilder->pixmapDatabase()->textureSize());

	return item;
}

void LandscapeScene::deleteZoneItem(const ZonePosition &zonePos)
{
	QGraphicsItem *item = itemAt(zonePos.x * m_cellSize, abs(zonePos.y) * m_cellSize);
	if (item != 0)
	{
		removeItem(item);
		delete item;
	}
}

void LandscapeScene::processZoneRegion(const NLLIGO::CZoneRegion &zoneRegion)
{
	for (sint32 i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		for (sint32 j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
		{
			nlinfo(QString("%1 %2 %3").arg(i).arg(j).arg(zoneRegion.getName(i, j).c_str()).toStdString().c_str());
			std::string zoneName = zoneRegion.getName(i, j);
			if (zoneName == STRING_UNUSED)
			{
				ZonePosition zonePos(i, j, -1);
				QGraphicsItem *item = createEmptyZoneItem(zonePos);
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
				QGraphicsItem *item = createZoneItem(data, zonePos);
			}
		}
	}
}

void LandscapeScene::snapshot(const QString &fileName, int sizeSource)
{
	/*	if (m_zoneRegion == 0)
			return;

		sint32 regionMinX = m_zoneRegion->getMinX();
		sint32 regionMaxX = m_zoneRegion->getMaxX();
		sint32 regionMinY = m_zoneRegion->getMinY();
		sint32 regionMaxY = m_zoneRegion->getMaxY();

		int regionWidth = (regionMaxX - regionMinX + 1);
		int regionHeight = (regionMaxY - regionMinY + 1);

		snapshot(fileName, regionWidth * sizeSource, regionHeight * sizeSource);
		*/
}

void LandscapeScene::snapshot(const QString &fileName, int width, int height)
{
	if (m_zoneBuilder == 0)
		return;

	/*	if (m_zoneRegion == 0)
			return;

		sint32 regionMinX = m_zoneRegion->getMinX();
		sint32 regionMaxX = m_zoneRegion->getMaxX();
		sint32 regionMinY = m_zoneRegion->getMinY();
		sint32 regionMaxY = m_zoneRegion->getMaxY();

		int regionWidth = (regionMaxX - regionMinX + 1);
		int regionHeight = (regionMaxY - regionMinY + 1);

		QImage image(width, height, QImage::Format_RGB888);
		QPainter painter(&image);
		painter.setRenderHint(QPainter::Antialiasing, true);

		// add white background
		painter.setBrush(QBrush(Qt::white));
		painter.setPen(Qt::NoPen);
		painter.drawRect(0, 0, width, height);

		render(&painter, QRectF(0, 0, width, height),
			   QRectF(regionMinX * m_cellSize, abs(regionMaxY) * m_cellSize, regionWidth * m_cellSize, regionHeight * m_cellSize));

		image.save(fileName);
	*/
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

void LandscapeScene::checkUnderZone(const LigoData &data, const ZonePosition &zonePos)
{
//	NLLIGO::CZoneBankElement *zoneBankItem = m_zoneBuilder->getZoneBank().getElementByZoneName(data.zoneName);
//	uint8 sizeX = zoneBankItem->getSizeX();
//	uint8 sizeY = zoneBankItem->getSizeY();
//	std::vector<bool> &mask = zoneBankItem->getMask();
	deleteZoneItem(zonePos);
}

} /* namespace LandscapeEditor */

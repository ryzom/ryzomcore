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

static const int ZoneName = 0;

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
	nlinfo(QString("%1,%2 (%3,%4)-%7 (%5,%6)").arg(zonePos.x).arg(zonePos.y).arg(data.posX).arg(data.posY).arg(data.rot).arg(data.flip).arg(data.zoneName.c_str()).toStdString().c_str());
	if ((data.zoneName == STRING_OUT_OF_BOUND) || (checkUnderZone(zonePos.x, zonePos.y)))
		return 0;

	if (data.zoneName == STRING_UNUSED)
		return createItemEmptyZone(zonePos);

	if ((m_zoneBuilder == 0) || (data.zoneName.empty()) ||
			(data.posX != 0) || (data.posY != 0))
		return 0;

//	if ((m_zoneBuilder == 0) || (data.zoneName.empty()))
//		return 0;

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

	int delta = zoneBankItem->getSizeY() - 1;
	if ((data.rot == 1) || (data.rot == 3))
		delta = zoneBankItem->getSizeX() - 1;

	//item->setPos((zonePos.x - data.posX) * m_cellSize, (abs(int(zonePos.y)) + data.posY - delta) * m_cellSize);
	item->setPos((zonePos.x) * m_cellSize, (abs(int(zonePos.y)) - delta) * m_cellSize);

	// The size graphics item should be equal or proportional m_cellSize
	item->setScale(float(m_cellSize) / m_zoneBuilder->pixmapDatabase()->textureSize());

	item->setData(ZoneName, QString(data.zoneName.c_str()));
	nlinfo("render");
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

	QGraphicsPixmapItem *item = new QGraphicsPixmapItem(*pixmap, 0, this);

	// Enable bilinear filtering
	item->setTransformationMode(Qt::SmoothTransformation);

	// Set position graphics item
	item->setPos(zonePos.x * m_cellSize, abs(int(zonePos.y)) * m_cellSize);

	// The size graphics item should be equal or proportional m_cellSize
	item->setScale(float(m_cellSize) / m_zoneBuilder->pixmapDatabase()->textureSize());
	nlinfo("render");
	return item;
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

void LandscapeScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
	m_mouseX = mouseEvent->scenePos().x();
	m_mouseY = mouseEvent->scenePos().y();
	QGraphicsScene::mouseMoveEvent(mouseEvent);
}

bool LandscapeScene::checkUnderZone(const int posX, const int posY)
{
	/*	QGraphicsItem *item = itemAt((posX * m_cellSize), abs(posY) * m_cellSize);
		if (item != 0)
		{
			QString zoneName = item->data(ZoneName).toString();
			return true;
		}
	*/	return false;
}

void LandscapeScene::drawForeground(QPainter *painter, const QRectF &rect)
{
	QGraphicsScene::drawForeground(painter, rect);
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
}

} /* namespace LandscapeEditor */

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
#include "builder_zone.h"
#include "landscape_actions.h"
#include "list_zones_widget.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QGraphicsSimpleTextItem>

namespace LandscapeEditor
{

LandscapeScene::LandscapeScene(QUndoStack *undoStack, ListZonesWidget *listZonesWidget, ZoneBuilder *zoneBuilder, QObject *parent)
	: QGraphicsScene(parent),
	  m_undoStack(undoStack),
	  m_listZonesWidget(listZonesWidget),
	  m_zoneBuilder(zoneBuilder),
	  m_zoneRegion(0)
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

QGraphicsItem *LandscapeScene::createZoneItem(const LigoData &data)
{
	// Get image from pixmap database
	QPixmap *pixmap = m_zoneBuilder->pixmapDatabase()->pixmap(QString(data.ZoneName.c_str()));
	if (pixmap == 0)
		return 0;

	// Rotate the image counterclockwise
	QMatrix matrix;
	matrix.rotate(-data.Rot * 90.0);

	QGraphicsPixmapItem *item;

	if (data.Flip == 0)
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
	NLLIGO::CZoneBankElement *zoneBankItem = m_zoneBuilder->getZoneBank().getElementByZoneName(data.ZoneName);
	item->setPos(data.PosX * m_cellSize, (abs(data.PosY) - zoneBankItem->getSizeY() + 1) * m_cellSize);

	// The size graphics item should be equal or proportional m_cellSize
	item->setScale(m_cellSize / 256.0);

	// add debug info
	QGraphicsSimpleTextItem *itemText = addSimpleText(QString("%1,%2 R-%3 F-%4").
										arg(data.PosX).arg(data.PosY).
										arg(data.Rot * 90.0).
										arg(data.Flip),
										QFont("Helvetica [Cronyx]", 14));

	itemText->setZValue(2);
	itemText->setPos(data.PosX * m_cellSize + 10, (abs(data.PosY) - zoneBankItem->getSizeY() + 1) * m_cellSize + 10);
	itemText->setBrush(QBrush(Qt::white));

	return item;
}

void LandscapeScene::processZoneRegion(const NLLIGO::CZoneRegion &zoneRegion)
{
	for (sint32 i = zoneRegion.getMinX(); i <= zoneRegion.getMaxX(); ++i)
	{
		for (sint32 j = zoneRegion.getMinY(); j <= zoneRegion.getMaxY(); ++j)
		{
			std::string zoneName = zoneRegion.getName(i, j);
			if ((!zoneName.empty()) &&
					(zoneName != STRING_UNUSED) &&
					(zoneRegion.getPosX(i, j) == 0) &&
					(zoneRegion.getPosY(i, j) == 0))
			{
				LigoData data;
				data.PosX = i;
				data.PosY = j;
				data.ZoneName = zoneName;
				data.Rot = zoneRegion.getRot(i, j);
				data.Flip = zoneRegion.getFlip(i, j);
				QGraphicsItem *item = createZoneItem(data);
			}
		}
	}
}

void LandscapeScene::setCurrentZoneRegion(NLLIGO::CZoneRegion *zoneRegion)
{
	m_zoneRegion = zoneRegion;
}

void LandscapeScene::snapshot(const QString &fileName, int sizeSource)
{
	if (m_zoneRegion == 0)
		return;

	sint32 regionMinX = m_zoneRegion->getMinX();
	sint32 regionMaxX = m_zoneRegion->getMaxX();
	sint32 regionMinY = m_zoneRegion->getMinY();
	sint32 regionMaxY = m_zoneRegion->getMaxY();

	int regionWidth = (regionMaxX - regionMinX + 1);
	int regionHeight = (regionMaxY - regionMinY + 1);

	snapshot(fileName, regionWidth * sizeSource, regionHeight * sizeSource);
}

void LandscapeScene::snapshot(const QString &fileName, int width, int height)
{
	if (m_zoneRegion == 0)
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

}

void LandscapeScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	qreal x = mouseEvent->scenePos().rx();
	qreal y = mouseEvent->scenePos().ry();
	if ((x < 0) || (y < 0))
		return;

	if (mouseEvent->button() == Qt::LeftButton)
	{
		// Add new zone brick
		LigoData ligoData = m_listZonesWidget->currentLigoData();
		if (ligoData.ZoneName == "")
			return;

		ligoData.PosX = int(floor(x / m_cellSize));
		ligoData.PosY = int(-floor(y / m_cellSize));

		AddLigoTileCommand *action = new AddLigoTileCommand(ligoData, this);
		m_undoStack->push(action);
	}

	/*if (mouseEvent->button() == Qt::RightButton)
	{
		// Delete zone brick
		LigoData ligoData;

		ligoData.PosX = int(floor(x / m_cellSize));
		ligoData.PosY = int(floor(y / m_cellSize));
		ligoData.ZoneName = m_zoneRegion->getName(ligoData.PosX, -ligoData.PosY);
		ligoData.Flip = m_zoneRegion->getFlip(ligoData.PosX, -ligoData.PosY);
		ligoData.Rot = m_zoneRegion->getRot(ligoData.PosX, -ligoData.PosY);
		DelLigoTileCommand *action = new DelLigoTileCommand(ligoData, this);
		m_undoStack->push(action);
	}*/
	QGraphicsScene::mousePressEvent(mouseEvent);
}

} /* namespace LandscapeEditor */

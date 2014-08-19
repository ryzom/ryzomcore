// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include <QtGui/QtGui>

#include <nel/misc/debug.h>

#include "tile_item_delegate.h"

#include "tile_model.h"

TileItemDelegate::TileItemDelegate()
{
	m_zoomFactor = ZoomNormal;
	m_imageHint = 128;
}

TileItemDelegate::~TileItemDelegate()
{
}

void TileItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyledItemDelegate::paint(painter,option,index);

	painter->save();

	QFont font = QApplication::font();

	//font.setBold(true);
	//SubFont.setWeight(SubFont.weight()-2);
	QFontMetrics fm(font);

	QPixmap tile = qvariant_cast<QPixmap>(index.data(TileModel::TilePixmapRole));
	QString tileFileText = qvariant_cast<QString>(index.data(TileModel::TileFilenameRole));
	QString tileIdText = qvariant_cast<QString>(index.data(TileModel::TileIndexRole));

	QSize tileSize = tile.size();

	//QRect headerRect = option.rect;
	QRect rect(option.rect);
	//QRect tileRect(option.rect);
	int textHeight = fm.height();
    int iconPosModX = PIXMAP_MARGIN + (tile.width() / 2);
    int iconPosModY = (option.rect.height() - tile.height()) / 2;

	painter->drawPixmap(rect.adjusted(iconPosModX, iconPosModY, iconPosModX, iconPosModY).topLeft(), tile);



	//tileRect.setRight(tileSize.width()+30);
	//tileRect.setTop(tileRect.top()+5);
	//headerRect.setLeft(tileRect.right());
	//subheaderRect.setLeft(tileRect.right());
	//headerRect.setTop(headerRect.top()+5);
	//headerRect.setBottom(headerRect.top()+fm.height());

	//subheaderRect.setTop(headerRect.bottom()+2);
    
	//painter->drawPixmap(targetrect, pixmap, sourcerect)
    
	//painter->drawPixmap(QPoint(iconRect.right()/2,iconRect.top()/2),icon.pixmap(iconsize.width(),iconsize.height()));
	//painter->drawPixmap(QPoint(tileRect.left()+tileSize.width()/2+2,tileRect.top()+tileSize.height()/2+3),tile);
    
	//painter->setFont(font);
	//painter->drawText(headerRect,headerText);


	//painter->setFont(SubFont);
	//painter->drawText(subheaderRect.left(),subheaderRect.top()+17,subText);

	painter->restore();
}

QSize TileItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QPixmap tile = qvariant_cast<QPixmap>(index.data(TileModel::TilePixmapRole));
	QSize tileSize = tile.size();
	QFont font = QApplication::font();
	QFontMetrics fm(font);
    
	return(QSize(tileSize.width()+(2*PIXMAP_MARGIN), tileSize.height()+fm.height()+(2*PIXMAP_MARGIN)));
}

TileItemDelegate::TZoomFactor TileItemDelegate::getZoomFactor()
{
	return m_zoomFactor;
}

void TileItemDelegate::setZoomFactor(TileItemDelegate::TZoomFactor zoomFactor)
{
	m_zoomFactor = zoomFactor;
}

// SLOTS

void TileItemDelegate::currentTab(int index)
{
	if(index == 1)
	{
		nlinfo("switching delegate to 1 or 256");
		m_imageHint = 256;
	}
	else
	{
		nlinfo("switching delegate to 0,2,3 or 128");
		m_imageHint = 128;
	}	
}
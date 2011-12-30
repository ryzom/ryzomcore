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
}

TileItemDelegate::~TileItemDelegate()
{
}

void TileItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyledItemDelegate::paint(painter,option,index);

	painter->save();

	QFont font = QApplication::font();
	QFont SubFont = QApplication::font();
	//font.setPixelSize(font.weight()+);
	font.setBold(true);
	SubFont.setWeight(SubFont.weight()-2);
	QFontMetrics fm(font);

	QPixmap tile = qvariant_cast<QPixmap>(index.data(TileModel::TilePixmapRole));
	QString headerText = qvariant_cast<QString>(index.data(TileModel::TileFilenameRole));
	QString subText = qvariant_cast<QString>(index.data(TileModel::TileIndexRole));

	QSize tileSize = tile.size();

	QRect headerRect = option.rect;
	QRect subheaderRect = option.rect;
	QRect tileRect = subheaderRect;

	tileRect.setRight(tileSize.width()+30);
	tileRect.setTop(tileRect.top()+5);
	headerRect.setLeft(tileRect.right());
	subheaderRect.setLeft(tileRect.right());
	headerRect.setTop(headerRect.top()+5);
	headerRect.setBottom(headerRect.top()+fm.height());

	subheaderRect.setTop(headerRect.bottom()+2);
    
    
	//painter->drawPixmap(QPoint(iconRect.right()/2,iconRect.top()/2),icon.pixmap(iconsize.width(),iconsize.height()));
	painter->drawPixmap(QPoint(tileRect.left()+tileSize.width()/2+2,tileRect.top()+tileSize.height()/2+3),tile);
    
	painter->setFont(font);
	painter->drawText(headerRect,headerText);


	painter->setFont(SubFont);
	painter->drawText(subheaderRect.left(),subheaderRect.top()+17,subText);

	painter->restore();
}

QSize TileItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
	QPixmap tile = qvariant_cast<QPixmap>(index.data(TileModel::TilePixmapRole));
	QSize tileSize = tile.size();//tile.actualSize(option.decorationSize);
	QFont font = QApplication::font();
	QFontMetrics fm(font);
    
	return(QSize(tileSize.width(), tileSize.height()+fm.height()+8 ));
}

TileItemDelegate::TZoomFactor TileItemDelegate::getZoomFactor()
{
	return m_zoomFactor;
}

void TileItemDelegate::setZoomFactor(TileItemDelegate::TZoomFactor zoomFactor)
{
	m_zoomFactor = zoomFactor;
}
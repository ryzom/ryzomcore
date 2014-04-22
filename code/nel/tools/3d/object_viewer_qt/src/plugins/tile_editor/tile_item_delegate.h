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
#ifndef TILE_ITEM_DELEGATE_H
#define TILE_ITEM_DELEGATE_H

#include <QtGui/QWidget>
#include <QPixmap>
#include <QStyledItemDelegate>

#include <nel/misc/types_nl.h>

class TileItemDelegate : public QStyledItemDelegate
 {
 public:

	enum TZoomFactor
	{
		ZoomSmall = 0,
		ZoomNormal = 1,
		ZoomLarge = 2
	};

	static const int PIXMAP_MARGIN = 5;

	TileItemDelegate();
	virtual ~TileItemDelegate();

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

	TZoomFactor getZoomFactor();
	void setZoomFactor(TZoomFactor zoomFactor);

public Q_SLOTS:
	void currentTab(int index);

 private:
	TZoomFactor m_zoomFactor;
	int m_imageHint;
 };

#endif // TILE_ITEM_DELEGATE_H
